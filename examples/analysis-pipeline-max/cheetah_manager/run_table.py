import re,csv
import run

class RunTable:
    def __init__(self,C):
        self.C = C
        self.R = {}
        self._readTable()
        self._writeTable()
    def getValidRuns(self):
        return [r.name for r in self.R if r.type != ""]
    def _getRunsWithXTCs(self):
        xtcdir = self.C["locations"]["xtcdir"]
        return list(set([re.search("r[0-9][0-9][0-9][0-9]",l).group() for l in os.listdir(xtcdir) if (re.search("r[0-9][0-9][0-9][0-9].*xtc$",l) != None)])).sort()
    def checkForNewXTCs(self):
        runs = self._getRunsWithXTCs()        
        for r in runs if r not in self.R:
            self.R[r] = Run(r,self.C)
    def setType(self,r,t):
        self.R[r].setType(t)
    def get(self):
        out = []
        for (n,r) in self.R.items():
            out.append({"name": n, "type": r.type})
        return out
    def _readTable(self):
        filename = C["general"]["runtable_filename"] 
        runs_with_xtcs = self.getRunsWithXTCs()
        for r in runs_with_xtcs if r not in R:
            self.R[r] = Run(r,self.C)
        if os.path.isfile(filename):
            with open(filename,"r") as csvfile:
                reader = csv.reader(csvfile)
                for [n,t] in reader:
                    if name in R:
                        self.R[n].setType(t)
    def _writeTable(self):
        filename = C["general"]["runtable_filename"]
        with open(filename,"w") as csvfile:
            writer = csv.writer(csvfile)
            runs_names = self.R.keys()
            runs_names.sort()
            for r in runs_names:                
                writer.writerow([self.R[r].name,self.R[r].type])
    
                        
