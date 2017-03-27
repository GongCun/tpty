#include "tpty.h"

int	padding = RSA_PKCS1_PADDING;

RSA *
create_rsa_with_filename(char *filename, int public)
{
	FILE *fp = fopen(filename, "r");

	if (fp == NULL)
		err_sys("Unable to open file %s", filename);

	RSA *rsa = RSA_new();

    if (rsa != NULL) {
	    if (public) {
	    	rsa = PEM_read_RSA_PUBKEY(fp, &rsa, NULL, NULL);
	    } else {
	    	rsa = PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
	    }
    }

	return rsa;
}

int
public_encrypt(char *data, int data_len, char *key, char *encrypted)
{
    int ret;
    RSA *rsa;

	rsa = create_rsa_with_filename(key, 1);
    if (rsa == NULL)
        err_ssl();

#define PKCS1_PADDING_REM 11
    if (data_len >= RSA_size(rsa) - PKCS1_PADDING_REM)
        err_quit("Encrypt buffer length (%d) out of rsa size.", data_len);

	ret = RSA_public_encrypt(data_len,
            (unsigned char *)data,
            (unsigned char *)encrypted, rsa, padding);
	return ret;
}

int
private_decrypt(char *enc_data, int data_len, char *key, char *decrypted)
{
    int ret;
    RSA *rsa;
	rsa = create_rsa_with_filename(key, 0);
    if (rsa == NULL)
        err_ssl();

	ret = RSA_private_decrypt(data_len,
            (unsigned char *)enc_data,
            (unsigned char *)decrypted, rsa, padding);
	return ret;
}

void err_ssl(void)
{
    char err[121]; /* at least 120 bytes long (man ERR_error_string) */

    ERR_load_crypto_strings();
    ERR_error_string_n(ERR_get_error(), err, 130);
    err_quit("%s", err);

}
