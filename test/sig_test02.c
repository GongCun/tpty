#include "tpty.h"
#include <signal.h>

volatile sig_atomic_t quitflag;


static void sig_handler(int signo)
{
    if (signo == SIGINT)
        printf("caught SIGINT\n");
    if (signo == SIGQUIT)
        quitflag = 1;
}

int main(void)
{
    sigset_t newmask, zeromask, oldmask;

    sigemptyset(&newmask);
    sigemptyset(&zeromask);
    sigaddset(&newmask, SIGQUIT);

    if (signal(SIGQUIT, sig_handler) == SIG_ERR)
        err_sys("signal (SIGQUIT) error");
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        err_sys("signal (SIGINT) error");

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
        err_sys("sigprocmask error");


    while (quitflag == 0)
        if (sigsuspend(&zeromask) != -1)
            err_sys("sigsuspend error");
    quitflag = 0;

    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
        err_sys("sigprocmask error");

    exit(0);

}
    

