/*
 * The MIT License (MIT) 
 *
 * Copyright (c) 2015,2016,2017 Cun Gong <gong_cun@bocmacau.com> 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE. 
 *
 */
#include "tpty.h"

/*
 * Encrypt and decrypt using RSA_PKCS1_PADDING (PKCS #1 v1.5 padding).
 */

static void 
err_ssl(void);

RSA            *
create_rsa_with_filename(char *filename, int public)
{
	FILE           *fp = fopen(filename, "r");
	RSA            *rsa;

	if (fp == NULL)
		err_sys("Unable to open file %s", filename);

	if ((rsa = RSA_new()) == NULL)
		err_ssl();

	if (public) {
		rsa = PEM_read_RSA_PUBKEY(fp, &rsa, NULL, NULL);
	} else {
		rsa = PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL);
	}
	return rsa;
}

int
public_encrypt(char *data, int data_len, char *key, char *encrypted)
{
	int             ret;
	RSA            *rsa;

	rsa = create_rsa_with_filename(key, 1);
	if (rsa == NULL)
		err_ssl();

	if (data_len >= RSA_size(rsa) - RSA_PKCS1_PADDING_SIZE)
		err_quit("Encrypt buffer length (%d) out of rsa size.", data_len);

	ret = RSA_public_encrypt(data_len,
				 (unsigned char *) data,
				 (unsigned char *) encrypted, rsa, RSA_PKCS1_PADDING);
	return ret;
}

int
private_decrypt(char *enc_data, int data_len, char *key, char *decrypted)
{
	int             ret;
	RSA            *rsa;
	rsa = create_rsa_with_filename(key, 0);
	if (rsa == NULL)
		err_ssl();

	ret = RSA_private_decrypt(data_len,
				  (unsigned char *) enc_data,
				  (unsigned char *) decrypted, rsa, RSA_PKCS1_PADDING);
	return ret;
}

static void
err_ssl(void)
{
	unsigned long   err;
	ERR_load_crypto_strings();
	while ((err = ERR_get_error()) != 0)
		err_msg("%s", ERR_error_string(err, NULL));
	exit(1);
}
