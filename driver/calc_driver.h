#ifndef _CALC_DRIVER_H
#define _CALC_DRIVER_H

#define ADD 0
#define SUB 1
#define MUL 2
#define DIV 3

#define CALC_IOCTL_RESET _IO('M', 0)
#define CALC_IOCTL_CHANGE_OP _IOW('M', 1, long)

#endif
