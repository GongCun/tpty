#include "tpty.h"
#include <signal.h>
#include <setjmp.h>

#define INTER_EOF 0
#define INTER_TIMEOUT 1
#define INTER_ERR 2

int
inter_driver(void)
{
	int             fd, len, ret, return_val = INTER_ERR, maxfd;
	char            readin[BUFSIZ];
	fd_set          rset;
#define return_inter_driver(x) {return_val = x; goto over;}

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
