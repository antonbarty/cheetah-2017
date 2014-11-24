import os,sys
import configobj
import time,datetime
import numpy as np
import run,run_table


def loop(configfilename,email,password):
    # Read configuration
    C = configobj.ConfigObj(configfilename)
    
    # Initialize ZMQ server
    S = server.Server(C)

    # Try to open run table
    rtab = run_table.RunTable(C)

    # Try to open google spreadsheet
    #gtab = get_gtab(C,email,password)

    runs = {}
    
    counter = 0
    
    while True:
        # Send out current run table
        S.send(run_table.get())

        # Check files in XTC directory and if new runs appeared add them to the table
        rtab.checkForNewXTCs()

        # Update information / status of all runs
        #ttab.note("Updating run status")
        ns = list(runs.keys())
        ns.sort()
        n_jobs = 0
        runs_to_start = []
        for n in ns:
            r = runs[n]
            #ttab.note("%s - updating" % r.run_name)
            r.update()
            if r.status == "runs" or r.status == "started":
                n_jobs += 1
            if r.status == "new" or r.status == "ready":
                runs_to_start.append(n)
    
        #ttab.note("Updating run information")
        #ttab.set_runs(runs)
    
        #if (counter % 10) == 0 or gtab.tabDict_modified:
        #    ttab.note("Writing to google spreadsheet")
        #    gtab.write(runs)

        counter += 1

        #sys.stdout.write("\r"+mill[i%len(mill)] + " Waiting for new XTC files to appear...")
        #ttab.note("Waiting for user to press a key...") 
        
        #ttab.screen.getch()

        # Receive message from client
        m = S.recv(10000)
        if m != None:
            runs[m["name"]].update(m)
        
        
