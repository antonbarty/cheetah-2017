#!/usr/bin/env python3
#
#   CXIview
#   A python/Qt viewer for .cxi files (and other files output by Cheetah)
#   A replacement for the IDL Cheetah file viewer
#   Based on peak_viewer_cxi by Valerio Mariani, CFEL, December 2015
#
#   Tested using Anaconda / Python 3.4
#

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

from lib.streamfile_parser.Streamfile import *


class NoCrystalException(Exception):
    """
    Class implementing an exception for the case that no crystal is found in the
    streamfile.
    """

    pass

class InsufficientInformationException(Exception):
    """
    Class implementing an exception for the case that required information is
    missing to perform some task.
    """

    pass

#
#	CXI viewer code
#
class cxiview(PyQt4.QtGui.QMainWindow):
    
    #
    # display the main image
    #
    def draw_things(self):
        
        # Retrieve CXI file data all at once (avoid opening and closing file for each data set)
        # Skip frame on any error (make this more sensible later on)
        show_masks = self.ui.masksCheckBox.isChecked()
        show_found_peaks = self.ui.foundPeaksCheckBox.isChecked()
        try:
            cxi = cfel_file.read_event(self.event_list, self.img_index,  data=True, photon_energy=True, camera_length=True, mask=show_masks, peaks=show_found_peaks)
        except:
            print('Error encountered reading data from file (image data, peaks, energy or masks).  Skipping frame.')
            return

        # Photon energy - use command line eV if provided
        self.photon_energy_ok = False
        if self.default_eV != 'None':
            self.photon_energy = float(self.default_eV)
            self.photon_energy_ok = True
        else:
            # reading photon energy from streamfile failed, try cxi
            self.photon_energy = cxi['photon_energy_eV']
            if not numpy.isnan(self.photon_energy):
                self.photon_energy_ok = True

            # get photon energy from streamfile
            if self.stream_filepath != "":
                self.photon_energy = self.streamfile.chunks[self.img_index].photon_energy
                if self.photon_energy > 0:
                    self.photon_energy_ok = True
            if self.photon_energy < 0 or numpy.isnan(self.photon_energy):
                self.photon_energy_ok = False

        # Photon energy to wavelength
        if self.photon_energy_ok:
            self.lambd = scipy.constants.h * scipy.constants.c /(scipy.constants.e * self.photon_energy)
        else:
            self.lambd = float('nan')

        # Detector distance - use command line detector distance if provided
        self.detector_distance_ok = False
        if self.default_z != 'None':
            self.detector_z_m = float(self.default_z)
            self.detector_distance_ok = True
        else:
            detector_distance = cxi['EncoderValue']

            # get detector distance from streamfile
            if self.stream_filepath != "":
                detector_distance = self.streamfile.chunks[self.img_index].clen
            if detector_distance is None:
                detector_distance = cxi['EncoderValue']

            if not numpy.isnan(detector_distance) and not numpy.isnan(self.geometry['coffset']):
                self.detector_distance_ok = True
                self.detector_z_m = (1e-3*detector_distance + self.geometry['coffset'])
            else:
                self.detector_z_m = float('nan')
        self.detector_z_mm = self.detector_z_m * 1e3


        # Do not allow deceptive resolution values if we have insufficient information to calculate resolution
        if self.geometry_ok and self.photon_energy_ok and self.detector_distance_ok:
            self.resolution_ok = True
        else:
            self.resolution_ok = False



        # Set window title
        file_str = os.path.basename(self.event_list['filename'][self.img_index])
        title = file_str + ' #' + str(self.event_list['event'][self.img_index]) + ' - (' + str(self.img_index)+'/'+ str(self.num_lines-1) + ')'
        self.setWindowTitle(title)
        self.ui.jumpToLineEdit.setText(str(self.img_index))


        # Apply geometry to image and display
        img_data = cxi['data']
        if self.geometry_ok:
            self.img_to_draw = cfel_img.pixel_remap(img_data, self.geometry['x'], self.geometry['y'], dx=1.0)
        else:
            self.img_to_draw = numpy.transpose(img_data)
        self.ui.imageView.setImage(self.img_to_draw, autoLevels=False, autoRange=False)


        # Auto-scale the image
        if self.ui.actionAutoscale.isChecked():
            if self.ui.actionHistogram_clip.isChecked() == True:
                # Histogram equalisation (saturate top 0.1% of pixels)
                bottom, top  = cfel_img.histogram_clip_levels(img_data.ravel(),0.001)
            else:
                # Scale from 0 to maximum intensity value
                top = numpy.amax(img_data.ravel())

            self.ui.imageView.setLevels(0,top)
        #end autoscale



        # Set the histogram widget scale bar to behave politely and not jump around
        if self.ui.actionAuto_scale_levels.isChecked() == True:
            hist = self.ui.imageView.getHistogramWidget()
            hist.setHistogramRange(-100, 16384, padding=0.05)
        else:
            hist = self.ui.imageView.getHistogramWidget()
            hist.setHistogramRange(numpy.amin(img_data.ravel()), numpy.amax(img_data.ravel()), padding=0.1)
        #end histogramscale




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
        if self.ui.masksCheckBox.isChecked():
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
        if self.ui.foundPeaksCheckBox.isChecked():
            peak_x = []
            peak_y = []

            # Read peaks in raw coordinates
            if self.streamfile is not None:
                # read the peaks from the streamfile
                peak_x_data, peak_y_data = self.streamfile.get_peak_data(self.img_index)
                n_peaks = len(peak_x_data)
            else:
                n_peaks = cxi['n_peaks']
                peak_x_data = cxi['peakXPosRaw']
                peak_y_data = cxi['peakYPosRaw']
            
            for ind in xrange(0,n_peaks):                
                peak_fs = peak_x_data[ind]                
                peak_ss = peak_y_data[ind]         
                
                # Peak coordinate to pixel in image
                peak_in_slab = int(round(peak_ss))*self.slab_shape[1]+int(round(peak_fs))
                peak_x.append(self.geometry['x'][peak_in_slab] + self.img_shape[0] / 2)
                peak_y.append(self.geometry['y'][peak_in_slab] + self.img_shape[1] / 2)

            ring_pen = pyqtgraph.mkPen('r', width=2)
            self.found_peak_canvas.setData(peak_x, peak_y, symbol = 's', size = 10, pen = ring_pen, brush = (0,0,0,0), pxMode = False)

        else:
            self.found_peak_canvas.setData([])

        # Draw predicted peaks
        if self.ui.predictedPeaksCheckBox.isChecked():
            # Draw resolution limit ring
            self.draw_resolution_limit_ring()

            for index, item in enumerate(self.resolution_rings_textitems):
                item.setText('')


            if(self.streamfile.has_crystal(self.img_index)):
                peak_x = []
                peak_y = []
                
                #print("Number of peaks found: ", 
                #self.streamfile.get_number_of_crystals(self.img_index))
                #print("At the moment just displaying the first one.")
                peak_x_data, peak_y_data = self.streamfile.get_predicted_peak_data(
                    self.img_index)
                n_peaks = len(peak_x_data)

                for ind in range(0,n_peaks):                
                    peak_fs = peak_x_data[ind]                
                    peak_ss = peak_y_data[ind]         
                    
                    # Peak coordinate to pixel in image
                    peak_in_slab = int(round(peak_ss))*self.slab_shape[1]+int(round(peak_fs))
                    peak_x.append(self.geometry['x'][peak_in_slab] + self.img_shape[0] / 2)
                    peak_y.append(self.geometry['y'][peak_in_slab] + self.img_shape[1] / 2)
                    #peak_x.append(self.pixel_map[0][peak_in_slab] + self.img_shape[0]/2)
                    #peak_y.append(self.pixel_map[1][peak_in_slab] + self.img_shape[1]/2)
            else:
                # no crystal and thus no peaks
                peak_x = []
                peak_y = []

            ring_pen = pyqtgraph.mkPen('b', width=2)
            self.predicted_peak_canvas.setData(peak_x, peak_y, symbol = 'o', 
                size = 2*self.predicted_peak_circle_radius, pen = ring_pen, 
                brush = (0,0,0,0), pxMode = False)
        else:
            self.predicted_peak_canvas.setData([])
            self.resolution_limit_ring_canvas.setData([])
        


        # Draw resolution rings
        if self.ui.resolutionCheckBox.isChecked():
            self.update_resolution_rings()
        else:
            self.resolution_rings_canvas.setData([], [])
            for index, item in enumerate(self.resolution_rings_textitems):
                item.setText('')

        self.show_unit_cell_info()

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
    #end next_pattern()
    

    #
    #   Resolution ring stuff
    #   Pinched from Onda GUI - clean up code later
    #
    def update_resolution_rings(self):

        # Refuse to draw deceptive resolution rings
        if self.resolution_ok == False:
            #print("Insufficient information to calculate resolution")
            return


        items = ['3.0', '4.0', '6.0', '8.0', '10.0','20.0']
        for ti in self.resolution_rings_textitems:
            self.ui.imageView.getView().removeItem(ti)
        if len(items) == 0:
            self.resolution_rings_in_A = []
        self.resolution_rings_in_A = [ float(item) for item in items if item != '' and float(item) != 0.0 ]
        self.resolution_rings_textitems = [pyqtgraph.TextItem(str(x)+'A', anchor=(0.5,0.8)) for x in self.resolution_rings_in_A]
        for ti in self.resolution_rings_textitems:
            self.ui.imageView.getView().addItem(ti)
        self.draw_resolution_rings()


    def draw_resolution_limit_ring(self):
        """
        This method draws the resolution limit ring corresponding to the
        resolution limit given in the crystal of the current displayed
        chunk. If no crystal is present no ring is drawn.
        """

        try:
            if not self.streamfile.has_crystal(self.img_index):
                raise NoCrystalException
            number_of_crystals = self.streamfile.get_number_of_crystals(
                self.img_index)
            crystal = self.streamfile.chunks[self.img_index].crystals[0]
            resolution_limit = crystal.resolution_limit
            
            if resolution_limit is None: 
                raise InsufficientInformationException
                
            if numpy.isnan(self.detector_z_m):
                raise InsufficientInformationException

            if numpy.isnan(self.lambd):
                raise InsufficientInformationException
                
            dx = self.geometry['dx']
            resolution_limit_pix = (2.0 / dx) * self.detector_z_m * numpy.tan(
                2.0 * numpy.arcsin(self.lambd / (
                2.0 * resolution_limit * 1e-10)))

            self.resolution_limit_ring_canvas.setData(
                [self.img_shape[0] / 2], [self.img_shape[1] / 2],
                    symbol='o',
                    size=[resolution_limit_pix],
                    pen=self.resolution_limit_ring_pen,
                    brush=(0, 0, 0, 0), pxMode=False)
        except (NoCrystalException, InsufficientInformationException):
            print("Insufficient information to draw the resolution limit ring")
            self.resolution_limit_ring_canvas.setData([],[])


    def draw_resolution_rings(self):
        dx = self.geometry['dx']
        resolution_rings_in_pix = [2.0]
        for resolution in self.resolution_rings_in_A:
            resolution = float(resolution)
            res_in_pix = (2.0 / dx) * self.detector_z_m * numpy.tan(2.0 * numpy.arcsin(self.lambd / (2.0 * resolution * 1e-10)))
            resolution_rings_in_pix.append(res_in_pix)

        if self.ui.resolutionCheckBox.isChecked():
            nrings = len(resolution_rings_in_pix)
            self.resolution_rings_canvas.setData([self.img_shape[0]/2] * nrings, [self.img_shape[1] / 2] * nrings,
                                                 symbol='o',
                                                 size=resolution_rings_in_pix,
                                                 pen=self.resolution_rings_pen,
                                                 brush=(0, 0, 0, 0), pxMode=False)
            for index, item in enumerate(self.resolution_rings_textitems):
                item.setText(str(self.resolution_rings_in_A[index]) + 'A', color='r')
                #item.setText(str(self.resolution_rings_textitems[index]) + 'A')
                item.setPos(self.img_shape[0]/2, self.img_shape[1]/2 + resolution_rings_in_pix[index + 1] / 2.0)
        else:
            self.resolution_rings_canvas.setData([], [])
            for index, item in enumerate(self.resolution_rings_textitems):
                item.setText('')


    #
    # Go to random pattern
    #
    def random_pattern(self):
        pattern_to_jump = self.num_lines*numpy.random.random(1)[0]
        pattern_to_jump = pattern_to_jump.astype(numpy.int64)
        
        if 0<=pattern_to_jump<self.num_lines:
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
        
        if 0<=pattern_to_jump<self.num_lines:
            self.img_index = pattern_to_jump
            self.draw_things()
        else:
            self.ui.jumpToLineEdit.setText(str(self.img_index))
    #end jump_to_pattern()
        
    def _overline_string(self, string):
        out = ""
        for c in string:
            out += c
            out += u"\u0305"
            
        return out

    def mouse_in_predicted_peak(self, mouse_x, mouse_y, text):
        """
        This method displays the predicted peak hkl indices in the lower
        left corner of the gui window if the mouse pointer has been clicked
        on the corresponding predicted peak.
        """

        if self.streamfile is None:
            return
        
        if not self.streamfile.has_crystal(self.img_index):
            return

        peak_x_data, peak_y_data = self.streamfile.get_predicted_peak_data(
            self.img_index)
        n_peaks = len(peak_x_data)

        for ind in range(0,n_peaks):                
            peak_fs = peak_x_data[ind]                
            peak_ss = peak_y_data[ind]         
            
            # Peak coordinate to pixel in image
            peak_in_slab = int(round(peak_ss))*self.slab_shape[1]+int(
                round(peak_fs))
            peak_x = (self.geometry['x'][peak_in_slab] + 
                self.img_shape[0] / 2)
            peak_y = (self.geometry['y'][peak_in_slab] + 
                self.img_shape[1] / 2)
                
            if(numpy.sqrt((peak_x - mouse_x)**2 + (peak_y - mouse_y)**2) <
                self.predicted_peak_circle_radius):
                hkl = self.streamfile.get_hkl_indices(peak_fs, peak_ss, 
                    self.img_index)
                overline = "\u0305"
                text += "     hkl: "
                if hkl[0] < 0:
                    text += self._overline_string(str(abs(hkl[0])))
                else:
                    text += str(hkl[0])
                text += "  "
                if hkl[1] < 0:
                    text += self._overline_string(str(abs(hkl[1])))
                else:
                    text += str(hkl[1]) 
                text += "  "
                if hkl[2] < 0:
                    text += self._overline_string(str(abs(hkl[2])))
                else:
                    text += str(hkl[2])
                self.ui.statusBar.setText(text)

    #
    # Mouse clicked somewhere in the window
    #
    def mouse_clicked(self, event):
        #pos = event[0].pos()
        pos = event.scenePos()
        #print(pos)
        if self.ui.imageView.getView().sceneBoundingRect().contains(pos):
            mouse_point = self.ui.imageView.getView().mapSceneToView(pos)
            x_mouse = int(mouse_point.x())
            y_mouse = int(mouse_point.y()) 
            #print("x: ", x_mouse, "y: ", y_mouse)
            x_mouse_centered = x_mouse - self.img_shape[0]/2 + 1
            y_mouse_centered = y_mouse - self.img_shape[0]/2 + 1

            radius_in_m = self.geometry['dx'] * numpy.sqrt(x_mouse_centered**2 + y_mouse_centered**2)

            try:
                text = 'Last clicked pixel:     x: %4i     y: %4i     value: %4i' % (x_mouse_centered, y_mouse_centered, self.img_to_draw[x_mouse, y_mouse])
            except IndexError:
                # click is not in image anymore
                return

            # Refuse to report an incorrect detector distance
            if self.detector_distance_ok:
                camera_z_in = self.detector_z_mm / 25.4
                text += '     z: %.2f mm' % (self.detector_z_mm)

            # Refuse to lie about the resolution
            if self.resolution_ok:
                #resolution = 10e9*self.lambd/(2.0*numpy.sin(0.5*numpy.arctan(radius_in_m/(self.camera_length+self.geometry['coffset']))))
                resolution = 1e10*self.lambd/(2.0*numpy.sin(0.5*numpy.arctan(radius_in_m/self.detector_z_m)))
                text += '     resolution: %4.2f Å' % (resolution)


            self.ui.statusBar.setText(text)
            self.mouse_in_predicted_peak(x_mouse, y_mouse, text)
            # self.ui.statusBar.setText(
            #    'Last clicked pixel:     x: %4i     y: %4i     value: %4i     z: %.2f mm     resolution: %4.2f Å' % (
            #    x_mouse_centered, y_mouse_centered, self.img_to_draw[x_mouse, y_mouse], self.camera_z_mm, resolution))

    #end mouse_clicked()


    def show_unit_cell_info(self):
        """
        This method displays the unit cell information in the lower left corner
        of the gui windows if a crystal has been found.
        """

        try:
            if self.streamfile is None:
                raise NoCrystalException
            if not self.streamfile.has_crystal(self.img_index):
                raise NoCrystalException
            unit_cell = self.streamfile.get_unit_cell(self.img_index)

            if unit_cell is None:
                # If something went wrong just don't display the crystal 
                # information
                raise NoCrystalException

            text = "Unit cell: "
            text += unit_cell.centering + ", "
            text += "a = " + "{:.2f}".format(unit_cell.a) + " \u212B, "
            text += "b = " + "{:.2f}".format(unit_cell.b) + " \u212B,"
            text += "c = " + "{:.2f}".format(unit_cell.c) + " \u212B, "
            text += "\u03B1 = " + "{:.1f}".format(unit_cell.alpha) + "\u00B0, "
            text += "\u03B2 = " + "{:.1f}".format(unit_cell.beta) + "\u00B0, "
            text += "\u03B3 = " + "{:.1f}".format(unit_cell.gamma) + "\u00B0"
            #qtext = PyQt4.QtCore.QString(text)
            self.ui.statusBar.setText(text) 
        except NoCrystalException:
            self.ui.statusBar.setText("Ready") 
            

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

        filename = cfel_file.dialog_pickfile(write=True, path=file_hint, qtmainwin=self)
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
        # read files from streamfile if streamfile is there
        if self.streamfile is not None:
            self.event_list = self.streamfile.get_cxiview_event_list()
            # we override the number of events because there may be fewer
            # chunks in the streamfile than in the cxi
            self.event_list['nevents'] = self.streamfile.get_number_of_chunks()
        else:
            self.event_list = cfel_file.list_events(self.img_file_pattern, 
                field=self.img_h5_field)
        self.nframes = self.event_list['nevents']
        self.num_lines = self.nframes

        # No events?  May as well exit now
        if self.nframes == 0:
            print('Exiting (no events found to display)')
            exit(1)

        file_str = os.path.basename(self.event_list['filename'][self.img_index])
        title = file_str + ' #' + str(self.event_list['event'][self.img_index]) + ' - (' + str(self.img_index)+'/'+ str(self.num_lines-1) + ')'
        self.ui.jumpToLineEdit.setText(str(self.img_index))
        self.setWindowTitle(title)
    #end action_update_files

    """
    def mousePressEvent(self, event):
        p = PyQt4.QtGui.QCursor.pos()
        print("pressed here: " + str(p.x()) + ", " + str(p.y()))
    """

    def keyPressEvent(self, e):
        if e.key() == PyQt4.QtCore.Qt.Key_Escape:
            self.show_unit_cell_info()

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
        self.default_z = args.z
        self.default_eV = args.v
        self.stream_filepath = args.s
        self.streamfile = None

        # Load geometry
        # read_geometry currently exits program on failure
        if self.stream_filepath is not "":
            self.streamfile = Streamfile(self.stream_filepath)
            self.geometry = self.streamfile.get_geometry()
            self.geometry_ok = True
            self.img_shape = self.geometry['shape']
        elif self.geom_filename is not "":
            self.geometry = cfel_geom.read_geometry(self.geom_filename)
            self.geometry_ok = True
            self.img_shape = self.geometry['shape']
        else:
            self.geometry_ok = False
            self.img_shape = self.slab_shape
            # the commented code has been moved downwards to avoid redundancy 
            #self.image_center = (self.img_shape[0] / 2, self.img_shape[1] / 2)
            #self.img_to_draw = numpy.zeros(self.img_shape, dtype=numpy.float32)
            #self.mask_to_draw = numpy.zeros(self.img_shape + (3,), dtype=numpy.uint8)

            # faking self.geometry is a hack to stop crashes down the line.
            # Fix more elegantly later
            self.geometry = {
                'x': numpy.zeros(self.img_shape).flatten(),
                'y': numpy.zeros(self.img_shape).flatten(),
                'r': numpy.zeros(self.img_shape).flatten(),
                'dx': 1.0,
                'coffset': 'nan',
                'shape': self.slab_shape
            }
        self.image_center = (self.img_shape[0] / 2, self.img_shape[1] / 2)
        self.img_to_draw = numpy.zeros(self.img_shape, dtype=numpy.float32)
        self.mask_to_draw = numpy.zeros(self.img_shape+(3,), dtype=numpy.uint8)
        self.predicted_peak_circle_radius = 5

        #
        # Set up the UI
        #
        super(cxiview, self).__init__()
        pyqtgraph.setConfigOption('background', 0.0)
        pyqtgraph.setConfigOption('background', 'k')
        pyqtgraph.setConfigOption('foreground', 'w')
        self.ui = UI.cxiview_ui.Ui_MainWindow()
        self.ui.setupUi(self)

        if self.streamfile is not None:
            self.ui.predictedPeaksCheckBox.setEnabled(True)

        # Create event list of all events in all files matching pattern
        # This is for multi-file flexibility - importing of file lists, enables
        # multiple input files, format flexibility
        self.img_index = 0
        self.action_update_files()

        # Size of images (assume all images have the same size as frame 0)
        temp = cfel_file.read_event(self.event_list, 0, data=True)
        self.slab_shape = temp['data'].shape

        # Sanity check: Do geometry and data shape match?
        if self.geometry_ok and (temp['data'].flatten().shape != self.geometry['x'].shape):
            print("Error: Shape of geometry and image data do not match")
            print('Data size: ', temp.data.flatten().shape)
            print('Geometry size: ', self.geometry['x'].shape)
            exit(1)



        #
        #   UI configuration stuff
        #
        self.ui.imageView.ui.menuBtn.hide()
        self.ui.imageView.ui.roiBtn.hide()
        
        # Buttons on front pannel
        self.ui.refreshfilesPushButton.clicked.connect(self.action_update_files)
        self.ui.previousPushButton.clicked.connect(self.previous_pattern)
        self.ui.nextPushButton.clicked.connect(self.next_pattern)
        self.ui.playPushButton.clicked.connect(self.play)
        self.ui.randomPushButton.clicked.connect(self.random_pattern)
        self.ui.shufflePushButton.clicked.connect(self.shuffle)
        self.ui.jumpToLineEdit.editingFinished.connect(self.jump_to_pattern)
        self.intregex = PyQt4.QtCore.QRegExp('[0-9]+')
        self.qtintvalidator = PyQt4.QtGui.QRegExpValidator()
        self.qtintvalidator.setRegExp(self.intregex)
        self.ui.jumpToLineEdit.setValidator(self.qtintvalidator)

        # Check boxes on bottom line
        self.ui.foundPeaksCheckBox.setChecked(False)
        self.ui.predictedPeaksCheckBox.setChecked(False)
        self.ui.masksCheckBox.setChecked(False)
        self.ui.resolutionCheckBox.setChecked(False)
        self.ui.foundPeaksCheckBox.stateChanged.connect(self.draw_things)
        self.ui.predictedPeaksCheckBox.stateChanged.connect(self.draw_things)
        self.ui.masksCheckBox.stateChanged.connect(self.draw_things)
        self.ui.resolutionCheckBox.stateChanged.connect(self.draw_things)


        # View menu
        self.ui.actionAutoscale.setChecked(True)
        self.ui.actionHistogram_clip.setChecked(True)
        self.ui.actionAuto_scale_levels.setChecked(True)
        self.ui.actionHistogram_clip.triggered.connect(self.draw_things)
        self.ui.actionAuto_scale_levels.triggered.connect(self.draw_things)

        # File menu
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
        self.ui.imageView.getView().scene().sigMouseClicked.connect(self.mouse_clicked)
        #self.proxy = pyqtgraph.SignalProxy(self.ui.imageView.getView().scene().sigMouseClicked, rateLimit=60, slot=self.mouse_clicked)


        # Masks
        self.mask_view = pyqtgraph.ImageItem()
        self.ui.imageView.getView().addItem(self.mask_view)

        # Found peaks
        self.found_peak_canvas = pyqtgraph.ScatterPlotItem()
        self.ui.imageView.getView().addItem(self.found_peak_canvas)

        # Predicted peaks
        self.predicted_peak_canvas = pyqtgraph.ScatterPlotItem()
        self.ui.imageView.getView().addItem(self.predicted_peak_canvas)

        # Resolution rings
        self.resolution_ok = False
        self.resolution_rings_textitems = []
        self.resolution_rings_canvas = pyqtgraph.ScatterPlotItem()
        self.ui.imageView.getView().addItem(self.resolution_rings_canvas)
        self.resolution_rings_pen = pyqtgraph.mkPen('b', width=1)     # float=greyscale(0-1) or (r,g,b) or "r, g, b, c, m, y, k, w"

        # Resolution limit ring
        self.resolution_limit_ring_canvas = pyqtgraph.ScatterPlotItem()
        self.ui.imageView.getView().addItem(self.resolution_limit_ring_canvas)
        self.resolution_limit_ring_pen = pyqtgraph.mkPen(color=(0, 200, 0), width=2, style = PyQt4.QtCore.Qt.DashLine)    


        self.resolution_rings_in_A = [10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0]
        self.resolution_rings_textitems = [pyqtgraph.TextItem('', anchor=(0.5, 0.8)) for x in self.resolution_rings_in_A]
        for ti in self.resolution_rings_textitems:
            self.ui.imageView.getView().addItem(ti)


        # Start on the first frame
        self.ui.jumpToLineEdit.setText(str(self.img_index))
        self.exportdir = ''
        
        
        # Set the colour table to inverse-BW (thanks Valerio)
        pos = numpy.array([0.0,0.5,1.0])
        color = numpy.array([[255,255,255,255], [128,128,128,255], [0,0,0,255]], dtype=numpy.ubyte)
        self.new_color_map = pyqtgraph.ColorMap(pos,color)
        self.ui.imageView.ui.histogram.gradient.setColorMap(self.new_color_map)


        # Initial image
        self.draw_things()
        self.ui.imageView.imageItem.setAutoDownsample(False)     # True/False
        #self.ui.imageView.imageItem.setZoom(1)
        #self.ui.imageView.imageItem.clipToView(False)     # True/False
        #self.ui.imageView.imageItem.antialias(True)     # True/False
        #self.ui.statusBar.setText('Ready')
        #self.show_unit_cell_info()

    #end __init()__
