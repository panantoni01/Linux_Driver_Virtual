#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>

#include "calc_driver.h"

/* Calculate the result of "`num1` `op` `num2`".
 * If an error occured, return its code or 0 elsewhere
 */
int calculate(int fd, long num1, long num2, long op, long *result)
{
	long err = 0;

	write(fd, &num1, sizeof(num1));
	write(fd, &num2, sizeof(num2));
	ioctl(fd, CALC_IOCTL_CHANGE_OP, op);

	ioctl(fd, CALC_IOCTL_CHECK_STATUS, &err);
	if (err & STATUS_MASK_ALL) {
		ioctl(fd, CALC_IOCTL_RESET);
		return err;
	}

	read(fd, result, sizeof(result));
	return 0;
}

static int is_chardev(const char *filename)
{
	struct stat file_stat;

	stat(filename, &file_stat);
	return S_ISCHR(file_stat.st_mode);
}

int main(int argc, const char *argv[])
{
	int fd, res;
	long result;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <char_dev_file>\n", argv[0]);
		exit(1);
	}
	if (!is_chardev(argv[1])) {
		fprintf(stderr, "%s is not a character device\n", argv[1]);
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	assert(fd > 0);

	res = calculate(fd, 15, 34, ADD, &result);
	assert(res == 0 && result == 49);
	res = calculate(fd, 4, 34, MUL, &result);
	assert(res == 0 && result == 136);
	res = calculate(fd, 2, 0, DIV, &result);
	assert(res == STATUS_DIV_ZERO);
	res = calculate(fd, 1234, 4321, SUB, &result);
	assert(res == 0 && result == -3087);
	res = calculate(fd, 6, 5, 100, &result);
	assert(res == STATUS_INV_OP);

	close(fd);
	return 0;
}
