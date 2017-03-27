#include "tpty.h"

int main(void)
{
	int ch = 'a';
	unsigned char c = 'x';
	int t = c;

	printf("int = %d\n", ch);
	printf("ch = %c\n", (unsigned char)ch);
	printf("c = %c\n", c);
	printf("t = %d\n", t);
	exit(0);
}
