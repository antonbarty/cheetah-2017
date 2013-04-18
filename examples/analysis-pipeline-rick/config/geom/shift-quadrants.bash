#!/bin/bash

# shift detector quadrant positions in a CrystFEL "geom" file
#
# e.g., the command below shifts quadrant 0 by 10 pixels in x, and
# quadrant 1 by -5 pixels in y:
#
# > ./shift-quadrants $configdir/geom/default.geom 10 0 0 -5 0 0 0 0 > new.geom
#
# r. kirian

geom=$1
x0=$2
y0=$3
x1=$4
y1=$5
x2=$6
y2=$7
x3=$8
y3=$9

temp=temp1.geom
out=out.geom
cp $geom $out

# quadrant 0

[ $x0 ] || x0=0
[ $y0 ] || y0=0

cat $out | awk -v cc=$x0 -F '=' '{OFS=" = "; if ($1 ~ /^q0a[0-9]+\/corner_x/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out
cat $out | awk -v cc=$y0 -F '=' '{OFS=" = "; if ($1 ~ /^q0a[0-9]+\/corner_y/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out

# quadrant 1, and so on...

[ $x1 ] || x1=0
[ $y1 ] || y1=0

cat $out | awk -v cc=$x1 -F '=' '{OFS=" = "; if ($1 ~ /^q1a[0-9]+\/corner_x/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out
cat $out | awk -v cc=$y1 -F '=' '{OFS=" = "; if ($1 ~ /^q1a[0-9]+\/corner_y/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out

[ $x2 ] || x2=0
[ $y2 ] || y2=0

cat $out | awk -v cc=$x2 -F '=' '{OFS=" = "; if ($1 ~ /^q2a[0-9]+\/corner_x/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out
cat $out | awk -v cc=$y2 -F '=' '{OFS=" = "; if ($1 ~ /^q2a[0-9]+\/corner_y/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out

[ $x3 ] || x3=0
[ $y3 ] || y3=0

cat $out | awk -v cc=$x3 -F '=' '{OFS=" = "; if ($1 ~ /^q3a[0-9]+\/corner_x/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out
cat $out | awk -v cc=$y3 -F '=' '{OFS=" = "; if ($1 ~ /^q3a[0-9]+\/corner_y/) print $1, $2+cc; else print $0}' > $temp
mv $temp $out

cat $out

rm $out

