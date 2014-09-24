import os,sys
import configobj
import time,datetime
import numpy as np
import run,terminal,google

def loop(configfilename,password):
    C = configobj.ConfigObj(configfilename)
    Cl = C["locations"]
    L = {}
    for k,l in Cl.items():
        L[k] = os.path.expandvars(l)
    Cs = C["spreadsheet"]
    email = Cs["email"]
    spreadsheet_name = Cs["spreadsheet_name"]
    worksheet_name = Cs["worksheet_name"]
    Cg = C["general"]
    dry_run = Cg.as_bool("dry_run")
    swmr = Cg.as_bool("swmr")

    # Init google table client
    gtab = google.GoogleTable(email,password,spreadsheet_name,worksheet_name)
    ttab = terminal.TerminalTable()
    runs = {}
    
    counter = 0
    
    while True:
        gtab.update_table_dict()
        valid_runs = set(gtab.get_valid_runs())
        if counter == 0:
            runs_to_update = valid_runs
        else:
            runs_to_update = set(gtab.get_runs_to_update())
        runs_to_add = valid_runs - set(runs.keys())

        ns = list(runs_to_add)
        ns.sort()
        for n in ns:
            ttab.note("%s - adding" % n)
            runs[n] = run.Run(n,gtab,L,C)

        ns = list(runs_to_update)
        ns.sort()
        for n in ns:
            r = runs[n]
            ttab.note("%s - updating" % r.run_name)
            r.update()
            if r.status == "new":
                ttab.note("%s - initialization" % r.run_name)
                r.init_process()
                if r.prepared:
                    r.start()
                    ttab.note("%s - started" % r.run_name)
                    if swmr:
                        r.start_swmr()                                                     
                        ttab.note("%s - started (swmr)" % r.run_name)
                else:
                    ttab.note("%s - processing postponed." % r.run_name)
    
        ttab.set_runs(runs)
    
        if (counter % 10) == 0:
            ttab.note("Writing to google spreadsheet...")
            gtab.write(runs)

        counter += 1
    #mill = ["-","\\","|","/"]
    #sys.stdout.write("\r"+mill[i%len(mill)] + " Waiting for new XTC files to appear...")
    
        time.sleep(0.1)
    #except:
    #    print "ERROR OCCURED - RESTARTING IN A FEW SECONDS..."
    #    time.sleep(5)
