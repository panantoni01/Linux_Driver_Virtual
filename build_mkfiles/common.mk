# common Makefile to use in the driver_* directories
# 
# the following variables need to be provided:
# DTS_SRC - device tree source file
# MOD_SRC - driver source file
# TEST_SRC - name of the test application source file

BUILD_DIR          ?= $(PWD)/build
BUILD_DIR_MAKEFILE ?= $(BUILD_DIR)/Makefile
VIRTIO_BUILD       ?= $(PWD)/drive.img

RV_DTB   ?= $(DTS_SRC:%.dts=$(BUILD_DIR)/%.dtb)
MOD_KO   ?= $(MOD_SRC:%.c=$(BUILD_DIR)/%.ko)
TEST_EXE ?= $(TEST_SRC:%.c=$(BUILD_DIR)/%)

dtb: $(RV_DTB)
test: $(TEST_EXE)
modules: $(MOD_KO)

# build device tree blob
$(RV_DTB): $(DTS_SRC) $(BUILD_DIR)
	dtc -I dts -O dtb -o $@ $<

# build test application
$(TEST_EXE): $(TEST_SRC) $(BUILD_DIR)
	$(CROSS_COMP)$(CC) -Og -Wall -o $@ $<

# build the kernel module
$(MOD_KO): $(MOD_SRC) $(BUILD_DIR_MAKEFILE)
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
