# pnccd_geometry.py
# =============================================================================
# Generats geometry of one pnCCD detector (two panes)
# Max Hantke

from pylab import *
import h5py,ConfigParser

if len(sys.argv) < 2:
    print "ERROR: Specify configuration file."
    print "# Example configuration file"
    print "pane1_dx = -10."
    print "pane1_dy = 1."
    print "pane2_dx = 0."
    print "pane2_dy = 234."
    print "pixel_offset = 1"
    exit(0)

C = ConfigParser.ConfigParser()
C.readfp(open(sys.argv[1],'r'))

pane1_dx = C.get_float('pane1_dx',0.)
pane1_dy = C.get_float('pane1_dy',0.)
pane2_dx = C.get_float('pane2_dx',0.)
pane2_dy = C.get_float('pane2_dy',0.)
pixel_offset = C.get_int('pixel_offset',0)

N = 1024

Xi,Yi = meshgrid(arange(N,dtype='int'),arange(N,dtype='int'))

  
P1 = Yi<N/2
P2 = P1==False
             
X = array(Xi-(N-1)/2.),dtype='<f4')
Y = array(Yi-(N-1)/2.),dtype='<f4')
Z = zeros(shape=(N,N),dtype='<f4')

if pixel_offset > 0:
    # correct for pixel offset
    X[Yi>=N/2] += pixel_offset

# position pane1
X[P1] += dx
Y[P1] += dy

# position pane2
X[P2] += dx
Y[P2] += dy

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
