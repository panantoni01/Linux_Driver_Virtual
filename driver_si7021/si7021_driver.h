#ifndef _SI7021_H
#define _SI7021_H

#define SI7021_IOCTL_RESET _IO('S', 0)
#define SI7021_IOCTL_READ_ID _IOR('S', 1, long long)
#define SI7021_IOCTL_SET_USER_REG _IOW('S', 2, char)
#define SI7021_IOCTL_GET_USER_REG _IOR('S', 3, char)

struct si7021_result {
    short temp;
    unsigned short rl_hum;
};

#define SI7021_USER_REG_BIT_HEATER 2

#define SI7021_HEATER_ON(user_reg)  (user_reg |= (1 << SI7021_USER_REG_BIT_HEATER))
#define SI7021_HEATER_OFF(user_reg) (user_reg &= ~(1 << SI7021_USER_REG_BIT_HEATER))

#endif /* _SI7021_H */
