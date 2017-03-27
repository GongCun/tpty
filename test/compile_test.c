#include <stdio.h>
#include <stdlib.h>

char (*f)() = unlockpt;

int main(void)
{
#ifdef _X_
	/* nothing to do */
#else
#error "exit"
#endif

	printf("hello world\n");
	return(f != unlockpt);

}
