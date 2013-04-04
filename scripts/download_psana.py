#!/usr/bin/env python

import sys
import os

if(len(sys.argv) < 3):
    print "Usage: download_psana.py <ana-version> <output-directory>"
    print ""
    print "Example: download_psana.py ana-0.9.1 /opt/psana/g/psdm/portable/sw/releases/"
    print ""
    print "Will download the components of ana necessary to build cheetah into the output directory"
    sys.exit(0)

ana_version = sys.argv[1] 
output_dir = sys.argv[2]

os.system('ssh psexport.slac.stanford.edu "cd /reg/g/psdm/sw/releases/ && tar czf - '+ana_version+'" | tar xzvf - -C '+output_dir)
