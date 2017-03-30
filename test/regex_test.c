#include "tpty.h"

int
main(void)
{

	int             ret;
	char            str1[] = "345";
	char            str2[] = "0123456789";
	char *ch;

	/* if ((ret = strregex(str1, str2)) > 0) */
	ch = strstr(str2, str1);
	if (ch != NULL)
		printf("str ed = %d\n", ch - str2 + strlen(str1));
		printf("ed = %d\n", strregex(str1, str2));
	exit(0);
}
