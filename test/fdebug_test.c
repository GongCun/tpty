#include "tpty.h"

int main(void)
{
char buf[] = "a test";
FILE *fp = fopen("./test.txt", "w");
fdebug(fp, "%s\n", buf);
exit(0);
}
