#include "tpty.h"

int main(void)
{
    FILE *fp;

    fp = fopen("test.txt", "r");
    if (fp == NULL)
        return -1;
    
    char prompt[BUFSIZ], cmd[BUFSIZ], nl;

    if (fscanf(fp, "%s %% %s%c\n", prompt, cmd, &nl) != 3
            || nl != '\n') {
    printf("prompt = %s\n", prompt);
    printf("cmd = %s\n", cmd);
        if (feof(fp))
            printf("end\n");
        if (ferror(fp))
            err_sys("fscanf error");
        err_quit("format error");
    }

    printf("prompt = %s\n", prompt);
    printf("cmd = %s\n", cmd);

    return 0;
}
