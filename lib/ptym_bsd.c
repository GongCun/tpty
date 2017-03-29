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
	char *ptr1, *ptr2;
	char ptm_name[16];

	strcpy(ptm_name, "/dev/ptyXY");

	for (ptr1 = "pqrstuvwxyzPQRST"; *ptr1 != 0; ptr1++) {
		ptm_name[8] = *ptr1;
		for (ptr2 = "012345678abcdef"; *ptr2 != 0; ptr2++) {
			ptm_name[9] = *ptr2;
			if ((fdm = open(ptm_name, oflag)) < 0) {
				if (errno == ENOENT)	/* different from EIO */
					return(-1);	/* out of pty devices */
				else
					continue;
			}
			return(fdm);	/* got it, return fd of master */
		}
	}
	errno = EAGAIN;
	return(-1);	/* out of pty devices */
}

#ifndef HAVE_PTSNAME
char *ptsname(int fdm)
{
	static char pts_name[16];
	char *ptm_name;

	if ((ptm_name = ttyname(fdm)) == NULL)
		return(NULL);
	strncpy(pts_name, ptm_name, sizeof(pts_name));
	pts_name[sizeof(pts_name)-1] = '\0';
	if (strncmp(pts_name, "/dev/pty/", 9) == 0)
		pts_name[9] = 's';	/* change /dev/pty/mXX to /dev/pty/sXX */
	else
		pts_name[5] = 't';	/* change /dev/ptyXX to /dev/ttyXX */
	return(pts_name);
}
#endif /* HAVE_PTSNAME */

#ifndef HAVE_GRANTPT
int grantpt(int fdm)
{
	struct group *grptr;
	int gid;
	char *pts_name;

	if ((pts_name = ptsname(fdm)) == NULL)
		return(-1);

	if ((grptr = getgrnam("tty")) != NULL)
		gid = grptr->gr_pid;
	else
		gid = -1; /* Group tty is not in the group file,
			     a value of -1 indicates the group is not changed */
	/*
 	 * The following two calls won't work unless we're the superuser.
	 */
	if (chown(pts_name, getuid(), gid) < 0)
		return(-1);

	/* chmod() 0666 */
	return(chmod(pts_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
}
#endif /* HAVE_GRANTPT */

#ifndef HAVE_UNLOCKPT
int unlockpt(int fdm)
{
	return(0); /* nothing to do */
}
#endif /* HAVE_UNLOCKPT */