#end cxiview



        
#
#	Main function defining this as a program to be called
#
if __name__ == '__main__':
    
    #
    #   Use parser to process command line arguments
    #    
    # TODO: option -p does not work
    parser = argparse.ArgumentParser(description='CFEL CXI file viewer')
    parser.add_argument("-g", default="", help="Geometry file (.geom/.h5)")
    parser.add_argument("-i", 
        default="", help="Input file pattern (eg: *.cxi, LCLS*.h5)")
    parser.add_argument("-e", default="data/data", help="HDF5 field to read")
    parser.add_argument("-z", default='None', help="Detector distance (m)")
    parser.add_argument("-v", default='None', help="Photon energy (eV)")
    parser.add_argument("-l", default='None', help="Read event list")
    parser.add_argument("-p", default=False, help="Circle peaks by default")
    # Add the functionality to read a stream file
    parser.add_argument("-s", default="", help="""CrystFEL stream file. If 
    the stream file is passed to the program all other options except -e are 
    ignored. The relevant information is generated automatically from the 
    stream file.""")
    #parser.add_argument("-x", default='110e-6', help="Detector pixel size (m)")
    args = parser.parse_args()


    """
    print("----------")    
    print("Parsed command line arguments")
    print(args)
    print("----------")    
    """
    
    # Perform consistency checking on the command line arguments.
    if (args.i == "" and args.s == ""):
        print("No input or stream file given")
        exit()
    if (args.i is not "" and args.s is not ""):
        print("Error! Provide either a stream file or an input file")
        exit()
    if args.i != "":
        #if not os.path.isfile(args.i):
        #    print("Error! Input file does not exist")
        #    exit()
        if (args.g == ""):
            print("Error! A geometry file is required")
            exit()
    if args.s != "":
        if not os.path.isfile(args.s):
            print("Error! Stream file does not exist")
            exit()

    # This bit may be irrelevent if we can make parser.parse_args() require this
    # field    
    # if args.i == "":
    #    print("""Usage: cxiview.py -i data_file_pattern [-g geom_file .geom/.h5]
    #        [-e HDF5 field] [-z Detector distance m] [-v photon energy eV]""")
    #    sys.exit()
    # endif        


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
    
    # This function does the following in an attempt to ‘safely’ terminate the 
    # process:
    #   Invoke atexit callbacks
    #   Close all open file handles
    os._exit(ret)
    sys.exit(ret)
#end __main__
