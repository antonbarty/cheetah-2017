# pnccd_geometry.py
# =============================================================================
# Generats geometry of one pnCCD detector (two panes)
# Max Hantke

from pylab import *
import h5py,ConfigParser,sys

if len(sys.argv) < 2:
    print "WARNING: No configuration file specified. Assume the following settings:"
    print "[geometry]"
    print "pane1_dx = 0."
    print "pane1_dy = 0."
    print "pane2_dx = 0."
    print "pane2_dy = 0."
    print "pixel_offset = 0"
    print "[arrangement]"
    print "swap_axes=False"
    print "rotation_in_degrees_ccw=0"

    pane1_dx = pane1_dy = pane2_dx = pane2_dy = 0.
    pixel_offset = 0
    swap_axes = False
    rotation_in_degrees_ccw = 0.

else:
    C = ConfigParser.ConfigParser()
    C.readfp(open(sys.argv[1],'r'))
    pane1_dx = C.getfloat('geometry','pane1_dx')
    pane1_dy = C.getfloat('geometry','pane1_dy')
    pane2_dx = C.getfloat('geometry','pane2_dx')
    pane2_dy = C.getfloat('geometry','pane2_dy')
    pixel_offset = C.getint('geometry','pixel_offset')
    swap_axes = C.getboolean('arrangement','swap_axes')
    rotation_in_degrees_ccw = C.getfloat('arrangement','rotation_in_degrees_ccw')

N = 1024

Xi,Yi = meshgrid(arange(N,dtype='int'),arange(N,dtype='int'))
  
P1 = Xi<N/2
P2 = P1==False
             
X = array(Xi,dtype='<f4')-(N-1)/2.
Y = array(Yi,dtype='<f4')-(N-1)/2.
Z = zeros(shape=(N,N),dtype='<f4')

if pixel_offset != 0:
    # correct for pixel offset
    X[Yi>=0] += pixel_offset

# position pane1
X[P1] += pane1_dx
Y[P1] += pane1_dy

# position pane2
X[P2] += pane2_dx
Y[P2] += pane2_dy

if swap_axes:
    Ynew = X.copy()
    Xnew = Y.copy()
    X = Xnew
    Y = Ynew

if rotation_in_degrees_ccw != 0.:
    angle = rotation_in_degrees_ccw/360.*2*pi
    Xnew = zeros_like(X)
    Ynew = zeros_like(Y)
    Xnew = X*cos(angle) - Y*sin(angle)
    Ynew = Y*cos(angle) + X*sin(angle)
    X = Xnew
    Y = Ynew

F = h5py.File('pixelmap.h5','w')
F.create_dataset('x',(N,N),'<f4')
F.create_dataset('y',(N,N),'<f4')
F.create_dataset('z',(N,N),'<f4')
F['x'].write_direct(X)
F['y'].write_direct(Y)
F['z'].write_direct(Z)
F.close()

R = sqrt(X**2+Y**2+Z**2)
imsave('pixelmask_r.png',R)
imsave('pixelmask_x.png',X)
imsave('pixelmask_y.png',Y)
imsave('pixelmask_z.png',Z)
