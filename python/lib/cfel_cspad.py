# -*- coding: utf-8 -*-
#
#	CFEL image handling tools
#	Anton Barty
#

import numpy
import pyqtgraph
import lib.cfel_filetools as cfel_file


def badpix_from_darkcal(filename="", edges=True):

    # Debug
    print("cfel_cspad.badpix_from_darkcal")
    print("filename = ", filename)


    # Read in data
    a = cfel_file.read_h5(filename, field='data/non_assembled_raw')
    b = cfel_file.read_h5(filename, field='data/non_assembled_raw_sigma')
    n = cfel_file.read_h5(filename, field='data/nframes')
    a = a / n[0]

    # Thresholds
    noisethresh = 5
    valuethresh = 5

    # Size of input array
    s = numpy.asarray(a.shape)
    ss = numpy.divide(s,8).astype(int)
    print("Shape of dark array: ", s)
    print("ASIC size: ", ss)

    # Output array
    #m = numpy.zeros(a.shape, dtype='b')
    mask = numpy.ndarray(a.shape, dtype='b')
    mask[:] = 1
    #print(mask.shape)
    #print(mask.dtype)

    # Check for cspad
    # Return "OK everywhere" if not the right size
    if s[0] != 1480 or s[1] != 1552:
        print("cfel_cspad.badpix_from_darkcal()")
        print("This does not look like a cspad array")
        print("Expected shape: [1480 1552]")
        print("Actual shape: ", s)
        print("Returning blank mask...")
        return mask


    # Check for noisy pixels
    # (non_assembled_raw_sigma >= nposethresh)
    wn = numpy.where(b >= noisethresh)
    #print("Number of noisy pixels: ", wn[0].shape)
    mask[wn[0], wn[1]] = 0


    # Check for pixels with significantly different dark values within ASIC
    for j in range(0,7):
        for i in range(0,7):

            asic = a[i*ss[0]:(i+1)*ss[0]-1, j*ss[1]:(j+1)*ss[1]-1]
            #print(asic.shape)

            m = numpy.mean(asic)
            dev = numpy.std(asic)
            #print(m, dev)

            wd = numpy.where(asic >= m + valuethresh*dev)
            mask[i*ss[0]+wd[0], j*ss[1]+wd[1]] = 0



    # How many bad pixels?
    wbad = numpy.where(mask == 0)
    print("Number of bad pixels: ", wbad[0].shape)


    # Mask edges of the ASICs
    if edges:
        for i in range(0,7):
            mask[i*ss[0],:] = 0
            mask[(i+1)*ss[0]-1,:] = 0
            mask[:,i*ss[1]] = 0
            mask[:,(i+1)*ss[1]-1] = 0



    return mask
