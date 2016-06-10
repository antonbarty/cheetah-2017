#!/reg/g/cfel/anaconda/bin/python3
#!/nfs/cfel/cxi/common/cfelsoft-rh7/anaconda-py3/bin/python3
#
#   CXIview
#   A python/Qt viewer for .cxi files (and other files output by Cheetah)
#   A replacement for the IDL Cheetah file viewer
#   Based on peak_viewer_cxi by Valerio Mariani, CFEL, December 2015
#
#   Tested using Anaconda / Python 3.4
#
#   Top line (#!/reg/g/cfel/anaconda/bin/python3) needed for interface to IDL GUI at SLAC to work properly 
#   !/Applications/anaconda/bin/python3

import argparse
import os
import sys
import numpy

import PyQt4.QtCore
import PyQt4.QtGui
import pyqtgraph
import scipy.constants

import UI.cxiview_ui
import lib.cfel_filetools as cfel_file
import lib.cfel_geometry as cfel_geom
import lib.cfel_imgtools as cfel_img


#
#	CXI viewer code
#
class cxiview(PyQt4.QtGui.QMainWindow):
    
    #
    # display the main image
    #
    def draw_things(self):
        
        # Retrieve CXI file data all at once
        # (Saves opening and closing file many times)
        #cxi = cfel_file.read_cxi(self.filename, self.img_index,  data=True, photon_energy=True, camera_length=True, mask=self.show_masks, peaks=self.show_found_peaks)
        #cxi = cfel_file.read_cxi(self.event_list['filename'][self.img_index], self.event_list['event'][self.img_index],  data=True, photon_energy=True, camera_length=True, mask=self.show_masks, peaks=self.show_found_peaks)
        cxi = cfel_file.read_event(self.event_list, self.img_index,  data=True, photon_energy=True, camera_length=True, mask=self.show_masks, peaks=self.show_found_peaks)



        # Retrieve resolution related stuff
        self.photon_energy = cxi['photon_energy_eV']
        self.camera_length = cxi['EncoderValue']
        self.camera_length *= 1e-3
        if (self.photon_energy > 0):
            self.lambd = scipy.constants.h * scipy.constants.c /(scipy.constants.e * self.photon_energy)
        else:
            self.lambd = 1e-10
        
        # Set title 
        #title = str(self.img_index)+'/'+ str(self.num_lines) + ' - ' + self.filename
        #file_str = self.event_list['filename'][self.img_index]
        file_str = os.path.basename(self.event_list['filename'][self.img_index])
        title = file_str + ' #' + str(self.event_list['event'][self.img_index]) + ' - (' + str(self.img_index)+'/'+ str(self.num_lines) + ')'
        self.ui.jumpToLineEdit.setText(str(self.img_index))

        # Extract image to display
        # http://www.pyqtgraph.org/documentation/graphicsItems/imageitem.html
        img_data = cxi['data']
        self.img_to_draw = cfel_img.pixel_remap(img_data, self.geometry['x'], self.geometry['y'], dx=1.0)
        self.ui.imageView.setImage(self.img_to_draw, autoLevels=True, autoRange=False)

        # Histogram equalisation (saturate top 0.1% of pixels)   
        if self.histogram_clip == True:
            bottom, top  = cfel_img.histogram_clip_levels(img_data.ravel(),0.001)
            self.ui.imageView.setLevels(0,top) #, update=True)
        else:
            top = numpy.amax(img_data.ravel())
            self.ui.imageView.setLevels(0,top) #, update=True)
        
        # Set the histogram widget to auto-scale politely
        # http://www.pyqtgraph.org/documentation/graphicsItems/histogramlutitem.html#pyqtgraph.HistogramLUTItem
        if self.auto_levels == True:
            hist = self.ui.imageView.getHistogramWidget()
            hist.setHistogramRange(-100, 10000, padding=0.1)
        else:
            hist = self.ui.imageView.getHistogramWidget()
            hist.setHistogramRange(numpy.amin(img_data.ravel()), numpy.amax(img_data.ravel()), padding=0.1)
            



        # Static level
        #self.ui.imageView.setLevels(0,1000) #, update=True)


        # Edit the colour table (unduly complicated - edit the editor rather than load a table of values)
        # http://www.pyqtgraph.org/documentation/graphicsItems/gradienteditoritem.html#pyqtgraph.GradientEditorItem.__init__
        #self.ui.imageView.ui.GradientEditorItem.addTick(0.5,'#207020')
        #grad = self.ui.imageView.getGradientWidget()

        # Find gradient editor item reference
        #grad =  self.ui.imageView.ui.histogram.gradient
        
         #self.ui.imageview.ui.histogram.gradient.setColorMap(colormap)
        #col = grad.colorMap()
        #grad.addTick(0.5,color=(1.0,0.0,0.0,1.0))
        #grad.addTick(0.5,color='r')
        #grad.item.addTick(1,color='r')

        
        #map = histogram
        #map = self.ui.imageView.ui.gradient
        #map = self.ui.imageView.ui.TickSliderItem().listTicks();

        # Suggestion from https://groups.google.com/forum/#!topic/pyqtgraph/gEjC08Vb8NQ
        #pos = numpy.array([0.0, 0.5, 1.0])
        #color = numpy.array([[0,0,0,255], [255,128,0,255], [255,255,0,255]], dtype=numpy.ubyte)
        #map = pyqtgraph.ColorMap(pos, color)
        #lut = map.getLookupTable(0.0, 1.0, 256)
        #self.ui.imageView.ui.setLookupTable(lut)


        # Modifying the colour table - from Valerio - and it works
        #pos = numpy.array([0.0,0.5,1.0])
        #color = numpy.array([[255,255,255,255], [128,128,128,255], [0,0,0,255]], dtype=numpy.ubyte)
        #self.new_color_map = pyqtgraph.ColorMap(pos,color)
        #self.ui.imageView.ui.histogram.gradient.setColorMap(self.new_color_map)

        # Call colour tables by name        
        #self.ui.imageView.ui.histogram.gradient.loadPreset('idl4')
       
       
        # Draw pixel mask overlay
        if self.show_masks == True:
            #mask_from_file = read_cxi(self.filename, self.img_index, mask=True)
            mask_from_file = cxi['mask']
            bitmask = 0xFFFF

            mask_img = cfel_img.pixel_remap(mask_from_file, self.geometry['x'], self.geometry['y'], dx=1.0)
            mask_img = numpy.int_(mask_img)
            w = (mask_img & bitmask) != 0
            mask_img[w] = 255
            mask_img[~w] = 0
            
            self.mask_to_draw = numpy.zeros(mask_img.shape+(4,), dtype=numpy.uint8)
            self.mask_to_draw[:,:,0] = mask_img
            self.mask_to_draw[:,:,3] = 0.7*mask_img
            self.mask_view.setImage(self.mask_to_draw, autoLevels=False, autoRange=False, opacity=1.0)
        else:
            self.mask_to_draw = numpy.zeros(self.img_shape+(4,), dtype=numpy.uint8)
            self.mask_view.setImage(self.mask_to_draw, autoLevels=False, autoRange=False, opacity=0.0)
       

        # Draw found peaks
        if self.show_found_peaks == True:

            peak_x = []
            peak_y = []

            # Read peaks in raw coordinates
            n_peaks = cxi['n_peaks']
            peak_x_data = cxi['peakXPosRaw']
            peak_y_data = cxi['peakYPosRaw']
            
            for ind in range(0,n_peaks):                
                peak_fs = peak_x_data[ind]                
                peak_ss = peak_y_data[ind]         
                
                # Peak coordinate to pixel in image
                peak_in_slab = int(round(peak_ss))*self.slab_shape[1]+int(round(peak_fs))
                peak_x.append(self.geometry['x'][peak_in_slab] + self.img_shape[0] / 2)
                peak_y.append(self.geometry['y'][peak_in_slab] + self.img_shape[1] / 2)

            ring_pen = pyqtgraph.mkPen('r', width=2)
            self.found_peak_canvas.setData(peak_x, peak_y, symbol = 'o', size = 10, pen = ring_pen, brush = (0,0,0,0), pxMode = False)

        else:
            self.found_peak_canvas.setData([])

        # Draw predicted peaks
        """
        if self.show_predicted_peaks == True:

            peak_x = []
            peak_y = []
            
            for ind in range(0,n_peaks):                
                peak_fs = peak_x_data[ind]                
                peak_ss = peak_y_data[ind]         
                
                # Peak coordinate to pixel in image
                peak_in_slab = int(round(peak_ss))*self.slab_shape[1]+int(round(peak_fs))
                peak_x.append(self.pixel_map[0][peak_in_slab] + self.img_shape[0]/2)
                peak_y.append(self.pixel_map[1][peak_in_slab] + self.img_shape[1]/2)

            ring_pen = pyqtgraph.mkPen('b', width=2)
            self.predicted_peak_canvas.setData(peak_x, peak_y, symbol = 'o', size = 10, pen = ring_pen, brush = (0,0,0,0), pxMode = False)

        else:
            self.predicted_peak_canvas.setData([])
        """
        

        # Set title
        self.setWindowTitle(title)
  
    #end draw_things()
    

    #
    # Go to the previous pattern 
    #
    def previous_pattern(self):

        if self.img_index != 0:
            self.img_index -= 1
        else:
            self.img_index = self.num_lines-1
        self.draw_things()
    #end previous_pattern()


    #
    # Go to the next pattern 
    #
    def next_pattern(self):

        if self.img_index != self.num_lines-1:
            self.img_index += 1
        else:
            self.img_index = 0
        self.draw_things()
        
        #if self.play_mode == True:
        #    self.refresh_timer.start(1000)
    #end next_pattern()
    

    #
    # Go to random pattern
    #
    def random_pattern(self):
        pattern_to_jump = self.num_lines*numpy.random.random(1)[0]
        pattern_to_jump = pattern_to_jump.astype(numpy.int64)
        
        if 0<pattern_to_jump<self.num_lines:
            self.img_index = pattern_to_jump
            self.draw_things()
        else:
            self.ui.jumpToLineEdit.setText(str(self.img_index))
    #end random_pattern()
    

    #
    # Shuffle (play random patterns)
    #
    def shuffle(self):
        if self.shuffle_mode == False:
            self.shuffle_mode = True
            self.ui.shufflePushButton.setText("Stop")
            self.refresh_timer.timeout.connect(self.random_pattern)   
            self.random_pattern()
            self.refresh_timer.start(1000)

        else: 
            self.shuffle_mode = False
            self.ui.shufflePushButton.setText("Shuffle")
            self.refresh_timer.stop()
    #end shuffle()


    #
    # Play (display patterns in order)
    #
    def play(self):
        if self.play_mode == False:
            self.play_mode = True
            self.ui.playPushButton.setText("Stop")
            self.refresh_timer.timeout.connect(self.next_pattern)   
            self.next_pattern()
            self.refresh_timer.start(1000)

        else: 
            self.play_mode = False
            self.ui.playPushButton.setText("Play")
            self.refresh_timer.stop()
    #end play()


    #
    # Go to particular pattern
    #
    def jump_to_pattern(self):
        pattern_to_jump = int(self.ui.jumpToLineEdit.text())
        
        if 0<pattern_to_jump<self.num_lines:
            self.img_index = pattern_to_jump
            self.draw_things()
        else:
            self.ui.jumpToLineEdit.setText(str(self.img_index))
    #end jump_to_pattern()
        

    #
    # Toggle show or hide stuff
    #
    def showhidefoundpeaks(self, state):
        if state == PyQt4.QtCore.Qt.Checked:
            self.show_found_peaks = True
        else:
            self.show_found_peaks = False
        self.draw_things()
    #end showhidepeaks()

    def showhidepredictedpeaks(self, state):
        if state == PyQt4.QtCore.Qt.Checked:
            self.show_predicted_peaks = True
        else:
            self.show_predicted_peaks = False
        self.draw_things()
    #end showhidepredictedpeaks()

    def showhidemasks(self, state):
        if state == PyQt4.QtCore.Qt.Checked:
            self.show_masks = True
        else:
            self.show_masks = False
        self.draw_things()
    #end showhidemasks()

    def action_histclip(self, state):
        self.histogram_clip = state        
        self.draw_things()
    #end action_histclip()
        
    def action_autolevels(self, state):
        self.auto_levels = state
        self.draw_things()
    #end action_autolevels()



    #
    # Mouse clicked somewhere in the window
    #
    def mouse_clicked(self, event):
        pos = event[0].pos()
        if self.ui.imageView.getView().sceneBoundingRect().contains(pos):
            mouse_point = self.ui.imageView.getView().mapSceneToView(pos)
            x_mouse = int(mouse_point.x())
            y_mouse = int(mouse_point.y()) 
            x_mouse_centered = x_mouse - self.img_shape[0]/2 + 1
            y_mouse_centered = y_mouse - self.img_shape[0]/2 + 1
            radius = self.geometry['dx'] * numpy.sqrt(x_mouse_centered**2 + y_mouse_centered**2)
            self.camera_z = (self.camera_length+self.geometry['coffset'])
            camera_z_mm = self.camera_z * 1e3
            camera_z_in = camera_z_mm / 25.4
            resolution = 10e9*self.lambd/(2.0*numpy.sin(0.5*numpy.arctan(radius/(self.camera_length+self.geometry['coffset']))))

            self.ui.statusBar.setText(
                'Last clicked pixel:     x: %4i     y: %4i     value: %4i     z: %.2f mm     resolution: %4.2f Å' % (
                x_mouse_centered, y_mouse_centered, self.img_to_draw[x_mouse, y_mouse], camera_z_mm, resolution))

    #end mouse_clicked()


    #
    #   Saving and other file functions
    #
    def action_save_png(self):
        file_hint = os.path.basename(self.event_list['filename'][self.img_index])
        file_hint = os.path.splitext(file_hint)[0]
        file_hint += '-#'
        file_hint += str(self.event_list['event'][self.img_index])
        file_hint += '.png'
        file_hint = os.path.join(self.exportdir, file_hint)

        filename = cfel_file.dialog_pickfile(write=True, path=file_hint)
        if filename=='':
            return
        if filename.endswith('.png') == False:
            filename += '.png'
        print('Saving image to PNG: ', filename)
        self.exportdir = os.path.dirname(filename)

        # Using pypng, which doesn't seem to work in Python3 (???)
        #image = self.img_to_draw
        #cfel_img.write_png(filename, image)

        # Using pyQtGraph exporters
        # http://www.pyqtgraph.org/documentation/exporting.html
        exporter = pyqtgraph.exporters.ImageExporter(self.ui.imageView.getView())
        exporter.parameters()['height'] = numpy.max(self.img_to_draw.shape)
        exporter.export(filename)
    #end action_save_png()


    def action_update_files(self):
        self.event_list = cfel_file.list_events(self.img_file_pattern, field=self.img_h5_field)
        self.nframes = self.event_list['nevents']
        self.num_lines = self.nframes
        print('Number of frames', self.nframes)

        # No events?  May as well exit now
        if self.nframes == 0:
            print('Exiting (no events found to display)')
            exit(1)
    #end action_update_files



    #
    #	Initialisation function
    #
    def __init__(self, args):

        # Import CFEL colour scales
        import lib.cfel_colours

        #
        # Initialisation stuff
        #
        # Extract info from command line arguments
        self.geom_filename = args.g
        self.img_file_pattern = args.i
        self.img_h5_field = args.e

        # Create event list of all events in all files matching pattern
        # This is for multi-file flexibility - importing of file lists, enables multiple input files, format flexibility
        self.action_update_files()


        # Load geometry
        self.geometry = cfel_geom.read_geometry(self.geom_filename)
        self.img_shape = self.geometry['shape']
        self.img_to_draw = numpy.zeros(self.img_shape, dtype=numpy.float32)
        self.mask_to_draw = numpy.zeros(self.img_shape+(3,), dtype=numpy.uint8)

        # Size of images (assume all images have the same size as frame 0)
        temp = cfel_file.read_event(self.event_list, 0, data=True)
        print("Data shape: ", temp['data'].shape)
        self.slab_shape = temp['data'].shape

        # Sanity check: Do geometry and data shape match?
        if (temp['data'].flatten().shape != self.geometry['x'].shape):
            print("Error: Shape of geometry and image data do not match")
            print('Data size: ', temp.data.flatten().shape)
            print('Geometry size: ', self.geometry['x'].shape)
            exit(1)


        #
        # Set up the UI
        #
        super(cxiview, self).__init__()
        pyqtgraph.setConfigOption('background', 0.0)
        pyqtgraph.setConfigOption('background', 'k')
        pyqtgraph.setConfigOption('foreground', 'w')
        self.ui = UI.cxiview_ui.Ui_MainWindow()
        self.ui.setupUi(self)
        self.ui.imageView.ui.menuBtn.hide()
        self.ui.imageView.ui.roiBtn.hide()
        
        # Masks
        self.mask_view = pyqtgraph.ImageItem()
        self.ui.imageView.getView().addItem(self.mask_view)

        # Found peaks
        self.found_peak_canvas = pyqtgraph.ScatterPlotItem()
        self.ui.imageView.getView().addItem(self.found_peak_canvas)

        # Predicted peaks
        #self.predicted_peak_canvas = pyqtgraph.ScatterPlotItem()
        #self.ui.imageView.getView().addItem(self.predicted_peak_canvas)


        self.intregex = PyQt4.QtCore.QRegExp('[0-9]+')
        self.qtintvalidator = PyQt4.QtGui.QRegExpValidator()
        self.qtintvalidator.setRegExp(self.intregex)        

        self.ui.refreshfilesPushButton.clicked.connect(self.action_update_files)
        self.ui.previousPushButton.clicked.connect(self.previous_pattern)
        self.ui.nextPushButton.clicked.connect(self.next_pattern)
        self.ui.playPushButton.clicked.connect(self.play)
        self.ui.randomPushButton.clicked.connect(self.random_pattern)
        self.ui.shufflePushButton.clicked.connect(self.shuffle)
        self.ui.jumpToLineEdit.editingFinished.connect(self.jump_to_pattern)
        self.ui.jumpToLineEdit.setValidator(self.qtintvalidator)

        self.show_found_peaks = False
        self.ui.foundPeaksCheckBox.setChecked(False)
        self.ui.foundPeaksCheckBox.stateChanged.connect(self.showhidefoundpeaks)

        self.show_predicted_peaks = False
        self.ui.predictedPeaksCheckBox.setChecked(False)
        self.ui.predictedPeaksCheckBox.stateChanged.connect(self.showhidepredictedpeaks)

        self.show_masks = False
        self.ui.masksCheckBox.setChecked(False)
        self.ui.masksCheckBox.stateChanged.connect(self.showhidemasks)
        
        self.histogram_clip = True
        self.ui.actionHistogram_clip.setChecked(True)
        self.ui.actionHistogram_clip.triggered.connect(self.action_histclip)

        self.auto_levels = True
        self.ui.actionAuto_scale_levels.setChecked(True)
        self.ui.actionAuto_scale_levels.triggered.connect(self.action_autolevels)

        self.ui.actionSave_image.triggered.connect(self.action_save_png)
        self.ui.actionRefresh_file_list.triggered.connect(self.action_update_files)

        # Disabled stuff
        self.ui.actionSave_data.setEnabled(False)
        self.ui.actionLoad_geometry.setEnabled(False)
        self.ui.menuColours.setEnabled(False)

        # Flags needed for play and shuffle (can probably do this better)
        self.shuffle_mode = False
        self.play_mode = False 
        self.refresh_timer = PyQt4.QtCore.QTimer()

        # Put menu inside the window on Macintosh and elsewhere
        self.ui.menuBar.setNativeMenuBar(False)
        self.proxy = pyqtgraph.SignalProxy(self.ui.imageView.getView().scene().sigMouseClicked, rateLimit=60, slot=self.mouse_clicked)




        # Start on the first frame
        self.img_index = 0
        self.ui.jumpToLineEdit.setText(str(self.img_index))
        self.exportdir = ''
        
        
        # Set the colour table to inverse-BW (thanks Valerio)
        pos = numpy.array([0.0,0.5,1.0])
        color = numpy.array([[255,255,255,255], [128,128,128,255], [0,0,0,255]], dtype=numpy.ubyte)
        self.new_color_map = pyqtgraph.ColorMap(pos,color)
        self.ui.imageView.ui.histogram.gradient.setColorMap(self.new_color_map)



        # Scale QtGraph histogram to not auto-range
        #qt_Histogram = self.ui.imageView.ui.HistogramLUTItem()
        #qt_Histogram.autoHistogramRange()
        #qt_Histogram.setHistogramRange(-100, 10000, padding=0.1)
        #self.ui.imageView.ui.HistogramLUTItem().setHistogramRange(-100, 10000, padding=0.1)
        self.ui.imageView.ui.histogram.setHistogramRange(-100, 10000, padding=0.1)
                
        self.draw_things()
        #self.ui.imageView.imageItem.setZoom(1)
        self.ui.imageView.imageItem.setAutoDownsample(False)     # True/False
        #self.ui.imageView.imageItem.clipToView(False)     # True/False
        #self.ui.imageView.imageItem.antialias(True)     # True/False
        self.ui.statusBar.setText('Ready')
        
        
        
    #end __init()__
