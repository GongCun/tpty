#include <stdio.h>
#include <stdlib.h>

typedef char func(void);

func *f = (func *)unlockpt;

/* char (*f)() = (char (*)(void)) unlockpt; */

int main(void)
{
#ifdef _X_
	/* nothing to do */
#else
#error "exit"
#endif

	printf("hello world\n");
	return(f != (func *)unlockpt);

}
