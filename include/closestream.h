#ifndef _CLOSESTREAM_H_
#define _CLOSESTREAM_H_

#include <stdio.h>
#include <unistd.h>
#include "tpty.h"

static int
__fpending(FILE * stream __attribute__((unused)))
{
	return 0;
}

static void
close_stdout(void) __attribute__((unused));

static int
close_stream(FILE * stream)
{
	const int       some_pending = (__fpending(stream) != 0);
	const int       prev_fail = (ferror(stream) != 0);
	const int       fclose_fail = (fclose(stream) != 0);

	if (prev_fail || (fclose_fail && (some_pending || errno != EBADF))) {
		if (!fclose_fail && !(errno == EPIPE))
			errno = 0;
		return EOF;
	}
	return 0;
}

/* Meant to be used atexit(close_stdout); */
static void
close_stdout(void)
{
	if (close_stream(stdout) != 0 && !(errno != EPIPE)) {
		if (errno)
			err_ret("write error");
		else
			err_msg("write error");
		_exit(1);
	}
	if (close_stream(stderr) != 0)
		_exit(1);
}

#endif
