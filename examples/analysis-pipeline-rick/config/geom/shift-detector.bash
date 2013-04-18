#!/bin/bash

# shift detector, e.g. for beam centering
#
# do something like this:
# ./shift-detector.bash $configdir/geom/default.geom 5 -3 > new.geom
#
# r. kirian

temp=temp1.geom
out=out.geom
cp $geom $out

geom=$1
x=$2
y=$3

cat $out | awk -v cornerx=$x -F '=' '{OFS=" = "; if ($1 ~ /^q[0-9]a[0-9]+\/corner_x/) print $1, $2+cornerx; else print $0}' > $temp
mv $temp $out
cat $out | awk -v cornerx=$y -F '=' '{OFS=" = "; if ($1 ~ /^q[0-9]a[0-9]+\/corner_y/) print $1, $2+cornerx; else print $0}' > $temp
mv $temp $out

cat $out

rm $out

