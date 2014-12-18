import os,sys
import time,datetime


class Run:
    def __init__(self,name,C,xtcs):
        self.name = name
        self.run_nr = int(name[1:])
        self.C = C
        self.clear()
        self.xtcs = xtcs
    def clear(self):
        self.attrs_copy = {}
        self.new = True
        self.blocked = True
        self.test_blocked = True
        self.attrs = {"Name": self.name,
                      "Type": "",
                      "No. Frames": "-",
                      "No. Hits": "-",
                      "Hit Ratio": "-",
                      "Process Rate": "-"}
        self.logfile = None
        self.processdir = None
        self.processdir_test = None
        self.processdirs_started = []
        self.processdirs_tests_started = []
        self.darkcal = None
        self.xtcs = []
        self.messages = ""
        self._refresh_status()
    def update(self,attrs=None):
        if attrs != None:
            self._change_attrs(attrs)
        if self.attrs["Type"] in ["Dark","Data"]:
            if self.processdir == None:
                self._load_processdir()
            if self.processdir != None and self.logfile == None:
                self._load_logfile()
        self._refresh_status()
        self._refresh_attrs()
        if not self.test_blocked and self.attrs["Status"] not in ["Invalid","Waiting"]:
            if self.darkcal == None:
                self.darkcal = self._get_prior_darkcal()
            if self.processdir_test == None and self.darkcal != None:
                self._init_processdir(test=True)
            if self.processdir_test != None and self.darkcal != None:
                self._start(test=True)
                self.test_blocked = True
        if not self.blocked and self.attrs["Status"] in ["Ready"]:
            if self.processdir == None:
                self._init_processdir(test=False)
            if self.processdir != None:
                self._start(test=False)
        self.check_for_finished_runs()
    def check_for_finished_runs(self):
        for s,P in zip(["test ",""],[self.processdirs_tests_started,self.processdirs_started]):
            P_cp = list(P)
            for p in P_cp:
                if self._finished(p):
                    self.append_message("Finished %srun (%s)." % (s,p))
                    P.remove(p)
    def _finished(self,processdir):
        l = processdir[:-len(processdir.split("/")[-1])]+self.name
        if os.path.exists(l):
            if os.readlink(l) == processdir.split("/")[-1]:
                return True
        return False
    def append_message(self,s):
        ts = time.time()
        st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
        self.messages += st + " " + self.name + ": " + s + "\n" 
    def get_messages(self):
        m = str(self.messages)
        self.messages = ""
        return m
    def get(self):
        attrs = dict(self.attrs)
        attrs["Messages"] = self.get_messages() 
        return attrs
    def get_if_touched(self):
        touched = self.new
        if not touched:
            for n,i in self.attrs_copy.items():
                if n in self.attrs:
                    if self.attrs[n] != self.attrs_copy[n]:
                        touched = True
                        break
                else:
                    touched = True
                    break
        if not touched:
            for n,i in self.attrs.items():
                if n in self.attrs_copy:
                    if self.attrs[n] != self.attrs_copy[n]:
                        touched = True
                        break
                else:
                    touched = True
                    break
        self.attrs_copy = dict(self.attrs)
        attrs = dict(self.attrs)
        attrs["Messages"] = self.get_messages()
        touched = touched or (attrs["Messages"] != "")
        self.new = False
        if touched:
            return attrs
        else:
            return None
    def _delete(self):
        if os.path.expandvars(self.C["general"]["job_manager"]) == "lsf":
            os.system("bkill -J C%s" % self.name)
            os.system("bkill -J C%sS" % self.name)
        elif os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            os.system("scancel --name=C%s" % self.name)
            os.system("scancel --name=C%sS" % self.name)
        if len([f for f in os.listdir(self.C["locations"]["h5dir"]) if self.name in f]) > 0:
            os.system("rm -r %s/*%s*" % (self.C["locations"]["h5dir"],self.name))
        if len([f for f in os.listdir(self.C["locations"]["h5dir_dark"]) if self.name in f]) > 0:
            os.system("rm -r %s/*%s*" % (self.C["locations"]["h5dir_dark"],self.name))
        self.clear()
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
    def _refresh_status(self):
        if self.attrs["Type"] not in ["Data","Dark"]:
            self.attrs["Status"] = "Invalid"
        elif self.processdir == None:
            if self.attrs["Type"] == "Data":
                if self.darkcal == None:
                    self.darkcal = self._get_prior_darkcal()
                if self.darkcal == None:
                    self.attrs["Status"] = "Waiting"
                else:
                    self.attrs["Status"] = "Ready"
            else:
                self.attrs["Status"] = "Ready"
        elif self.logfile == None:
            self.attrs["Status"] = "Started"
        else:
            with open(self.logfile,"r") as f:
                loglines = f.readlines()
                if ">-------- End of job --------<\n" in loglines:
                    self.attrs["Status"] = "Finished"
                else:
                    self.attrs["Status"] = "Running"
    def _change_attrs(self,attrs):
        for n,it in attrs.items():
            if n == "Cmd":
                if attrs["Cmd"] == "Delete":
                    self._delete()
                    self.blocked = True
                elif attrs["Cmd"] == "Start":
                    self.blocked = False
                elif attrs["Cmd"] == "Start Test":
                    self.test_blocked = False
                # STILL NEEDED?
                elif attrs["Cmd"] == "refreshDark":
                    self.darkcal = self._get_prior_darkcal()
                    if self.darkcal == None:
                        self.attrs["Darkcal"] = "-"
                    else:
                        self.attrs["Darkcal"] = re.search("r[0-9][0-9][0-9][0-9]",self.darkcal).group()
            else:
                self.attrs[n] = it
    def _refresh_attrs(self):
        if self.logfile == None:
            self.attrs["No. Frames"] = "-"
            self.attrs["No. Hits"] = "-"
            self.attrs["Hit Ratio"] = "-"
            self.attrs["Process Rate"] = "-"
        else:
            with open(self.logfile,"r") as f:
                loglines = f.readlines()
                if self.attrs["Status"] == "Running":
                    for line in loglines:
                        for frag in line.split(", "):
                            if "nFrames:" in frag:
                                self.attrs["No. Frames"] = frag.split(" ")[-1]
                            if "nHits:" in frag:
                                self.attrs["No. Hits"] = frag.split(" ")[-2]
                                self.attrs["Hit Ratio"] = frag.split(" ")[-1][1:-2] + " %"                   
                            if "wallTime" in frag:
                                self.attrs["Process Rate"] = frag.split(" ")[-2][1:] + " Hz"
                elif self.attrs["Status"] == "Finished":
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
    def _init_processdir(self,test=False):
        st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y%m%d_%H%M%S')
        if self.attrs["Type"] == "Data":
            if test:
                rootdir = self.C["locations"]["h5dir"] + "/test"
            else:
                rootdir = self.C["locations"]["h5dir"]
            processdir = rootdir + "/" + self.name + "_" + st
        elif self.attrs["Type"] == "Dark":
            if test:
                rootdir = self.C["locations"]["h5dir_dark"] + "/test"
            else:
                rootdir = self.C["locations"]["h5dir_dark"]
            processdir = rootdir + "/" + self.name + "_" + st
        if not os.path.exists(rootdir):
            os.system("mkdir %s" % rootdir)
        os.system("mkdir %s" % processdir)
        processexec = processdir + "/" + "process.sh"
        if test:
            self.processdir_test = processdir
            self.processexec_test = processexec
            self.processout_test = processdir + "/" + "process.out"
            processout = self.processout_test
            self.cheetah_ini_test = processdir + "/" + "cheetah.ini"
            self.psana_cfg_test = processdir + "/" + "psana.cfg"
        else:
            self.processdir = processdir
            self.processexec = processexec
            self.processout = processdir + "/" + "process.out"
            processout = self.processout
            self.cheetah_ini = processdir + "/" + "cheetah.ini"
            self.psana_cfg = processdir + "/" + "psana.cfg"
        self._init_process_config(test)
        self._write_processexec(processout,processdir,self.C["locations"]["psanaexec"],processexec,test)
    def _start(self,test=False):
        if test:
            processout = self.processout_test
            processexec = self.processexec_test
        else:
            processout = self.processout
            processexec = self.processexec
        if os.path.expandvars(self.C["general"]["job_manager"]) == "lsf":
            s = "bsub -n %s -q psnehq -J C%s -o %s %s" % (self.C["general"]["n_cores_per_job"],self.name,processout,processexec)
        elif os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            s = "sbatch %s" % processexec
        os.system(s)
        if test:
            self.processdirs_tests_started.append(self.processdir_test)
            self.processdir_test = None
            self.append_message("Started test run (%s).\n" % self.processdirs_tests_started[-1])
        else:
            self.processdirs_started.append(self.processdir)
            self.append_message("Started run (%s)." % self.processdirs_started[-1])           
    def _init_process_config(self,test=False):
        # select source files for configurations
        if self.attrs["Type"] == "Data":
            cdir = self.C["locations"]["confdir"]
        elif self.attrs["Type"] == "Dark":
            cdir = self.C["locations"]["confdir_dark"]
        c = {}
        if test:
            cheetah_ini = self.cheetah_ini_test
            psana_cfg = self.psana_cfg_test
        else:
            cheetah_ini = self.cheetah_ini
            psana_cfg = self.psana_cfg
        for f,prefix,suffix in zip([cheetah_ini,psana_cfg],["cheetah","psana"],["ini","cfg"]): 
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
                elif "=" in l:
                    ll = l.split("=")
                    ls_n.append(ll[0] + "=" + os.path.expandvars(ll[1]))

                    vname = l.split("=")[0].replace(" ","").lower()
                    if self.attrs["Type"] == "Data":
                        if vname == "detectorid":
                            detectorID = int(l[:-1].split("=")[-1])
                            pixelmask = self._get_pixelmask(detectorID)
                            geometry = self._get_geometry(detectorID)
                            ls_n.append("initialPixelmask=%s\n" % pixelmask)
                            ls_n.append("geometry=%s\n" % geometry)
                            #print self.name,self._get_prior_darkcal()
                            ls_n.append("darkcal=%s\n" % self.darkcal.replace("#",str(detectorID)))
                else:
                    ls_n.append(l)
        with open(cheetah_ini,"w") as f:
            f.writelines(ls_n)
        os.system("cp %s %s" % (c["psana"],psana_cfg))       
    def _get_pixelmask(self,detectorID):
        ls = os.listdir(self.C["locations"]["pixelmaskdir"])
        pixelmask_run = "pixelmask-detectorID%i_%s.h5" % (detectorID,self.name)
        pixelmask_default = "pixelmask-detectorID%i.h5" % detectorID
        if pixelmask_run in ls:
            return (self.C["locations"]["pixelmaskdir"]+"/"+pixelmask_run)
        elif pixelmask_default in ls:
            return (self.C["locations"]["pixelmaskdir"]+"/"+pixelmask_default)
        else:
            print "ERROR: Did not find suitable pixelmask in %s!" % self.C["locations"]["pixelmaskdir"]
            return None
    def _get_geometry(self,detectorID):
        ls = os.listdir(self.C["locations"]["geometrydir"])
        geometry_run = "geometry-detectorID%i_%s.h5" % (detectorID,self.name)
        geometry_default = "geometry-detectorID%i.h5" % detectorID
        if geometry_run in ls:
            return (self.C["locations"]["geometrydir"]+"/"+geometry_run)
        elif geometry_default in ls:
            return (self.C["locations"]["geometrydir"]+"/"+geometry_default)
        else:
            print "ERROR: Did not find suitable geometry in %s!" % self.C["locations"]["geometrydir"]
            return None
    def _write_processexec(self,processout,processdir,psanaexec,processexec,test=False):
        txt = ["#!/bin/bash\n"]
        if os.path.expandvars(self.C["general"]["job_manager"]) == "slurm":
            txt += "#SBATCH --job-name=C%s\n" % self.name
            txt += "#SBATCH -N1\n"
            txt += "#SBATCH -n%s\n" % self.C["general"]["n_cores_per_job"]
            txt += "#SBATCH --output=%s\n" % processout
        txt += ["cd %s\n" % processdir]
        if test:
            s_nframes = ("-n %s " % self.C["general"]["n_frames_test"])
        else:
            s_nframes = ""
        txt += ["%s -c psana.cfg %s%s/*%s*.xtc\n" % (psanaexec,s_nframes,self.C["locations"]["xtcdir"],self.name)]
        txt += ["if [ ! -f *.cxi ] && [ ! -f *.h5 ] ; then exit ; fi\n"]
        txt += ["cd ..\n"]
        s = processdir.split("/")[-1]
        txt += ["ln -sfn %s %s\n" % (s,self.name)]
        txt += ["chgrp -R %s %s\n" % (self.C["general"]["unix_group"],s)]
        txt += ["chmod -R g+xr %s\n" % s]
        txt += ["chgrp %s %s\n" % (self.C["general"]["unix_group"],self.name)]
        txt += ["chmod g+xr %s\n" % (self.name)]
        with open(processexec,"w") as f:
            f.writelines(txt)
        os.system("chmod u+x %s" % processexec)
