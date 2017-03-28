#include "tpty.h"

int main(void)
{
    FILE *fp = fopen("./encrypt.txt", "r");
    char private_key[] = "./private.pem";
    char encrypted[4098];
    char decrypted[4098];
    int len;

    len = fread(encrypted, 1, sizeof(encrypted), fp);

    /* decrypt */

    int decrypted_length = private_decrypt(encrypted,
            len,
            private_key,
            decrypted);
    printf("Decrypted Text = %s\n",decrypted);
    printf("Length = %d\n", decrypted_length);

    return 0;
}
