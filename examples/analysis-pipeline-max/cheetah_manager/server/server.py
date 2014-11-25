#!/usr/bin/env python
import os,sys
import socket, zmq
import getpass
import loop

# Are we on the right host machine?
hostname = socket.gethostname()
if not ("psexport" in hostname or "login" == hostname):
    sys.exit("ERROR: Cannot submit jobs from this host. You are logged in to %s but you need to be logged in to psexport or davinci (login node) to simultaneously submit jobs to the queue and communicate with the google spreadsheet." % hostname)

# Read configuration
if len(sys.argv) < 2:
    sys.exit("Usage: cheman_server.py cheman_server.cfg")

debug = False
if len(sys.argv) >= 3:
    if sys.argv[2] == "DEBUG":
        debug = True

configfilename = sys.argv[1]

print "Gmail login"
email = raw_input("Email address: ")
password = getpass.getpass("Password: ")

if debug:
    loop.loop(configfilename,email,password)
else:
    while True:
        try:
            loop.loop(configfilename,email,password)
        except:
            print "WARNING: loop() crashed, restarting..."
