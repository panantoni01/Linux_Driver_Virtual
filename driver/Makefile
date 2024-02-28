TOPDIR := $(realpath ..)

include ${TOPDIR}/config.mk

build: dtb test modules

dtb: ${TOPDIR}/build/rv32.dtb
test: test_app
modules: calc_driver.ko

${TOPDIR}/build/%.dtb: %.dts
	dtc -I dts -O dtb -o $@ $^

test_app: test_app.c
	$(CROSS_COMP)$(CC) -Og -Wall -o $@ $^

calc_driver.ko: calc_driver.c
	${MAKE} -C ${LINUX_SOURCE} O=${LINUX_BUILD} M=${PWD} modules

clean:
	${MAKE} -C ${LINUX_SOURCE} M=${PWD} clean
	rm -f modules.order Module.symvers *.o calc_driver.ko calc_driver.mod.c calc_driver.mod.o calc_driver.o calc_driver.mod .modules.* .Module.* .calc_driver.*
	rm -f test_app
	rm -f ${TOPDIR}/build/*.dtb

.PHONY: clean
