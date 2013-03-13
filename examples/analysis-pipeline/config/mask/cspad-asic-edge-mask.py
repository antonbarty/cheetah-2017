#!/usr/bin/env python

# mask the edges of each CSPad ASIC
# r. kirian

import h5py
import sys
import numpy as np
import matplotlib.pyplot as plt

mask = np.ones((1480,1552))
mask = np.uint16(mask)

asicfs = 194;
asicss = 185;

for i in range(0,8):
	mask[:,i*asicfs] = 0
	mask[:,(i+1)*asicfs-1] = 0
	mask[i*asicss,:] = 0
	mask[(i+1)*asicss-1,:] = 0


f = h5py.File("cspad-asic-edge-mask.h5","w")
data = f.create_group("data");
data.create_dataset("data",data=mask);
f.close()


