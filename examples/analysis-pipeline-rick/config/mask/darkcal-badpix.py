#!/usr/bin/env python

# first attempt to mask crappy pixels exposed by dark frame statistics
#
# r. kirian

import h5py
import sys
import numpy as np
import matplotlib.pyplot as plt

mask = np.ones((1480,1552))
mask = np.uint16(mask)



# Let's mask out the regions of the dark image that have very low and very high dark current.  Low dark current probably means a dead pixel.  High dark current probably means the pixel is hot.

h5FileName="/reg/d/psdm/cxi/cxi43312/scratch/hdf5/r0147-dark/r0147-darkcal.h5"
f = h5py.File(h5FileName, "r")
data = f["data"]
tim = data["data"]
tim = np.array(tim)
dark = tim.copy()
f.close()

mask[dark < 10] = 0
mask[dark > 5000] = 0

# Let's also look at the standard deviation in the dark image, and get rid of pixels with large sigma.  This is pretty arbitrary; maybe it helps, maybe not.  It's fairly clear that there are some outliers with relatively high sigma.

h5FileName="/reg/d/psdm/cxi/cxi43312/scratch/hdf5/r0147-dark/r0147-class0-sumRawSigma.h5"
f = h5py.File(h5FileName, "r")
data = f["data"]
tim = data["data"]
tim = np.array(tim)
sigma = tim.copy()
f.close()

mask[sigma > 50] = 0

#plt.ion()
plt.imshow(mask*sigma,interpolation="nearest",norm=None)
plt.show()

print("writing file...\n")
f = h5py.File("darkcal-badpix.h5","w")
data = f.create_group("data");
data.create_dataset("data",data=mask);
f.close()

