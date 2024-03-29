# `driver_litex_gpio` description

The driver controls a simple LiteX GPIO peripheral, that raises an interrupt once a virtual button is pressed. A Renode's model of the device is available [here](https://github.com/renode/renode-infrastructure/blob/master/src/Emulator/Peripherals/Peripherals/GPIOPort/LiteX_GPIO.cs). The main tasks of this driver are:
* implement the logic counting the interrupts caught by the driver
* return the number of caught interrupts in the `read` function
* reset the interrupts counter using `GPIO_IOCTL_RESET` ioctl
* for handling the data read completion mechanism is used (`read` function waits for the interrupt and ISR signals that the interrupt has been caught)

For learning purposes 2 GPIOs are used in this example (`gpio_in_1`, `gpio_in_2`) and they both share the same PLIC's interrupt line number 3.

## Interrupt trigger
In Renode the interrupts can be triggered using `PressAndRelease` command on a specific button, for example:
```
(machine-0) gpio_in_1.button_1 PressAndRelease 
```
The interrupts can be also triggered periodically using `watch` command:
```
(machine-0) watch "gpio_in_1.button_1 PressAndRelease" 1000 
```
