/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015,2016,2017 Cun Gong <gong_cun@bocmacau.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include "tpty.h"
#include <termios.h>
#include <signal.h>
#include <setjmp.h>

/*
 *  -I: interact manually
 *  -a <auditfile>
 *  -r: record data for replay
 *  -t: set expect timeout
 *  -d <driver>: driver for stdin/stdout
 *  -c <config>: config file for auto interact
 *  -e: noecho for slave pty's line discipline
 *  -i: ignore EOF on standard input
 *  -n: not interactive
 *  -v: verbose output
 *  -x: use default driver
 *  -o <output>: dump the session to file
 *  -X: output to /dev/null
 *  -p <PrivateKey>: RSA private key
 *  -k <EncryptKey>: RSA encrypted key
 *  -u: remove key file
 */


#define OPTSTR "Ia:r:t:d:c:einvxo:Xp:k:u"
#define HELP "usage: tpty [ -x -f config ] [ -t timeout -d driver -einuvXI -o output -r record -a auditfile -p RSAPrivateKey -k EncryptedKey ] program [ args ... ]"


static void
set_noecho(int);		/* at the end of this file */

extern void
do_driver(int (*f_driver) (void));	/* at the file do_driver.c */

extern void
loop(int, int);			/* at the file loop.c */

static void
sig_winch(int);

static void 
sig_alrm(int);


int             fdm;
char           *pathconfig;
char           *driver;
int             outputfd = -1;
FILE           *timingfd;
int             timeout = 10;	/* default 10 seconds */
int             tflg;
int             oflg;
int             zeroflg;
int             manflg = 0;
FILE           *keyfd;
char           *rsafd;
int             encflg = 0;
int             auditflg;
FILE           *auditfd;
int             tty = -1;
int             rmflg = 0;
int             status;
int             ret;
int		interactive;
static sigjmp_buf jmpbuf;

int
main(int argc, char **argv)
{
	int             c, ignoreeof, noecho, verbose, defdriver;
	pid_t           pid, childpid = 0;
	char            slave_name[20];
	struct termios  orig_termios;
	struct winsize  size;
	int             fd;
	char           *key_name;
	char		buf[MAXLINE];

	umask(0);
	interactive = isatty(STDIN_FILENO);
	ignoreeof = 0;
	noecho = 0;
	verbose = 0;
	defdriver = 0;

	opterr = 0;		/* don't want getopt() writing to stderr */
	while ((c = getopt(argc, argv, OPTSTR)) != EOF) {
		switch (c) {
		case 'a':	/* open audit record */
			auditflg = 1;
			auditfd = fopen(optarg, "w");
			if (auditfd == NULL)
				err_sys("cannot open file %s", optarg);
			break;
		case 'd':	/* driver for stdin/stdout */
			driver = optarg;
			break;
		case 'e':	/* noecho for slave pty's line discipline */
			noecho = 1;
			break;
		case 'c':	/* read the config file for driver */
			pathconfig = optarg;
			break;
		case 'i':	/* ignore EOF on standard input */
			ignoreeof = 1;
			break;
		case 'k':
#ifdef HAVE_OPENSSL
			if (optarg && !(keyfd = fopen(optarg, "r")))
				err_sys("cannot open %s", optarg);
			encflg = 1;
			key_name = optarg;
#else
			err_quit("The -k option is not supported because can't find the OPENSSL lib.");
#endif
			break;
		case 'n':	/* not interactive */
			interactive = 0;
			break;
		case 'o':	/* output file */
			if (optarg) {
				outputfd = open(optarg, O_WRONLY | O_TRUNC | O_CREAT, 0666);
				if (outputfd < 0)
					err_sys("cannot open %s", optarg);
			}
			oflg = 1;
			break;
		case 'p':
#ifdef HAVE_OPENSSL
			rsafd = optarg;
			encflg = 1;
#else
			err_quit("The -p option is not supported because can't find the OPENSSL lib.");
#endif
			break;
		case 'r':	/* reacord data for replay */
			if (optarg && !(timingfd = fopen(optarg, "w")))
				err_sys("cannot open %s", optarg);
			/*
			 * Lock file for using tptyreplay at the same time. 
			 */
			fd = fileno(timingfd);
			if (fd < 0)
				err_sys("fileno error");
			if (lock_reg(fd, F_SETLK, F_WRLCK, 0, SEEK_SET, 0) < 0)
				err_sys("lock_reg error");
			tflg = 1;
			break;
		case 't':	/* set timeout */
			timeout = atoi(optarg);
			break;
		case 'u':	/* remove key file */
			rmflg = 1;
			break;
		case 'v':	/* verbose */
			verbose = 1;
			break;
		case 'x':	/* default driver */
			defdriver = 1;
			break;
		case 'X':	/* output to /dev/null */
			zeroflg = 1;
			break;
		case 'I':	/* interact manually */
			manflg = 1;
			break;
		case '?':
			err_quit("unrecognized option: -%c", optopt);
		}
	}
	if (optind >= argc)
		err_quit(HELP);

	if (defdriver == 1 && driver)
		err_quit(HELP);
	if (defdriver == 1 && (!pathconfig))
		err_quit(HELP);
	if (defdriver == 0 && pathconfig)
		err_quit(HELP);
	if (encflg && !(keyfd && rsafd))
		err_quit(HELP);

	if (encflg && rmflg)
		if (unlink(key_name) < 0)
			err_sys("unlink error");

	if (interactive) {	/* fetch current termios and window size */
		if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &size) < 0)
			err_sys("TIOCGWINSZ error");
		snprintf(buf, sizeof(buf), "TPTY started on %s (%d rows, %d columns)\n",
				gettime(), size.ws_row, size.ws_col);