#end cxiview

        
#
#	Main function defining this as a program to be called
#
if __name__ == '__main__':
    
    #
    #   Use parser to process command line arguments
    #    
    parser = argparse.ArgumentParser(description='CFEL CXI file viewer')
    parser.add_argument("-g", default="none", help="Geometry file (.geom/.h5)")
    parser.add_argument("-i", default="none", help="Input file pattern (eg: *.cxi, LCLS*.h5)")
    parser.add_argument("-e", default="none", help="HDF5 field to read")
    parser.add_argument("-p", default=False, help="Circle peaks by default")    
    #parser.add_argument("--rmin", type=float, help="minimum pixel resolution cutoff")
    #parser.add_argument("--nmax", default=np.inf, type=int, help="maximum number of peaks to read")
    args = parser.parse_args()
    
    print("----------")    
    print("Parsed command line arguments")
    print(args)
    print("----------")    
    
    # This bit may be irrelevent if we can make parser.parse_args() require this field    
    if args.i == "none" and args.g == "none":
        print('Usage: CXIview.py -i data_file_pattern -g geom_file [-e HDF5 field]')
        sys.exit()
    #endif        


    #
    #   Spawn the viewer
    #        
    app = PyQt4.QtGui.QApplication(sys.argv)    
        
    #ex = cxiview(args.g, args.i)
    ex = cxiview(args)
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