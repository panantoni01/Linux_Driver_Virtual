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

