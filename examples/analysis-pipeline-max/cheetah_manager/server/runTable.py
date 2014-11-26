import os
import re,csv
import run

class RunTable:
    def __init__(self,C):
        self.C = C
        self.R = {}
        self.readTable()
    def getValidRuns(self):
        return [r.name for r in self.R if r.type != ""]
    def _getRunsWithXTCs(self):
        print "Collecting runs with XTCs"
        xtcdir = self.C["locations"]["xtcdir"]
        tmp = list(set([re.search("r[0-9][0-9][0-9][0-9]",l).group() for l in os.listdir(xtcdir) if (re.search("r[0-9][0-9][0-9][0-9].*xtc$",l) != None)]))
        tmp.sort()
        return tmp
    def checkForNewXTCs(self):
        runs = self._getRunsWithXTCs()        
        for r in runs:
            if r not in self.R:
                self.R[r] = run.Run(r,self.C)
    def update(self):
        # Check files in XTC directory and if new runs appeared add them to the table
        self.checkForNewXTCs()
        names = self.R.keys()
        names.sort()
        for n in names:
            self.R[n].update()
    def getList(self,getAll=True):
        out = []
        names = self.R.keys()
        names.sort()
        for n in names:
            r = self.R[n]
            attrs = r.get_if_touched()
            if attrs != None or getAll:
                out.append(attrs)
        return out
    def getListUpdate(self):
        return self.getList(False)
    def readTable(self):
        filename = self.C["general"]["runtable_filename"] 
        runs_with_xtcs = self._getRunsWithXTCs()
        for r in runs_with_xtcs:
            if r not in self.R:
                print "Adding run " + r
                self.R[r] = run.Run(r,self.C)
        if os.path.isfile(filename):
            with open(filename,"r") as csvfile:
                reader = csv.reader(csvfile)
                for [n,t] in reader:
                    if n in self.R:
                        print "Setting type for run " + r
                        self.R[n].attrs["Type"] = t 
    def writeTable(self):
        filename = self.C["general"]["runtable_filename"]
        with open(filename,"w") as csvfile:
            writer = csv.writer(csvfile)
            runs_names = self.R.keys()
            runs_names.sort()
            for r in runs_names:
                row = []
                row.append(self.R[r].attrs["Name"])
                row.append(self.R[r].attrs["Type"])
                writer.writerow(row)
    
                        

