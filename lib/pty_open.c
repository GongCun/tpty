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
#include <errno.h>
#include <fcntl.h>

#ifndef HAVE_POSIX_OPENPT
#if defined(AIX) || defined(FREEBSD)
#include "ptym_bsd.c"
#elif defined(SOLARIS) || defined(LINUX)
#include "ptym.c"
#else
#error "No method to find the master pty."
#endif /* getptymaster() */
#endif /* HAVE_POSIX_OPENPT */

int 
ptym_open(char *pts_name, int pts_namesz)
{
	char           *ptr;
	int		fdm       , err;

	if ((fdm = posix_openpt(O_RDWR)) < 0)
		return(-1);
	if (grantpt(fdm) < 0)	/* grant access to slave */
		goto errout;
	if (unlockpt(fdm) < 0)	/* clear slave's lock flag */
		goto errout;
	if ((ptr = ptsname(fdm)) == NULL)	/* get slave's name */
		goto errout;

	/*
	 * return name of slave, null terminate to handle case where
	 * strlen(ptr) > pts_namesz
	 */
	strncpy(pts_name, ptr, pts_namesz);
	pts_name[pts_namesz - 1] = '\0';
	return(fdm);		/* return fd of master */
errout:
	err = errno;
	close(fdm);
	errno = err;
	return(-1);
}

int 
ptys_open(char *pts_name)
{
	int		fds;

	if ((fds = open(pts_name, O_RDWR)) < 0)
		return(-1);
#ifndef SOLARIS
	return(fds);
#else
	/*
 	 * Check if stream is already set up by autopush facility.
	 * APUE 2nd. Edition Figure 19.8. STREAMS-based pseudo-terminal open
	 * functions.
	 */
	int setup, err;
	if ((setup = ioctl(fds, I_FIND, "ldterm")) < 0)
		goto errout;
	if (setup == 0) {
		/*
 		 * ptem - STREAMS Pseudo Terminal Emulation module
		 */
		if (ioctl(fds, I_PUSH, "ptem") < 0)
			goto errout;
		/*
 		 * ldterm - standard STREAMS terminal line discipline module
		 */
		if (ioctl(fds, I_PUSH, "ldterm") < 0)
			goto errout;
		/*
 		 * ttcompat - V7, 4BSD and XENIX STREAMS compatibility module
 		 */
		if (ioctl(fds, I_PUSH, "ttcompat") < 0)
			goto errout;
	}
	return(fds);
errout:
	err = errno;
	close(fds);
	errno = err;
	return(-1);
#endif
}
