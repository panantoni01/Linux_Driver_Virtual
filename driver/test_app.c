#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<assert.h>

#include"calc_driver.h"


static int is_chardev(const char* filename) {
    struct stat file_stat;

    stat(filename, &file_stat);
    return S_ISCHR(file_stat.st_mode);
} 

int main(int argc, const char* argv[]) {
    int fd, res;
    long var, num_to_send;

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

    res = read(fd, &var, sizeof(long));
    printf("Var initially: %ld\n", var);

    num_to_send = 15;
    res = write(fd, &num_to_send, sizeof(long));
    assert(res > 0);
    num_to_send = 34;
    res = write(fd, &num_to_send, sizeof(long));
    assert(res > 0);

    res = read(fd, &var, sizeof(long));
    assert(res >= 0);
    printf("15 + 34 = %ld\n", var);

    res = ioctl(fd, CALC_IOCTL_CHANGE_OP, MUL);
    assert(res >= 0);

    num_to_send = 49;
    res = write(fd, &num_to_send, sizeof(long));
    assert(res > 0);

    res = read(fd, &var, sizeof(long));
    assert(res >= 0);
    printf("49 * 49 = %ld\n", var);
    
    close(fd);
    return 0;
}
