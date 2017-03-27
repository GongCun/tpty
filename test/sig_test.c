#include "tpty.h"

static void sig_int(int);
static void sig_quit(int);
static void sig_hup(int);

volatile sig_atomic_t flg;

int main(void)
{
    sigset_t newmask, oldmask, waitmask;

    if (signal(SIGINT, sig_int) == SIG_ERR)
        err_sys("SIGINT");
    if (signal(SIGQUIT, sig_quit) == SIG_ERR)
        err_sys("SIGQUIT");
    if (signal(SIGHUP, sig_hup) == SIG_ERR)
        err_sys("SIGHUP");

    sigemptyset(&newmask);
    sigemptyset(&waitmask);
    sigaddset(&newmask, SIGINT);
    sigaddset(&newmask, SIGQUIT);
    sigaddset(&waitmask, SIGQUIT);

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
        err_sys("SIG_BLOCK error");

    while (flg == 0)
        sigsuspend(&waitmask);

    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
        err_sys("SIG_SETMASK error");

    exit(0);
}

static void sig_int(int signo)
{
    flg = 1;
    printf("caught SIGINT\n");
}

static void sig_quit(int signo)
{
    printf("caught SIGQUIT\n");
}

static void sig_hup(int signo)
{
    sigset_t pendmask;

    if (sigpending(&pendmask) < 0)
        err_sys("sigpending error");
    if (sigismember(&pendmask, SIGQUIT))
        printf("\nSIGQUIT pending\n");
}
