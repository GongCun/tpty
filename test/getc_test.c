#include "tpty.h"

int main(void)
{
	int ch = fgetc(stdin);
	unsigned char c = -1;
	printf("ch = %c\n", ch);
	printf("c = %d\n", c);
	c = -2;
	printf("c = %d\n", c);
	return 0;
}
