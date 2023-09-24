#!/bin/sh
POS="pcba/kapula-top-pos.csv"
TEMP="${POS}_new"
echo "Designator,Val,Package,Mid X,Mid Y,Rotation,Layer" > "${TEMP}"
tail -n +2 -- "${POS}" >> "${TEMP}"
mv -- "${TEMP}" "${POS}"
