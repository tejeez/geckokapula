# Gekkokapula
Silicon Labs EFR32 transceivers give access to filtered IQ samples on receive,
so it's possible to make a multimode handheld transceiver using them.

## Features
* Waterfall display (12 kHz wide)
* Receives FM, AM, USB, LSB, CW
* Transmits FM, CW, [USB*](#ssb-transmit), [LSB*](#ssb-transmit)
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

There is also a [tutorial for flashing firmware](firmware/flashing.md).

## Frequency range for the dual band model
The second version was initially designed for the 70 cm (432 MHz)
and 13 cm (2.3 GHz) amateur radio bands. Later it was found that
the EFR32 can be made to tuner over a much broader range.
Matching circuits of sub-GHz path in current hardware are optimized
for 70 cm but work reasonably well for lower frequencies as well.
Modification for the 23 cm band should be possible but has not
been attempted yet.

Tuning range has some gaps but covers the following frequencies:
* 13.2 - 725 MHz
* 766.7 - 966.6 MHz
* 1.15 - 1.45 GHz
* 2.3 - 2.9 GHz

## SSB transmit

\* SSB is transmitted as a constant envelope signal by only modulating
the frequency of a synthesizer. Amplitude variations are discarded and
the synthesizer is made to approximately track the phase of an SSB
modulated signal. This results is somewhat distorted audio and some
splatter to neighboring channels, but it sounds surprisingly
intelligible and has even received reports of good sound quality.
During quiet moments, some carrier is added in order to have
something to transmit.

Having a slightly dirty spectrum, this SSB transmitting trick
may not be suitable for high transmit power on a crowded bands,
but is probably acceptable for VHF/UHF contacts at low transmit power.

## Licensing

The firmware is released under the [MIT license](firmware/LICENSE).
It depends on the Gecko SDK from Silicon Labs,
mostly licensed under the Zlib license.

I haven't decided on licensing of the hardware design yet.
