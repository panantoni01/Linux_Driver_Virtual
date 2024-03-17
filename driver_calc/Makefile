TOPDIR := $(realpath ..)

include ${TOPDIR}/config.mk

BUILD_DIR ?= $(PWD)/build
BUILD_DIR_MAKEFILE ?= $(BUILD_DIR)/Makefile

build: dtb test modules

dtb: $(BUILD_DIR)/rv32.dtb
test: $(BUILD_DIR)/test_app
modules: $(BUILD_DIR)/calc_driver.ko

$(BUILD_DIR)/%.dtb: %.dts $(BUILD_DIR)
	dtc -I dts -O dtb -o $@ $<

$(BUILD_DIR)/test_app: test_app.c $(BUILD_DIR)
	$(CROSS_COMP)$(CC) -Og -Wall -o $@ $<

$(BUILD_DIR)/calc_driver.ko: calc_driver.c $(BUILD_DIR_MAKEFILE)
	${MAKE} -C ${LINUX_SOURCE} O=${LINUX_BUILD} M=${BUILD_DIR} src=$(PWD) modules

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch $@
	
$(BUILD_DIR):
	mkdir -p $@

clean:
	${MAKE} -C ${LINUX_SOURCE} M=${PWD} clean
	rm -f $(BUILD_DIR)

.PHONY: clean
