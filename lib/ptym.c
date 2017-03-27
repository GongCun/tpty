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
int posix_openpt(int oflag)
{
	int fdm;
	fdm = open("/dev/ptmx", oflag);
	return(fdm);
}

#ifndef HAVE_PTSNAME
char *ptsname(int fdm)
{
	int sminor;
	static char pts_name[16];

	/* Get Pty Number (of pty-mux device) */
	if (ioctl(fdm, TIOCGPTN, &sminor) < 0)
		return(NULL);

	snprintf(pts_name, sizeof(pts_name), "/dev/pts/%d", sminor);
	return(pts_name);
}
#endif

#ifndef HAVE_GRANTPT
int grantpt(int fdm)
{
	char *pts_name;

	if ((pts_name = ptsname(fdm)) == NULL)
		return(-1);
	return(chmod(pts_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
}
#endif /* HAVE_GRANTPT */

#ifndef HAVE_UNLOCKPT
/* Unlocks a pesudo-terminal device. */
int unlockpt(int fdm)
{
	int lock = 0;
	return(ioctl(fdm, TIOCSPTLCK, &lock));
}
#endif /* HAVE_UNLOCKPT */

