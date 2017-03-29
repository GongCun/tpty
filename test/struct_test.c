#include "tpty.h"

#define EXP_FULL 64
#define DELIM "%\t\n"

static struct f {
	char            prompt[BUFSIZ];
	char            cmd[BUFSIZ];
}               exp_list[EXP_FULL];

static void
fp2f(FILE * fp, int *p)
{
	char           *buf, *ptr, ch;
	int             i;

	buf = malloc((unsigned) (2 * BUFSIZ));
	if (buf == NULL)
		err_sys("malloc failed");
	for (i = 0;; i++) {
		if (fgets(buf, 2 * BUFSIZ, fp) == NULL) {
			if (feof(fp))
				break;
			if (ferror(fp))
				err_sys("fgets error");
		}
		if (strtok(buf, DELIM) == NULL)
			err_quit("format error: line %d.", i + 1);
		if (strlen(buf) > BUFSIZ - 1)
			err_quit("prompt string out of buffer: line %d.", i + 1);
		strncpy(exp_list[i].prompt, buf, BUFSIZ - 1);
		exp_list[i].prompt[BUFSIZ - 1] = '\0';

		ptr = strtok(NULL, DELIM);
		if (ptr == NULL) {
			strcpy(exp_list[i].cmd, "\r");
		} else {
			if (strlen(ptr) > BUFSIZ - 2)
				err_quit("cmd string out of buffer: line %d.", i + 1);
			strncpy(exp_list[i].cmd, ptr, BUFSIZ - 2);
			ch = exp_list[i].cmd[strlen(exp_list[i].cmd) - 1];
			if (ch != '\r') {
				if (ch == '\n')
					exp_list[i].cmd[strlen(exp_list[i].cmd) - 1] = '\0';
				strncat(exp_list[i].cmd, "\r", 1);
			}
		}
		exp_list[i].cmd[BUFSIZ - 1] = '\0';
	}
	*p = i;
	free(buf);
}



int
main(void)
{
	FILE           *fp;
	int             i, n;

	fp = fopen("./test.conf", "r");
	if (fp == NULL)
		err_sys("fopen error");
	fp2f(fp, &n);
	for (i = 0; i < n; i++) {
		printf("prompt = %s; cmd = %s\n",
		       exp_list[i].prompt, exp_list[i].cmd);
	}
}
