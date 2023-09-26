# Hardware version 2.1

## Basic parts

### Gekkokapula board
The Gekkokapula PCB v2.1 contains most of the electronics.
All bigger and more "mechanical" parts are wired to pin
headers on the board. This lets you choose any enclosure
you like and arrange all the knobs, buttons and connectors
the way you like.

#### Bugs
PCB v2.1 has its RF switch control lines swapped.
This can be fixed by moving series resistors
R76 and R77 to cross the lines.

This could have been fixed by swapping the pins in software,
but swapping them in hardware keeps the new hardware more
compatible with older firmware versions.
Next PCB order (v2.2) will have this fixed.

### Display
A display can be mounted directly on top of a Gekkokapula board.
The display is a
[128x160 RGB TFT module with an 8-pin header](https://www.ebay.com/itm/403774302965).

These display modules also include an SD card slot
but it is not used for anything in Gekkokapula.
The 4-pin SD card header, however, can be placed and used for
additional mechanical support and ground pins.

### Encoder
The most important part of the user interface is a quadrature
encoder with a push button.

TODO: diagram showing encoder connections

### Speaker
Connect a speaker between SPK+ and SPK- pins
(pins 13 and 14 on header J1).
Almost any speaker should work.
A 4 Ω or 8 Ω speaker with a power handling of at least 1 W
is recommended.

### Microphone
Connect an electret microphone capsule between MIC and GND
(pins 9 and 10 on header J1).
Most electret microphones should work here.

### Battery
The board includes a protection circuit and a charger for a single
cell Li-Ion or Li-Po battery. Connect one between BAT+ and BAT-
pins (pins 17 and 18 on header J1).

Note that battery minus is not directly connected to ground
(because of the protection circuit).

### Charging
The battery charger works with any supply voltage from 4 V to 9 V
between VCharge pin and GND (pins 19 and 20 header J1).

5 V from a USB port or a cellphone charger should work fine.

### RF connections
All RF connectors on the board are U.FL. Use pigtail cables
to adapt them to your favourite RF connectors.

13 cm band has its own connector, J3. It is used for
receiving and transmitting in the 2300-2900 MHz range.
All lower frequency bands (13 MHz to 1.45 GHz) use the
connector J5.

### Flashing and debugging
The board has two alternative connectors for SWD.
One is a 4-pin 2.54 mm header with the pinout of
[cheap "J-Link" clones](https://www.ebay.com/itm/256009191453).

GND, SWCLK and SWDIO pins are required and shall be connected to
corresponding pins on your SWD adapter.

The VDD pin is an output meant for SWD adapters with a reference
voltage input (used for level shifters or target detection).
If your SWD adapter does not have a reference voltage input,
leave the VDD pin unconnected.

The other connector is a 10-pin 1.27 mm header with standard
Cortex Debug pinout. It gives access to SWO and reset pins
and could be also used for JTAG in addition to SWD.

Any SWD adapter supported by OpenOCD should work but may need
additional configuration for OpenOCD.
I can try to add configuration for more adapters
if you can give me an adapter to test with.


## Optional parts

### PTT button
A PTT button is optional since transmitting can be also started
from the "menu" using the encoder.

If you prefer to have a dedicated button for PTT, connect one
between PTT and GND (pins 11 and 12 on header J1).

### Power switch
A power switch is optional since there is software and hardware
support for a sleep mode which reduces current consumption
to a few µA.

If you prefer to have a "real" power switch, connect a toggle
switch between PWSW pins (pins 15 and 16 on header J1).

If a switch is not used, connect pins 15 and 16 directly together.

### Volume potentiometer
A volume potentiometer is optional since volume can be also
adjusted from the "menu" using the encoder.

If you prefer to have a dedicated knob for volume, connect
a potentiometer between pins 5, 6 and 7 on header J1
as illustrated on the silkscreen.
Resistance of the potentiometer should be something from
10 kΩ to 50 kΩ.

If a potentiometer is not connected, connect pins 5 and 6
together. Volume can be also reduced by adding a fixed
series resistor or a voltage divider here.


## Advanced use and modifications

### Using without battery
If you want to directly use an external power supply and bypass
battery protection and charging circuits, connect the supply
between power switch pin (pin 16 on J1) and ground
(such as pin 20 on J1).

The circuit can handle supply voltages from 3 V to 6 V.
Keep in mind that a higher voltage will result in higher power
dissipation in 3.3 V regulator and audio amplifier chips.
Recommended range is between 3.5 V and 5.0 V to keep voltage
regulator working and to keep its dissipation reasonable.

### Headphone connector
Output of the audio power amplifier is bridged, so neither end of
the speaker is connected to ground.

If you want to add a headphone jack, it may be more convenient to
connect one end of headphones to ground, particularly if you use
a 4-pin headset connector with a microphone pin. This can be done
but an additional DC blocking capacitor is needed.
Series resistors are also recommended to protect the audio amplifier
from short circuits.

TODO: diagram showing additional circuitry for a headphone jack

### Use on bands other than 13 cm and 70 cm
The board includes a low-pass filter (LPF) for the 70 cm band.
This is needed to suppress transmitter harmonics and receiver
harmonic mixing products.
RF matching circuits are also optimized for 70 cm but will
work on other bands with somewhat reduced transmit power
and receive sensitivity.

For use on other bands, the LPF should be bypassed
(by moving C82 and R78) and an external LPF should be
connected between J5 and an antenna.
Receiving lower bands without modification or external
filtering is possible but the receiver is prone to harmonic
mixing if a proper LPF is not used. This means you may hear
spurious responses from higher frequencies, such as
FM broadcast signals when trying to listen to HF,
or 70cm band signals when trying to listen to 2m.

### Separate connectors for RX and TX
J4 can be used as a separate receiving connector for frequencies
below 1.45 GHz. This can be useful with an external power amplifier,
low noise amplifier or a transverter.
To use J4, move capacitor C74 to connect receiving chain to J4
through TP5 (and to disconnect it from the RX/TX switch).

On 2300-2900 MHz, J3 is always used for both TX and RX.
Separate connectors for 13 cm are not available.

### Changing charging current
Charge current is limited to approximately 400 mA.
If you want to change the current limit, check the
[data sheet of the charger chip](https://datasheetspdf.com/pdf-file/1090540/NanJingTopPower/TP4054/1)
and change resistor R2 value.

### Expansion header
Header J2 gives access to different supply voltages,
TX/RX switch controls and some free EFR32 pins.
These could be used to control an additional power amplifier
or switched band filters. No such expansions exist at the moment
but they might be developed in the future.

Free EFR32 pins could be even used to interface some external
sensors or a computer, allowing some "IoT" or "modem" kind of
applications. I have no plans for such expansions at the moment
but anything is possible. Feel free to experiment!
