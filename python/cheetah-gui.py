#!/usr/bin/env python3
#
#   cheetah-gui
#   A python/Qt replacement for the IDL cheetah GUI
#

import os
import sys
import glob
import argparse
import datetime
import subprocess
import PyQt4.QtCore
import PyQt4.QtGui

import UI.cheetahgui_ui
import lib.cfel_filetools as cfel_file
import lib.gui_dialogs as gui_dialogs

#TODO: Cheetah GUI
#TODO: Setup new experiment function
#TODO: Relabel datasets (including directory renaming and database update)

#TODO: cxiview.py
#TODO: Sensible behaviour when geometry not specified (display without geometry applied)
#TODO: Do not display resolution rings if wavelength, z, or geometry not defined
#TODO: Wavelength and z on command line (optional)
#TODO: Take file list as an argument

#
#	Cheetah GUI code
#
class cheetah_gui(PyQt4.QtGui.QMainWindow):

    #
    # Launch a subprocess (eg: viewer or analysis script) without blocking the GUI
    # Separate routine makes it easy change this globally if needed
    #
    def spawn_subprocess(self, cmdarr, wait=False, test=False):
        command = str.join(' ', cmdarr)
        print(command)

        if test:
            return

        if wait:
            subprocess.run(cmdarr)
        else:
            subprocess.Popen(cmdarr)


    #
    # Quick wrapper for viewing images (this code got repeated over-and-over)
    #
    def show_selected_images(self, filepat, field='data/data'):
        runs = self.selected_runs()

        # No run selected
        if len(runs['run']) == 0:
            return
        file = runs['path'][0] + filepat

        # Display if some file matches pattern (avoids error when no match)
        if len(glob.glob(file)) != 0:
            cmdarr = ['cxiview.py', '-g', self.config['geometry'], '-e', field, '-i', file]
            self.spawn_subprocess(cmdarr)
        else:
            print("File does not seem to exist:")
            print(file)
    #end show_selected_images()



    #
    #   Actions to be done each refresh cycle
    #
    def refresh_table(self):
        print('Table refreshed at ', str(datetime.datetime.now()))

        # Button is busy
        self.ui.button_refresh.setEnabled(False)

        # Load the table data
        status = cfel_file.csv_to_dict('crawler.txt')

        # Fix some legacy issues with old crawler.txt file format (different key names in crawler.txt)
        if not 'Run' in status.keys() and '#Run' in status.keys():
            status.update({'Run': status['#Run']})
            status.update({'H5Directory': status['H5 Directory']})
            del status['#Run']
            del status['H5 Directory']
            status['fieldnames'][status['fieldnames'].index('#Run')] = 'Run'
            status['fieldnames'][status['fieldnames'].index('H5 Directory')] = 'H5Directory'

        # Remember table for later use in other functions
        ncols = len(list(status.keys()))-1
        self.crawler_txt = status

        # Length of first list is number of rows - except when it is the fieldnames list
        #nrows = len(status[list(status.keys())[0]])
        nrows = len(status['Run'])
        self.table.setRowCount(nrows)
        self.table.setColumnCount(ncols)
        self.table.updateGeometry()

        # Populate the table
        numbercols = [0]
        for col, key in enumerate(status['fieldnames']):
            for row, item in enumerate(status[key]):

                #if col in numbercols:
                if item.isnumeric():
                    newitem = PyQt4.QtGui.QTableWidgetItem()
                    newitem.setData(PyQt4.QtCore.Qt.DisplayRole, float(item))
                else:
                    newitem = PyQt4.QtGui.QTableWidgetItem(item)

                self.table.setItem(row,col,newitem)

        # Table fiddling
        #TODO: Make columns resizeable
        self.table.setWordWrap(False)
        self.table.setHorizontalHeaderLabels(status['fieldnames'])
        self.table.verticalHeader().setVisible(False)
        #self.table.resizeColumnsToContents()
        #self.table.horizontalHeader().setSectionResizeMode(PyQt4.QtGui.QHeaderView.Interactive)
        self.table.resizeRowsToContents()
        self.table.show()

        # Button is no longer busy; set timer for next refresh
        self.ui.button_refresh.setEnabled(True)
        self.refresh_timer.start(60000)

    #end refresh()


    #
    #   Determine selected runs
    #   Return run number, datasetID, and directory
    #   Use actual table entries to handle sorting
    #
    def selected_runs(self):
        # Option 1
        # Rows where all columns are selected (entire row must be selected)
        #indexes = self.table.selectionModel().selectedRows()
        #for index in sorted(indexes):
        #    print('1: Row %d is selected' % index.row())

        # Option 2
        # Rows where at least one cell is selected (any box in row is selected)
        rows = sorted(set(index.row() for index in self.table.selectedIndexes()))
        #for row in rows:
        #    print('2: Row %d is selected' % row)


        # Extract info from selected rows
        run_out = []
        dataset_out = []
        directory_out = []
        for row in rows:
            #print("Row: ", row)
            run = self.table.item(row,0).text()
            dataset = self.table.item(row,1).text()
            directory = self.table.item(row,5).text()
            #print("Run is: ", run)
            #print("Dataset is: ", dataset)
            #print("Directory is: ", directory)
            run_out.append(run)
            dataset_out.append(dataset)
            directory_out.append(directory)

        # Relatve path for data directories (simplifies later lookup - we mostly want to know where to look for the data)
        path_out = []
        for dir in directory_out:
            path = self.config['hdf5dir'] + '/' + dir + '/'
            path_out.append(path)

        # Return value
        result = {
            'row' : rows,
            'run' : run_out,
            'dataset' : dataset_out,
            'directory' : directory_out,
            'path' : path_out
        }
        return result
    #end selected_runs


    #
    #   Read list of past experiments
    #
    def list_experiments(self):

        expfile = os.path.expanduser('~/.cheetah-crawler')
        #expfile = '~/.cheetah-crawler'
        #expfile = './cheetah-crawler'

        # Does it exist?
        if os.path.isfile(expfile):
            f = open(expfile)
            exptlist = f.readlines()
            f.close()
        else:
            print('File not found: ', expfile)
            exptlist = []

        # Remove carriage returns
        for i in range(len(exptlist)):
            exptlist[i] = exptlist[i].strip()

        return exptlist
    #end list_experiments

    #
    #   Set up a new experiment based on template and selected directory
    #   Uses LCLS schema; will need modification to run anywhere else; do this later
    #
    def setup_new_experiment(self):
        dir = cfel_file.dialog_pickfile(directory=True)
        if dir == '':
            self.exit_gui()
        print('Selected directory: ')
        os.chdir(dir)

        #   Deduce experiment number, etc using de-referenced paths
		#   Assumes the file path follows the pattern:   /reg/d/psdm/cxi/cxij4915/scratch/...
        realdir = os.getcwd()

        #   Now for some LCLS-specific stuff
        ss = realdir.split('/')
        ss = ss[1:]

        ss[1] = 'd'
        ss[2] = 'psdm'
        instr = ss[3]
        expt = ss[4]
        xtcdir = '/' + str.join('/', ss[0:5]) + '/xtc'
        userdir = '/' + str.join('/', ss) + '/cheetah'

        print('Deduced experiment information:')
        print('    Relative path: ', dir)
        print('    Absolute path: ', realdir)
        print('    Instrument: ', instr)
        print('    Experiment: ', expt)
        print('    XTC directory: ', xtcdir)
        print('    Output directory: ', userdir)


        #
        # QMessageBox for confirmation before proceeding
        #
        msgBox = PyQt4.QtGui.QMessageBox()
        str1 = []
        str1.append('<b>Instrument:</b> ' + str(instr))
        str1.append('<b>Experiment:</b> ' + str(expt))
        str1.append('<b>XTC directory:</b> ' + str(xtcdir))
        str1.append('<b>Output directory:</b> ' + str(userdir))
        #str1.append('<b>Relative path:</b> '+ str(dir))
        str2 = str.join('<br>', str1)
        msgBox.setText(str2)

        msgBox.setInformativeText('<b>Proceed?</b>')
        msgBox.addButton(PyQt4.QtGui.QMessageBox.Yes)
        msgBox.addButton(PyQt4.QtGui.QMessageBox.Cancel)
        msgBox.setDefaultButton(PyQt4.QtGui.QMessageBox.Yes)
        ret = msgBox.exec_();

        if ret == PyQt4.QtGui.QMessageBox.Cancel:
            print("So long and thanks for all the fish.")
            self.exit_gui()


        # Unpack template
        print('>---------------------<')
        print('Extracting template...')
        cmd = ['tar','-xf','/reg/g/cfel/cheetah/template.tar']
        self.spawn_subprocess(cmd, wait=True)
        print("Done")

        # Fix permissions
        print('>---------------------<')
        print('Fixing permissions...')
        cmd = ['chgrp',  '-R', expt, 'cheetah/']
        self.spawn_subprocess(cmd, wait=True)
        cmd = ['chmod',  '-R', 'g+w', 'cheetah/']
        self.spawn_subprocess(cmd, wait=True)
        print("Done")

        # Place configuration into /res
        #print, 'Placing configuration files into /res...'
        #cmd = ['/reg/g/cfel/cheetah/cheetah-stable/bin/make-labrynth']
        #self.spawn_subprocess(cmd, wait=True)
        #print("Done")


        # Modify gui/crawler.config
        print('>---------------------<')
        file = 'cheetah/gui/crawler.config'
        print('Modifying ', file)

        # Replace XTC directory with the correct location using sed
        xtcsedstr = str.replace(xtcdir, '/', '\\/')
        cmd = ["sed", "-i", "-r", "s/(xtcdir=).*/\\1" + xtcsedstr + "/", file]
        self.spawn_subprocess(cmd, wait=True)

        print('>-------------------------<')
        cmd = ['cat', file]
        self.spawn_subprocess(cmd, wait=True)
        print('>-------------------------<')


        # Modify process/process
        file = 'cheetah/process/process'
        print('Modifying ', file)

        cmd = ["sed", "-i", "-r", "s/(expt=).*/\\1\"" + expt + "\"/", file]
        self.spawn_subprocess(cmd, wait=True)

        xtcsedstr = str.replace(xtcdir, '/', '\\/')
        cmd = ["sed", "-i", "-r", "s/(XTCDIR=).*/\\1\"" + xtcsedstr + "\"/", file]
        self.spawn_subprocess(cmd, wait=True)

        h5sedstr = str.replace(userdir, '/', '\\/') + '\/hdf5'
        cmd = ["sed", "-i", "-r", "s/(H5DIR=).*/\\1\"" + h5sedstr + "\"/", file]
        self.spawn_subprocess(cmd, wait=True)

        confsedstr= str.replace(userdir, '/', '\\/') + '\/process'
        cmd = ["sed", "-i", "-r", "s/(CONFIGDIR=).*/\\1\"" + confsedstr + "\"/", file]
        self.spawn_subprocess(cmd, wait=True)

        print('>-------------------------<')
        cmd = ['head', file]
        self.spawn_subprocess(cmd, wait=True)
        print('>-------------------------<')

        print("Working directory: ")
        print(os.getcwd() + '/cheetah')
    #end setup_new_experiment


    #
    #   Select an experiment, or find a new one
    #
    def select_experiment(self):

        # Dialog box with list of past experiments
        past_expts = self.list_experiments()
        gui = gui_dialogs.expt_select_gui.get_expt(past_expts)

        if gui['action'] == 'goto':
            dir = gui['selected_expt']
            return dir


        elif gui['action'] == 'find':
            cfile = cfel_file.dialog_pickfile(filter='crawler.config')
            if cfile == '':
                print('Selection canceled')
                self.exit_gui()

            basename = os.path.basename(cfile)
            dir = os.path.dirname(cfile)

            # Update the past experiments list
            past_expts.insert(0,dir)
            expfile = os.path.expanduser('~/.cheetah-crawler')
            with open(expfile, mode='w') as f:
                f.write('\n'.join(past_expts))
            return dir


        elif gui['action'] == 'setup_new':
            self.setup_new_experiment()
            cwd = os.getcwd()
            dir = cwd + '/cheetah/gui'

            # Update the past experiments list
            past_expts.insert(0,dir)
            expfile = os.path.expanduser('~/.cheetah-crawler')
            with open(expfile, mode='w') as f:
                f.write('\n'.join(past_expts))
            return dir

        else:
            print("Catch you another time.")
            self.exit_gui()
            return ''




    #
    #   Action button items
    #
    def run_cheetah(self):

        # Find .ini files for dropdown list
        inifile_list = []
        for file in glob.iglob('../process/*.ini'):
            basename = os.path.basename(file)
            inifile_list.append(basename)
        #inifile_list = ['test1.ini','test2.ini']

        # Info needed for the dialog box
        dialog_info = {
            'inifile_list' : inifile_list,
            'lastini' : self.lastini,
            'lasttag' : self.lasttag
        }
        # Dialog box for dataset label and ini file
        gui, ok = gui_dialogs.run_cheetah_gui.cheetah_dialog(dialog_info)

        # Extract values from return dict
        dataset = gui['dataset']
        inifile = gui['inifile']

        # Exit if cancel was pressed
        if ok == False:
            return

        dataset_csv = cfel_file.csv_to_dict('datasets.csv')

        self.lasttag = dataset
        self.lastini = inifile

        # Process all selected runs
        runs = self.selected_runs()
        for i, run in enumerate(runs['run']):
            print('------------ Start Cheetah process script ------------')
            cmdarr = [self.config['process'], run, inifile, dataset]
            self.spawn_subprocess(cmdarr)

            # Format directory string
            dir = 'r{:04d}'.format(int(run))
            dir += '-'+dataset


            #Update Dataset and Cheetah status in table
            table_row = runs['row'][i]
            self.table.setItem(table_row, 1, PyQt4.QtGui.QTableWidgetItem(dataset))
            self.table.setItem(table_row, 5, PyQt4.QtGui.QTableWidgetItem(dir))
            self.table.setItem(table_row, 3, PyQt4.QtGui.QTableWidgetItem('Submitted'))

            # Update dataset file
            if run in dataset_csv['Run']:
                ds_indx = dataset_csv['Run'].index(run)
                dataset_csv['DatasetID'][ds_indx] = dataset
                dataset_csv['Directory'][ds_indx] = dir
                dataset_csv['iniFile'][ds_indx] = inifile
            else:
                dataset_csv['Run'].append(run)
                dataset_csv['DatasetID'].append(dataset)
                dataset_csv['Directory'].append(dir)
                dataset_csv['iniFile'].append(inifile)
            print('------------ Finish Cheetah process script ------------')

        # Sort dataset file to keep it in order


        # Save datasets file
        keys_to_save = ['Run', 'DatasetID', 'Directory', 'iniFile']
        cfel_file.dict_to_csv('datasets.csv', dataset_csv, keys_to_save)
    #end run_cheetah()


    def run_crystfel(self):
        print("Run crystfel selected")

    def view_hits(self):
        file = '*.cxi'
        field = '/entry_1/data_1/data'
        self.show_selected_images(file, field)
    #end view_hits()


    def view_powder(self):
        self.show_powder_hits_det()
    #end view_powder()

    def view_peakogram(self):
        self.show_peakogram()
    #end view_peakogram()


    #
    #   Cheetah menu items
    #
    def enableCommands(self):
        self.ui.button_runCheetah.setEnabled(True)
        self.ui.menu_file_startcrawler.setEnabled(True)
        self.ui.menu_cheetah_processselected.setEnabled(True)
        self.ui.menu_cheetah_autorun.setEnabled(True)
        self.ui.menu_cheetah_relabel.setEnabled(True)
    #end enableCommands()

    def start_crawler(self):
        cmdarr = ['cheetah-crawler.py', '-l', 'LCLS', '-d', self.config['xtcdir'], '-c', self.config['hdf5dir']]
        self.spawn_subprocess(cmdarr)


    def relabel_dataset(self):

        # Simple dialog box: http: // www.tutorialspoint.com / pyqt / pyqt_qinputdialog_widget.htm
        text, ok = PyQt4.QtGui.QInputDialog.getText(self, 'Change dataset label', 'New label:')
        if ok == False:
            return
        newlabel = str(text)
        print('New label is: ', newlabel)

        dataset_csv = cfel_file.csv_to_dict('datasets.csv')

        # Label all selected runs
        runs = self.selected_runs()
        for i, run in enumerate(runs['run']):

            # Format directory string
            olddir = runs['directory'][i]
            newdir = '---'

            if olddir != '---':
                newdir = 'r{:04d}'.format(int(run))
                newdir += '-' + newlabel

            # Update Dataset in table
            table_row = runs['row'][i]
            self.table.setItem(table_row, 1, PyQt4.QtGui.QTableWidgetItem(newlabel))
            self.table.setItem(table_row, 5, PyQt4.QtGui.QTableWidgetItem(newdir))

            # Update dataset file
            if run in dataset_csv['Run']:
                ds_indx = dataset_csv['Run'].index(run)
                dataset_csv['DatasetID'][ds_indx] = newlabel
                dataset_csv['Directory'][ds_indx] = newdir
            else:
                dataset_csv['Run'].append(run)
                dataset_csv['DatasetID'].append(newlabel)
                dataset_csv['Directory'].append(newdir)
                dataset_csv['iniFile'].append('---')

            # Rename the directory
            if olddir != '---':
                cmdarr = ['mv', self.config['hdf5dir']+'/'+olddir, self.config['hdf5dir']+'/'+newdir]
                self.spawn_subprocess(cmdarr)


        # Sort dataset file to keep it in order
        # Save datasets file
        keys_to_save = ['Run', 'DatasetID', 'Directory', 'iniFile']
        cfel_file.dict_to_csv('datasets.csv', dataset_csv, keys_to_save)



    def autorun(self):
        print("Autorun selected")

    def set_new_geometry(self):
        gfile = cfel_file.dialog_pickfile(path='../calib/geometry', filter='Geometry files (*.h5 *.geom);;All files (*.*)')
        if gfile == '':
            return
        gfile = os.path.relpath(gfile)
        print('Selected geometry file:')
        print(gfile)
        self.config['geometry'] = gfile


    #
    #   Mask menu items
    #
    def maskmaker(self):
        print("Mask maker selected")
        print("Talk to Andrew Morgan to add his mask maker")

    def badpix_from_darkcal(self):
        print("Badpix from darkcal selected")

    def badpix_from_bright(self):
        print("Badpix from bright selected")

    def combine_masks(self):
        print("Combine masks selected")

    def show_mask_file(self):
        file = cfel_file.dialog_pickfile(path='../calib/mask', filter='*.h5')
        field = 'data/data'
        if file != '':
            file = os.path.relpath(file)
            self.show_selected_images(file, field)

    #
    #   Analysis menu items
    #
    def show_hitrate(self):
        print("Show hitrate not yet implemented")

    def show_peakogram(self):
        runs = self.selected_runs()
        if len(runs['run']) == 0:
            return;
        pkfile = runs['path'][0] + 'peaks.txt'
        cmdarr = ['peakogram.py', '-i', pkfile]
        self.spawn_subprocess(cmdarr)
    #end show_peakogram

    def show_resolution(self):
        print("Show resolution not yet implemented")

    def show_saturation(self):
        self.show_peakogram()


    #
    #   Powder menu items
    #
    def show_powder_hits(self):
        file = '*detector0-class1-sum.h5'
        field = 'data/non_assembled_detector_and_photon_corrected'
        self.show_selected_images(file, field)

    def show_powder_blanks(self):
        file = '*detector0-class0-sum.h5'
        field = 'data/non_assembled_detector_and_photon_corrected'
        self.show_selected_images(file, field)

    def show_powder_hits_det(self):
        file = '*detector0-class1-sum.h5'
        field = 'data/non_assembled_detector_corrected'
        self.show_selected_images(file, field)

    def show_powder_blanks_det(self):
        file = '*detector0-class0-sum.h5'
        field = 'data/non_assembled_detector_corrected'
        self.show_selected_images(file, field)

    def show_powder_peaks_hits(self):
        file = '*detector0-class1-sum.h5'
        field = 'data/peakpowder'
        self.show_selected_images(file, field)

    def show_powder_peaks_blanks(self):
        file = '*detector0-class0-sum.h5'
        field = 'data/peakpowder'
        self.show_selected_images(file, field)

    def show_darkcal(self):
        file = '*detector0-darkcal.h5'
        field = 'data/data'
        self.show_selected_images(file, field)

    def copy_darkcal(self):
        runs = self.selected_runs()
        if len(runs['run']) == 0:
            return;
        for path in runs['path']:
            path += '*detector0-darkcal.h5'
            for dkcal in glob.iglob(path):
                cmdarr = ['cp', dkcal, '../calib/darkcal/.']
                self.spawn_subprocess(cmdarr)



    #
    # Log menu actions
    #
    def view_batch_log(self):
        print("View batch log selected")

    def view_cheetah_log(self):
        print("View cheetah log selected")

    def view_cheetah_status(self):
        print("View cheetah status selected")



    #
    #   Parse crawler.config file
    #
    def parse_config(self):
        configfile = 'crawler.config'

        # Double-check again that the file exists
        if not os.path.isfile(configfile):
            print("Uh oh - there does not seem to be a crawler.config file in this directory.")
            print("Please check")
            self.exit_gui()

        config= {}
        with open(configfile) as f:
            for line in f:
                name, var = line.partition("=")[::2]
                config[name.strip()] = var.strip()


        # Trap some legacy case with fewer keys in file
        if not 'cheetahini' in config.keys():
            config.update({'cheetahini': 'darkcal.ini'})
        if not 'cheetahtag' in config.keys():
            config.update({'cheetahtag': 'darkcal'})


        return config
    #end parse_config



    #
    #   Try to avoid the worst of the ugly Linux GUI layouts
    #
    def change_skins(self):

        styles = PyQt4.QtGui.QStyleFactory.keys()
        #print("Available Qt4 styles: ", styles)

        style = styles[-1]
        #print("Setting Qt4 style: ", style)

        try:
            PyQt4.QtGui.QApplication.setStyle(PyQt4.QtGui.QStyleFactory.create(style))
        except:
            print("Style not available:", style)
    #end change_skins


    #
    #   Exit cleanly
    #
    def exit_gui(self):
        print("Bye bye.")
        app.exit()
        os._exit(1)
        sys.exit(1)
    #end exit_gui




    #
    #	GUI Initialisation function
    #
    def __init__(self, args):

        # Extract info from command line arguments
        #self.hdf5_dir = args.c

        #
        # Set up the UI
        #
        super(cheetah_gui, self).__init__()
        self.ui = UI.cheetahgui_ui.Ui_MainWindow()
        self.ui.setupUi(self)
        self.ui.menuBar.setNativeMenuBar(False)
        self.table = self.ui.table_status
        self.table.horizontalHeader().setDefaultSectionSize(75)
        self.table.horizontalHeader().setResizeMode(PyQt4.QtGui.QHeaderView.Interactive)
        #self.table.horizontalHeader().setResizeMode(PyQt4.QtGui.QHeaderView.Stretch)
        self.table.setSortingEnabled(True)
        #self.change_skins()



        # Experiment selector (if not already in a gui directory)
        if not os.path.isfile('crawler.config'):
            expdir = self.select_experiment()
            print("Thank you.")
            print("Moving to working directory:")
            print(expdir)
            try:
                os.chdir(expdir)
            except:
                print("Uh oh - it looks like that directory does not exist any more.")
                print("It may have been moved or deleted.  Plesae check it still exists.")
                self.exit_gui()


        # Parse configuration file
        print("Loading configuration file: ./crawler.config")
        self.config = self.parse_config()
        self.lastini = self.config['cheetahini']
        self.lasttag = self.config['cheetahtag']

        # Update window title
        dir = os.getcwd()
        title = 'Cheetah GUI: ' + dir
        self.setWindowTitle(title)


        # Connect front panel buttons to actions
        self.ui.button_refresh.clicked.connect(self.refresh_table)
        self.ui.button_runCheetah.clicked.connect(self.run_cheetah)
        self.ui.button_index.clicked.connect(self.run_crystfel)
        self.ui.button_viewhits.clicked.connect(self.view_hits)
        self.ui.button_virtualpowder.clicked.connect(self.view_powder)
        self.ui.button_peakogram.clicked.connect(self.view_peakogram)

        # File menu actions
        self.ui.menu_file_startcrawler.triggered.connect(self.start_crawler)
        self.ui.menu_file_refreshtable.triggered.connect(self.refresh_table)
        self.ui.menu_file_newgeometry.triggered.connect(self.set_new_geometry)

        # Cheetah menu actions
        self.ui.menu_cheetah_processselected.triggered.connect(self.run_cheetah)
        self.ui.menu_cheetah_relabel.triggered.connect(self.relabel_dataset)
        self.ui.menu_cheetah_autorun.triggered.connect(self.autorun)

        # Mask menu actions
        self.ui.menu_mask_maker.triggered.connect(self.maskmaker)
        self.ui.menu_mask_badpixdark.triggered.connect(self.badpix_from_darkcal)
        self.ui.menu_mask_badpixbright.triggered.connect(self.badpix_from_bright)
        self.ui.menu_mask_combine.triggered.connect(self.combine_masks)
        self.ui.menu_mask_view.triggered.connect(self.show_mask_file)
        self.ui.menu_mask_makecspadgain.setEnabled(False)
        self.ui.menu_mask_translatecspadgain.setEnabled(False)

        # Analysis menu items
        self.ui.menu_analysis_hitrate.triggered.connect(self.show_hitrate)
        self.ui.menu_analysis_peakogram.triggered.connect(self.show_peakogram)
        self.ui.menu_analysis_resolution.triggered.connect(self.show_resolution)
        self.ui.menu_analysis_saturation.triggered.connect(self.show_saturation)

        # Powder menu actions
        self.ui.menu_powder_hits.triggered.connect(self.show_powder_hits)
        self.ui.menu_powder_blank.triggered.connect(self.show_powder_blanks)
        self.ui.menu_powder_hits_det.triggered.connect(self.show_powder_hits_det)
        self.ui.menu_powder_blank_det.triggered.connect(self.show_powder_blanks_det)
        self.ui.menu_powder_peaks_hits.triggered.connect(self.show_powder_peaks_hits)
        self.ui.menu_powder_peaks_blank.triggered.connect(self.show_powder_peaks_blanks)
        self.ui.menu_powder_darkcal.triggered.connect(self.show_darkcal)
        self.ui.menu_powder_copydarkcal.triggered.connect(self.copy_darkcal)

        # Log menu actions
        self.ui.menu_log_batch.triggered.connect(self.view_batch_log)
        self.ui.menu_log_cheetah.triggered.connect(self.view_cheetah_log)
        self.ui.menu_log_cheetahstatus.triggered.connect(self.view_cheetah_status)
        self.ui.menu_log_cheetah.setEnabled(False)
        self.ui.menu_log_cheetahstatus.setEnabled(False)


        # Disable action commands until enabled
        #self.ui.button_runCheetah.setEnabled(False)
        self.ui.button_index.setEnabled(False)
        self.ui.menu_file_startcrawler.setEnabled(False)
        self.ui.menu_cheetah_processselected.setEnabled(False)
        self.ui.menu_cheetah_autorun.setEnabled(False)
        self.ui.menu_cheetah_relabel.setEnabled(False)
        self.ui.menu_file_command.triggered.connect(self.enableCommands)


        # Timer for auto-refresh
        self.refresh_timer = PyQt4.QtCore.QTimer()
        self.refresh_timer.timeout.connect(self.refresh_table)


        #
        # Populate the table
        #
        self.refresh_table()
    #end __init()__

#end cheetah_gui()

        
#
#	Main function defining this as a program to be called
#
if __name__ == '__main__':
    
    #
    #   Use parser to process command line arguments
    #    
    parser = argparse.ArgumentParser(description='CFEL cheetah GUI')
    #parser.add_argument("-l", default="none", help="Location (LCLS, P11)")
    #parser.add_argument("-d", default="none", help="Data directory (XTC, CBF, etc)")
    #parser.add_argument("-c", default="none", help="Cheetah HDF5 directory")
    args = parser.parse_args()
    
    print("----------")    
    print("Parsed command line arguments")
    print(args)
    print("----------")    
    

    #
    #   Spawn the viewer
    #
    app = PyQt4.QtGui.QApplication(sys.argv)
        
    ex = cheetah_gui(args)
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