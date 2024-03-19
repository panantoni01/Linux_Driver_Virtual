TOPDIR := $(realpath ..)

include ${TOPDIR}/config.mk

BUILD_DIR ?= $(PWD)/build
BUILD_DIR_MAKEFILE ?= $(BUILD_DIR)/Makefile
VIRTIO_BUILD ?= $(PWD)/drive.img

build: dtb modules

dtb: $(BUILD_DIR)/rv32.dtb
modules: $(BUILD_DIR)/litex_gpio_driver.ko

$(BUILD_DIR)/%.dtb: %.dts $(BUILD_DIR)
	dtc -I dts -O dtb -o $@ $<

$(BUILD_DIR)/litex_gpio_driver.ko: litex_gpio_driver.c $(BUILD_DIR_MAKEFILE)
	${MAKE} -C ${LINUX_SOURCE} O=${LINUX_BUILD} M=${BUILD_DIR} src=$(PWD) modules

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch $@
	
$(BUILD_DIR):
	mkdir -p $@

build-virtio:
	truncate -s 64M ${VIRTIO_BUILD}
	mkfs.ext4 -d ${BUILD_DIR} ${VIRTIO_BUILD}

clean-virtio:
	rm -f ${VIRTIO_BUILD}

clean: clean-virtio
	${MAKE} -C ${LINUX_SOURCE} M=${PWD} clean
	rm -rf $(BUILD_DIR)

.PHONY: clean build-virtio clean-virtio
