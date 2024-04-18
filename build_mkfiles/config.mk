NPROC := $(shell nproc)

CROSS_COMP ?= riscv32-linux-

ifeq (${GITHUB_ACTIONS}, true)
LINUX_SOURCE    = /root/linux

BUILDROOT_BUILD = /root/build/buildroot
LINUX_BUILD     = /root/build/linux
else
BUILDROOT_SOURCE ?= ${TOPDIR}/third_party/buildroot
LINUX_SOURCE     ?= ${TOPDIR}/third_party/linux
OPENSBI_SOURCE   ?= ${TOPDIR}/third_party/opensbi
RENODE_SOURCE    ?= ${TOPDIR}/third_party/renode

BUILDROOT_BUILD ?= ${TOPDIR}/build/buildroot
LINUX_BUILD     ?= ${TOPDIR}/build/linux
OPENSBI_BUILD   ?= ${TOPDIR}/build/opensbi
RENODE_BUILD    ?= ${TOPDIR}/build/renode
endif

export ARCH=riscv
export CROSS_COMPILE=${CROSS_COMP}
export PATH=$(shell echo "${BUILDROOT_BUILD}/host/bin/:$$PATH")