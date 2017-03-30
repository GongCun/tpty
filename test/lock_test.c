#include "tpty.h"

int main(int argc, char *argv[])
{
	int fd, rc;

	if ((fd = open(argv[1], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
		err_sys("open error");
	while((rc = lock_reg(fd, F_SETLKW, F_WRLCK, 0, SEEK_SET, 0)) < 0) {
		if (errno == EINTR)
			continue;
		else
			err_sys("lock_reg() F_SETLKW error");
	}
	printf("got\n");
	sleep(10);
	printf("released\n");
	lock_reg(fd, F_SETLKW, F_UNLCK, 0, SEEK_SET, 0);
	sleep(10);
	return(0);
}
