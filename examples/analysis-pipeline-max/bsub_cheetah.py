#!/usr/bin/env python
import os,sys
import socket
import runner

fqdn = socket.getfqdn()

# Are we on a suitable host machine?
if not ((("psexport" in fqdn or "pslogin" in fqdn) and "slac" in fqdn) or ("login" in fqdn and "davinci" in fqdn)):
    sys.exit("ERROR: Cannot submit jobs from this host. You are logged in to %s but you need to be logged in to pslogin.slac.stanford.edu, psexport.slac.stanford.edu or davinci.icm.uu.se to submit jobs to a queue." % (fqdn))

# Check configuration
if len(sys.argv) < 2:
    sys.exit("Usage: busb_cheetah.py bsub_cheetah.cfg [DEBUG]")

# Are we running in debugging mode?
debug = False
if len(sys.argv) >= 3:
    if sys.argv[2] == "DEBUG":
        debug = True

# Configuration file
configfilename = sys.argv[1]

if debug:
    runner.run(configfilename)
else:
    while True:
        try:
            runner.run(configfilename)
        except:
            print "WARNING: run() crashed, restarting..."
