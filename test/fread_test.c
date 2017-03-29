#include "tpty.h"

int main(int argc, char *argv[])
{
    FILE *fp = fopen(argv[1], "r");
    int ret;
    fread(&ret, sizeof ret, 1, fp);
    printf("ret = %d\n", ret);
    return 0;
}
