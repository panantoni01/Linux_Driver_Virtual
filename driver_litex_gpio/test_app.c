#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "litex_gpio_driver.h"

static void count_until(int gpio_dev_fd, unsigned int limit)
{
	unsigned int current = 0;

	while (current < limit) {
		read(gpio_dev_fd, &current, sizeof(current));
		printf("Interrupt has been caught!\n");
	}
}

static int is_chardev(const char *filename)
{
	struct stat file_stat;

	stat(filename, &file_stat);
	return S_ISCHR(file_stat.st_mode);
}

int main(int argc, const char *argv[])
{
	int fd;
	unsigned int limit = 7;

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

	while (1) {
		count_until(fd, limit);
		printf("Counter reached %d, resetting...\n", limit);
		ioctl(fd, GPIO_IOCTL_RESET);
	}

	return 0;
}
