#!/usr/bin/env python
# pngmap_to_h5map.py
# =============================================================================
# Generates h5 map from s/w png image. Black values are interpreted as 0, white values as 1.
# Max Hantke

from pylab import *
import h5py,ConfigParser,sys,Image

if len(sys.argv) < 2:
    print "ERROR: No png file specified."
    exit(0)

filename = sys.argv[1]

I = Image.open(filename)
D = array(I.getdata(),dtype='int')[:,0]
D = array(D!=0,dtype='int')
N = int(sqrt(len(D)))
D = D.reshape((N,N))

f = h5py.File(filename[:-4]+'.h5','w')
g = f.create_group('/data')
f['data/data'] = D[:,:]
f.close()
        
