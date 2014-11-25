import os,sys
import time,datetime


class Run:
    def __init__(self,name,C):
        self.name = name
        self.run_nr = int(name[1:])
        self.C = C
        self.clear()
    def clear(self):
        self.attrs_copy = {}
        self.touched = True
        self.blocked = True
        self.prepared = False
        self.started = False
        self.attrs = {"Name": self.name, "Type": ""}
        self.xtcs = []
        self.logfile = None
        self.processdir = None
        self.darkcal = None
        self._load_xtcs()
        self._load_processdir()
        self._load_logfile()
        self._refresh_status()
    def update(self,attrs=None):
        if attrs != None:
            self._change_attrs(attrs)
        if self.xtcs == []:
            self._load_xtcs()
        if self.attrs["Type"] in ["Dark","Data"]:
            if self.processdir == None:
                self._load_processdir()
            if self.processdir != None and self.logfile == None:
                self._load_logfile()
        self._refresh_status()
        self._refresh_attrs()
        if not self.blocked and self.attrs["Status"] == "Waiting":
            if self.processdir == None:
                self._init_processdir()
            if self.prepared:
                self._start()
        self._check_touched()
    def _check_touched(self):
        self.touched = False
        for n,i in self.attrs_copy.items():
            if n in self.attrs:
                if self.attrs[n] != self.attrs_copy[n]:
                    self.touched = True
                    break
            else:
                self.touched = True
                break
        for n,i in self.attrs.items():
            if n in self.attrs_copy:
                if self.attrs[n] != self.attrs_copy[n]:
                    self.touched = True
                    break
            else:
                self.touched = True
                break
        self.attrs_copy = dict(self.attrs)
    def _start(self):
        if os.path.expandvars(self.C["general"]["job_manager"]) == "lsf":
            s = "bsub -n %s -q psnehq -J C%s -o %s %s" % (self.C["general"]["n_cores_per_job"],self.name,self.processout,self.processexec)
        elif os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            s = "sbatch %s" % self.processexec
        os.system(s)
        self.started = True
    def _delete(self):
        if os.path.expandvars(self.C["general"]["job_manager"]) == "lsf":
            os.system("bkill -J C%s" % self.name)
            os.system("bkill -J C%sS" % self.name)
        elif os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            os.system("scancel --name=C%s" % self.name)
            os.system("scancel --name=C%sS" % self.name)
        os.system("rm -r %s/*%s*" % (self.C["locations"]["h5dir"],self.name))
        os.system("rm -r %s/*%s*" % (self.C["locations"]["h5dir_dark"],self.name))
        self.clear()
    def _load_xtcs(self):
        self.xtcs =  [(self.C["locations"]["xtcdir"]+"/"+l) for l in os.listdir(self.C["locations"]["xtcdir"]) if self.name in l]
    def _load_processdir(self):
        if self.attrs["Type"] == "":
            self.processdir = None
            return
        rootdir = {"Dark":self.C["locations"]["h5dir_dark"],"Data":self.C["locations"]["h5dir"]}[self.attrs["Type"]]
        dirs = [(rootdir+"/"+l) for l in os.listdir(rootdir) if self.name in l and len(l) > 5]
        if dirs == []:
            self.processdir = None
        else:
            dirs.sort()
            self.processdir = dirs[-1] # load most recent one
    def _load_logfile(self):
        if self.processdir == None:
            self.logfile = None
            return
        logf = self.processdir + "/" + "log.txt"
        if os.path.isfile(logf):
            self.logfile = logf
            self.logfile_present = True
        else:
            self.logfile = logf
            self.logfile_present = False
    def _refresh_status(self):
        if self.attrs["Type"] not in ["Data","Dark"]:
            self.attrs["Status"] = "Invalid"
        elif self.processdir == None:
            if self.attrs["Type"] == "Data":
                if self.darkcal == None:
                    self.darkcal = self._get_prior_darkcal()
                if self.darkcal == None:
                    self.attrs["Status"] = "Postponed"
                else:
                    self.attrs["Status"] = "Waiting"
            else:
                self.attrs["Status"] = "Waiting"
        elif self.logfile == None:
            self.attrs["Status"] = "Started"
        else:
            with open(self.logfile,"r") as f:
                loglines = f.readlines()
                if ">-------- End of job --------<\n" in loglines:
                    self.attrs["Status"] = "Finished"
                else:
                    self.attrs["Status"] = "Runs"
    def _change_attrs(self,attrs):
        for n,it in attrs.items():
            if n == "Cmd":
                if attrs["Cmd"] == "Delete":
                    self._delete()
                    self.blocked = True
                elif attrs["Cmd"] == "Start":
                    self.blocked = False
                elif attrs["Cmd"] == "refreshDark":
                    self.darkcal = self._get_prior_darkcal()
                    if self.darkcal == None:
                        self.attrs["Darkcal"] = "-"
                    else:
                        self.attrs["Darkcal"] = re.search("r[0-9][0-9][0-9][0-9]",self.darkcal).group()
            else:
                self.attrs[n] = it
    def _refresh_attrs(self):
        if self.logfile != None and self.logfile_present:
            with open(self.logfile,"r") as f:
                loglines = f.readlines()
                if self.rAttr["Status"] == "Runs":
                    for line in loglines:
                        for frag in line.split(", "):
                            if "nFrames:" in frag:
                                self.attrs["No. Frames"] = frag.split(" ")[-1]
                            if "nHits:" in frag:
                                self.attrs["No. Hits"] = frag.split(" ")[-2]
                                self.attrs["Hit Ratio"] = frag.split(" ")[-1][1:-2] + " %"                   
                            if "wallTime" in frag:
                                self.attrs["Process Rate"] = frag.split(" ")[-2][1:] + " Hz"
                elif self.rAttr["Status"] == "finished":
                    for line in loglines:
                        if "Frames processed:" in line:
                            self.attrs["No. Frames"] = line.split(" ")[-1][:-1]
                        elif "Number of hits:" in line:
                            self.attrs["No. Hits"] = line.split(" ")[-1][:-1]
                        elif "Average hit rate:" in line:
                            self.attrs["Hit Ratio"] = line.split(" ")[-2][:-1] + " %"
                        elif "Average frame rate:" in line:
                            self.attrs["Process Rate"] = line.split(" ")[-2] + " Hz"
    def _get_prior_darkcal(self):
        dcals = [(self.C["locations"]["h5dir_dark"]+"/"+l+"/"+("%s-pnCCD-detectorID#-darkcal.h5" % (l[:5]))) for l in os.listdir(self.C["locations"]["h5dir_dark"]) if (len(l) == 5) and (int(l[1:]) < self.run_nr)]
        if dcals == []:
            return None
        dcals.sort()
        return dcals[-1]       
    def _init_processdir(self):
        self.st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y%m%d_%H%M%S')
        if self.rAttr["Type"] == "Data":
            self.processdir = self.C["locations"]["h5dir"] + "/" + self.name + "_" + self.st
        elif self.rAttr["Type"] == "Dark":
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
        if self.rAttr["Type"] == "Data":
            cdir = self.C["locations"]["confdir"]
        elif self.rAttr["Type"] == "Dark":
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
                        if self.rAttr["Type"] == "Data":
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
