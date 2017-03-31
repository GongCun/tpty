#ifndef _TPTY_H_
#define _TPTY_H_

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <stddef.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <regex.h>
#include <signal.h>
#include <grp.h>

#if defined(AIX) || defined(SOLARIS)
#include <stropts.h>
#endif

#ifdef HAVE_OPENSSL /* OpenSSL Library in C */
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#endif

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif
#define MAXLINE 8192
#define EXP_FULL 64
#define FILE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)	/* -rw-r--r-- */
#define LOCKFILE "lock.file"
#define DELIM "%\t\n"

#define EXP_EOF 0
#define EXP_TIMEOUT 1
#define EXP_ERRNO 2

#undef err_line
#define err_line() \
	((void)fprintf(stderr, ">> %s:%u <<\n", __FILE__, __LINE__), \
	 fflush(stderr))

typedef void
Sigfunc(int);

void
err_sys(const char *,...) __attribute__((noreturn));
void
err_quit(const char *,...) __attribute__((noreturn));
void
err_dump(const char *,...) __attribute__((noreturn));
void
err_dump(const char *,...) __attribute__((noreturn));
void
err_exit(int, const char *,...) __attribute__((noreturn));
void
err_ret(const char *,...);
void
err_msg(const char *,...);

ssize_t
readn(int filedes, void *buf, size_t nbytes);
ssize_t
writen(int filedes, void *buf, size_t nbytes);

Sigfunc        *
signal_rest(int, Sigfunc);
Sigfunc        *
signal_intr(int, Sigfunc);

int
s_pipe(int fd[2]);

int
tty_cbreak(int);
int
tty_raw(int);
int
tty_reset(int);
void
tty_atexit(void);
struct termios *
tty_termios(void);

int
ptym_open(char *, int);
int
ptys_open(char *);
pid_t
pty_fork(int *, char *, int,
	 const struct termios *,
	 const struct winsize *);
extern char    *pathconfig;
extern char    *driver;
extern int      timeout;
extern int      tflg;
extern int      oflg;
extern int      zeroflg;
extern int      manflg;
extern int      encflg;
extern FILE    *keyfd;
extern char    *rsafd;
extern int      auditflg;
extern FILE    *timingfd;
extern int      outputfd;
extern FILE    *auditfd;
extern int      tty;
extern FILE    *fdbg;
extern int      rmflg;

int
strregex(char *, char *);
int
def_driver(void);
int
inter_driver(void);

void
set_fl(int, int);
void
clr_fl(int, int);

int
lock_reg(int, int, int, off_t, int, off_t);
pid_t
lock_test(int, int, off_t, int, off_t);
int
sys_exit(int status);
void
fdebug(FILE *, const char *,...);
char *gettime(void);

#ifdef HAVE_OPENSSL
RSA            *
create_rsa_with_filename(char *, int);

int
public_encrypt(char *, int, char *, char *);

int
private_decrypt(char *, int, char *, char *);

#endif /* OpenSSL functions */

#endif
