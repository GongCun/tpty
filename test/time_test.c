#include "tpty.h"

int main(void)
{
	time_t tp;
	if (time(&tp) == (time_t)-1)
		err_sys("time() error");
	printf("%s", ctime(&tp));
	/* printf("%s", asctime(gmtime(&tp))); */
	return 0;
}

