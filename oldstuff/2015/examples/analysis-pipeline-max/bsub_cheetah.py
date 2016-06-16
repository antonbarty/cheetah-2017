#!/usr/bin/env python
import configobj
import os,sys
import time,datetime
import socket
import numpy as np

import getpass

print "Gmail password [maxhantke@gmail.com]:"
email = "maxhantke@gmail.com"
password = getpass.getpass()

spreadsheet_key = '1hAWODbWaFRUnmIoTO1LhxS8tEcyrwzJu_sN2jlkEmc0'
worksheet_id = 'obafa2y'

run_col = 0
run_type_col = 1

dry_run = False



def get_xtcs(run):
    L = os.listdir(xtcdir)
    M = [run in l for l in L]
    return L[M]
def get_submitted_run_set():
    L_data = os.listdir(h5dir)
    lens = np.array([len(l) for l in L_data],dtype="int")
    L_data = np.array(L_data)[lens>5]
    L_data = [l[:5] for l in L_data]
    L = set(L_data)
    L_dark = os.listdir(h5dir_dark)
    lens = np.array([len(l) for l in L_dark],dtype="int")
    L_dark = np.array(L_dark)[lens>5]
    L_dark = [l[:5] for l in L_dark]
    L = L.union(set(np.array(L_dark)))
    return L
def get_available_run_set():
    xtcs = os.listdir(xtcdir)
    #print xtcs
    runs = set([xtc[5:5+5] for xtc in xtcs]) - set([""])
    return runs
def get_folders_run_list():
    L = []
    R = []
    L_data = os.listdir(h5dir)
    lens = np.array([len(l) for l in L_data],dtype="int")
    L_data = list(np.array(L_data)[lens>5])
    for l in L_data:
	L.append(h5dir+"/"+l)
	R.append(l[:5])
    L_dark = os.listdir(h5dir_dark)
    lens = np.array([len(l) for l in L_dark],dtype="int")
    L_dark = list(np.array(L_dark)[lens>5])
    for l in L_dark:
	L.append(h5dir_dark+"/"+l)
	R.append(l[:5])
    L.sort()
    R.sort()
    return R,L
def prepare_run(run):
    print "prepare",run
    run_type = runs_types[run]
    t = time.time()
    st = datetime.datetime.fromtimestamp(t).strftime('%Y%m%d_%H%M%S')
    if run_type == "data": 
        darkcal = get_prior_darkcal(run)
        print "dark",darkcal
        if darkcal == None:
            return None,None
	runfolder = h5dir + "/" + run + "_" + st
	runlink = h5dir + "/" + run
    elif run_type == "dark":
	runfolder = h5dir_dark + "/" + run + "_" + st
	runlink = h5dir_dark + "/" + run
    os.system("mkdir %s" % runfolder)
    runfile = runfolder + "/process.sh"
    outfile = runfolder + "/bsub.out"
    cheetah_configfile = get_cheetah_configfile(run,run_type)
    os.system("cp %s %s/cheetah.ini" % (cheetah_configfile,runfolder))
    if run_type == "data":
	insert_darkcal_in_cheetah_configfile(darkcal,"%s/cheetah.ini" % runfolder)
    psana_configfile = "%s/psana_%s.cfg" % (confdir,run)  
    if not os.path.isfile(psana_configfile):
        psana_configfile = "%s/psana.cfg" % (confdir)  
    psana_configfile = confdir + "/psana.cfg"
    os.system("cp %s %s/psana.cfg" % (psana_configfile,runfolder))
    txt = ["#!/bin/bash\n"]
    txt += ["source %s\n" % environment]
    txt += ["cd %s\n" % runfolder]
    txt += ["psana -c psana.cfg %s/*%s*.xtc\n" % (xtcdir,run)]
    txt += ["ln -s -f %s %s\n" % (runfolder,runlink)]
    with open(runfile,"w") as f:
        f.writelines(txt)
    os.system("chmod u+x %s" % runfile)
    return outfile,runfile
def get_runs_types():
    #print "getrt"
    g_runs = read_col_google(run_col)[1:]
    g_types = read_col_google(run_type_col)[1:]
    #print g_runs,g_types
    D = {}
    N = min([len(g_runs),len(g_types)])
    for r,t in zip(g_runs[:N],g_types[:N]):
	D[r] = t
    return D
def get_cheetah_configfile(run,run_type):
    if run_type == "data":
	cheetah_configfile = "%s/cheetah_template_%s.ini" % (confdir,run)
        if not os.path.isfile(cheetah_configfile):
	    cheetah_configfile = "%s/cheetah_template.ini" % (confdir)
    elif run_type == "dark":
	cheetah_configfile = "%s/cheetah_%s.ini" % (confdir_dark,run)  
        if not os.path.isfile(cheetah_configfile):
	    cheetah_configfile = "%s/cheetah.ini" % (confdir_dark)
    return cheetah_configfile
def insert_darkcal_in_cheetah_configfile(darkcal,cheetah_configfile):
    with open(cheetah_configfile,"r") as f:
	ls = f.readlines()
	ls_n = []
	for l in ls:
	    if l[0] == "#":
		ls_n.append(l)
	    else:
		if "darkcal" in l:
		    ls_n.append("darkcal=%s\n" % darkcal)
		else:
                    if "=" in l:
                        ls_n.append(os.path.expandvars(l.split("=")[0]) + "=" + os.path.expandvars(l.split("=")[1]))	    
                    else:
                        ls_n.append(l)
    #print ls_n
    #sys.exit()
    with open(cheetah_configfile,"w") as f:
	f.writelines(ls_n)
