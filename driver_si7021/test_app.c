#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "si7021_driver.h"

static int is_chardev(const char *filename)
{
	struct stat file_stat;

	stat(filename, &file_stat);
	return S_ISCHR(file_stat.st_mode);
}

static void get_measurement(int fd, struct si7021_result *result)
{
	if (read(fd, result, sizeof(*result)) < 0) {
		fprintf(stderr, "si7021: read error\n");
		exit(1);
	}
}

static void test_read(int fd)
{
	struct si7021_result result;

	get_measurement(fd, &result);

	assert(result.rl_hum >= 0 && result.rl_hum <= 100);
	assert(result.temp >= -40 && result.temp <= 125);

	printf("temperature: %d\n", result.temp);
	printf("rl_humidity: %d\n", result.rl_hum);
}

static void get_user_reg(int fd, char *user_reg)
{
	if (ioctl(fd, SI7021_IOCTL_GET_USER_REG, user_reg) < 0) {
		fprintf(stderr, "si7021: get_user_reg ioctl error\n");
		exit(1);
	}
}

static void set_user_reg(int fd, char user_reg)
{
	if (ioctl(fd, SI7021_IOCTL_SET_USER_REG, user_reg) < 0) {
		fprintf(stderr, "si7021: set_user_reg ioctl error\n");
		exit(1);
	}
}

static void get_heater_reg(int fd, char *heater_reg)
{
	if (ioctl(fd, SI7021_IOCTL_GET_HEATER_REG, heater_reg) < 0) {
		fprintf(stderr, "si7021: get_heat_reg ioctl error\n");
		exit(1);
	}
}

static void set_heater_reg(int fd, char heater_reg)
{
	if (ioctl(fd, SI7021_IOCTL_SET_HEATER_REG, heater_reg) < 0) {
		fprintf(stderr, "si7021: set_heat_reg ioctl error\n");
		exit(1);
	}
}

static void test_user_reg(int fd)
{
	char user_reg = 0;

	printf("%s running...\n", __func__);

	get_user_reg(fd, &user_reg);
	assert(user_reg == 0x3A);

	SI7021_HEATER_ON(user_reg);
	set_user_reg(fd, user_reg);

	get_user_reg(fd, &user_reg);
	assert(user_reg == 0x3E);

	SI7021_HEATER_OFF(user_reg);
	set_user_reg(fd, user_reg);

	get_user_reg(fd, &user_reg);
	assert(user_reg == 0x3A);

	printf("%s succeeded!\n", __func__);
}

static void test_heater_reg(int fd)
{
	char user_reg = 0;
	char heater_reg = 0;
	struct si7021_result result;

	printf("%s running...\n", __func__);

	get_user_reg(fd, &user_reg);
	SI7021_HEATER_ON(user_reg);
	set_user_reg(fd, user_reg);

	get_heater_reg(fd, &heater_reg);
	assert(heater_reg == 0);

	heater_reg = 0x0F;
	set_heater_reg(fd, heater_reg);

	get_heater_reg(fd, &heater_reg);
	assert(heater_reg == 0x0F);

	/* Once the heater is on, the measurements should be different */
	get_measurement(fd, &result);
	printf("temperature: %d\n", result.temp);
	printf("rl_humidity: %d\n", result.rl_hum);

	get_user_reg(fd, &user_reg);
	SI7021_HEATER_OFF(user_reg);
	set_user_reg(fd, user_reg);

	heater_reg = 0;
	set_heater_reg(fd, heater_reg);

	printf("%s succeeded!\n", __func__);
}

int main(int argc, const char *argv[])
{
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
	printf("serial id: 0x%llx\n", serial_id);

	test_read(fd);
	test_user_reg(fd);
	test_heater_reg(fd);

	close(fd);

	return 0;
}
