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

export ARCH='riscv'
export CROSS_COMPILE=${CROSS_COMPILE}
export PATH=$(shell echo "${BUILDROOT_BUILD}/host/bin/:$$PATH")