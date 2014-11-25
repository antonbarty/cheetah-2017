import os,sys
import configobj
import time,datetime
import numpy as np
import run,runTable
import serverZMQ

def loop(configfilename,email,password):
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
    
    while True:
        print "Update"
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
        print "Receive request"
        req = S.recvReq()
        if req != None:
            print req
            if req == [{}]:
                # Send out full rList for initialization of client
                print "Send full list"
                S.answerReq(rtab.getList())
            else:
                print "Update requests"
                for r in req:
                    n = r["Name"]
                    rtab.R[n].update(r)       
                # Answer request
                print "Answer request"
                # Send out update
                print "Send list update"
                S.answerReq(rtab.getListUpdate())

        rtab.writeTable()
            