def get_prior_darkcal(run):
    run_nr = int(run[1:])
    dirs = os.listdir(h5dir_dark)
    lens = [len(l) for l in dirs]
    L = []
    for i,l in enumerate(lens):
	if l == 5:
	    L.append(dirs[i][1:])
    if L == []:
	return None
    L = np.array(L,dtype="int")
    if L.min() > run_nr:
	return None
    #print L,run_nr
    L = L[run_nr > L]
    #print L
    drun = "r%04i" % L[-1]
    # check whether run is finished
    print runs_info
    if runs_info.get(drun,{}).get("status",None) != "done":
        return None
    darkcal = h5dir_dark + "/" + drun + "/" + ("%s-pnCCD-darkcal.h5" % drun)
    return darkcal
def get_terminal_shape():
    rows, columns = os.popen('stty size', 'r').read().split()
    return (int(rows),int(columns))  
def clear_screen():
    sys.stdout.write("\033[2J") # clear screen and goes to position 0      
    sys.stdout.write("\033[H")
    sys.stdout.flush()
def update_runs_info():
    global changed
    global runs_info
    global finished_runs
    global submitted_runs
    runs_info = {}
    runs,folders_runs = get_folders_run_list()
    #print runs,folders_runs
    for r,l in zip(runs,folders_runs):
	runs_info[r]={}
        runs_info[r]["type"] = runs_types[r]
        logf = l+"/log.txt"
        if os.path.isfile(logf):
            with open(logf,"r") as f:
                loglines = f.readlines()
                if ">-------- End of job --------<\n" in loglines:
                    runs_info[r]["status"] = "done"
                    for line in loglines:
                        if "Frames processed:" in line:
                            runs_info[r]["#frames"] = line.split(" ")[-1][:-1]
                        elif "Number of hits:" in line:
                            runs_info[r]["#hits"] = line.split(" ")[-1][:-1]
                        elif "Average hit rate:" in line:
                            runs_info[r]["hrate"] = line.split(" ")[-2][:-1]
		    finished_runs.add(r)
                else:
                    runs_info[r]["status"] = "subm"
		    submitted_runs.add(r)
	    changed = True
	else:
	    runs_info[r]["status"] = "-"
def get_runs_with_valid_types_set():
    ir = set([])
    for r,t in runs_types.items():
	if t in ["data","dark"]:
	    ir.add(r)
    return ir

if len(sys.argv) < 2:
    sys.exit("Usage: busb_cheetah.py configurationfile")

hostname = socket.gethostname()
if not ("psexport" in hostname or "pslogin" in hostname):
    sys.exit("ERROR: Cannot submit jobs from this host. You are logged in to %s but you need to be logged in to psexport of pslogin to submit jobs to the queue." % hostname)

configfilename = sys.argv[1]

C = configobj.ConfigObj(configfilename)
h5dir = os.path.expandvars(C["h5dir"])
h5dir_dark = os.path.expandvars(C["h5dir_dark"])
xtcdir = os.path.expandvars(C["xtcdir"])
confdir = os.path.expandvars(C["confdir"])
confdir_dark = os.path.expandvars(C["confdir_dark"])
environment = os.path.expandvars(C["environment"])

processed_runs = set([])
submitted_runs = get_submitted_run_set()
finished_runs = set([])
runs_info = {}
info_submitted_runs = {}

while True:
    changed = False
    sys.stdout.write("\r- Checking process status...                        ")
    sys.stdout.flush()
    runs_types = get_runs_types()
    runs_with_types_set = get_runs_with_valid_types_set()
    available_runs = get_available_run_set()
    print "ar",available_runs
    print "sr",submitted_runs
    print "rwt",runs_with_types_set
    update_runs_info()
    runs_to_process = (available_runs - submitted_runs).intersection(runs_with_types_set)
    clear_screen()
    print_table_terminal()
    print runs_to_process
    if len(runs_to_process) > 0:
	tmp = list(runs_to_process)
	tmp.sort()
        run = tmp[0]
	sys.stdout.write("Submitting job for run %s..." % run)
	sys.stdout.flush()
	outfile,runfile = prepare_run(run)
	if not dry_run and outfile != None and runfile != None:
	    substr = "bsub -q psnehq -o %s %s" % (outfile,runfile)
	    os.system(substr)
	    sys.stdout.write("\r"+substr)
	    sys.stdout.flush()
	    submitted_runs.add(run)
	    changed = True
	else:
	    sys.stdout.write("\r"+"Submission postponed...                       ")
	    sys.stdout.flush()
	    time.sleep(1.)
    else:
        dt = 10
        mill = ["-","\\","|","/"]
        sys.stdout.write("\n")
        for i in range(dt):
            sys.stdout.write("\r"+mill[i%len(mill)] + " Waiting for new XTC files to appear...")
            sys.stdout.flush()
            time.sleep(0.1)
    if changed:
        sys.stdout.write("\r- Writing to google spreadsheet...                        ")
        sys.stdout.flush()
        write_table_google()
    
