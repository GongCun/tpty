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
#include <errno.h>
#undef TTY_RESET
#undef TTY_RAW
#undef TTY_CBREAK

static struct termios save_termios;
static int      ttysavefd = -1;
static enum {
	TTY_RESET, TTY_RAW, TTY_CBREAK
}               ttystate = TTY_RESET;

int
tty_cbreak(int fd)
{				/* put terminal into a cbreak mode */
	struct termios  buf;
	int err;

	if (tcgetattr(fd, &save_termios) < 0)
		return (-1);

	buf = save_termios;	/* structure copy */
	buf.c_lflag &= ~(ECHO | ICANON);	/* echo off, canonical mode
						 * off */
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;

	if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
		return (-1);
	/*
 	 * Verify that the changes stuck.
	 */
	if (tcgetattr(fd, &buf) < 0) {
		err = errno;
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = err;
		return(-1);
	}
	if ((buf.c_lflag & (ECHO | ICANON)) || buf.c_cc[VMIN] != 1 ||
			buf.c_cc[VTIME] != 0)
	{
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = EINVAL;
		return(-1);
	}
	/*-------------------------------------*/

	ttystate = TTY_CBREAK;
	ttysavefd = fd;
	return (0);
}

int
tty_raw(int fd)
{
	struct termios  buf;
	int err;

	if (tcgetattr(fd, &save_termios) < 0)
		return (-1);

	buf = save_termios;

	/*
	 * echo off, canonical mode off, extended input processing off,
	 * signal chars off 
	 */
	buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	/*
	 * No SIGINT on BREAK, CR-to-NL off, input parity check off, don't
	 * strip 8th bit on input, output flow control off 
	 */
	buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

	/*
	 * clear size bits, parity checking off 
	 */
	buf.c_cflag &= ~(CSIZE | PARENB);

	/* set 8 bits/char */
	buf.c_cflag |= CS8;

	/* output processing off */
	buf.c_oflag &= ~(OPOST);

	buf.c_cc[VMIN] = 1;	/* 1 byte at a time, no timer */
	buf.c_cc[VTIME] = 0;

	if (tcsetattr(fd, TCSAFLUSH, &buf) < 0) {
		err = errno;
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = err;
		return (-1);
	}

	/*
	 * Verify that the changes stuck.
	 */
	if (tcgetattr(fd, &buf) < 0) {
		err = errno;
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = err;
		return(-1);
	}
	if ((buf.c_lflag & (ECHO | ICANON | IEXTEN | ISIG)) ||
			(buf.c_iflag & (BRKINT | ICRNL | INPCK | ISTRIP | IXON)) ||
			((buf.c_cflag & (CSIZE | PARENB | CS8)) != CS8) ||
			(buf.c_oflag & OPOST) || buf.c_cc[VMIN] != 1 ||
			buf.c_cc[VTIME] != 0)
	{
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = EINVAL;
		return(-1);
	}
	/*------------------------------------------------------------*/

	ttystate = TTY_RAW;
	ttysavefd = fd;
	return (0);
}

int
tty_reset(int fd)
{				/* restore terminal's mode */
	if (ttystate != TTY_CBREAK && ttystate != TTY_RAW)
		return (0);
	if (tcsetattr(fd, TCSAFLUSH, &save_termios) < 0)
		return (-1);
	ttystate = TTY_RESET;
	return (0);
}

void
tty_atexit(void)
{				/* can be set up by atexit(tty_atexit) */
	if (ttysavefd >= 0)
		tty_reset(ttysavefd);
}

struct termios *
tty_termios(void)
{				/* let caller see original tty state */
	return (&save_termios);
}
