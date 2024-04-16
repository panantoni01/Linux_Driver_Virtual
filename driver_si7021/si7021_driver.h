#ifndef _SI7021_H
#define _SI7021_H

#define SI7021_IOCTL_RESET _IO('S', 0)
#define SI7021_IOCTL_READ_ID _IOR('S', 1, long long)

struct si7021_result {
    short temp;
    unsigned short rl_hum;
};

#endif /* _SI7021_H */