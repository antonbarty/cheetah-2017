#!/bin/bash


# combine the masks you've made with e.g. cspad-asig-edge-mask.py, 
# and create links
#
# r. kirian

./multiply-masks.py -i cspad-asic-edge-mask.h5 visibly-bad-mask.h5 -o combined.h5

rm default* &> /dev/null
ln -s combined.h5 default-ds1.h5
ln -s default-ds1.h5 default.h5
