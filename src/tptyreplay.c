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
#include "closestream.h"
#include <math.h>
#include <setjmp.h>

#ifdef LINUX
#define OPTSTR "+t:s:d:m:"
#else
#define OPTSTR "t:s:d:m:"
#endif

#define HELP "tptyreplay [options] -t timingfile -s typetpty"
#define TPTY_MIN_DELAY 0.0001
#define BACKROWS 1

volatile sig_atomic_t playflg;
volatile sig_atomic_t pauseflg;
volatile sig_atomic_t speedflg;
volatile sig_atomic_t backflg;
volatile sig_atomic_t forwardflg;
volatile sig_atomic_t pendflg;
static sigjmp_buf jmpbuf;
static struct termios savetty;
static int      ttysavefd = -1;

static double
getnum(const char *);
static void
delay_for(double delay);
static void
emit(FILE *, const char *, size_t);
static void
sig_usr1(int);
static void
sig_usr2(int);
static void
sig_term(int);
static void
sig_int_parent(int);
static void
sig_int_child(int);
static void
sig_alrm(int);
static void
sig_hup(int);
static void
reset_tput(void);
static void
reprint(long, FILE *, FILE *, const char *, const char *);
static void
tty_resume(void);
static int      fd[2];


int
main(int argc, char *argv[])
{
	FILE           *tfile, *sfile;
	const char     *sname = NULL, *tname = NULL;
	double          divi = 1, maxdelay = 0, origdivi = 1;
	int             diviopt __attribute__((unused)) = 0, maxdelayopt = 0;
	volatile unsigned long line;
	long            n = 0;
	int             ch, ret, tfd;
	char            c;
	pid_t           pid;
	sigset_t        newmask, oldmask;


	atexit(close_stdout);
	atexit(tty_resume);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGHUP);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);

	if (tcgetattr(STDIN_FILENO, &savetty) < 0)
		err_sys("tcgetattr stdin error");
	ttysavefd = STDIN_FILENO;


	opterr = 0;
	while ((ch = getopt(argc, argv, OPTSTR)) != EOF) {
		switch (ch) {
		case 't':
			tname = optarg;
			break;
		case 's':
			sname = optarg;
			break;
		case 'd':
			diviopt = 1;
			divi = getnum(optarg);
			origdivi = divi;
			break;
		case 'm':
			maxdelayopt = 1;
			maxdelay = getnum(optarg);
			break;
		case '?':
			err_quit("unrecognized option: -%c", optopt);
		}
	}

	if (!tname || !sname)
		err_quit("usage: %s", HELP);
	if (maxdelay < 0)
		maxdelay = 0;

	tfile = fopen(tname, "r");
	if (!tfile)
		err_sys("cannot open %s", tname);
	sfile = fopen(sname, "r");
	if (!sfile)
		err_sys("cannot open %s", sname);

	tfd = fileno(tfile);
	if (tfd < 0)
		err_sys("fileno error");

	if (pipe(fd) < 0)
		err_sys("pipe error");

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid > 0) {	/* parent */
		if (signal(SIGINT, sig_int_parent) == SIG_ERR)
			err_sys("signal SIGINT");
		if (signal(SIGUSR1, sig_usr1) == SIG_ERR)
			err_sys("signal SIGUSR1");
		if (signal(SIGUSR2, sig_usr2) == SIG_ERR)
			err_sys("signal SIGUSR2");
		if (signal(SIGALRM, sig_alrm) == SIG_ERR)
			err_sys("signal SIGALRM");
		if (signal(SIGHUP, sig_hup) == SIG_ERR)
			err_sys("signal SIGHUP");

		close(fd[1]);

		line = 0;
		while (1) {
			double          delay;
			size_t          blk;
			char            nl;
			fd_set          rset;
			struct timeval  tv;

			sigsetjmp(jmpbuf, 1);
			n = line;
			for (;;) {
				FD_ZERO(&rset);
				FD_SET(fd[0], &rset);
				tv.tv_sec = 0;
				tv.tv_usec = 0;
				ret = select(fd[0] + 1, &rset, NULL, NULL, &tv);
				if (ret < 0) {
					if (errno == EINTR)
						continue;
					err_sys("select error");
				}
				if (ret == 0)
					break;

				ret = read(fd[0], &c, 1);
				if (ret < 0)
					err_sys("read error");
				if (ret == 0)
					break;

				switch (c) {
				case 'u':
					divi *= 2;
					break;
				case 'd':
					divi /= 2;
					break;
				case '\n':
					divi = origdivi;
					break;
				case 'b':
					backflg = 1;
					n -= BACKROWS;
					break;
				case 'f':
					forwardflg = 1;
					break;
				default:
					err_quit("incorrect data");
				}
			}

			if (backflg) {
				backflg = 0;
				if (n < 0)
					n = 0;

				if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
					err_sys("SIG_BLOCK error");
				reprint(n, tfile, sfile, tname, sname);
				if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
					err_sys("SIG_SETMASK error");

				line = n;
				if (pendflg) {
					pendflg = 0;
					if (n == 0) {
						fputs("Already in the beginning of the file.", stdout);
						fflush(stdout);
					}
					if (kill(getpid(), SIGUSR1) < 0)
						err_sys("send SIGUSR1");
				}
			}
			if (line == 0) {
				ret = system("clear");
				if (ret < 0)
					err_sys("system() error");
				if (ret != 0)
					err_quit("shell exit %d", ret);
			}
			if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
				err_sys("SIG_BLOCK error");

			if (fscanf(tfile, "%lf %zu%c\n", &delay, &blk, &nl) != 3 ||
			    nl != '\n') {
				if (feof(tfile)) {
					if (lock_test(tfd, F_WRLCK, 0, SEEK_SET, 0) != 0) {
#ifdef MACOS
						long            offset = ftell(tfile);
						if (offset == -1L)
							err_sys("ftell error");
						tfile = freopen(tname, "r", tfile);
						if (tfile == NULL)
							err_sys("freopen error");
						if (fseek(tfile, offset, SEEK_SET) != 0)
							err_sys("fseek error");
#endif
						continue;
					}
					break;
				}
				if (ferror(tfile))
					err_sys("failed to read timing file %s", tname);
				err_sys("timings file %s: %lu: unexpected format",
					tname, line);
			}
			delay /= divi;
			if (maxdelayopt && delay > maxdelay)
				delay = maxdelay;
			if (forwardflg) {
				forwardflg = 0;
				delay = 0;
			}
			if (delay > TPTY_MIN_DELAY)
				delay_for(delay);
			emit(sfile, sname, blk);
			line++;

			if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
				err_sys("SIG_SETMASK error");

			if (pendflg) {
				pendflg = 0;
				if (kill(getpid(), SIGUSR1) < 0)
					err_sys("send SIGUSR1");
			}
		}

		if (kill(pid, SIGTERM) < 0)
			err_sys("send SIGTERM");
		fclose(sfile);
		fclose(tfile);
		printf("\n");
	} else {		/* child */
		int             i;
		char            c;

		/* atexit(tty_atexit); */

		if (signal(SIGTERM, sig_term) == SIG_ERR)
			err_sys("signal SIGTERM");
		if (signal(SIGINT, sig_int_child) == SIG_ERR)
			err_sys("signal SIGINT");

		close(fd[0]);

		if (tty_cbreak(STDIN_FILENO) < 0)
			err_sys("tty_cbreak error");
		while ((i = read(STDIN_FILENO, &c, 1)) == 1) {
			if (c == ' ') {
				if (pauseflg == 0) {
					if (kill(getppid(), SIGUSR1) < 0)
						err_sys("send SIGUSR1");
				} else {
					if (kill(getppid(), SIGUSR2) < 0)
						err_sys("send SIGUSR2");
				}
				pauseflg = 1 - pauseflg;
			}
			if (c == 'u' || c == 'd' || c == '\n' ||
			    c == 'b' || c == 'f') {
				if (write(fd[1], &c, 1) != 1)
					err_sys("write error");
				if (c == 'u')
					if (kill(getppid(), SIGALRM) < 0)
						err_sys("send SIGALRM");
				if (c == 'b' || c == 'f') {
					if (kill(getppid(), SIGALRM) < 0)
						err_sys("send SIGALRM");
					if (kill(getppid(), SIGHUP) < 0)
						err_sys("send SIGHUP");
				}
			}
		}

		if (tty_reset(STDIN_FILENO) < 0)
			err_sys("tty_reset error");
#if defined(LINUX) || defined(MACOS)
		if (i <= 0 && errno != EIO)
#else
		if (i <= 0 && errno != EINTR)
#endif
			err_sys("read error");
	}

	exit(0);
}


