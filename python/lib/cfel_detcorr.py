# -*- coding: utf-8 -*-
#
#	CFEL image handling tools
#	Anton Barty
#

import numpy
import pyqtgraph
import lib.cfel_filetools as cfel_file
import lib.cfel_cspad as cfel_cspad


#
#   Determine bad pixel mask from darkcal
#
#   Basic algorithm simple but needs to be modified for different detectors
#
def badpix_from_darkcal(filename="", qtmainwin=None):


    # Use dialog_pickfile() if no filename is provided
    if filename is "":
        filename = cfel_file.dialog_pickfile(filter='*.h5', qtmainwin=qtmainwin)
        if filename is "":
            return



    # Determine what type of detector we are using
    # CsPad only for now - make fancy later (or use a selection box)
    detector = 'cspad'




    # Call routine for appropriate detector system
    if detector == 'cspad':
        mask = cfel_cspad.badpix_from_darkcal(filename)




    # Display mask (pyqtgraph.show)
    temp = mask.astype(int)
    pyqtgraph.image(temp, levels=(0,1))
    #pg.imageView.ui.menuBtn.hide()
    #pg.imageView.ui.roiBtn.hide()


    # Save mask
    outfile = cfel_file.dialog_pickfile(write=True, path='../calib/mask', filter='*.h5', qtmainwin=qtmainwin)
    if outfile is not '':
        print("Saving mask: ", outfile)
        cfel_file.write_h5(mask, outfile, field='data/data')

