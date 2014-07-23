#!/usr/bin/env python
import configobj
import os,sys
import time,datetime
import socket
import numpy as np
import gdata.spreadsheet.service
import getpass

print "Gmail password [maxhantke@gmail.com]:"
email = "maxhantke@gmail.com"
password = getpass.getpass()

# Find this value in the url with 'key=XXX' and copy XXX below
spreadsheet_key = '1hAWODbWaFRUnmIoTO1LhxS8tEcyrwzJu_sN2jlkEmc0'
#spreadsheet_key = '1YUYX53zULUBthrczoEOQ6mClTLNO43H3Xl_A7yvym74'
# All spreadsheets have worksheets. I think worksheet #1 by default always
# has a value of 'od6'
#worksheet_id = '329921770'
worksheet_id = 'obafa2y'
#worksheet_id = 'od6'

spr_client = gdata.spreadsheet.service.SpreadsheetsService()
spr_client.email = email
spr_client.password = password
spr_client.source = 'AMO-LC69'
spr_client.ProgrammaticLogin()

def get_xtcs(run):
    L = os.listdir(xtcdir)
    M = [run in l for l in L]
    return L[M]
def get_processed_run_set():
    return set(os.listdir(h5dir))
def get_available_run_set():
    xtcs = os.listdir(xtcdir)
    runs = set([xtc[5:5+5] for xtc in xtcs])
    return runs
def get_folders_run_set():
    L = os.listdir(h5dir)
    M = [(len(l) == 5) for l in L]
    LS = []
    for m,l in zip(M,L):
        if m:
            LS.append(l)
    return set(LS)
def prepare_run(run):
    t = time.time()
    st = datetime.datetime.fromtimestamp(t).strftime('%Y%m%d_%H%M%S')
    runfolder = h5dir + "/" + run + "_" + st + "/"
    runlink = h5dir + "/" + run
    os.system("mkdir %s" % runfolder)
    runfile = runfolder + "/process.sh"
    outfile = runfolder + "/bsub.out"
    cheetah_configfile = "%s/cheetah_%s.ini" % (confdir,run)  
    if not os.path.ispath(cheetah_configfile):
        cheetah_configfile = "%s/cheetah.ini" % (confdir)  
    os.system("cp %s %s" % (cheetah_configfile,runfolder))
    psana_configfile = "%s/psana_%s.cfg" % (confdir,run)  
    if not os.path.ispath(psana_configfile):
        psana_configfile = "%s/psana.cfg" % (confdir)  
    psana_configfile = confdir + "/psana.cfg"
    os.system("cp %s %s" % (psana_configfile,runfolder))
    txt = ["#!/bin/bash\n"]
    txt += ["source %s\n" % environment]
    txt += ["cd %s\n" % runfolder]
    txt += ["psana -c psana.cfg %s/*%s*.xtc\n" % (xtcdir,run)]
    txt += ["ln -s -f %s %s\n" % (runfolder,runlink)]
    with open(runfile,"w") as f:
        f.writelines(txt)
    os.system("chmod u+x %s" % runfile)
    return outfile,runfile
def get_terminal_shape():
    rows, columns = os.popen('stty size', 'r').read().split()
    return (int(rows),int(columns))  
def clear_screen():
    sys.stdout.write("\033[2J") # clear screen and goes to position 0      
    sys.stdout.write("\033[H")
    sys.stdout.flush()
def update_runs_info():
    global finished_runs
    global changed
    global runs_info
    folders_runs = get_folders_run_set()
    folders_runs = list(folders_runs - finished_runs)
    for l in folders_runs:
        logf = h5dir+"/"+l+"/log.txt"
        if os.path.isfile(logf):
            runs_info[l] = {} 
            with open(logf,"r") as f:
                loglines = f.readlines()
                if ">-------- End of job --------<\n" in loglines:
                    runs_info[l]["status"] = "done"
                    for line in loglines:
                        if "Frames processed:" in line:
                            runs_info[l]["#frames"] = line.split(" ")[-1][:-1]
                        elif "Number of hits:" in line:
                            runs_info[l]["#hits"] = line.split(" ")[-1][:-1]
                        elif "Average hit rate:" in line:
                            runs_info[l]["hit rate"] = line.split(" ")[-2][:-1]
                    finished_runs.add(l)
                    changed = True
                else:
                    runs_info[l]["status"] = "submitted"
def print_table_terminal():
    rs = runs_info.keys()
    rs.sort()
    print "# Cheetah queue status"
    print ("-"*term_shape[1])
    print ("Run\tstatus\t#frames\t#hits\thit rate")
    for i in range(term_shape[0]-5):
        if i <= (len(rs)-1):
            d = runs_info[rs[i]]
            print rs[i] + "\t" + d["status"] + "\t" + d["#frames"] + "\t" + d["#hits"] + "\t" + d["hit rate"]
        else:
            print "-\t-\t-\t-\t-" 
    print ("-"*term_shape[1])
def write_table_google():
    write_row_google(0,["Run","status","#frames","#hits","hit rate [%]"])
    rs = runs_info.keys()
    rs.sort()
    for i,r in zip(range(len(rs)),rs):
        d = runs_info[rs[i]]
        write_row_google(i+1,[rs[i],d["status"],d["#frames"],d["#hits"],d["hit rate"]])
def write_row_google(row,values): 
    for col,value in zip(range(len(values)),values):
        entry = spr_client.UpdateCell(row=row+1, col=col+1, inputValue=value,
                                      key=spreadsheet_key, wksht_id=worksheet_id)



if len(sys.argv) < 2:
    sys.exit("Usage: busb_cheetah.py configurationfile")



hostname = socket.gethostname()
if not ("psexport" in hostname or "pslogin" in hostname):
    sys.exit("ERROR: Cannot submit jobs from this host. You are logged in to %s but you need to be logged in to psexport of pslogin to submit jobs to the queue." % hostname)

configfilename = sys.argv[1]

C = configobj.ConfigObj(configfilename)
h5dir = os.path.expandvars(C["h5dir"])
xtcdir = os.path.expandvars(C["xtcdir"])
confdir = os.path.expandvars(C["confdir"])
environment = os.path.expandvars(C["environment"])

processed_runs = get_processed_run_set()
submitted_runs = set([])
finished_runs = set([])
runs_info = {}
info_submitted_runs = {}

while True:
    sys.stdout.write("\r- Checking process status...                        ")
    sys.stdout.flush()
    changed = False
    term_shape = get_terminal_shape()
    clear_screen()
    print_table_terminal()
    available_runs = get_available_run_set()
    runs_to_process = available_runs - processed_runs - submitted_runs
    if len(runs_to_process) > 0:
        for run in runs_to_process:
            print "Submitting job for run %s..." % run
            outfile,runfile = prepare_run(run)
            os.system("bsub -q psnehq -o %s %s" % (outfile,runfile))
            submitted_runs.add(run)
            changed = True
    else:
        dt = 10
        mill = ["-","\\","|","/"]
        sys.stdout.write("\n")
        for i in range(dt):
            sys.stdout.write("\r"+mill[i%len(mill)] + " Waiting for new XTC files to appear...")
            sys.stdout.flush()
            time.sleep(0.1)
    update_runs_info()
    if changed:
        sys.stdout.write("\r- Writing to google spreadsheet...                        ")
        sys.stdout.flush()
        write_table_google()
    
