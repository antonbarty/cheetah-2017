#!/usr/bin/env python
# cxi_frame_to_png.py
# =============================================================================
# Generates png image from a frame of a cxi dataset
# Max Hantke

from pylab import *
import h5py,sys

if len(sys.argv) < 2:
    print "ERROR: No cxi file specified."
    print "Usage: ./cxi_frame_to_png.py FILENAME [FRAME=0] [DATASET=/entry_1/data_1/data] [OUTFILE=FILENAME+frame+png]"
    exit(0)

filename = sys.argv[1]

if len(sys.argv) >= 3:
    frame = int(sys.argv[2])
else:
    frame = 0

if len(sys.argv) >= 4:
    dataset = sys.argv[3]
else:
    dataset = "/entry_1/data_1/data"

if len(sys.argv) >= 5:
    out_filename = sys.argv[4]
else:
    out_filename = filename
    if ".h5" in filename:
        out_filename = out_filename[:-3]
    elif ".cxi" in filename:
        out_filename = out_filename[:-4]
    out_filename += "_%i.png" % frame

f = h5py.File(filename,"r")
I = f[dataset][frame,:,:]
I[I<=0.1] = 0.1
imsave(out_filename,log10(I),vmin=-1)

