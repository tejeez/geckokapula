#!/bin/sh
set -e
echo 'Make sure you have done these:'
echo '* Schematic Editor -> Tools -> Generate BOM'
echo '  * With: xsltproc -o "%O.csv" "/path/to/bom2grouped_csv_jlcpcb.xsl" "%I"'
echo '* PCB Editor -> File -> Fabrication Outputs -> Gerbers'
echo '  * Also remember drill and map files'
echo '* PCB Editor -> File -> Fabrication Outputs -> Component Placement (.pos)'
echo '* Check file timestamps to make sure they were all exported now'
if [ -z "$1" ]; then
echo ''
echo "Usage: $0 \"output_directory_path\""
exit
fi
OUTDIR="$1"
mkdir -p -- "$OUTDIR"
(cd gerber && ls -l && zip -r "$OUTDIR/kapula_v2_2.zip" .)
./fix_pos_for_jlcpcb.sh
cp -ai -- kapula.csv pcba/kapula-top-pos.csv "$OUTDIR"
ls -l -- "$OUTDIR"
echo 'Done'

