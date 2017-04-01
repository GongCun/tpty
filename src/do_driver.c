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
pid_t		child;

void do_driver (int (*f_driver)(void))
{
	int		pipe[2];

	/*
	 * create a stream pipe to communication with the driver.
	 */
	if (s_pipe(pipe) < 0)
		err_sys("can't create stream pipe");

	if ((child = fork()) < 0) {
		err_sys("fork error");
	} else if (child == 0) {/* child */
		int		ret;
		close(pipe[1]);

		/* stdin for driver */
		if (dup2(pipe[0], STDIN_FILENO) != STDIN_FILENO)
			err_sys("dup2 error to stdin");

		/* stdout for driver */
		if (dup2(pipe[0], STDOUT_FILENO) != STDOUT_FILENO)
			err_sys("dup2 error to stdout");
		if (pipe[0] != STDIN_FILENO && pipe[1] != STDOUT_FILENO)
			close(pipe[0]);

		/* leave stderr for driver alone */
		if (f_driver == (int (*)(void))0) {
			if (driver)
				execlp(driver, driver, (char *)0);
			else
				errno = EINVAL;
			err_sys("execlp error for: %s", driver);
		} else {
			ret = (*f_driver) ();
			exit(ret);
		}
	}
	/*
 	 * Parent process continue...
	 */
	if (verbose)
		fprintf(stderr, "do_driver() fork child process %ld\n", (long)child);

	close(pipe[0]);		/* parent */
	/* leave stderr for parent alone */
	if (dup2(pipe[1], STDIN_FILENO) != STDIN_FILENO)
		err_sys("dup2 error to stdin");
	if (dup2(pipe[1], STDOUT_FILENO) != STDOUT_FILENO)
		err_sys("dup2 error to stdout");
	if (pipe[1] != STDIN_FILENO && pipe[1] != STDOUT_FILENO)
		close(pipe[1]);

	/*
	 * parent returns, but with stdin and stdout connected to the driver.
	 */
}
