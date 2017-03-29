#include "tpty.h"

int main(void)
{
    char str[BUFSIZ];
    printf("strlen = %zd\n", strlen(str));

    strncpy(str, "abcd1234", BUFSIZ);
    str[BUFSIZ - 1] = 0;
    printf("strlen = %zd\n", strlen(str));

    memset(str, 0, BUFSIZ);
    printf("strlen = %zd\n", strlen(str));

    return 0;
}
