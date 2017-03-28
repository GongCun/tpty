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
#include "tpty.h"
#include <signal.h>
#include <setjmp.h>

enum { DRIVER_EOF = -1, DRIVER_ERROR = -2, DRIVER_TIMEDOUT = -3 };

static struct exp {
	int             matched;	/* have matched or not		*/
	int             repeat;		/* repeat this command ?	*/
	int             nocr;		/* no carriage return		*/
	char            prompt[BUFSIZ];	/* prompt or match string	*/
	char            cmd[BUFSIZ];	/* run cmd when prompt matched	*/
}               exp_list[EXP_FULL];

int             i_read_errno;

FILE           *fdbg;

static int
match_readin(char *, char *);

static int
i_read(int, int, char *, int);

static void
fp2f(FILE * fp, int *p);

static int 
rm_nulls(char *s, int c)
{
	char           *s2 = s;
	int             count = 0;
	int             i;

	for (i = 0; i < c; i++, s++) {
		if (*s == 0) {
			count++;
			continue;
		}
		if (count)
			*s2 = *s;
		s2++;
	}
	return count;
}


int
def_driver(void)
{
	FILE           *fp;
	int             i, n, cc;
	int             zero;
	char           *readin;
	int             len, return_val = EXP_ERRNO;
	int             ret, buf_len, readin_len, old_len;
	char           *readin_end, *match_end;
#define return_def_driver(x) ({return_val = x; goto cleanup;})
#ifdef DEBUG
	fdbg = fopen("./dbg.out", "w");
	if (fdbg == NULL)
		err_sys("fopen dbg.out error");
#endif
	readin_len = 2 * BUFSIZ;
	if ((readin = calloc(readin_len, 1)) == NULL)
		err_sys("calloc failed");

	match_end = readin;
	readin_end = readin;
	buf_len = 0;

	if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
		err_sys("SIGHUP");
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		err_sys("SIGPIPE");

	fp = fopen(pathconfig, "r");
	if (fp == NULL)
		err_sys("fopen config");

	fp2f(fp, &n);

	if (tty < 0)
		err_quit("tty error");
	if (zeroflg) {
		if ((zero = open("/dev/null", O_RDWR)) < 0)
			err_sys("open /dev/null");
		if (dup2(zero, tty) != tty)
			err_sys("dup2 error to tty");
		if (zero != tty)
			close(zero);
	}
	for (;;) {
		cc = i_read(tty, timeout, readin_end, readin_len - 1 - buf_len);

		if (cc == DRIVER_EOF)
			return_def_driver(EXP_EOF);
		if (cc == DRIVER_ERROR) {
			if (i_read_errno == EIO)
				return_def_driver(EXP_EOF);
			errno = i_read_errno;
			return_def_driver(EXP_ERRNO);
		}
		if (cc == DRIVER_TIMEDOUT)
			return_def_driver(EXP_TIMEOUT);

		old_len = buf_len;
		buf_len += cc;
		buf_len -= rm_nulls(readin + old_len, cc);
		readin_end = readin + buf_len;
		*readin_end = '\0';

#ifdef DEBUG
		fdebug(fdbg, "\nreadin = <<%s>>\n", readin);
#endif

		for (i = 0; i < n; i++) {
			if (!exp_list[i].matched &&
			 (ret = match_readin(readin, exp_list[i].prompt))) {
				match_end = readin + ret;
				buf_len = readin_end - match_end;
				memmove(readin, match_end, buf_len);
				readin_end = readin + buf_len;

				if (!exp_list[i].repeat)
					exp_list[i].matched = 1;
				if (strstr(exp_list[i].cmd, "<INTERACT>") != NULL) {
				       	if(manflg)
						return_def_driver(EXP_EOF);
					else {
						err_msg("Illegal keyword <INTERACT>, please use -I option");
						return_def_driver(EXP_ERRNO);
					}
				}
				len = strlen(exp_list[i].cmd);
				if (writen(STDOUT_FILENO, exp_list[i].cmd, len) != len)
					if (errno != EPIPE && errno != EIO)
						err_sys("writen error");
				break;
			}
		}

		/* move last half to first half */
		if (buf_len == readin_len - 1) {
			memmove(readin, readin + buf_len / 2, readin_len - 1 - buf_len / 2);
			buf_len = readin_len - 1 - buf_len / 2;
			readin_end = readin + buf_len;
		}
	}

cleanup:
	fclose(fp);
	free(readin);
	return return_val;
}