static double
getnum(const char *s)
{
	double          d;
	char           *end;

	errno = 0;
	d = strtod(s, &end);
	if (end && *end != '\0')
		err_quit("expected a number, but got '%s'", s);
	if ((d == HUGE_VAL || d == -HUGE_VAL) && errno == ERANGE)
		err_sys("divisor '%s'", s);
	if (!(d == d)) {
		errno = EINVAL;
		err_sys("divisor '%s'", s);
	}
	return d;
}

static void
delay_for(double delay)
{
#ifdef HAVE_NANOSLEEP
	struct timespec ts, remainder;
	ts.tv_sec = (time_t) delay;
	ts.tv_nsec = (delay - ts.tv_sec) * 1.0e9;

	while (nanosleep(&ts, &remainder) < 0) {
		if (errno == EINTR) {
			if (speedflg == 1) {
				speedflg = 0;
				break;
			}
			ts.tv_sec = remainder.tv_sec;
			ts.tv_nsec = remainder.tv_nsec;
		} else
			break;
	}
#else
	struct timeval  tv;
	tv.tv_sec = (long) delay;
	tv.tv_usec = (delay - tv.tv_sec) * 1.0e6;
	select(0, NULL, NULL, NULL, &tv);
#endif
}

static void
emit(FILE * fd, const char *filename, size_t ct)
{
	char            buf[BUFSIZ];

	while (ct) {
		size_t          len, cc;
		cc = ct > sizeof(buf) ? sizeof(buf) : ct;
		len = fread(buf, 1, cc, fd);
		if (!len)
			break;
		ct -= len;
		cc = write(STDOUT_FILENO, buf, len);
		if (cc != len)
			err_sys("write to stdout failed");
	}

	if (!ct)
		return;
	if (feof(fd))
		err_quit("unexpected end of file on %s", filename);
	err_sys("failed to read typetpty file %s", filename);
}

