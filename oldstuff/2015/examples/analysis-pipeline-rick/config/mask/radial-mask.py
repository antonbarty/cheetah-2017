#!/usr/bin/env python

# mask out anuli
# r. kirian

import h5py
import sys
import os
import numpy as np
import matplotlib.pyplot as plt


# a pixelmap which radii are based on
h5FileName = "/reg/neh/home/rkirian/cxi/cxi49012/config/pixelmap/cspad-front-4feb2012-107mm.h5"
f = h5py.File(h5FileName, "r")
tim = f["x"]
tim = np.array(tim)
x = tim.copy()
tim = f["y"]
tim = np.array(tim)
y = tim.copy()
tim = f["z"]
tim = np.array(tim)
z = tim.copy()
f.close()
# here's what we want: the radius of each pixel
r = np.sqrt(x**2 + y**2 + z**2)/110e-6

# a file to use as a template
h5FileName = "/reg/d/psdm/cxi/cxi49012/scratch/hdf5/r0074-HG1/r0074-class1-sumRaw.h5"
f = h5py.File(h5FileName, "r")
data = f["data"]
tim = data["data"]
tim = np.array(tim)
im = tim.copy()
f.close()

# pick the regions to mask
mask = np.ones((im.shape[0],im.shape[1]))
mask = np.uint16(mask)
mask[(r > 270)*(r < 375)] = 0
mask[(r > 150)*(r < 155)] = 0
mask[(r > 97 )*(r < 103)] = 0
mask[(r > 72 )*(r < 78 )] = 0
mask[(r > 47 )*(r < 52 )] = 0

# print the masked image
f = h5py.File("temp.h5","w")
data = f.create_group("data");
data.create_dataset("data",data=(mask*im));
f.close()

# now view the masked image in hdfsee
os.system("hdfsee temp.h5 -i 1 -b 1 -g /reg/neh/home/rkirian/cxi/cxi49012/config/geom/default.geom")
os.system("rm temp.h5")

# now write the masked file
print("writing file newmask.h5...\n")
f = h5py.File("newmask.h5","w")
data = f.create_group("data");
data.create_dataset("data",data=mask);
f.close()