int
match_readin(char *readin, char *str)
{				/* return the end position of the match
				 * string */
	int             ret;
	char           *ch;

	ch = strstr(readin, str);
	if (ch != NULL)
		return ch - readin + strlen(str);
	if ((ret = strregex(str, readin)) > 0)
		return ret;
	return 0;

}

static int
i_read(int fd, int tm, char *buf, int buf_size)
{
	fd_set          rset;
	int             cc, ret;
	struct timeval  tv, *tvptr;

	if (tm < 0)
		tvptr = (struct timeval *) 0;
	else {
		tv.tv_sec = tm;
		tv.tv_usec = 0;
		tvptr = &tv;
	}

start:
	FD_ZERO(&rset);
	FD_SET(STDIN_FILENO, &rset);

	if ((ret = select(STDIN_FILENO + 1, &rset, NULL, NULL, tvptr)) < 0) {
		if (errno == EINTR)
			goto start;
		i_read_errno = errno;
		return DRIVER_ERROR;
	}
	if (ret == 0) {		/* timeout */
		return DRIVER_TIMEDOUT;
	}
	if (FD_ISSET(STDIN_FILENO, &rset)) {
		cc = read(STDIN_FILENO, buf, buf_size);
		if (cc < 0)
			err_sys("read error");
		if (cc == 0)
			return DRIVER_EOF;
		if (cc > 0) {
#ifdef DEBUG
			fdebug(fdbg, "\nbuf = <<%s>>\n", buf);
#endif
			if (writen(fd, buf, cc) != cc)
				if (errno != EIO)
					err_sys("writen error");
		}
	}
	return cc;
}

static void
fp2f(FILE * fp, int *p)
{
	char           *buf, *ptr, ch;
	int             i, field, len, offset;
	char            encrypted[4098];

	buf = malloc(2 * BUFSIZ);
	if (buf == NULL)
		err_sys("malloc failed");
	for (i = 0; ; i++) {
		field = 0;
		if (i >= EXP_FULL)
			err_quit("lines out of scope");
		if (fgets(buf, 2 * BUFSIZ, fp) == NULL) {
			if (feof(fp))
				break;
			if (ferror(fp))
				err_sys("fgets error");
		}
		if (strtok(buf, DELIM) == NULL)
			err_quit("format error (no field separator %%): line %d.", i + 1);
		strncpy(exp_list[i].prompt, buf, BUFSIZ);
		exp_list[i].prompt[BUFSIZ - 1] = '\0'; /* if not terminated with a null byte */

		bzero(exp_list[i].cmd, sizeof exp_list[i].cmd);
		while ((ptr = strtok(NULL, DELIM)) != NULL) {
			if (strlen(ptr) > BUFSIZ - 3 - strlen(exp_list[i].cmd))	/* '%' + '\r' + null = 3 */
				err_quit("cmd string out of buff: line %d.", i + 1);
			if (strstr(ptr, "<REPEAT>") != NULL) {
				exp_list[i].repeat = 1;
			} else if (strstr(ptr, "<NOCR>") != NULL) {
				exp_list[i].nocr = 1;
			} else if (strstr(ptr, "<ENCRYPT>") != NULL) {
				if (!(rsafd && keyfd))
					err_quit("cannot open key or rsa file");
				if (!fread(&offset, sizeof offset, 1, keyfd))	/* the encrypted key
										 * length */
					err_sys("fread offset error");
				len = fread(encrypted, 1, offset, keyfd);
				if (!len)
					err_sys("fread encrypted string error");
				private_decrypt(encrypted, len, rsafd, exp_list[i].cmd);
			} else {
				if (field)
					strcat(exp_list[i].cmd, "%");
				strcat(exp_list[i].cmd, ptr);
			}
			field++;
		}

		if (strlen(exp_list[i].cmd) == 0) {
			strcpy(exp_list[i].cmd, "\r");
		} else {
			ch = exp_list[i].cmd[strlen(exp_list[i].cmd) - 1];
			if (ch != '\r' && !exp_list[i].nocr) {
				if (ch == '\n')
					exp_list[i].cmd[strlen(exp_list[i].cmd) - 1] = '\0';
				strcat(exp_list[i].cmd, "\r");
			}
		}
		exp_list[i].cmd[BUFSIZ - 1] = '\0';
	}
	*p = i;
	free(buf);
}
