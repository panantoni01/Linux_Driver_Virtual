#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "si7021_driver.h"


static int is_chardev(const char* filename) {
    struct stat file_stat;

    stat(filename, &file_stat);
    return S_ISCHR(file_stat.st_mode);
}

int main(int argc, const char* argv[]) {
    int fd;
    long long serial_id;

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

    if (ioctl(fd, SI7021_IOCTL_RESET) < 0) {
        fprintf(stderr, "si7021: ioctl reset error\n");
        exit(1);
    }

    if (ioctl(fd, SI7021_IOCTL_READ_ID, &serial_id) < 0) {
        fprintf(stderr, "si7021: ioctl read_id error\n");
        exit(1);
    }
    else
        printf("serial id: %lld\n", serial_id);

    return 0;
}
