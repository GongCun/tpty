/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015,2016,2017 Cun Gong <gong_cun@bocmacau.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include "tpty.h"

/*
 * tpty-crypt <File> <Keyword> <PubKey> <SaveFile>
 * Find the record of the keyword, encrypt the corresponding content and save
 * it to the savefile.
 */
int 
main(int argc, char *argv[])
{
	int             i, ret;
	char            buf[2 * BUFSIZ], *ch;
	char            encrypt[4098];
	FILE           *fp, *tfp, *sfp;

	if (argc != 5)
		err_quit("usage: %s FileName SegmentName PublicKey StoreFile", *argv);

	if ((fp = fopen(argv[1], "r+")) == NULL)
		err_sys("fopen error");
	if ((sfp = fopen(argv[4], "w")) == NULL)
		err_sys("fopen error");
	/* store the temporary file that replaced the password to <ENCRYPT> */
	if ((tfp = tmpfile()) == NULL)
		err_sys("tmpfile error");

	for (i = 0;; i++) {
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
		if (ch != NULL && strstr(buf, "<ENCRYPT>") == NULL) { /* to avoid encrypt the password
									 which have been encrypted */
			ch = strchr(buf, '%');
			if (ch == NULL)
				err_quit("config format error: not have field seprator %%.");
			ch++;
			if (strlen(ch) == 0)
				err_quit("config format error: the corresponding content is null.");
			if (ch[strlen(ch) - 1] == '\n')
				ch[strlen(ch) - 1] = '\0';
			ret = public_encrypt(ch, strlen(ch) + 1, argv[3], encrypt);
			if (ret < 0)
				err_quit("public_encrypt() error");
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

	/* mv tempfile to config file */
	if (fseek(fp, 0L, SEEK_SET) < 0)
		err_sys("fseek error");
	if (ftruncate(fileno(fp), 0) < 0)
		err_sys("ftruncate error");
	if (fseek(tfp, 0L, SEEK_SET) < 0)
		err_sys("fseek error");

	while (fgets(buf, sizeof buf, tfp) != NULL)
		if (fputs(buf, fp) == EOF)
			err_sys("fputs error");
	if (ferror(tfp))
		err_sys("fgets error");

	return 0;
}
