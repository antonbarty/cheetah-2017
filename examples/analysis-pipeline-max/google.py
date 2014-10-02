import os,sys
import gspread
import numpy as np
import time

run_col = 0
run_type_col = 1
run_cmd_col = 2
title = ["Run","Type","Cmd","Status","#Frames","#Hits","HRate"]

class GoogleTable:
    def __init__(self,email,password,spreadsheet_name,worksheet_name):
        self.email = email
        self.password = password
        self.gc = gspread.login(email,password)
        self.sh = self.gc.open(spreadsheet_name)
        self.wks = self.sh.worksheet(os.path.expandvars(worksheet_name))
        self.write_row(0,title)
        self.tabDict = None
        self.tabDict_copy = None
        self.tabDict_modified_by_script = False
        self.tabDict_modified_by_user = False
    # WRITING
    def write(self,runs):
        ks = runs.keys()
        D = self.tabDict
        # check and reapply if a cmd was changed by user
        Dusr = self._read()
        for t,c in self.tabDict_copy.items():
            cusr = Dusr[t]
            for i in range(len(c)):
                if c[i] != cusr[i]:
                    D[t][i] = cusr[i]
        run_names_old = D["Run"]
        rows = [title]
        for i,r in zip(range(len(run_names_old)),run_names_old):
            if r in ks:
                d = runs[r].attrs
                vs = [r,runs[r].type,d.get("Cmd","auto"),d.get("Status",""),d.get("#Frames",""),d.get("#Hits",""),d.get("HRate","")]
            else:
                vs = [r,D["Type"][i],D["Cmd"][i],D["Status"][i],D["#Frames"][i],D["#Hits"][i],D["HRate"][i]]
            rows.append(vs)
        # Select a range
        s = "A1:%s%i" % (chr(len(title) - 1 + ord('a')).upper(),len(rows))
        cell_list = self.wks.range(s)
        for i,c in zip(range(len(cell_list)),cell_list):
            icol = i % len(title)
            irow = i / len(title)
            c.value = rows[irow][icol]
        self.wks.update_cells(cell_list)
        self.tabDict_modified = False
    def write_cell(self,row,col,value):
        t0 = time.time()
        if value != None:
            self.wks.update_cell(row+1,col+1,value)
    def write_row(self,row,values):
        if len(values) == 1:
            write_cell(row,0,values[0])
        else:
            s = "%s%i:%s%i" % (chr(0 + ord('a')).upper(),row+1,chr(len(values) - 1 + ord('a')).upper(),row+1)
            #print s
            cell_list = self.wks.range(s)
            for v,c in zip(values,cell_list):
                c.value = v
            self.wks.update_cells(cell_list)
    # READING
    def _read(self):
        tab0 = self.wks.get_all_values()
        titles = tab0[0]
        tabDict = {}
        for t in titles:
            tabDict[t] = []
        for c in tab0[1:]:
            for i,t in zip(range(len(titles)),titles):
                tabDict[t].append(c[i])
        return tabDict
    def read(self):
        self.tabDict = self._read()
        self.tabDict_copy = dict(self.tabDict)
    # CONVENIENCE
    def get_run_type(self,run_name):
        if self.tabDict == None:
            return None
        runs = self.tabDict["Run"]
        types = self.tabDict["Type"]
        return types[runs.index(run_name)]
    def pop_run_cmd(self,run_name):
        if self.tabDict == None:
            return None
        runs = self.tabDict["Run"]
        cmds = self.tabDict["Cmd"]
        if cmds[runs.index(run_name)] == "clear":
            self.tabDict["Cmd"][runs.index(run_name)] = "auto"
            self.tabDict_modified = True
            return "clear"
        else:
            return cmds[runs.index(run_name)]
    def get_valid_runs(self):
        runs = self.tabDict["Run"]
        types = self.tabDict["Type"]
        cmds = self.tabDict["Cmd"]
        return [r for i,r,c in zip(range(len(runs)),runs,cmds) if (types[i] in ["dark","data"] and runs[i] != "" and cmds[i] != "")]    
