# Flashing the firmware

The firmware is flashed using Serial Wire Debug (SWD), a standard interface
of ARM microcontrollers. SWD is like JTAG but needs less pins.
There are lots of different USB-SWD adapters that could be used.

See [hardware documentation](../hardware_v2/README.md#flashing-and-debugging)
for connections between SWD adapter and board.

## Some possible SWD adapters

### J-Link
Some J-Link compatible adapters that could work:
* Genuine J-Link from SEGGER (really expensive)
* [A cheap "J-Link" clone](https://www.ebay.com/itm/256009191453).
  These do not work with SEGGER software but work fine with OpenOCD.
* J-Link integrated on a development board such as Silicon Labs WSTK.
  (There is one you can use at OH2TI radio club.)

### ST-Link
* ST-Link integrated on some STM32 Nucleo development board.
  Remove two jumpers that connect SWD to STM32 and use the
  SWD pin header.
  (TODO: Find a clear picture of pinout and connections for these.)
* [A cheap ST-Link clone](https://www.ebay.com/itm/364490903972)
  (search eBay for ST-LINK).
  Not tested but might work too.

### CMSIS-DAP
Adapters using the standard CMSIS-DAP protocol
are available from several manufacturers. Maybe this means that
there are less problems with software refusing to work when they
detect a clone (the problem with J-Link clones).
I have not tested any of these yet but there are some cheap options:
* https://www.aliexpress.com/item/1005004095909283.html :
  Cheap and small enough you could just put it permanently inside
  your radio. It also has a 5 V output pin, so it would work
  as a USB charging port too.
* https://www.aliexpress.com/item/1005003095115792.html
* Many others. If you know a good one, tell me and I can add it
  here in the list.

### SBC GPIO pins
If you do not want to buy any USB adapter but happen to have a
Raspberry Pi or some other Linux single-board computer around,
you could also use its GPIO pins as an SWD adapter.
OpenOCD has
[drivers](https://openocd.org/doc/html/Debug-Adapter-Configuration.html)
*bcm2835gpio* for Raspberry Pi and *linuxgpiod* for any board.

You may have to compile OpenOCD yourself to get support for these,
so it may get difficult if you are not an experienced Linux user.

## Flashing using OpenOCD software
OpenOCD is good in that it supports a lot of different SWD adapters
and can do a lot of things.
Its downside is that it can feel somewhat complicated to configure
and use due to its command line user interface and use of TCL
language for configuration, scripting and commanding.
I try to provide straightforward instructions here so you do not need
to learn too much about OpenOCD just to get your firmware flashed.

### Installing on Linux
On Ubuntu, Debian, Mint or similar, do:

    sudo apt install openocd

Your user needs permissions to access SWD adapter hardware.
This can be done using udev rules. Download
https://raw.githubusercontent.com/openocd-org/openocd/master/contrib/60-openocd.rules
and copy it to the right place:

    sudo cp 60-openocd.rules /etc/udev/rules.d/
	sudo udevadm control --reload

Connect your SWD adapter after copying these udev rules.

### Installing on Windows
[OpenOCD website](https://openocd.org/pages/getting-openocd.html#unofficial-binary-packages)
lists a few places where you can download OpenOCD for Windows.
I do not actively use it on Windows so I cannot give detailed
instructions, but I tested it a few years ago and it worked.
I had to use  [Zadig](https://zadig.akeo.ie/)
to make Windows use the WinUSB driver for my J-Link clone.
I am not sure if that applies to other SWD adapters.

### Installing on Mac and other systems
OpenOCD supports Mac, too, but I do not have any Apple computer to
test with, so I do not know if it needs some extra configuration.

### Flashing
OpenOCD needs to know which SWD adapter you have.
The [openocd/](openocd/) directory here contains an OpenOCD script
with configuration for a few different adapters.
The adapter can be selected by setting a SWD_ADAPTER environment
variable before starting OpenOCD. For example:

    export SWD_ADAPTER=st-link
    cd openocd
    openocd -f adapter.cfg -c init -c "program path_to_firmware.bin verify reset" -c exit

Replace st-link with jlink or cmsis-dap depending on your SWD adapter.
Replace path_to_firmware.bin with the filename of your firmware binary.
If you [compiled firmware yourself](README.md), you can also do:

    openocd -f flash_v2.cfg

## Other software options
Some other software you could try if OpenOCD does not work for you:
* [Simplicity Commander](https://community.silabs.com/s/article/simplicity-commander?language=en_US):
  Silicon Labs tool to flash EFR32 chips.
  Last time I installed it was maybe 5 years ago, so I do not have
  up to date knowledge on it. It worked well, but installing SiLabs
  software was complicated.
* SEGGER J-Flash: Only works with genuine SEGGER J-Link adapters.
  I have never tried it but heard it is really easy to use.
* pyOCD: Sort of an alternative to OpenOCD. Might be easier to use,
  work better for some usecases or something, but does not seem to
  support as many SWD adapters as OpenOCD does.
