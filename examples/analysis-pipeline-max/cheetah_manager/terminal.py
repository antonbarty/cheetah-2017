import os,sys
import subprocess
import time
import curses


class TerminalTable:
    def __init__(self,conf):
        self.conf = conf
        self.mill_index = 0
        self.screen = curses.initscr()
        self.refresh_terminal_shape()
        curses.noecho() 
        curses.curs_set(0) 
        self.screen.keypad(1)
        self.col_names = ["Run","Type","Status","#Frames","#Hits","HRate"]
        self.title = ["# Cheetah queue status",("-"*self.shape[0]),"",("-"*self.shape[0])]
        self.runs = {}
        self.set_message("Initializing...")
    def set_message(self,message):
        self.message = [message]
    def note(self,message):
        self.message = [message]
        self.print_screen()
    def set_runs(self,runs):
        self.runs = runs
        self.print_screen()
    def _get_s_head_lines(self):
        s_head = []
        s_head.append("="*self.shape[1])
        s = "CHEETAH RUNNER"
        l = self.shape[1]-len(s)-4
        s_head.append("||"+" "*(l/2)+s+" "*(l/2)+"||")
        s_head.append("="*self.shape[1])
        width = self.shape[1]/56
        s = ""
        for w in range(width):
            for c in self.col_names:
                s += c + "\t"
            s += "   ||   "
        s_head.append(s[:-9])
        return s_head
    def _get_s_message(self):
        s_message = ""
        for m in self.message:
            s_message += m #+ "\n"
        return s_message
    def _get_s_content_lines(self):
        s_content = ""
        names = self.runs.keys()
        names.sort()
        length = self.shape[0]-8-self.Nlines_jobs-(self.Nlines_jobs>0)
        width = self.shape[1]/56
        s_content_lines = [""] * length
        for i in range(length*width):
            if i <= (len(names)-1):
                R = self.runs[names[-i-1]]
                s_content_lines[i%length] += names[-i-1] + "\t"
                for c in self.col_names[1:]:
                    s_content_lines[i%length] += R.attrs.get(c,"-") + "\t"
                s_content_lines[i%length] += "   ||   "
            else:
                s_content_lines[i%length] += ("-\t"*len(self.col_names)) + "   ||   "
        s_content = ""
        for i in range(length):
            s_content_lines[i] = s_content_lines[i][:-9]# + "\n"
        return s_content_lines
    def _get_s_jobs_lines(self):
        s_jobs = []
        if os.path.expandvars(self.conf["general"]["job_manager"]) == "lsf":
            c = ["bjobs"]
        elif os.path.expandvars(self.conf["general"]["job_manager"]) == "slurm":
            c = ["squeue",os.path.expandvars("-u$USER")]
        #print c
        p = subprocess.Popen(c,stdout=subprocess.PIPE)
        p.wait()
        lines = p.stdout.readlines()
        self.Nlines_jobs = len(lines)
        for l in lines:
            s_jobs.append(l[:-1])
        return s_jobs
    def refresh_terminal_shape(self):
        self.shape = self.screen.getmaxyx()
    def print_screen(self):
        self.refresh_terminal_shape()
        s_head_lines = self._get_s_head_lines()
        s_message = self._get_s_message()
        s_jobs_lines = self._get_s_jobs_lines()
        s_content_lines = self._get_s_content_lines()
        #self.clear_screen()
        self.screen.clear()
        y = 0
        for s_head_line in s_head_lines:
            self.screen.addstr(y,0,s_head_line); y+=1
        self.screen.addstr(y,0,"-"*self.shape[1]); y+=1
        for s_content_line in s_content_lines:
            self.screen.addstr(y,0,s_content_line); y+=1
        self.screen.addstr(y,0,"-"*self.shape[1]); y+=1
        for s_jobs_line in s_jobs_lines:
            self.screen.addstr(y,0,s_jobs_line); y+=1
        self.screen.addstr(y,0,"-"*self.shape[1]); y+=1
        mill =  ["-","\\","|","/"][self.mill_index%4]
        self.mill_index += 1
        self.screen.addstr(y,0,mill + " " +  s_message); y+=1
        self.screen.refresh()

