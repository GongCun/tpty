#include "tpty.h"

int main(int argc, char *argv[])
{
    FILE *fp = fopen(argv[1], "w");
    char *plain_text = argv[2];
    char *public_key = argv[3];
    char encrypted[4098];

    int encrypted_length= public_encrypt(plain_text,
            strlen(plain_text) + 1,
            public_key,
            encrypted);
    printf("Length = %d\n", encrypted_length);
    fwrite(encrypted, 1, encrypted_length, fp);
    return 0;
}

