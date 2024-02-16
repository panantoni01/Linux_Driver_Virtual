TOPDIR := $(realpath .)

include config.mk

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
		PLATFORM=litex/vexriscv platform-cflags-y=-fno-stack-protector \
		PLATFORM_RISCV_XLEN=32 \
		PLATFORM_RISCV_ABI=ilp32 \
		PLATFORM_RISCV_ISA=rv32ima_zicsr_zifencei

clean-dts:
	rm -rf ${DTS_BUILD}

clean-buildroot:
	rm -rf ${BUILDROOT_BUILD}

clean-linux:
	make -C ${LINUX_SOURCE} distclean
	rm -rf ${LINUX_BUILD}

clean-opensbi:
	rm -rf ${OPENSBI_BUILD}

clean:
	rm -rf ${BUILDDIR}

.PHONY: build-dts build-linux build-buildroot build-opensbi