#define __BUFLEN strlen(buf)
		if (!zeroflg) {
			if (writen(STDERR_FILENO, buf, __BUFLEN) != __BUFLEN)
				err_sys("writen() error");
		}
		if (outputfd >= 0) {
			if (writen(outputfd, buf, __BUFLEN) != __BUFLEN)
				err_sys("writen() error");
		}
#undef __BUFLEN
		if (tcgetattr(STDIN_FILENO, &orig_termios) < 0)
			err_sys("tcgetattr error on stdin");
		pid = pty_fork(&fdm, slave_name, sizeof(slave_name),
			       &orig_termios, &size);
	} else {
		pid = pty_fork(&fdm, slave_name, sizeof(slave_name),
			       NULL, NULL);
	}

	if (pid < 0) {
		err_sys("fork error");
	} else if (pid == 0) {	/* child */
		if (noecho)
			set_noecho(STDIN_FILENO);	/* stdin is slave pty */
		if (execvp(argv[optind], &argv[optind]) < 0)
			err_sys("can't execute: %s", argv[optind]);
	}
	/* parent continue... */
	tty = dup(fileno(stdout));	/* save the original stdout */
	if (tty < 0)
		err_sys("dup stdout error");

	if (signal(SIGWINCH, sig_winch) == SIG_ERR)
		err_sys("SIGWINCH error");

	if (verbose) {
		fprintf(stderr, "slave name = %s\n", slave_name);
		if (driver != NULL)
			fprintf(stderr, "driver = %s\n", driver);
	}
	if (interactive && driver == NULL && defdriver == 0) {
		if (tty_raw(STDIN_FILENO) < 0)	/* user's tty to raw mode */
			err_sys("tty_raw error");
		if (atexit(tty_atexit) < 0)	/* reset user's tty on exit */
			err_sys("atexit error");
	}
	if (driver || defdriver == 1) {
		if ((childpid = fork()) < 0) {
			err_sys("fork error");
		} else if (childpid == 0) {	/* child */
			/* use do_driver to changes our stdin/stdout */
			if (driver)
				do_driver((int (*)(void))0);
			else
				do_driver(def_driver);
		} else {	/* parent */
			/* wait driver first */
			if (waitpid(childpid, &status, 0) != childpid)
				err_sys("waitpid");
			ret = sys_exit(status);
			if (manflg == 0) { /* if no need interact mannually, wait and exit */
				if (signal(SIGALRM, sig_alrm) == SIG_ERR)
					err_sys("signal SIGALRM error");
				if (timeout > 0)
					alarm(timeout);
				if (sigsetjmp(jmpbuf, 1))
					err_quit("tpty timed out");
				if (waitpid(pid, &status, 0) < 0)
					err_sys("waitpid error");
				alarm(0);
				ret |= sys_exit(status);
				exit(ret);
			}
			if (!ret)
				err_quit("driver exception: rc=%d\n", ret);
			/* open /dev/tty to give user keyboard control */
			do_driver(inter_driver);
		}
	}
	loop(fdm, ignoreeof);	/* copies stdin -> ptym, ptym -> stdout */

	while (waitpid((pid_t) - 1, &status, WNOHANG) > 0)	/* must be non-block
								 * mode */
		ret |= sys_exit(status);

	if (childpid != getpid()) { /* parent process exit */
		if (interactive) {
			snprintf(buf, sizeof(buf), "TPTY done on %s\n", gettime());
			if (!zeroflg) {
				tty_atexit();
				if (writen(STDERR_FILENO, buf, strlen(buf)) != strlen(buf))
					err_sys("writen() error");
			}

			/* don't write exit message into output file,
 			 * otherwise tptyreplay will have exception */
#if 0
			if (outputfd >= 0) {
				if (writen(outputfd, buf, strlen(buf)) != strlen(buf))
					err_sys("writen() error");
			}
#endif
		}
	}

	exit(ret);
}

static void
set_noecho(int fd)
{				/* turn off echo (for slave pty) */
	struct termios  stermios;

	if (tcgetattr(fd, &stermios) < 0)
		err_sys("tcgetattr error");

	stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

	/*
	 * also turn off NL to CR/NL mapping on output 
	 */
	stermios.c_oflag &= ~(ONLCR);

	if (tcsetattr(fd, TCSANOW, &stermios) < 0)
		err_sys("tcsetattr error");
}

static void
sig_winch(int signo)
{
	struct winsize  size;

#ifdef AIX
	if (signal(SIGWINCH, sig_winch) == SIG_ERR)
		err_sys("signal SIGWINCH error");
#endif

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *) &size) < 0) {
		if (errno == EOPNOTSUPP)
			return;
		err_sys("TIOCGWINSZ error");
	}
	if (ioctl(fdm, TIOCSWINSZ, &size) < 0)
		err_sys("TIOCSWINSZ, error");

}

static void 
sig_alrm(int signo)
{
	siglongjmp(jmpbuf, 1);	/* jump back to main, and exit */
}

