# -*- coding: utf-8 -*-
"""
CFEL colour table tools

Created on Tue Mar  1 17:14:16 2016

@author: Valerio Mariani 
"""

import pyqtgraph


"""
  How to set a gradient.
  Suppose you create a white->black gradient (cheetah style) like this:
  
      pos = numpy.array([0.0,0.5,1.0])
      color = numpy.array([[255,255,255,255], [128,128,128,255], [0,0,0,255]], dtype=numpy.ubyte)
      self.new_color_map = pyqtgraph.ColorMap(pos,color)
  
  (You define positions and RGB values of colors, then create a color map).
  
  You can access the GradientEditorItem and set the new colormap like this:
  
      self.ui.imageView.ui.histogram.gradient.setColorMap(self.new_color_map)
  
  One of the drawbacks is that the new colormap does not get added to the list 
  of available LUTs that you get when you right click on the histogram. 
  
  
  I did find a way of overriding the gradient table in pyqtgraph. 
  It is a little bit of a hack but it works!
  
  Immdiately after importing pyqtgraph, you can override the gradient table like this:
  
  pyqtgraph.graphicsItems.GradientEditorItem.Gradients = pyqtgraph.pgcollections.OrderedDict([
                 ('grey', {'ticks': [(0.0, (0, 0, 0, 255)), (1.0, (255, 255, 255, 255))], 'mode': 'rgb'}),
                 ('revgrey', {'ticks': [(0.0, (255, 255, 255, 255)), (1.0, (0, 0, 0, 255))], 'mode': 'rgb'})
         ])
  
  This needs to be done before instantiating the GUI. Furthermore, the list 
  *must* contain a gradient name "grey" because the image widget looks for a 
  default LUT with that name.
  
  For reference, see the default LUTS at the top of the GradientEditorItem:
  https://github.com/campagnola/test/blob/master/pyqtgraph/graphicsItems/GradientEditorItem.py
  
  Gradients = OrderedDict([
     ('thermal', {'ticks': [(0.3333, (185, 0, 0, 255)), (0.6666, (255, 220, 0, 255)), (1, (255, 255, 255, 255)), (0, (0, 0, 0, 255))], 'mode': 'rgb'}),
     ('flame', {'ticks': [(0.2, (7, 0, 220, 255)), (0.5, (236, 0, 134, 255)), (0.8, (246, 246, 0, 255)), (1.0, (255, 255, 255, 255)), (0.0, (0, 0, 0, 255))], 'mode': 'rgb'}),
     ('yellowy', {'ticks': [(0.0, (0, 0, 0, 255)), (0.2328863796753704, (32, 0, 129, 255)), (0.8362738179251941, (255, 255, 0, 255)), (0.5257586450247, (115, 15, 255, 255)), (1.0, (255, 255, 255, 255))], 'mode': 'rgb'} ),
     ('bipolar', {'ticks': [(0.0, (0, 255, 255, 255)), (1.0, (255, 255, 0, 255)), (0.5, (0, 0, 0, 255)), (0.25, (0, 0, 255, 255)), (0.75, (255, 0, 0, 255))], 'mode': 'rgb'}),
     ('spectrum', {'ticks': [(1.0, (255, 0, 255, 255)), (0.0, (255, 0, 0, 255))], 'mode': 'hsv'}),
     ('cyclic', {'ticks': [(0.0, (255, 0, 4, 255)), (1.0, (255, 0, 0, 255))], 'mode': 'hsv'}),
     ('greyclip', {'ticks': [(0.0, (0, 0, 0, 255)), (0.99, (255, 255, 255, 255)), (1.0, (255, 0, 0, 255))], 'mode': 'rgb'}),
     ('grey', {'ticks': [(0.0, (0, 0, 0, 255)), (1.0, (255, 255, 255, 255))], 'mode': 'rgb'}),
  ])
  
  Like this, you don't even need to define the ColorMap as before. The format of 
  the dictionary entries is:
  
  ‘ticks’: a list of tuples (pos, (r,g,b,a))
  ‘mode’: hsv or rgb
"""



"""
  The following colour tables appear in context menu
"""
pyqtgraph.graphicsItems.GradientEditorItem.Gradients = pyqtgraph.pgcollections.OrderedDict([
      ('grey', {'ticks': [(0.0, (0, 0, 0, 255)), (1.0, (255, 255, 255, 255))], 'mode': 'rgb'}),
      ('invgrey', {'ticks': [(0.0, (255, 255, 255, 255)), (1.0, (0, 0, 0, 255))], 'mode': 'rgb'}),
      ('greyclip', {'ticks': [(0.0, (0, 0, 64, 255)), (0.01, (0, 0, 0, 255)), (0.99, (255, 255, 255, 255)), (1.0, (255, 0, 0, 255))], 'mode': 'rgb'}),
      ('idl4', {'ticks': [(0.0, (0,0,0,255)), (0.12, (0,0,64,255)), (0.19, (0,50,100,255)), (0.31, (0,150,100,255)), (0.375, (0,150,50,255)), (0.44, (0,140,0,255)), (0.5, (120,100,0,255)), (0.55, (200,0,0,255)), (1.0, (255,255,0,255))], 'mode': 'rgb'}),
      ('hdfsee', {'ticks':[(0.0, (0,0,0,255)), (0.16, (0,0,255,255)), (0.33, (255,0,255,255)), (0.5, (255,0,0,255)), (0.66, (255, 128, 0, 255)), (0.83, (255,255,0,255)), (1.0, (255,255,255,255))], 'mode': 'rgb'}),
      ('thermal', {'ticks': [(0.3333, (185, 0, 0, 255)), (0.6666, (255, 220, 0, 255)), (1, (255, 255, 255, 255)), (0, (0, 0, 0, 255))], 'mode': 'rgb'}),
      ('flame', {'ticks': [(0.2, (7, 0, 220, 255)), (0.5, (236, 0, 134, 255)), (0.8, (246, 246, 0, 255)), (1.0, (255, 255, 255, 255)), (0.0, (0, 0, 0, 255))], 'mode': 'rgb'}),
      ('yellowy', {'ticks': [(0.0, (0, 0, 0, 255)), (0.2328863796753704, (32, 0, 129, 255)), (0.8362738179251941, (255, 255, 0, 255)), (0.5257586450247, (115, 15, 255, 255)), (1.0, (255, 255, 255, 255))], 'mode': 'rgb'} ),
      ('spectrum', {'ticks': [(1.0, (255, 0, 255, 255)), (0.0, (255, 0, 0, 255))], 'mode': 'hsv'}),
      ('cyclic', {'ticks': [(0.0, (255, 0, 4, 255)), (1.0, (255, 0, 0, 255))], 'mode': 'hsv'}),
      ('bipolar', {'ticks': [(0.0, (0, 255, 255, 255)), (0.25, (0, 0, 255, 255)), (0.5, (0, 0, 0, 255)), (0.75, (255, 0, 0, 255)), (1.0, (255, 255, 0, 255))], 'mode': 'rgb'}),
  ])


       