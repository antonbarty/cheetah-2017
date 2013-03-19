#--------------------------------------------------------------------------
# File and Version Information:
#  $Id$
#
# Description:
#  Module psana
#
#------------------------------------------------------------------------

"""High-level wrapper module for psana.

Typical usage::

    from psana import *
    
    setConfigFile('config-file.cfg')   # optional, default is to use psana.cfg if present
    setOption("psana.modules", "EventKeys")   # can set/change options from script

    # define "data source" specifying experiment and run number
    datasrc = DataSource("exp=cxi63112:run=111")

    # specify source device for data   
    cam_src = Source("CxiDg4.0:Tm6740.0")

    # loop over all events in a data source
    for evt in datasrc.events():
        frame = evt.get(Camera.FrameV1, cam_src)
        image = frame.data16()

This software was developed for the SIT project.  If you use all or 
part of it, please give an appropriate acknowledgment.

@version $Id$

@author Andy Salnikov
"""

import sys
import os
import logging

if sys.platform.startswith('linux'):
    # on Linux with g++ one needs RTLD_GLOBAL for dlopen
    # which Python does not set by default
    import DLFCN
    flags = sys.getdlopenflags()
    sys.setdlopenflags( flags | DLFCN.RTLD_GLOBAL )    
    import _psana
    sys.setdlopenflags( flags )    
    del flags
    del DLFCN
else:
    import _psana
del sys

#
# import everything from _psana
#
from _psana import *

# this is not used here directly but still imported for documentation
from _psana import _DataSource

#----------------------------------
# Local non-exported definitions --
#----------------------------------

_cfgFile = None
_options = {}


def setConfigFile(name):
    """
    Set the name of the psana configuration file, default configuration
    file name is psana.cfg. If you want to avoid reading any configuration
    file then set it to empty string. 
    
    Configuration file name set with setConfigFile() is used in a next call to DataSource().
    """
    
    _cfgFile = name

def setOption(name, value):
    """
    Set the psana configuration option, this will override any configuration 
    option in a file. Option name has format "section.option" where "section" 
    is the name of the section in configuration file (such as "psana" or 
    "psana_examples.DumpPrinceton"). Value can be any string, possibly empty,
    non-string values will be converted to strings using str() call.

    Configuration options set with setOption() are used in a next call to DataSource().
    """
    
    _options[name] = str(value)

def setOptions(mapping):
    """
    Set the psana configuration option, this will override any configuration 
    option in a file. Argument to this calls is a mapping (dictionary) whose
    keys are option names. Option name has format "section.option" where "section" 
    is the name of the section in configuration file (such as "psana" or 
    "psana_examples.DumpPrinceton"). Value can be any string, possibly empty,
    non-string values will be converted to strings using str() call.

    Configuration options set with setOptions() are used in a next call to DataSource().
    """

    _options[name] = str(value)
    for key, val in mapping.items():
        _options[key] = str(val)


def DataSource(*args):
    """
    Makes an instance of the data source object (:py:class:`psana._DataSource`).
    Arguments can be either a single list of strings or any number of strings,
    each string represents either an input file name or event collection.
    """
    
    # make instance of the framework
    cfgFile = _cfgFile
    if cfgFile is None:
        if os.path.exists("psana.cfg"): 
            cfgFile = "psana.cfg"
        else:
            cfgFile = ""
    fwk = _psana.PSAna(cfgFile, _options)
    
    return fwk.dataSource(*args)

