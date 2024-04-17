# `driver_si7021` description

The driver controls a SI7021 device, which is a temperature and humidity sensor. The datasheet is available on [Silabs site](https://www.silabs.com/documents/public/data-sheets/Si7021-A20.pdf). The driver provides only those functionalities that are supported by the Renode's model of the device ([link](https://github.com/renode/renode-infrastructure/blob/master/src/Emulator/Peripherals/Peripherals/Sensors/SI70xx.cs)), that is:
* device reset by issuing `SI7021_IOCTL_RESET` ioctl
* getting serial number of the device by issuing `SI7021_IOCTL_READ_ID` ioctl
* getting the temperature and relative humidity measurements by reading from the character device

In this example two sensors are used in the platform description: one is SI7021 and the other one is SI7006. They have different serial numbers, but all the other functionalities are exactly the same for these sensors (at least in the Renode's model).