static void
sig_usr1(int signo)
{
	sigset_t        newmask, oldmask;

#ifdef AIX
	/* reset for next interrupt */
	if (signal(SIGUSR1, sig_usr1) == SIG_ERR)
		err_sys("signal SIGUSR1");
#endif

	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);

	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
		err_sys("SIG_BLOCK error");

	while (playflg == 0)
		sigsuspend(&newmask);
	playflg = 0;

	/*
	 * Reset signalmask which unblocks SIGALRM 
	 */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
		err_sys("SIG_SETMASK error");

}

static void
sig_usr2(int signo)
{
#ifdef AIX
	/* reset for next interrupt */
	if (signal(SIGUSR2, sig_usr2) == SIG_ERR)
		err_sys("signal SIGUSR2");
#endif
	playflg = 1;
}

static void
sig_term(int signo)
{
	tty_reset(STDIN_FILENO);
	exit(0);
}

static void
sig_int_parent(int signo)
{
	reset_tput();
	exit(0);
}

static void
sig_int_child(int signo)
{
	if (kill(getppid(), SIGINT) < 0)
		err_sys("send SIGINT");
	exit(0);
}

static void
sig_alrm(int signo)
{
#ifdef AIX
	/* reset for next interrupt */
	if (signal(SIGALRM, sig_alrm) == SIG_ERR)
		err_sys("signal SIGALRM");
#endif
	speedflg = 1;
}

static void
sig_hup(int signo)
{
	sigset_t        pendmask;

#ifdef AIX
	if (signal(SIGHUP, sig_hup) == SIG_ERR)
		err_sys("signal SIGHUP");
#endif

	if (sigpending(&pendmask) < 0)
		err_sys("sigpending error");
	if (sigismember(&pendmask, SIGALRM))
		pendflg = 1;

	/*
	 * Jump to print, don't return. 
	 */
	siglongjmp(jmpbuf, 1);
}

static void
reset_tput(void)
{
	int             status;

	if ((status = system("clear")) < 0)
		err_sys("system() error");
	if (status != 0)
		err_quit("shell exit status = %d\n", status);

	if ((status = system("tput cnorm")) < 0)
		err_sys("system() error");
	if (status != 0)
		err_quit("shell exit status = %d\n", status);
}

static void
reprint(long n, FILE * tfile, FILE * sfile, const char *tname, const char *sname)
{

	int             ret;
	unsigned long   i;
	double          delay;
	size_t          blk;
	char            nl;

	if (fseek(tfile, 0, SEEK_SET) != 0)
		err_sys("fseek error (%s)", tname);
	if (fseek(sfile, 0, SEEK_SET) != 0)
		err_sys("fseek error (%s)", sname);

	ret = system("clear");
	if (ret < 0)
		err_sys("system() error");
	if (ret != 0)
		err_quit("shell exit %d\n", ret);

	for (i = 0; i < n; i++) {
		if (fscanf(tfile, "%lf %zu%c\n", &delay, &blk, &nl) != 3 ||
		    nl != '\n') {
			if (feof(tfile)) {
				errno = EINVAL;
				err_sys("failed to read timing file %s", tname);
			}
			if (ferror(tfile))
				err_sys("failed to read timing file %s", tname);
			err_sys("timings file %s: %lu: unexpected format",
				tname, i);
		}
		emit(sfile, sname, blk);
	}

}

static void
tty_resume(void)
{
	if (ttysavefd >= 0)
		if (tcsetattr(ttysavefd, TCSANOW, &savetty) < 0)
			err_sys("tcsetattr ttysavefd error");
}
