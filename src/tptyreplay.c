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
#include <sys/mman.h>

#ifdef LINUX
#define OPTSTR "+t:s:d:m:"
#else
#define OPTSTR "t:s:d:m:"
#endif

#define HELP "tptyreplay [options] -t timingfile -s typetpty"
#define TPTY_MIN_DELAY 0.0001
#define BACKROWS 1

static struct flags {
	unsigned char pauseflg, backflg, forwardflg;
	double divi;
} *flags;

static sigjmp_buf jmpbuf;
static struct termios savetty;

static double getnum(const char *);
static void delay_for(double delay);
static void emit(FILE *, const char *, size_t);
static void reprint(long, FILE *, FILE *, const char *, const char *);
static void tty_resume(void);


int
main(int argc, char *argv[])
{
	FILE           *tfile, *sfile;
	const char     *sname = NULL, *tname = NULL;
	double          divi = 1, maxdelay = 0;
	int             diviopt __attribute__((unused)) = 0, maxdelayopt = 0;
	long	line = 0;
	long	n = 0;
	int             ch, ret, tfd;
	pid_t           pid;


	atexit(close_stdout);

	if (tcgetattr(STDIN_FILENO, &savetty) < 0)
		err_sys("tcgetattr stdin error");

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

#ifdef MAP_ANON
	flags = mmap(0, sizeof(struct flags),
			PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
#else
	int fd;
	if ((fd = open("/dev/zero", O_RDWR, 0)) < 0)
		err_sys("open() /dev/zero error");
	flags = mmap(0, sizeof(struct flags),
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
#endif
	if (flags == MAP_FAILED)
		err_sys("mmap() error");
	memset(flags, 0, sizeof(struct flags));
	flags->divi = divi;
	

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid > 0) {	/* parent replay, child read subcommand */
		atexit(tty_resume);
		while (1) {
			double          delay;
			size_t          blk;
			char            nl;

			sigsetjmp(jmpbuf, 1);
			n = line;
			while (flags->pauseflg) {
				if (flags->backflg || flags->forwardflg)
					break;
			}
			divi = flags->divi;

			if (flags->backflg) {
				flags->backflg = 0;
				if ((line = --n) < 0)
					line = 0;

				reprint(line, tfile, sfile, tname, sname);
				if (flags->pauseflg && line == 0) {
					fputs(">> Already in the beginning of the file. <<", stdout);
					fflush(stdout);
				}

				continue;
			}
			if (line == 0) {
				/* ignore the first typescript line (start message) */
				while ((ch = fgetc(sfile)) != EOF && ch != '\n')
					;

				ret = system("clear");
				if (ret < 0)
					err_sys("system() error");
				if (ret != 0)
					err_quit("shell exit %d", ret);
			}

			if (fscanf(tfile, "%lf %zu%c\n", &delay, &blk, &nl) != 3 || nl != '\n') {
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
				if (ferror(tfile)) {
#ifdef DEBUG
					err_line();
#endif
					err_sys("failed to read timing file %s", tname);
				}
				err_sys("timings file %s: %lu: unexpected format",
					tname, line);
			}
			delay /= divi;
			if (maxdelayopt && delay > maxdelay)
				delay = maxdelay;
			if (flags->forwardflg) {
				flags->forwardflg = 0;
				delay = 0;
			}
			if (delay > TPTY_MIN_DELAY)
				delay_for(delay);
			emit(sfile, sname, blk);
			line++;

		}

		fclose(sfile);
		fclose(tfile);
		kill(pid, SIGTERM);
		while (wait(NULL) < 0 && errno == EINTR)
			;
	} else {		/* child read subcommand */
		int             i;
		char            c;

		if (tty_cbreak(STDIN_FILENO) < 0)
			err_sys("tty_cbreak error");
		while ((i = read(STDIN_FILENO, &c, 1)) == 1) {
			switch (c) {
				case ' ':
					flags->pauseflg = (flags->pauseflg + 1) % 2;
					break;
				case 'u':
					flags->divi *= 2;
					break;
				case 'd':
					flags->divi /= 2;
					break;
				case 'b':
					flags->backflg = 1;
					break;
				case 'f':
					flags->forwardflg = 1;
					break;
				default: /* resume to original speed */
					flags->divi = 1;
					break;
			}
		}

		if (i <= 0 && !(errno == EIO || errno == EINTR)) /* not killed by parent */
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
	if (feof(fd)) {
#ifdef DEBUG
		err_line();
#endif
		err_quit("unexpected end of file on %s", filename);
	}
	err_sys("failed to read typetpty file %s", filename);
}

static void
reprint(long n, FILE * tfile, FILE * sfile, const char *tname, const char *sname)
{

	int             ret;
	long   i;
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

	if (n == 0)
		return;

	/* ignore the first typescript line (start message) */
	while ((ret = fgetc(sfile)) != EOF && ret != '\n')
		;

	for (i = 0; i < n; i++) {
		if (fscanf(tfile, "%lf %zu%c\n", &delay, &blk, &nl) != 3 ||
		    nl != '\n') {
			if (feof(tfile)) {
#ifdef DEBUG
				printf("i = %ld, n = %ld\n", i, n);
				err_line();
#endif
				errno = EINVAL;
				err_sys("failed to read timing file %s", tname);
			}
			if (ferror(tfile)) {
#ifdef DEBUG
				err_line();
#endif
				err_sys("failed to read timing file %s", tname);
			}
			err_sys("timings file %s: %lu: unexpected format",
				tname, i);
		}
		emit(sfile, sname, blk);
	}

}

static void
tty_resume(void)
{
#undef ENDMSG
#define ENDMSG "<< tptyreplay end >>\n"
	if (tcsetattr(STDIN_FILENO, TCSANOW, &savetty) < 0)
		err_sys("tcsetattr() error");
	system("tput cnorm");
	writen(STDOUT_FILENO, ENDMSG, strlen(ENDMSG));
}
