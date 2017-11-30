# Geckokapula
Silicon Labs EFR32 transceivers give access to filtered IQ samples on receive,
so it's possible to make an all-mode handheld receiver on them.

Transmitting FM and constant-envelope SSB should be possible by approximating them
on the 4-FSK modulator if a somewhat dirty spectrum can be accepted.

Currently this project is just an early experiment and has a crude spectrogram display,
an FM demodulator and a CW transmitter. There's no user interface yet.

# Construction
BRD4151A radio board (with EFR32MG1P232F256GM48) is connected to a display and a speaker.

The display is something like this: https://www.ebay.com/itm/232327157750

Speaker is driven from a PWM output through a series capacitor and a resistor.

| Board pin | EFR32 pin | Connection   |
|-----------|-----------|--------------|
|   GND     |           | Ground plane |
| VMCU_IN   |           | +3.3 V       |
|   P1      |   PC6     | Display SDA  |
|   P3      |   PC7     | Display CS   |
|   P5      |   PC8     | Display SCK  |
|   P7      |   PC9     | Display AO   |
|   P9      |   PA0     | Debug UART TX (to RXD on USB-UART ) |
|   P11     |   PA1     | Debug UART RX (to TXD on USB-UART ) |
|   P34     |   PF6     | PTT or CW key (switch to GND) |
|   P4      |   PD10    | PWM Audio out |
|   P6      |   PD11    | Encoder A (switch to GND) |
|   P8      |   PD12    | Encoder B (switch to GND) |
|   P31     |   PD13    | Encoder push button (switch to GND) |
|   P24     |   PF0     | J-link SWCLK |
|   P26     |   PF1     | J-link SWDIO |

VCC, RESET and LED pins on display are connected to +3.3 V.

# Files in the project
The project should be opened in Simplicity Studio.

The interesting code is in main.c, display.c and ui.c.
Everything else is mostly silabs libraries and generated stuff.
The isc and hwconf files have the hardware parameters
that can be edited in Simplicity Studio GUI.


Font is from https://github.com/dhepper/font8x8/
