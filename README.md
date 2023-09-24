# Gekkokapula
Silicon Labs EFR32 transceivers give access to filtered IQ samples on receive,
so it's possible to make a multimode handheld transceiver using them.

## Features
* Waterfall display (12 kHz wide)
* Receives FM, AM and USB
* Transmits FM
* Minimalistic user interface with a single knob

## Construction
There are two major versions of the hardware.
The first version is built around a
[BRD4151](https://www.silabs.com/documents/public/reference-manuals/brd4151a-rm.pdf)
radio module and only works in the 2.3-2.9 GHz band.
See [hardware\_v1/](hardware_v1/README.md) for details.

The second version is built on a custom PCB and uses a dual band EFR32 chip.
See [hardware_v2/pcb/](hardware_v2/pcb/) for a KiCad schematic and PCB design.
Construction is documented in [hardware_v2 README file](hardware_v2/README.md).

## Frequency range for the dual band model
The second version was designed for the 70 cm (432 MHz) and 13 cm (2.3 GHz)
amateur radio bands.
Apparently, EFR32 can be tuned over a much broader range:
* 13.2 - 725 MHz
* 766.7 - 966.6 MHz
* 1.15 - 1.45 GHz
* 2.3 - 2.9 GHz

## Licensing

The firmware is released under the [MIT license](firmware/LICENSE).
It depends on the Gecko SDK from Silicon Labs,
mostly licensed under the Zlib license.

I haven't decided on licensing of the hardware design yet.
