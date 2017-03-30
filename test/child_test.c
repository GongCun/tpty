#include "tpty.h"
#include <sys/wait.h>

int
main(void)
{

	pid_t           pid;
	int             status;

	if ((pid = fork()) < 0) {
		err_sys("fork error");
	} else if (pid == 0) {
		printf("pid = %ld\n", (long) pid);
		exit(0);
	}
	printf("parent see pid = %ld\n", (long) pid);
	if (wait(&status) < 0)
		err_sys("wait error");
	exit(0);
}
