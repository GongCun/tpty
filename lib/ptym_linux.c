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


