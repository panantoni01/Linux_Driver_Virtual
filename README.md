# Linux Device Driver

This repository contains source code for a Linux device driver of a virtual peripheral (`scripts/calc_periph.py`) on RISC-V processor that is emulated in [Renode](https://github.com/renode/renode) framework. All the used materials are based on the contents of Linux Device Drivers course at the University of Wroclaw (a.y. 2021-2022).

## Prerequisites

### Renode
According to the [Renode's README](https://github.com/renode/renode/blob/master/README.rst#installation) the following commands can be executed to download the latest Renode version:
```
wget https://dl.antmicro.com/projects/renode/builds/renode-latest.linux-portable.tar.gz
mkdir renode_portable
tar xf  renode-latest.linux-portable.tar.gz -C renode_portable --strip-components=1
```

## Environment setup
* `make build-buildroot` - build `rootfs.cpio` initrd image. A 32-bit RISC-V toolchain is also built which can be later used to build the linux kernel, opensbi and the driver itself if there is no RISC-V toolchain available on the system. In order to use the buildroot toolchain, the PATH variable needs to be modified:
```
export PATH=${REPO_PATH}/build/buildroot/host/bin:${PATH}
```

* `make build-linux` - build a Linux kernel for RISC-V architecture.

* `make build-opensbi` - build OpenSBI bootloader.

## Driver description

The driver controls a simple arithmetic peripheral, that supports +,-,*,/ operations. It is simulated by in a virtual environment using Renode. The driver provides the following functionalities:

* IOCTL for selecting the operation
* IOCTL for checking status of the device
* IOCTL for resetting error code
* data is provided to the driver by writing the /dev file
* results are fetched by reading the /dev file

Example flow:
```
write <-- 2
write <-- 10
ioctl operation_add
ioctl check_status
read -> 12
```
Example error handling flow:
```
write <-- 2
write <-- 0
ioctl operation_div
ioctl check_status
ioctl error_ack
print error
```

* ```my_driver.c``` - main driver code
* ```my_driver.h``` - separate header file with defines for ioctls
* ```mydriver_test.c``` -  example userspace program to test the driver functionality
* ```rv32.dts``` - device tree file - contains hardware description (including the peripheral).
