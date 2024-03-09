#ifndef _CALC_DRIVER_H
#define _CALC_DRIVER_H

#define ADD (1 << 0)
#define SUB (1 << 1)
#define MUL (1 << 2)
#define DIV (1 << 3)

#define STATUS_INV_OP   (1 << 0)
#define STATUS_DIV_ZERO (1 << 1)
#define STATUS_MASK_ALL (STATUS_INV_OP | STATUS_DIV_ZERO)

#define CALC_IOCTL_RESET        _IO('C', 0)
#define CALC_IOCTL_CHANGE_OP    _IOW('C', 1, long)
#define CALC_IOCTL_CHECK_STATUS _IOR('C', 2, long)

#endif
