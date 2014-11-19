import os,sys
import configobj
import time,datetime
import numpy as np
import run,terminal,google

def loop(configfilename,email,password):
    C = configobj.ConfigObj(configfilename)
    Cl = C["locations"]
    L = {}
    for k,l in Cl.items():
        L[k] = os.path.expandvars(l)
    Cs = C["spreadsheet"]
    spreadsheet_name = Cs["spreadsheet_name"]
    worksheet_name = Cs["worksheet_name"]
    Cg = C["general"]
    dry_run = Cg.as_bool("dry_run")
    swmr = Cg.as_bool("swmr")
    n_jobs_max = Cg.as_int("n_jobs_max")

    if email == "":
        if "email" in Cs.keys():
            email = Cs["email"]
        else:
            print "ERROR: No email address given. Cannot login to google."
    if password == "":
        if "password" in Cs.keys():
            email = Cs["password"]
        else:
            print "ERROR: No password given. Cannot login to google."

    # Init google table client
    gtab = google.GoogleTable(email,password,spreadsheet_name,worksheet_name)
    ttab = terminal.TerminalTable(C)
    runs = {}
    
    counter = 0
    
    while True:
        # Read the google spread sheet from the web
        ttab.note("Reading from google spreadsheet")
        gtab.read()
        
        # extract valid runs (XTCs exist,etc.)
        valid_runs = set(gtab.get_valid_runs())
        # we want to add only valid runs and only those that we have not added before
        runs_to_add = valid_runs - set(runs.keys())

        # adding runs as "Run" objects to "runs"
        ttab.note("Looking for runs to add to data table")
        ns = list(runs_to_add)
        ns.sort()
        for n in ns:
            ttab.note("%s - adding" % n)
            runs[n] = run.Run(n,gtab,L,C)

        # update run information / status
        ttab.note("Updating run status")
        ns = list(valid_runs)
        ns.sort()
        n_jobs = 0
        runs_to_start = []
        for n in ns:
            r = runs[n]
            ttab.note("%s - updating" % r.run_name)
            r.update()
            if r.status == "runs" or r.status == "started":
                n_jobs += 1
            if r.status == "new" or r.status == "ready":
                runs_to_start.append(n)

        # start preprocessing of runs
        ns = list(runs_to_start)
        ns.sort()
        for n in ns:
            r = runs[n]
            if n_jobs > n_jobs_max:
                break
            if not r.prepared:
                ttab.note("%s - initialization" % r.run_name)
                r.init_process()
            if r.prepared:
                r.start()
                n_jobs += 1
                ttab.note("%s - started" % r.run_name)
                if swmr:
                    r.start_swmr()                                                     
                    n_jobs += 1
                    ttab.note("%s - started (swmr)" % r.run_name)
            else:
                ttab.note("%s - processing postponed." % r.run_name)
    
        ttab.note("Updating run information")
        ttab.set_runs(runs)
    
        if (counter % 10) == 0 or gtab.tabDict_modified:
            ttab.note("Writing to google spreadsheet")
            gtab.write(runs)

        counter += 1

    #sys.stdout.write("\r"+mill[i%len(mill)] + " Waiting for new XTC files to appear...")
    
        time.sleep(0.1)
    #except:
    #    print "ERROR OCCURED - RESTARTING IN A FEW SECONDS..."
    #    time.sleep(5)
