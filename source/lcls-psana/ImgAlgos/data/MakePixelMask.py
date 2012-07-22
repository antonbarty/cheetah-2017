#!/usr/bin/env python
#--------------------

import numpy as np
import sys
import os
#import matplotlib.pyplot as plt
#import h5py

#--------------------

def get_input_parameters() :

    infname_def   = 'ana-cxi49012/cspad-cxi49012-r0025-background-ave.dat'
    threshold_def = 30
    outfname_def  = None

    nargs = len(sys.argv)
    print 'get_input_parameters() :'
    print 'nargs : ', nargs, ' List of input parameters :'
    for par in sys.argv : print par

    if nargs == 1 : print 'Will use all default parameters\n',\
                    'Expected command: ' + sys.argv[0] + ' <infname> <Amin> <Amax>' 

    if nargs  > 1 : infname   = sys.argv[1]
    else          : infname   = infname_def

    if nargs  > 2 : threshold = int(sys.argv[2])
    else          : threshold = threshold_def 

    if nargs  > 3 : outfname  = sys.argv[3]
    else          : 
        path,fname = os.path.split(infname)
        outfname  = path + '/' + fname.split('.')[0] + '-mask-' + str(threshold) + '.dat'

    if nargs  > 4 :         
        print 'WARNING: Too many input arguments! Exit program.\n'
        sys.exit('CHECK INPUT PARAMETERS!')

    print 'Input file name  :', infname
    print 'Threshold        :', threshold
    print 'Output file name :', outfname

    return infname, threshold, outfname 
    
#--------------------

def get_array_from_file(fname) :
    print 'get_array_from_file:', fname
    return np.loadtxt(fname, dtype=np.float32)

#--------------------

def save_array_in_file(arr, fname='mask.dat') :
    print 'save_array_in_file:', fname
    print 'arr:\n', arr
    print 'arr.shape:', arr.shape
    np.savetxt(fname, arr, fmt='%d') 

#--------------------

def evaluate_mask_array(arr,threshold=30) :
    print 'evaluate_mask_array(...)'
    print 'Threshold: ', threshold
    print 'Input array:\n'
    print arr
    rows,cols = shape = arr.shape
    print 'shape, rows, cols=', shape, rows, cols
    #arr_mask = np.ones(shape, dtype=np.uint16) 
    #arr_mask = np.select([arr>threshold],[0], default=arr_mask)
    arr_mask = np.select([arr>threshold],[0], default=[1])
    #theta0 = np.select([theta<0, theta>=0],[theta+360,theta]) #[0,360]

    num_zeros, num_ones = np.bincount(arr_mask.flatten())

    print 'Total number of masked pixels is', num_zeros, 'of', arr.size, 'fraction of masked: %6.4f' % ( float(num_zeros)/arr.size )
    return arr_mask

#--------------------

def do_main() :
    infname, threshold, outfname = get_input_parameters()
    arr      = get_array_from_file(infname)
    arr_mask = evaluate_mask_array(arr,threshold)
    save_array_in_file(arr_mask,outfname)

#--------------------

if __name__ == '__main__' :

    do_main()

    sys.exit('The End')

#--------------------
