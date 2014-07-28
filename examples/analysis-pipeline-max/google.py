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
    # CONVENIENCE
    def get_run_type(self,run_name):
        if self.tabDict == None:
            return None
        runs = self.tabDict["Run"]
        types = self.tabDict["Type"]
        return types[runs.index(run_name)]
    def get_run_cmd(self,run_name):
        if self.tabDict == None:
            return None
        runs = self.tabDict["Run"]
        cmds = self.tabDict["Cmd"]
        return cmds[runs.index(run_name)]
    def get_valid_runs(self):
        if self.tabDict == None:
            return None
        runs = self.tabDict["Run"]
        stats = self.tabDict["Type"]
        vr = []
        for i,t in zip(range(len(types)),types):
            if t in ["dark","data"]:
                vr.append(runs[i])
        return vr
    def get_runs_to_update(self):
        if self.tabDict == None:
            return None
        runs = self.tabDict["Run"]
        cmds = self.tabDict["Cmd"]
        vr = []
        for i,c in zip(range(len(cmds)),cmds):
            if c != "auto":
                vr.append(runs[i])
        return vr
    def pop_run_cmd(self,run_name):
        g_run_names = self.read_col_table(run_col)
        if run_name not in g_run_names:
            return None
        else:
            s = self.read_cell_table(g_run_names.index(run_name),run_cmd_col)
            #print s
            if s == "":
                return None
            else:
                self.write_cell_table(g_run_names.index(run_name),run_cmd_col,"auto")
                return s
    def clear_run(self,run_name,run_type):
        g_run_names = self.read_col_table(run_col)
        if run_name not in g_run_names:
            return None
        else:
            self.write_row_table(g_run_names.index(run_name),[run_name,run_type,"auto"]+[""]*4)
    def get_valid_runs(self):
        g_run_names = self.read_col_table(run_col)
        g_run_types = self.read_col_table(run_type_col)
        g_run_names = g_run_names[:min([len(g_run_types),len(g_run_names)])]
        g_run_types = g_run_types[:min([len(g_run_types),len(g_run_names)])]
        return [r for i,r in enumerate(g_run_names) if g_run_types[i] in ["dark","data"]]    
    # WRITING
    def write_cell(self,row,col,value):
        t0 = time.time()
        if value != None:
            self.wks.update_cell(row+1,col+1,value)
    def write_cell_table(self,row,col,value):
        self.write_cell(row+1,col,value)
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
    def write_row_table(self,row,values):
        self.write_row(row+1,values)
    def write_col(self,col,values): 
        if len(values) == 1:
            write_cell(0,col,values[0])
        else:
            s = "%s%i:%s%i" % (chr(col + ord('a')).upper(),1,chr(col + ord('a')).upper(),len(values))
            cell_list = self.wks.range(s)
            for v,c in zip(values,cell_list):
                c.value = v
            self.wks.update_cells(cell_list)
    def write_col_table(self,col,values):
        self.write_col(col,values)
    # WRITING
    def write(self,runs):
        ks = runs.keys()
        D = self.tabDict
        run_names_old = D["Run"]
        rows = [title]
        #for i,r in zip(range(len(ks)),ks):
        for i,r in zip(range(len(run_names_old)),run_names_old):
            if r in ks:
                d = runs[r].attrs
                vs = [r,runs[r].type,d.get("Cmd","auto"),d.get("Status",""),d.get("#Frames",""),d.get("#Hits",""),d.get("HRate","")]
            else:
                vs = [r,D["Type"][i],D["Cmd"][i],D["Status"][i],D["#Frames"][i],D["#Hits"][i],D["HRate"][i]]
            rows.append(vs)
        #self.write_row_table(i,vs)
        # Select a range
        s = "A1:%s%i" % (chr(len(title) - 1 + ord('a')).upper(),len(rows))
        cell_list = self.wks.range(s)
        print len(cell_list),len(rows),len(title)
        #sys.exit()
        print rows
        for i,c in zip(range(len(cell_list)),cell_list):
            icol = i % len(title)
            irow = i / len(rows)
            #print i
            #print icol,irow
            #print len(rows),len(rows[irow])
            c.value = rows[irow][icol]
        self.wks.update_cells(cell_list)
    # READING
    def update_table_dict(self):
        tab0 = self.wks.get_all_values()
        self.tabDict = {}
        titles = tab0[0]
        for t in titles:
            self.tabDict[t] = []
        for c in tab0[1:]:
            for i,t in zip(range(len(titles)),titles):
                self.tabDict[t].append(c[i])
    def read_row(self,row):
        r = self.wks.row_values(row+1)
        #print r
        return r
    def read_row_table(self,row):
        return self.read_row(row)
    def read_col(self,col):
        c = self.wks.col_values(col+1)
        #print c
        return c 
    def read_cols(self,col_min,col_max):
        cs = []
        for col in range(col_min,col_max+1,1):
            cs.append(self.wks.col_values(col+1))
        return cs
    def read_col_table(self,col):
        return self.read_col(col)[1:]
    def read_cols_table(self,col_min,col_max):
        cs = self.read_cols(col_min,col_max)
        for i in range(len(cs)):
            cs[i] = cs[i][1:]
        return cs
    def read_cell(self,row,col):
        t0 = time.time()
        c = self.wks.cell(row+1,col+1).value
        #print "reading cell dt=%f" % (time.time()-t0)
        return c
    def read_cell_table(self,row,col):
        return self.read_cell(row+1,col)
