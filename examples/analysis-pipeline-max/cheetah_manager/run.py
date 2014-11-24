import os,sys
import time,datetime


class Run:
    def __init__(self,name,C):
        self.name = name
        self.run_nr = int(name[1:])
        #self.google_table = google_table
        self.C = C
        self.clear()
    def clear(self):
        self.stopped = True
        self.prepared = False
        self.logfile_present = False
        self.started = False
        self.started_swmr = False
        self.attrs = {}
        self.type = ""
        self.xtcs = []
        self.logfile = None
        self.status = None
        self.processdir = None
        self._load_xtcs()
        self._load_processdir()
        self._load_logfile()
        self._load_status()
    def setType(self,type):
        self.type = type
    def update(self,message): 
        if message["cmd"] == "delete":
            self._delete()
        if message["cmd"] == "start":
            self.stopped = False
        if self.xtcs == []:
            self._load_xtcs()
        if self.processdir == None:
            self._load_processdir()
        if not self.logfile_present:
            self._load_logfile()
        self._load_status()
        self._refresh_attrs()
        if not self.stopped:
            if not self.prepared:
                self._init_process()
            if self.prepared:
                self._start()
    def _init_process(self):
        ready = False
        if self.type == "data": 
            self.darkcal = self._get_prior_darkcal()
            if self.darkcal != None:
                ready = True
        elif self.type == "dark":
            ready = True
        if ready:
            self._init_processdir()
            self.prepared = True
    def _start(self):
        if os.path.expandvars(self.C["general"]["job_manager"]) == "lsf":
            s = "bsub -n %s -q psnehq -J C%s -o %s %s" % (self.C["general"]["n_cores_per_job"],self.name,self.processout,self.processexec)
        elif os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            s = "sbatch %s" % self.processexec
        os.system(s)
        self.started = True
    def _start_swmr(self):
        if self.type == "dark":
            return 
        if not self.prepared:
            print "ERROR: Trying to start non-prepared run. Aborting..."
            sys.exit(0)
        self.processdir_swmr = self.C["locations"]["h5dir_swmr"]+"/"+self.name+"_"+self.st
	self.psanaexec_swmr = self.C["locations"]["psanaexec_swmr"]
        self.processexec_swmr = self.processdir_swmr+"/process.sh"
        self.processout_swmr = self.processdir_swmr+"/process.out"
        os.system("cp -r %s %s/" % (self.processdir,self.processdir_swmr))
        self.cheetah_ini_swmr = self.processdir_swmr+"/cheetah.ini"
        with open(self.cheetah_ini_swmr,"r") as f:
            ls = f.readlines()
        ls_n = []
        for l in ls:
            if "cxiswmr" in l.lower():
                ls_n.append("cxiswmr=1\n")
            else:
                ls_n.append(l)
        with open(self.cheetah_ini_swmr,"w") as f:
            f.writelines(ls_n)
        self._write_processexec(self.processout_swmr,self.processdir_swmr,self.psanaexec_swmr,self.processexec_swmr)
        if os.path.expandvars(self.C["general"]["job_manager"]) == "lsf":
            s = "bsub -n 6 -q psnehq -J C%sS -o %s %s" % (self.name,self.processout_swmr,self.processexec_swmr)
        elif os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            s = "sbatch %s" % self.processexec_swmr
        os.system(s)
        self.started_swmr = True
    def _delete(self):
        if os.path.expandvars(self.C["general"]["job_manager"]) == "lsf":
            os.system("bkill -J C%s" % self.name)
            os.system("bkill -J C%sS" % self.name)
        elif os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            os.system("scancel --name=C%s" % self.name)
            os.system("scancel --name=C%sS" % self.name)
        os.system("rm -r %s/*%s*" % (self.C["locations"]["h5dir"],self.name))
        if self.C["general"].as_bool("swmr"): 
            os.system("rm -r %s/*%s*" % (self.C["locations"]["h5dir_swmr"],self.name))
        os.system("rm -r %s/*%s*" % (self.C["locations"]["h5dir_dark"],self.name))
        self.clear()
    #def _load_type(self):
    #    self.type = self.google_table.get_run_type(self.name)
    def _load_xtcs(self):
        self.xtcs =  [(self.C["locations"]["xtcdir"]+"/"+l) for l in os.listdir(self.C["locations"]["xtcdir"]) if self.name in l]
    def _load_processdir(self):
        if self.type == "":
            self.processdir = None
        else:
            rootdir = {"dark":self.C["locations"]["h5dir_dark"],"data":self.C["locations"]["h5dir"]}[self.type]
            dirs = [(rootdir+"/"+l) for l in os.listdir(rootdir) if self.name in l and len(l) > 5]
            if dirs == []:
                self.processdir = None
            else:
                dirs.sort()
                self.processdir = dirs[-1] # load most recent one
    def _load_logfile(self):
        if self.processdir == None:
            self.logfile = None
            self.logfile_present = False
            return
        logf = self.processdir + "/" + "log.txt"
        if os.path.isfile(logf):
            self.logfile = logf
            self.logfile_present = True
        else:
            self.logfile = logf
            self.logfile_present = False
    def _load_status(self):
        if self.type not in ["data","dark"]:
            self.status = "invalid"
        elif self.xtcs == []:
            self.status = "no XTCs"
        elif self.processdir == None:
            if self.started:
                self.status = "started"
            else:
                self.status = "new"
        elif not self.logfile_present:
            if self.started:
                self.status = "started"
            else:
                self.status = "ready"
        else:
            with open(self.logfile,"r") as f:
                loglines = f.readlines()

                if ">-------- End of job --------<\n" in loglines:
                    self.status = "ended"
                else:
                    self.status = "runs"
    def _refresh_attrs(self):
        self.attrs = {}
        # ["Run","Type","Status","#Frames","#Hits","HRate"]
        self.attrs["Type"] = self.type
        self.attrs["Status"] = self.status
        self.attrs["Cmd"] = self.cmd
        if self.logfile != None and self.logfile_present:
            with open(self.logfile,"r") as f:
                loglines = f.readlines()
                if self.status == "runs":
                    for line in loglines:
                        for frag in line.split(", "):
                            if "nFrames:" in frag:
                                self.attrs["#Frames"] = frag.split(" ")[-1]
                            if "nHits:" in frag:
                                self.attrs["#Hits"] = frag.split(" ")[-2]
                                self.attrs["HRate%"] = frag.split(" ")[-1][1:-2]                   
                            if "wallTime" in frag:
                                self.attrs["FRateHz"] = frag.split(" ")[-2][1:]
                elif self.status == "ended":
                    for line in loglines:
                        if "Frames processed:" in line:
                            self.attrs["#Frames"] = line.split(" ")[-1][:-1]
                        elif "Number of hits:" in line:
                            self.attrs["#Hits"] = line.split(" ")[-1][:-1]
                        elif "Average hit rate:" in line:
                            self.attrs["HRate%"] = line.split(" ")[-2][:-1]
                        elif "Average frame rate:" in line:
                            self.attrs["FRateHz"] = line.split(" ")[-2]
    def _get_prior_darkcal(self):
        dcals = [(self.C["locations"]["h5dir_dark"]+"/"+l+"/"+("%s-pnCCD-detectorID#-darkcal.h5" % (l[:5]))) for l in os.listdir(self.C["locations"]["h5dir_dark"]) if (len(l) == 5) and (int(l[1:]) < self.run_nr)]
        if dcals == []:
            return None
        dcals.sort()
        return dcals[-1]       
    def _init_processdir(self):
        self.st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y%m%d_%H%M%S')
        if self.type == "data":
            self.processdir = self.C["locations"]["h5dir"] + "/" + self.name + "_" + self.st
        elif self.type == "dark":
            self.processdir = self.C["locations"]["h5dir_dark"] + "/" + self.name + "_" + self.st
	self.psanaexec = self.C["locations"]["psanaexec"]
        self.processexec = self.processdir + "/" + "process.sh"
        self.processout = self.processdir + "/" + "process.out"
        self.cheetah_ini = self.processdir + "/" + "cheetah.ini"
        self.psana_cfg = self.processdir + "/" + "psana.cfg"
        os.system("mkdir %s" % self.processdir)
        self._init_process_config()
        self._write_processexec(self.processout,self.processdir,self.psanaexec,self.processexec)
    def _init_process_config(self):
        # select source files for configurations
        if self.type == "data":
            cdir = self.C["locations"]["confdir"]
        elif self.type == "dark":
            cdir = self.C["locations"]["confdir_dark"]
        c = {}
        for f,prefix,suffix in zip([self.cheetah_ini,self.psana_cfg],["cheetah","psana"],["ini","cfg"]): 
            c_template_run = cdir+"/"+("%s_template_%s.%s" % (prefix,self.name,suffix))
            c_template = cdir+"/"+("%s_template.%s" % (prefix,suffix))
            if os.path.isfile(c_template_run):
                c[prefix] = c_template_run
            elif os.path.isfile(c_template):
                c[prefix] = c_template
            else:
                print "ERROR: No configuration file available for %s" % self.name
        # write cheetah.ini
        with open(c["cheetah"],"r") as f:
            ls = f.readlines()
            ls_n = []
            for l in ls:
                if l[0] == "#":
                    ls_n.append(l)
                else:
                    appended = False
                    if "detectorid" in l.lower() and not "hitfinder" in l.lower():
                        ls_n.append(l)
                        if self.type == "data":
                            if self.darkcal != None:
                                detectorID = l[:-1].split("=")[-1]
                                ls_n.append("darkcal=%s\n" % self.darkcal.replace("#",detectorID))
                            appended = True
                    if "darkcal" in l.lower():
                        ls_n.append("#"+l)
                    if not appended:
                        if "=" in l:
                            ll = l.split("=")
                            ls_n.append(ll[0] + "=" + os.path.expandvars(ll[1]))	    
                        else:
                            ls_n.append(l)
                        
        with open(self.cheetah_ini,"w") as f:
                f.writelines(ls_n)
        #if self.C["swmr"]
        # write psana.cfg
        os.system("cp %s %s" % (c["psana"],self.psana_cfg))        
    def _write_processexec(self,processout,processdir,psanaexec,processexec):
        txt = ["#!/bin/bash\n"]
        if os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            txt += "#SBATCH --job-name=C%s\n" % self.name
            txt += "#SBATCH -N1\n"
            txt += "#SBATCH -n%s\n" % self.C["general"]["n_cores_per_job"]
            txt += "#SBATCH --output=%s\n" % processout
        txt += ["cd %s\n" % processdir]
        txt += ["%s -c psana.cfg %s/*%s*.xtc\n" % (psanaexec,self.C["locations"]["xtcdir"],self.name)]
        txt += ["if [ ! -f *.cxi ] && [ ! -f *.h5 ] ; then exit ; fi\n"]
        txt += ["cd ..\n"]
        s = processdir.split("/")[-1]
        txt += ["ln -s -f %s %s\n" % (s,self.name)]
        txt += ["chgrp -R %s %s\n" % (self.C["general"]["unix_group"],s)]
        txt += ["chmod -R g+xr %s\n" % s]
        txt += ["chgrp %s %s\n" % (self.C["general"]["unix_group"],self.name)]
        txt += ["chmod g+xr %s\n" % (self.name)]
        with open(processexec,"w") as f:
            f.writelines(txt)
        os.system("chmod u+x %s" % processexec)
