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
#include <signal.h>
#include <setjmp.h>

enum { INTER_EOF = 0, INTER_TIMEOUT = 1, INTER_ERR = 2 };

int
inter_driver(void)
{
	int             fd, len, ret, return_val = INTER_ERR, maxfd;
	char            readin[BUFSIZ];
	fd_set          rset;
#define return_inter_driver(x) ({return_val = x; goto over;})

	if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
		err_sys("SIGHUP");
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		err_sys("SIGPIPE");

	fd = open("/dev/tty", O_RDONLY, 0);
	if (fd < 0)
		err_sys("open /dev/tty error");
	/*
	 * The user's terminal is put into raw so that the connection is
	 * transparent. 
	 */
	if (tty_raw(fd) < 0)
		err_sys("tty_raw error");
	if (atexit(tty_atexit) < 0)
		err_sys("atexit error");
	maxfd = fd > STDIN_FILENO ? fd : STDIN_FILENO;

	if (tty < 0)		/* original STDOUT_FILENO */
		err_quit("tty error");

	for (;;) {
		FD_ZERO(&rset);
		FD_SET(STDIN_FILENO, &rset);
		FD_SET(fd, &rset);

		ret = select(maxfd + 1, &rset, NULL, NULL, (struct timeval *) 0);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return_inter_driver(INTER_ERR);
		} else if (!ret) {
			return_inter_driver(INTER_TIMEOUT);
		}
		if (FD_ISSET(STDIN_FILENO, &rset)) {
			len = read(STDIN_FILENO, readin, sizeof(readin));
			if (len <= 0)
				return_inter_driver(INTER_EOF);
			if (writen(tty, readin, len) != len)
				return_inter_driver(INTER_ERR);
		}
		if (FD_ISSET(fd, &rset)) {
			len = read(fd, readin, sizeof(readin));
			if (len <= 0)
				return_inter_driver(INTER_EOF);
			if (writen(STDOUT_FILENO, readin, len) != len)
				return_inter_driver(INTER_ERR);
		}
	}

over:
	return return_val;
}
