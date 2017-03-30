#include "tpty.h"

int main(void)
{

char buf[20];

bzero(buf, sizeof(buf));
strcat(buf, "x");

printf("buf = %s\n", buf);
exit(0);
}
