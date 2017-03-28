#include <stdio.h>
#include <stdlib.h>

typedef char func(void);

func *f = (func *)unlockpt;

/* char (*f)() = (char (*)(void)) unlockpt; */

int main(void)
{
#if 0
#ifdef _X_
	/* nothing to do */
#else
#error "exit"
#endif
#endif

	int i, j;
	({i=1; j=2;});
	printf("i = %d, j = %d\n", i, j);
	printf("hello world\n");
	return(f != (func *)unlockpt);

}
