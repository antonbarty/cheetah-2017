
# here's the plan:
#
# (1) You have a CrystFEL "geom" file, with approximate detector geometry.
# (2) You will convert the crystfel "geom" file to a Cheetah geometry file.
#     To do this, you will use the program called "make_pixelmap", which is
#     a part of the crystfel-programs suite, which Tom White has not yet made 
#     publicly available.  He says that he will - email him if he hasn't done
#     so by the time you read this :)
# (3) It's handy to make symbolic links to the best known geometry - then you
#     won't have to modify Cheetah "ini" files when improvements are made.
# 
# You may wish to fiddle with the geometry later, e.g. with "shift-quadrants.bash"
# You should then re-run this script if you manage to improve things.
#
# r. kirian


rm default* &> /dev/null

# setup for cxi-ds1 detector

# name of the detector
detname=ds1

# the best known geometry file
bestgeom=cspad-example.geom

# make a cheetah geometry file from the crystfel geometry file
make_pixelmap $bestgeom
mv pixelmap.h5 $bestgeom.h5

# make default links for this detector
ln -s $bestgeom default-$detname.geom
ln -s $bestgeom.h5 default-$detname.h5

# default links for unknown detector
ln -s default-ds1.geom default.geom
ln -s default-ds1.h5 default.h5

# add more detectors as needed
