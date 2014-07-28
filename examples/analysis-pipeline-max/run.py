import os,sys
import time,datetime


class Run:
    def __init__(self,run_name,google_table,conf):
        self.run_name = run_name
        self.run_nr = int(run_name[1:])
        self.google_table = google_table
        self.conf = conf
        self.init2()
    def init2(self):
        self.attrs = {}
        self.type = None
        self.xtcs = []
        self.logfile = None
        self.status = None
        self.processdir = None
        self.prepared = False
        self.logfile_present = False
        self.started = False
        self.started_swmr = False
        self._load_type()
        #self.google_table.clear_run(self.run_name,self.type)
        self._load_xtcs()
        self._load_processdir()
        self._load_logfile()
        self._load_status()
    def update(self): 
        self._load_type()
        self._load_xtcs()
        self._load_processdir()
        self._load_logfile()
        self._load_status()
        self._refresh_attrs()
        if self.google_table.get_cmd(self.run_name) != "auto":
            self.cmd = self.google_table.pop_run_cmd(self.run_name)
        if self.cmd == "clear":
            self.clear()
            self.cmd = "auto"           
        #print self.cmd,self.status
    def init_process(self):
        ready = False
        if self.type == "data": 
            if self.cmd != "nodark":
                self.darkcal = self._get_prior_darkcal()
                if self.darkcal != None:
                    ready = True
            else:
                self.darkcal = None
                ready = True
            self.cmd = "auto"
        elif self.type == "dark":
            ready = True
        if ready:
            self._init_processdir()
            self.prepared = True
            #if self.cmd == "nodark":
                #print "A"
                #sys.exit()
    def start(self):
        if not self.prepared:
            print "ERROR: Trying to start non-prepared run. Aborting..."
            sys.exit(0)
        s = "bsub -q psnehq -J C%s -o %s %s" % (self.run_name,self.processout,self.processexec)
        os.system(s)
        self.started = True
    def start_swmr(self):
        if self.type == "dark":
            return 
        if not self.prepared:
            print "ERROR: Trying to start non-prepared run. Aborting..."
            sys.exit(0)
        self.processdir_swmr = self.conf["h5dir_swmr"]+"/"+self.run_name+"_"+self.st
        self.processexec_swmr = self.processdir_swmr+"/process.sh"
        self.processout_swmr = self.processdir_swmr+"/bsub.out"
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
        self._write_processexec_swmr()
        s = "bsub -q psnehq -J C%sS -o %s %s" % (self.run_name,self.processout_swmr,self.processexec_swmr)
        os.system(s)
        self.started_swmr = True
    def clear(self):
        os.system("bkill -J C%s" % self.run_name)
        os.system("bkill -J C%sS" % self.run_name)
        os.system("rm -r %s/*%s*" % (self.conf["h5dir"],self.run_name))
        os.system("rm -r %s/*%s*" % (self.conf["h5dir_swmr"],self.run_name))
        self.init2()
    def _load_type(self):
        self.type = self.google_table.get_run_type(self.run_name)
    def _load_xtcs(self):
        self.xtcs =  [(self.conf["xtcdir"]+"/"+l) for l in os.listdir(self.conf["xtcdir"]) if self.run_name in l]
    def _load_processdir(self):
        if self.type == None:
            self.processdir = None
        else:
            rootdir = {"dark":self.conf["h5dir_dark"],"data":self.conf["h5dir"]}[self.type]
            dirs = [(rootdir+"/"+l) for l in os.listdir(rootdir) if self.run_name in l and len(l) > 5]
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
            self.status = "started"
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
        if self.logfile != None and self.logfile_present:
            with open(self.logfile,"r") as f:
                loglines = f.readlines()
                for line in loglines:
                    if "Frames processed:" in line:
                        self.attrs["#Frames"] = line.split(" ")[-1][:-1]
                    elif "Number of hits:" in line:
                        self.attrs["#Hits"] = line.split(" ")[-1][:-1]
                    elif "Average hit rate:" in line:
                        self.attrs["HRate"] = line.split(" ")[-2][:-1]
    def _get_prior_darkcal(self):
        dcals = [(self.conf["h5dir_dark"]+"/"+l+"/"+("%s-pnCCD-darkcal.h5" % l[:5])) for l in os.listdir(self.conf["h5dir_dark"]) if (len(l) == 5) and (int(l[1:]) < self.run_nr)]
        if dcals == []:
            return None
        dcals.sort()
        return dcals[-1]       
    def _init_processdir(self):
        self.st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y%m%d_%H%M%S')
        if self.type == "data":
            self.processdir = self.conf["h5dir"] + "/" + self.run_name + "_" + self.st
        elif self.type == "dark":
            self.processdir = self.conf["h5dir_dark"] + "/" + self.run_name + "_" + self.st
        self.processexec = self.processdir + "/" + "process.sh"
        self.processout = self.processdir + "/" + "bsub.out"
        self.cheetah_ini = self.processdir + "/" + "cheetah.ini"
        self.psana_cfg = self.processdir + "/" + "psana.cfg"
        os.system("mkdir %s" % self.processdir)
        self._init_process_config()
        self._write_processexec()
    def _init_process_config(self):
        # select source files for configurations
        if self.type == "data":
            cdir = self.conf["confdir"]
        elif self.type == "dark":
            cdir = self.conf["confdir_dark"]
        c = {}
        for f,prefix,suffix in zip([self.cheetah_ini,self.psana_cfg],["cheetah","psana"],["ini","cfg"]): 
            c_template_run = cdir+"/"+("%s_template_%s.%s" % (prefix,self.run_name,suffix))
            c_template = cdir+"/"+("%s_template.%s" % (prefix,suffix))
            if os.path.isfile(c_template_run):
                c[prefix] = c_template_run
            elif os.path.isfile(c_template):
                c[prefix] = c_template
            else:
                print "ERROR: No configuration file available for %s" % self.run_name
        # write cheetah.ini
        with open(c["cheetah"],"r") as f:
            ls = f.readlines()
            ls_n = []
            for l in ls:
                if l[0] == "#":
                    ls_n.append(l)
                else:
                    appended = False
                    if "darkcal" in l:
                        if self.type == "data":
                            if self.darkcal != None:
                                ls_n.append("darkcal=%s\n" % self.darkcal)
                            appended = True
                    if not appended:
                        if "=" in l:
                            ll = l.split("=")
                            ls_n.append(ll[0] + "=" + os.path.expandvars(ll[1]))	    
                        else:
                            ls_n.append(l)
                        
        with open(self.cheetah_ini,"w") as f:
                f.writelines(ls_n)
        #if self.conf["swmr"]
        # write psana.cfg
        os.system("cp %s %s" % (c["psana"],self.psana_cfg))        
    def _write_processexec(self):
        txt = ["#!/bin/bash\n"]
        txt += ["source %s\n" % self.conf["environment"]]
        txt += ["cd %s\n" % self.processdir]
        txt += ["psana -c psana.cfg %s/*%s*.xtc\n" % (self.conf["xtcdir"],self.run_name)]
        txt += ["if [ ! -f *.cxi ] && [ ! -f *.h5 ] ; then exit ; fi\n"]
        txt += ["cd ..\n"]
        s = self.processdir.split("/")[-1]
        txt += ["ln -s -f %s %s\n" % (s,self.run_name)]
        txt += ["chmod -R a+xr %s\n" % s]
        txt += ["chmod a+xr %s\n" % (self.run_name)]
        with open(self.processexec,"w") as f:
            f.writelines(txt)
        os.system("chmod u+x %s" % self.processexec)
    def _write_processexec_swmr(self):
        txt = ["#!/bin/bash\n"]
        txt += ["source %s\n" % self.conf["environment"]]
        txt += ["cd %s\n" % self.processdir_swmr]
        txt += ["psana -c psana.cfg %s/*%s*.xtc\n" % (self.conf["xtcdir"],self.run_name)]
        txt += ["cd ..\n"]
        s = self.processdir.split("/")[-1]
        txt += ["if [ ! -f *.cxi ] && [ ! -f *.h5 ] ; then exit ; fi\n"]
        txt += ["ln -s -f %s %s\n" % (s,self.run_name)]
        txt += ["chmod -R a+xr %s\n" % s]
        txt += ["chmod a+xr %s\n" % (self.run_name)]
        with open(self.processexec_swmr,"w") as f:
            f.writelines(txt)
        os.system("chmod u+x %s" % self.processexec)

