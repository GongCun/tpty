#include <stdio.h>
#include <stdlib.h>

int main(void)
{
#ifdef _X_
	/* nothing to do */
#else
#error "exit"
#endif

	printf("hello world\n");
	return(0);
}
