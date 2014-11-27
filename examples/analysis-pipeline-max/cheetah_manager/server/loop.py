import os,sys,signal
import configobj
import time,datetime
import numpy as np
import run,runTable
import serverZMQ

def loop(configfilename):#,email,password):
    print "Read config"
    # Read configuration
    C = configobj.ConfigObj(configfilename)
    
    print "Init ZMQ"
    # Initialize ZMQ server
    S = serverZMQ.Server(C)

    print "Init run table"
    # Try to open run table
    rtab = runTable.RunTable(C)

    # Try to open google spreadsheet
    #gtab = get_gtab(C,email,password)
    
    #counter = 0

    # Set the signal handler and a 5-second alarm
    signal.signal(signal.SIGINT, S.terminate)

    print "Enter server loop"
    
    while True:
        #print "Update"
        # Update information / status of all runs
        #ttab.note("Updating run status")
        rtab.update()

        #ttab.note("Updating run information")
        #ttab.set_runs(runs)
    
        #if (counter % 10) == 0 or gtab.tabDict_modified:
        #    ttab.note("Writing to google spreadsheet")
        #    gtab.write(runs)

        #counter += 1

        #sys.stdout.write("\r"+mill[i%len(mill)] + " Waiting for new XTC files to appear...")
        #ttab.note("Waiting for user to press a key...") 
        
        #ttab.screen.getch()

        # Receive request from clients
        #print "Receive requests"
        reqs = S.recvReqs()
        sendListFull = False
        sendListUpdate = False
        #print reqs
        #print "Update"
        for req in reqs:
            if req == "REQ_FULL_LIST":
                sendListFull = True
            elif req == "REQ_UPDATED_LIST":
                sendListUpdate = True
            elif isinstance(req,dict):
                sendListUpdate = True
                rtab.R[req["Name"]].update(req) 
            else:
                print "WARNING: Invalid package received."
        #print "Answer requests"       
        L = []
        if sendListFull: 
            L = rtab.getList()
        elif sendListUpdate:
            L = rtab.getListUpdate()
        S.answerReqs(L)

        #print "Cache table to file"
        rtab.writeTable()
            
    
