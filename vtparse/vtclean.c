/*
 * VTParse - an implementation of Paul Williams' DEC compatible state machine
 * parser
 * 
 * Author: Joshua Haberman <joshua@reverberate.org>
 * 
 * This code is in the public domain.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "vtparse.h"

#define MAXLINE 8192

void 
parser_callback(vtparse_t * parser, vtparse_action_t action, unsigned char ch)
{
	unsigned char buf[MAXLINE];
	static int i = 0;

	if (!strncmp(ACTION_NAMES[action], "PRINT", strlen("PRINT"))) {
		buf[i++] = ch;
		if (i == MAXLINE) {
			i = 0;
			write(1, buf, MAXLINE);
		}
	}
	if (!strncmp(ACTION_NAMES[action], "EXECUTE", strlen("EXECUTE"))) {
		if (ch == 0x0a) {
			buf[i++] = '\n';
			write(1, buf, i);
			i = 0;
		}
	}
}

int 
main()
{
	unsigned char	buf[MAXLINE];
	int		bytes;
	vtparse_t	parser;

	vtparse_init(&parser, parser_callback);

	while ((bytes = read(0, buf, sizeof(buf))) > 0) {
		vtparse(&parser, buf, bytes);
	}

	return(0);
}
