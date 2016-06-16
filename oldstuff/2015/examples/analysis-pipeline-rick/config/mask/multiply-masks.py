#!/usr/bin/env python

# combine masks.  i.e. multiply them.
#
# do this:
# > ./multiply-masks.py -i some/path.h5 another/path.h5 -o newmask.h5
#
# r. kirian

import argparse
import h5py
import sys
import numpy as np
import matplotlib.pyplot as plt

def readh5(filename):
	f = h5py.File(filename, "r")
	data = f["data"]
	tim = data["data"]
	tim = np.array(tim)
	im = tim.copy()
	f.close()
	return im

def writeh5(filename,mydat):
	f = h5py.File(filename,"w")
	data = f.create_group("data");
	data.create_dataset("data",data=mydat);
	f.close()
	return

parser = argparse.ArgumentParser()
parser.add_argument("-i", action="append", dest="h5FileNames", type=str, nargs='+', help="intput files")
parser.add_argument("-o", action="store", dest="h5SavePath", type=str, nargs=1, help="output file")
args = parser.parse_args()

h5FileNames = args.h5FileNames[0]
h5FileName = h5FileNames[0]
h5SavePath = args.h5SavePath[0]

mask = readh5(h5FileName)

for i in range(1,len(h5FileNames)):
	im = readh5(h5FileNames[i])
	mask *= im
	
writeh5(h5SavePath,mask)

