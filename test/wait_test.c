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
		execl("./a.ksh", "a.ksh", (char *) 0);
		err_sys("execl error");
	}
	kill(pid, SIGTERM);
	if (wait(&status) < 0)
		err_sys("wait error");
	if (WIFEXITED(status))
		printf("normal termination, exit status = %d\n",
		       WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		printf("abnormal termination, signal number = %d\n",
		       WTERMSIG(status));
}
