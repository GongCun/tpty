#include "tpty.h"

int main(int argc, char *argv[])
{
    int i, ret;
    char buf[2 * BUFSIZ], *ch;
    char encrypt[4098];
    FILE *fp, *tfp, *sfp;

    if (argc != 5)
        err_quit("usage: %s FileName SegmentName PublicKey StoreFile", *argv);

    if ((fp = fopen(argv[1], "r+")) == NULL)
        err_sys("fopen error");
    if ((sfp = fopen(argv[4], "a")) == NULL)
        err_sys("fopen error");
    if ((tfp = tmpfile()) == NULL)
        err_sys("tmpfile error");

    for (i = 0; ;i++) {
        if (i >= EXP_FULL)
            err_quit("lines out of scope");
        if (fgets(buf, sizeof buf, fp) == NULL) {
            if (feof(fp))
                break;
            if (ferror(fp))
                err_sys("fgets error");
        }
        buf[sizeof(buf) - 1] = '\0';
        ch = strstr(buf, argv[2]);
        if (ch != NULL && strstr(buf, "<ENCRYPT>") == NULL) {
            ch = strchr(buf, '%');
            if (ch == NULL)
                err_quit("config format error");
            ch++;
            if (strlen(ch) == 0)
                err_quit("config format error");
            if (ch[strlen(ch)-1] == '\n')
                ch[strlen(ch)-1] = '\0';
            ret = public_encrypt(ch, strlen(ch)+1, argv[3], encrypt);
            if (ret < 0)
                err_ssl();
            if (!fwrite(&ret, sizeof ret, 1, sfp))
                err_sys("fwrite error");
            if (!fwrite(encrypt, 1, ret, sfp))
                err_sys("fwrite error");
            fprintf(tfp, "%s%%<ENCRYPT>\n", argv[2]);
        } else {
            if (!fwrite(buf, 1, strlen(buf), tfp))
                err_sys("fwrite error");
        }
    }

    if (fseek(fp, 0L, SEEK_SET) < 0)
        err_sys("fseek error");
    if (ftruncate(fileno(fp), 0) < 0)
        err_sys("ftruncate error");
    if (fseek(tfp, 0L, SEEK_SET) < 0)
        err_sys("fseek error");

    while(fgets(buf, sizeof buf, tfp) != NULL)
        if (fputs(buf, fp) == EOF)
            err_sys("fputs error");
    if (ferror(tfp))
        err_sys("fgets error");

    return 0;
}

