#include "tpty.h"

int main(int argc, char *argv[])
{
	char *buf, *ptr;

	buf = argv[1];
	if (strtok(buf, DELIM) == NULL)
		err_quit("format error");
	printf("buf = %s\n", buf);
	ptr = strtok(NULL, DELIM);
	printf("ptr = %s\n", ptr);

	return(0);
}
