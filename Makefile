TOPDIR = $(realpath .)

include config.mk

env: build-buildroot build-linux build-opensbi

build-buildroot:
	git submodule update --init ${BUILDROOT_SOURCE}
	mkdir -p ${BUILDROOT_BUILD}
	cp ${TOPDIR}/config/buildroot_config ${BUILDROOT_BUILD}/.config
	${MAKE} -C ${BUILDROOT_SOURCE} O=${BUILDROOT_BUILD} -j${NPROC}

build-linux:
	git submodule update --depth 1 --init ${LINUX_SOURCE}
	mkdir -p ${LINUX_BUILD}
	cp ${TOPDIR}/config/linux_config ${LINUX_BUILD}/.config
	${MAKE} -C ${LINUX_SOURCE} O=${LINUX_BUILD} -j${NPROC}

build-opensbi:
	git submodule update --init ${OPENSBI_SOURCE}
	mkdir -p ${OPENSBI_BUILD}
	${MAKE} -C ${OPENSBI_SOURCE} O=${OPENSBI_BUILD} \
		PLATFORM=litex/vexriscv platform-cflags-y=-fno-stack-protector \
		PLATFORM_RISCV_XLEN=32 \
		PLATFORM_RISCV_ABI=ilp32 \
		PLATFORM_RISCV_ISA=rv32ima_zicsr_zifencei

build-virtio:
	truncate -s 64M ${VIRTIO_BUILD}
	mkfs.ext4 -d ${TOPDIR}/driver ${VIRTIO_BUILD}

clean-buildroot:
	rm -rf ${BUILDROOT_BUILD}

clean-linux:
	${MAKE} -C ${LINUX_SOURCE} distclean
	rm -rf ${LINUX_BUILD}

clean-opensbi:
	rm -rf ${OPENSBI_BUILD}

clean-virtio:
	rm -f ${VIRTIO_BUILD}

clean:
	rm -rf ${TOPDIR}/build

.PHONY: build-linux build-buildroot build-opensbi build-virtio env
.PHONY: clean-linux clean-buildroot clean-opensbi clean-virtio clean
