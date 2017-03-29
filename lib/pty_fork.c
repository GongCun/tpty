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

pid_t 
pty_fork(int *ptrfdm, char *slave_name, int slave_namesz,
	 const struct termios *slave_termios,
	 const struct winsize *slave_winsize)
{
	int		fdm       , fds;
	pid_t		pid;
	char		pts_name  [20];

	if ((fdm = ptym_open(pts_name, sizeof(pts_name))) < 0)
		err_sys("can't open master pty: %s, error %d", pts_name, fdm);

	if (slave_name != NULL) {
		/*
		 * return name of slave, null terminate to handle case where
		 * strlen(pts_name) > slave_namesz
		 */
		strncpy(slave_name, pts_name, slave_namesz);
		slave_name[slave_namesz - 1] = '\0';
	}
	if ((pid = fork()) < 0) {
		return (-1);
	} else if (pid == 0) {	/* child */
		/*
 		 * When child process called setsid(), the following steps
		 * occur:
		 *(a) a new session is created with the child as the session
		 * leader,
		 *(b) a new process group is created for the child, and
		 *(c) the child loses any association it might have had with
		 * its previous controlling terminal.
		 *
		 * Under Linux and Solaris, the slave becomes the controlling
		 * terminal of this new session wher ptys_open() is called.
		 * Under FreeBSD and Mac OS X, we have to call ioctl() with an
		 * argument of TIOCSCTTY to allocate the controlling terminal.
		 * (Linux also supports the ioctl() TIOCSCTTY command).
		 */
		if (setsid() < 0)
			err_sys("setsid error");

		/*
		 * System V acquires controlling terminal on open().
		 */
		if ((fds = ptys_open(pts_name)) < 0)
			err_sys("can't open slave pty");
		close(fdm);	/* all done with master in child */
#ifdef TIOCSCTTY
		if (ioctl(fds, TIOCSCTTY, (char *)0) < 0)
			err_sys("ioctl() TIOCSCTTY error");
#endif

		/*
		 * set slave's termios and window size
		 */
		if (slave_termios != NULL) {
			if (tcsetattr(fds, TCSAFLUSH, slave_termios) < 0)
				err_sys("tcsetattr error on slave pty");
		}
		if (slave_winsize != NULL) {
			if (ioctl(fds, TIOCSWINSZ, slave_winsize) < 0)
				err_sys("TIOCSWINSZ error on slave pty");
		}
		/*
		 * slave becomes stdin/stdout/stderr of child
		 */
		if (dup2(fds, STDIN_FILENO) != STDIN_FILENO)
			err_sys("dup2 error to stdin");
		if (dup2(fds, STDOUT_FILENO) != STDOUT_FILENO)
			err_sys("dup2 error to stdout");
		if (dup2(fds, STDERR_FILENO) != STDERR_FILENO)
			err_sys("dup2 error to stderr");
		if (fds != STDIN_FILENO && fds != STDOUT_FILENO &&
		    fds != STDERR_FILENO)
			close(fds);
		return (0);	/* child return 0 just like fork() */
	} else {
		*ptrfdm = fdm;	/* return fd of master */
		return (pid);	/* parent returns pid of child */
	}
}
