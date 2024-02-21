# Circuit and PCB design for hardware version 2.2

The PCB is designed so that all SMD components can be placed by
[JLCPCB](https://jlcpcb.com/).

Tutorials for ordering:
* [How to generate Gerber and Drill files in KiCad 7](https://jlcpcb.com/help/article/362-how-to-generate-gerber-and-drill-files-in-kicad-7)
* [How to generate the BOM and Centroid file from KiCAD](https://jlcpcb.com/help/article/81-How-to-generate-the-BOM-and-Centroid-file-from-KiCAD)

Make sure each component gets placed in the right orientation.
Pin 1 of each IC is marked with a dot on the silkscreen.

## Settings for JLCPCB Standard PCB/PCBA

* PCB Qty: 30
* Surface Finish: LeadFree HASL
* Specify Layer Sequence: Yes, F / In1 / In2 / B
* Impedance Control: Yes, JLC046161H-7628 (Prepreg thickness 0.2104 mm)
* Remove Order Number: Specify a location
* PCB Assembly: Yes
* Tooling holes: Added by Customer

## Component placement corrections

* U.FLs: Rotate right, move 4 steps
* U1: Rotate (forgot how much)
* U4: Rotate right
* U2, U3, U7, Q1, Q2, Q3, Q4, Q5, Q6: Rotate 180°

