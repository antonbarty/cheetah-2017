#--------------------------------------------------------------------------
# File and Version Information:
#  $Id: dump_acqiris.py 2622 2011-11-11 14:35:00Z salnikov@SLAC.STANFORD.EDU $
#
# Description:
#  Pyana user analysis module dump_princeton...
#
#------------------------------------------------------------------------

"""Example module for accessing SharedIpimb data.

This software was developed for the LCLS project.  If you use all or 
part of it, please give an appropriate acknowledgment.

@version $Id: dump_acqiris.py 2622 2011-11-11 14:35:00Z salnikov@SLAC.STANFORD.EDU $

@author Andy Salnikov
"""

#------------------------------
#  Module's version from SVN --
#------------------------------
__version__ = "$Revision: 2622 $"
# $Source$

#--------------------------------
#  Imports of standard modules --
#--------------------------------
import sys
import logging

#-----------------------------
# Imports for other modules --
#-----------------------------
from pypdsdata import xtc

#----------------------------------
# Local non-exported definitions --
#----------------------------------

# local definitions usually start with _

#---------------------
#  Class definition --
#---------------------
class dump_acqiris (object) :
    """Class whose instance will be used as a user analysis module. """

    #----------------
    #  Constructor --
    #----------------
    def __init__ ( self, source="" ) :
        """Class constructor takes the name of the data source.

        @param source   data source
        """
        
        self.m_src = source

    #-------------------
    #  Public methods --
    #-------------------
    def beginjob( self, evt, env ) :
        
        print "self.m_src=", self.m_src
        config = env.getConfig(xtc.TypeId.Type.Id_AcqConfig, self.m_src)
        if config:
        
            print "%s: %s" % (config.__class__.__name__, self.m_src)
            
            print "  nbrBanks =", config.nbrBanks(),
            print "channelMask =", config.channelMask(),
            print "nbrChannels =", config.nbrChannels(),
            print "nbrConvertersPerChannel =", config.nbrConvertersPerChannel()
     
            h = config.horiz()
            print "  horiz: sampInterval =", h.sampInterval(),
            print "delayTime =", h.delayTime(),
            print "nbrSegments =", h.nbrSegments(),
            print "nbrSamples =", h.nbrSamples()

            nch = config.nbrChannels()
            vert = config.vert()
            for ch in range(nch):
                v = vert[ch]
                print "  vert(%d):" % ch,
                print "fullScale =", v.fullScale()
                print "slope =", v.slope()
                print "offset =", v.offset()
                print "coupling =", v.coupling()
                print "bandwidth=", v.bandwidth()

    def event( self, evt, env ) :
        """This method is called for every L1Accept transition.

        @param evt    event data object
        @param env    environment object
        """

        print "hi"
        print "self.m_src=", self.m_src           # e.g. '|Acqiris'
        print "self.m_fullName=", self.m_fullName # e.g. 'psana_examples.dump_acqiris'
        acqData = evt.getByType("Psana::Acqiris::DataDesc", self.m_src)
        if not acqData:
            return

        # find matching config object
        acqConfig = env.getConfigByType("Psana::Acqiris::Config", self.m_src)

        # loop over channels
        nchan = acqData.data_shape()[0];
        for chan in range(nchan):
            elem = acqData.data(chan);
            v = acqConfig.vert()[chan]
            slope = v.slope()
            offset = v.offset()

            print "Acqiris::DataDescV1: channel=%d" % chan ### XXX should print real class name instead
            print "  nbrSegments=%d" % elem.nbrSegments()
            print "  nbrSamplesInSeg=%d" % elem.nbrSamplesInSeg()
            print "  indexFirstPoint=%d" % elem.indexFirstPoint()

            timestamps = elem.timestamp()
            waveforms = elem.waveforms()

            # loop over segments
            for seg in range(elem.nbrSegments()):
                print "  Segment #%d" % seg
                print "    timestamp=%d" % timestamps[seg].pos()
                print "    data=[",
                size = min(elem.nbrSamplesInSeg(), 32)
                for i in range(size):
                    print "%f," % (waveforms[seg][i]*slope + offset),
                print "...]"
