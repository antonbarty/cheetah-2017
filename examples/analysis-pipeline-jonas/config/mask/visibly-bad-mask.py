#!/usr/bin/env python

# try to mask the regions that look crappy
# r. kirian

import h5py
import sys
import numpy as np
import matplotlib.pyplot as plt

h5FileName = "/reg/d/psdm/cxi/cxi43112/scratch/hdf5/darkcals/r0005-darkcal.h5"
f = h5py.File(h5FileName, "r")
data = f["data"]
tim = data["data"]
tim = np.array(tim)
im = tim.copy()
f.close()

#nfs = im.shape[0]
#nss = im.shape[1]
#afs = nfs/8
#ass = nss/8

mask = np.ones((1480,1552))
mask = np.uint16(mask)

mask[185:370,603] = 0
mask[370:475,388:443] = 0
mask[1220:1285,260:382] = 0

#h5FileName="/reg/d/psdm/cxi/cxi43312/scratch/hdf5/r0037/LCLS_2012_Jan20_r0037_153231_a6c1_cspad.h5"
#f = h5py.File(h5FileName, "r")
#data = f["data"]
#tim = data["rawdata"]
#tim = np.array(tim)
#im = tim.copy()
#f.close()

#im[im < 0] = 0
#im[im > 500] = 500
#print(max(im.flatten(1)))
#print(min(im.flatten(1)))
#print(im.shape)

# if you want to see the mask 
#plt.ion()
#plt.imshow(mask*im,interpolation="nearest",norm=None)
#plt.show()

print("writing file visibly-bad-mask.h5 ...\n")
f = h5py.File("visibly-bad-mask.h5","w")
data = f.create_group("data");
data.create_dataset("data",data=mask);
f.close()

