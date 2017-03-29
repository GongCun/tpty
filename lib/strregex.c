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

#define CFLAGS (REG_EXTENDED | REG_ICASE)

/*
 * Return:
 * 	< 0	: Error
 *	0	: No match 
 *	> 0	: The offset of the end of the entire match substring + 1
 */

int
strregex(char *str1, char *str2)
{
	regex_t         regex;
	regmatch_t      pmatch[1];
	int             ret;

	ret = regcomp(&regex, str1, CFLAGS);
	if (ret)
		return -1;

	ret = regexec(&regex, str2, 1, pmatch, 0);
	regfree(&regex);
	if (!ret) {
		ret = pmatch[0].rm_eo;
		return ret;
	} else if (ret == REG_NOMATCH) {
		return 0;
	}
	return -2;
}
