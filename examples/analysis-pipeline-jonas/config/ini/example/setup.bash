#!/bin/bash

# the hitfinder script looks for files like r0054-tag.ini
# so we'll make symbolic links to such files.
#
# r. kirian

bn="$(basename $PWD)"

for i in {1..200}; do\
	thisini=r$(printf '%04d' $i)-$bn.ini 
	rm $thisini &> /dev/null
	ln -s cheetah.ini $thisini
done

