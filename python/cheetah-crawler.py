#!/usr/bin/env python3
#
#   cxiview
#   A python/Qt replacement for the IDL cheetah crawler
#

import os
import sys
import csv
import argparse
import PyQt4.QtCore
import PyQt4.QtGui

import UI.crawler_ui
import lib.crawler_slac as location
import lib.crawler_hdf5 as cheetah
import lib.crawler_merge as merge


#
#	CXI viewer code
#
class cheetah_crawler(PyQt4.QtGui.QMainWindow):

    #
    #   Actions to be done each refresh cycle
    #
    def refresh(self):
        # Button is busy
        self.ui.refreshButton.setEnabled(False)

        # Crawler data (facility dependent)
        self.ui.statusBar.setText('Scanning data files')
        self.ui.progressBar.setValue(0)
        location.scan_data(self.data_dir)


        # Crawler HDF5 (facility independent)
        self.ui.statusBar.setText('Scanning HDF5 files')
        self.ui.progressBar.setValue(25)
        cheetah.scan_hdf5(self.hdf5_dir)


        # Crawler Index (facility independent)
        self.ui.statusBar.setText('Scanning indexing results')
        self.ui.progressBar.setValue(50)


        # Crawler merge (facility independent)
        self.ui.statusBar.setText('Merging results')
        self.ui.progressBar.setValue(75)
        merge.crawler_merge()


        # Automatic update interval of 1 minute
        self.ui.statusBar.setText('Ready')
        self.ui.progressBar.setValue(100)
        self.ui.refreshButton.setEnabled(True)
        self.refresh_timer.start(60000)
    #end refresh()





    #
    #	Initialisation function
    #
    def __init__(self, args):
        #
        # Extract info from command line arguments
        #
        self.location = args.l
        self.data_dir = args.d
        self.hdf5_dir = args.c

        #
        # Set up the UI
        #
        super(cheetah_crawler, self).__init__()
        self.ui = UI.crawler_ui.Ui_MainWindow()
        self.ui.setupUi(self)
        self.ui.refreshButton.clicked.connect(self.refresh)
        self.refresh_timer = PyQt4.QtCore.QTimer()
        self.refresh_timer.timeout.connect(self.refresh)

        self.ui.statusBar.setText('Ready')
        self.ui.progressBar.setValue(100)

        #
        # Do the 1st scan
        #
        self.refresh()
    #end __init()__

#end cheetah_crawler()

        
#
#	Main function defining this as a program to be called
#
if __name__ == '__main__':
    
    #
    #   Use parser to process command line arguments
    #    
    parser = argparse.ArgumentParser(description='CFEL cheetah crawler')
    parser.add_argument("-l", default="none", help="Location (LCLS, P11)")
    parser.add_argument("-d", default="none", help="Data directory (XTC, CBF, etc)")
    parser.add_argument("-c", default="none", help="Cheetah HDF5 directory")
    args = parser.parse_args()
    
    print("----------")    
    print("Parsed command line arguments")
    print(args)
    print("----------")    
    
    # This bit may be irrelevent if we can make parser.parse_args() require this field    
    if args.l == "none" or args.d == "none" or args.c == "none":
        print('Usage: cheetah-crawler.py -l location (LCLS, P11) -d data_directory -c cheetah_hdf5_directory')
        sys.exit()
    #endif        


    #
    #   Spawn the viewer
    #        
    app = PyQt4.QtGui.QApplication(sys.argv)    
        
    ex = cheetah_crawler(args)
    ex.show()
    ret = app.exec_()

    
    #
    # Cleanup on exit    
    #
    app.exit()
    
    # This function does the following in an attempt to ‘safely’ terminate the process:
    #   Invoke atexit callbacks
    #   Close all open file handles
    os._exit(ret)
    sys.exit(ret)
#end __main__