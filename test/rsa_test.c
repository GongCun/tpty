#include "tpty.h"

int main(void)
{
    FILE *fp = fopen("./encrypt.txt", "w");
    char plain_text[256] = "1827gcGC";
    char public_key[] = "./public.pem";
    char encrypted[4098];

    int encrypted_length= public_encrypt(plain_text,
            strlen(plain_text) + 1,
            public_key,
            encrypted);
    if (encrypted_length < 0)
        err_ssl();
    printf("Encrypted: %s\n", encrypted);
    printf("Length = %d\n", encrypted_length);
    /* encrypted[encrypted_length] = '\0'; */
    fwrite(encrypted, 1, encrypted_length, fp);
    return 0;
}

