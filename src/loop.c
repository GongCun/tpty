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

static void
close_outputfd(void);
static void
close_timingfd(void);
static void
close_auditfd(void);

/*
 * Copy everything received from the stdin to the PTY master and everything
 * from the PTY master to the stdout and do some other processing.
 */
void
loop(int ptym, int ignoreeof)
{
	fd_set          rset;
	int             maxfd, ignore = 0, i;
	ssize_t         nread, nleft;
	char            buf[MAXLINE], *ptr;
	struct timeval  tv;
	double          oldtime = time(NULL), newtime;
	time_t          tp;
	char           *ctime_buf;

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		err_sys("Ignore SIGPIPE error");

	if (oflg && outputfd >= 0)
		if (atexit(close_outputfd) < 0)
			err_sys("atexit close_outputfd");
	if (tflg && timingfd)
		if (atexit(close_timingfd) < 0)
			err_sys("atexit close_timingfd");
	if (auditflg && auditfd)
		if (atexit(close_auditfd) < 0)
			err_sys("atexit close_auditfd");

#ifdef SOLARIS
	/*
	 * Solaris need try to setup raw mode again, ignore error
	 * if child process. (I don't know why.)
	 */
	tty_raw(STDIN_FILENO);
#endif

	for (;;) {
		FD_ZERO(&rset);
		FD_SET(ptym, &rset);
		if (ignore == 1) {
			maxfd = ptym;
		} else {
			maxfd = STDIN_FILENO > ptym ? STDIN_FILENO : ptym;
			FD_SET(STDIN_FILENO, &rset);
		}

		if (select(maxfd + 1, &rset, NULL, NULL, NULL) < 0) {
			if (errno == EINTR)
				continue;
			err_sys("select error");
		}
		if (ignore == 0 && FD_ISSET(STDIN_FILENO, &rset)) {
			if ((nread = read(STDIN_FILENO, buf, sizeof(buf))) < 0) {
				if (errno != ECONNRESET) /* mean what ?? */
					err_sys("read error from stdin");
			} else if (nread == 0) {
				if (ignoreeof == 0)
					break;
				else
					ignore = 1;
			} else {
				if (writen(ptym, buf, nread) != nread)
					err_sys("write error on master pty");
				if (auditflg && auditfd) {
					if (time(&tp) == (time_t) - 1)
						err_sys("time() error");
					ctime_buf = ctime(&tp);
					ctime_buf[strlen(ctime_buf) - 1] = 0;
					nleft = nread;
					ptr = buf;
					while (nleft > 0) {
						for (i = 0; i < nleft && (*(ptr + i) & 255) != 015; i++)
							;
						if (writen(fileno(auditfd), ptr, i) != i)	/* don't include CR */
							err_sys("writen() AuditFile error");
						ptr += i;
						nleft -= i;
						if (nleft > 0 && (*ptr & 255) == 015) {	/* bypass CR */
							nleft--;
							ptr++;
							if (fprintf(auditfd, "\n[%s]\n", ctime_buf) < 0)
								err_sys("fprintf error");
							fflush(auditfd);
						}
					}
				}
			}
		}
		if (FD_ISSET(ptym, &rset)) {
			if (tflg)
				gettimeofday(&tv, NULL);
			if ((nread = read(ptym, buf, sizeof(buf))) <= 0)
				break;	/* signal caught, error, or EOF */
			if (writen(STDOUT_FILENO, buf, nread) != nread) {
				if (errno != EPIPE)
					err_sys("writen error to stdout");
			}
			if (oflg && outputfd) {
				if (writen(outputfd, buf, nread) != nread)
					err_sys("writen error to tmp file");
				if (fsync(outputfd) < 0)
					err_sys("fsync error");
			}
			if (tflg && timingfd) {
				newtime = tv.tv_sec + (double) tv.tv_usec / 1000000;
				fprintf(timingfd, "%f %zd\n", newtime - oldtime, nread);
				oldtime = newtime;
				fflush(timingfd);
			}
		}
	}
	if (nread < 0 && errno != EIO)
		err_sys("read ptym");

	/*
 	 * Must close both stdin and stdout to let the driver known
	 * pts has closed.
	 */
	if (close(STDOUT_FILENO) < 0)
		err_sys("loop() close stdout error");
	if (close(STDIN_FILENO) < 0)
		err_sys("loop() close stdin error");

	return;
}

static void
close_outputfd(void)
{
	if (close(outputfd) < 0)
		_exit(1);
}

static void
close_timingfd(void)
{
	if (close_stream(timingfd) != 0 && !(errno != EPIPE)) {
		if (errno)
			err_ret("write error");
		else
			err_msg("write error");
		_exit(1);
	}
}

static void
close_auditfd(void)
{
	if (close_stream(auditfd) != 0 && !(errno != EPIPE)) {
		if (errno)
			err_ret("write error");
		else
			err_msg("write error");
		_exit(1);
	}
}
