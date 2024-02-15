TOPDIR := $(realpath .)

NPROC := $(shell nproc)

CROSS_COMPILE ?= riscv32-linux-

BUILDDIR  ?= ${TOPDIR}/build
DRIVERDIR ?= ${TOPDIR}/driver

DTS_SOURCE       ?= ${DRIVERDIR}/rv32.dts
BUILDROOT_SOURCE ?= ${TOPDIR}/third_party/buildroot
LINUX_SOURCE     ?= ${TOPDIR}/third_party/linux
OPENSBI_SOURCE   ?= ${TOPDIR}/third_party/opensbi

DTS_BUILD       ?= ${BUILDDIR}/rv32.dtb
BUILDROOT_BUILD ?= ${BUILDDIR}/buildroot
LINUX_BUILD     ?= ${BUILDDIR}/linux
OPENSBI_BUILD   ?= ${BUILDDIR}/opensbi


build-dts:
	dtc -I dts -O dtb -o ${DTS_BUILD} ${DTS_SOURCE}

build-buildroot:
	git submodule update --init ${BUILDROOT_SOURCE}
	mkdir -p ${BUILDROOT_BUILD}
	cp ${TOPDIR}/config/buildroot_config ${BUILDROOT_BUILD}/.config
	make -C ${BUILDROOT_SOURCE} O=${BUILDROOT_BUILD} -j${NPROC}

build-linux:
	git submodule update --init ${LINUX_SOURCE}
	mkdir -p ${LINUX_BUILD}
	cp ${TOPDIR}/config/linux_config ${LINUX_BUILD}/.config
	make -C ${LINUX_SOURCE} O=${LINUX_BUILD} -j${NPROC}

build-opensbi:
	git submodule update --init ${OPENSBI_SOURCE}
	mkdir -p ${OPENSBI_BUILD}
	make -C ${OPENSBI_SOURCE} O=${OPENSBI_BUILD} \
		CROSS_COMPILE=${CROSS_COMPILE} \
		PLATFORM=litex/vexriscv platform-cflags-y=-fno-stack-protector \
		PLATFORM_RISCV_XLEN=32 \
		PLATFORM_RISCV_ABI=ilp32 \
		PLATFORM_RISCV_ISA=rv32ima_zicsr_zifencei

clean-dts:
	rm -rf ${DTS_BUILD}

clean-buildroot:
	rm -rf ${BUILDROOT_BUILD}

clean-linux:
	rm -rf ${LINUX_BUILD}

clean-opensbi:
	rm -rf ${OPENSBI_BUILD}

clean:
	rm -rf ${BUILDDIR}

.PHONY: build-dts build-linux build-buildroot build-opensbi
