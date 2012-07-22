#!/usr/bin/env python
#--------------------

import numpy as np
import matplotlib.pyplot as plt
import sys
import h5py
import matplotlib.patches as patches # for patches.Circle

#--------------------

class Storage :
    def __init__(self) :
        print 'Storage object is created'

    def printStorage(self) :
        print 'You print the class Storage object'

#--------------------
# Define graphical methods

def plot_image (arr, range=None, zrange=None, store=None) :    # range = (left, right, low, high), zrange=(zmin,zmax)
    fig = plt.figure(num=1, figsize=(12,12), dpi=80, facecolor='w',edgecolor='w',frameon=True)
    fig.subplots_adjust(left=0.10, bottom=0.08, right=0.98, top=0.92, wspace=0.2, hspace=0.1)
    store.figAxes = figAxes = fig.add_subplot(111)
    store.imAxes  = imAxes = figAxes.imshow(arr, origin='upper', interpolation='nearest', aspect='auto',extent=range)
    #imAxes.set_clim(1300,2000)
    if zrange != None : imAxes.set_clim(zrange[0],zrange[1])
    colbar = fig.colorbar(imAxes, pad=0.03, fraction=0.04, shrink=1.0, aspect=40, orientation=1)


def plot_histogram(arr,range=(0,500),figsize=(5,5), store=None) :
    fig = plt.figure(figsize=figsize, dpi=80, facecolor='w',edgecolor='w',frameon=True)
    plt.hist(arr.flatten(), bins=60, range=range)
    #fig.canvas.manager.window.move(500,10)


def plot_peaks (arr_peaks, store=None) :  
    axes = store.figAxes
    ampave = np.average(arr_peaks,axis=0)[2]
    print 'ampave=', ampave
    for peak in arr_peaks :
        print peak[0], peak[1], peak[2]
        xy0, r0 = (peak[0], peak[1]), 10*peak[2]/ampave
        circ = patches.Circle(xy0, radius=r0, linewidth=2, color='r', fill=False)
        axes.add_artist(circ)

#--------------------

def get_array_from_file(fname) :
    print 'get_array_from_file:', fname
    return np.loadtxt(fname, dtype=np.float32)

#--------------------

def get_input_parameters() :

    fname_def = './image0_ev000115.txt'
    Amin_def  =   0
    Amax_def  = 100

    nargs = len(sys.argv)
    print 'sys.argv[0]: ', sys.argv[0]
    print 'nargs: ', nargs

    if nargs == 1 :
        print 'Will use all default parameters\n',\
              'Expected command: ' + sys.argv[0] + ' <infname> <Amin> <Amax>' 
        sys.exit('CHECK INPUT PARAMETERS!')

    if nargs  > 1 : fname = sys.argv[1]
    else          : fname = fname_def

    if nargs  > 2 : Amin = int(sys.argv[2])
    else          : Amin = Amin_def

    if nargs  > 3 : Amax = int(sys.argv[3])
    else          : Amax = Amax_def

    if nargs  > 4 :         
        print 'WARNING: Too many input arguments! Exit program.\n'
        sys.exit('CHECK INPUT PARAMETERS!')

    ampRange = (Amin, Amax)

    print 'Input file name  :', fname
    print 'ampRange         :', ampRange
 
    return fname,ampRange 

#--------------------

def do_main() :

    fname, ampRange = get_input_parameters()

    arr = get_array_from_file(fname)
    print 'arr:\n', arr
    print 'arr.shape=', arr.shape

    # Get peaks form file
    fname_peaks = 'image_peaks_' + fname.rsplit('_')[1]
    arr_peaks = np.loadtxt(fname_peaks, dtype=np.double)
    print 'Try to get peaks from file: ', fname_peaks

    s = Storage()

    plot_image(arr, zrange=ampRange, store=s)
    plt.get_current_fig_manager().window.move(10,10)

    plot_peaks(arr_peaks, store=s)

    plot_histogram(arr,range=ampRange, store=s)
    plt.get_current_fig_manager().window.move(950,10)

    plt.show()

#--------------------

if __name__ == '__main__' :
    do_main()
    sys.exit('The End')

#--------------------
