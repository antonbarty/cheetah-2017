#--------------------------------------------------------------------------
# File and Version Information:
#  $Id: dump_cspad.py 2622 2011-11-11 14:35:00Z salnikov@SLAC.STANFORD.EDU $
#
# Description:
#  Module myana...
#
#------------------------------------------------------------------------

"""User analysis job for XTC data.

This software was developed for the LUSI project.  If you use all or 
part of it, please give an appropriate acknowledgment.

@see RelatedModule

@version $Id: dump_cspad.py 2622 2011-11-11 14:35:00Z salnikov@SLAC.STANFORD.EDU $

@author Andrei Salnikov
"""


#------------------------------
#  Module's version from CVS --
#------------------------------
__version__ = "$Revision: 2622 $"
# $Source$

#--------------------------------
#  Imports of standard modules --
#--------------------------------
import sys
import logging

#---------------------------------
#  Imports of base class module --
#---------------------------------

#-----------------------------
# Imports for other modules --
#-----------------------------
from pypdsdata.xtc import *

#----------------------------------
# Local non-exported definitions --
#----------------------------------

#------------------------
# Exported definitions --
#------------------------

#---------------------
#  Class definition --
#---------------------
class dump_cspad ( object ) :
    """Example analysis module which dumps ControlConfig object info."""

    #--------------------
    #  Class variables --
    #--------------------

    #----------------
    #  Constructor --
    #----------------
    def __init__ ( self, address = "" ) :
        """Constructor. """
        self.m_src = address

    #-------------------
    #  Public methods --
    #-------------------

    def beginjob( self, evt, env ) :

        config = env.getConfig(TypeId.Type.Id_CspadConfig, self.m_src)
        if not config:
            return
        
        print "dump_cspad: %s: %s" % (config.__class__.__name__, self.m_src)
        print "  concentratorVersion =", config.concentratorVersion();
        print "  runDelay =", config.runDelay();
        print "  eventCode =", config.eventCode();
        print "  inactiveRunMode =", config.inactiveRunMode();
        print "  activeRunMode =", config.activeRunMode();
        print "  tdi =", config.tdi();
        print "  payloadSize =", config.payloadSize();
        print "  badAsicMask0 =", config.badAsicMask0();
        print "  badAsicMask1 =", config.badAsicMask1();
        print "  asicMask =", config.asicMask();
        print "  quadMask =", config.quadMask();
        print "  numAsicsRead =", config.numAsicsRead();
        print "  numQuads =", config.numQuads();
        try:
            # older versions may not have all methods
            print "  roiMask       : [%s]" % ', '.join([hex(config.roiMask(q)) for q in range(4)])
            print "  numAsicsStored: %s" % str(map(config.numAsicsStored, range(4)))
        except:
            pass
        try:
            print "  sections      : %s" % str(map(config.sections, range(4)))
        except:
            pass


    def event( self, evt, env ) :

        quads = evt.get(TypeId.Type.Id_CspadElement, self.m_src)
        if not quads :
            return

        # dump information about quadrants
        print "dump_cspad: %s: %s" % (quads[0].__class__.__name__, self.m_src)
        print "  Number of quadrants: %d" % len(quads)

        evt_data = evt.getBySource("Psana::CsPad::Data", self.source)
        quads = evt_data.quads
        if not quads :
            return

        # dump information about quadrants
        nQuads = evt_data.quads_shape()[0]
        print "dump_cspad: %s: %s" % (quads(0).__class__.__name__, self.m_src)
        print "  Number of quadrants: %d" % nQuads
        for i in range(nQuads):
            q = quads(i)
            
            print "  Quadrant %d" % q.quad()
            print "    virtual_channel: %s" % q.virtual_channel()
            print "    lane: %s" % q.lane()
            print "    tid: %s" % q.tid()
            print "    acq_count: %s" % q.acq_count()
            print "    op_code: %s" % q.op_code()
            print "    seq_count: %s" % q.seq_count()
            print "    ticks: %s" % q.ticks()
            print "    fiducials: %s" % q.fiducials()
            print "    frame_type: %s" % q.frame_type()
            print "    sb_temp: %s", str(q.sb_temp())

            # image data as 3-dimentional array
            data = q.data()
            print "    Data shape: %s" % str(data.shape)
