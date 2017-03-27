#include "tpty.h"
#include <ctype.h>

void
strip(char buf[], char *str)
{
	int             i = 0;
	char           *ptr = str;

	while (buf[i] != '\0') {
		if (buf[i] == 10) {	/* LF */
			*(ptr++) = '\n';
		} else if (!iscntrl(buf[i])) {
			*(ptr++) = buf[i];
		}
		i++;
	}
	*ptr = '\0';
}
