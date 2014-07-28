import os,sys
import subprocess

class TerminalTable:
    def __init__(self):
        self.refresh_terminal_shape()
        self.col_names = ["Run","Type","Status","#Frames","#Hits","HRate"]
        self.title = ["# Cheetah queue status",("-"*self.shape[1]),"",("-"*self.shape[1])]
        self.runs = {}
        self.set_message("Initializing...")
    def set_message(self,message):
        self.message = [message]
    def note(self,message):
        self.message = [message]
        self.print_screen()
    def set_runs(self,runs):
        self.runs = runs
        self.message = ["Updating run information"]
        self.print_screen()
    def _get_s_head(self):
        s_head = ""
        s_head += "="*self.shape[0]+"\n"
        s = "CHEETAH RUNNER"
        l = self.shape[0]-len(s)-4
        s_head += "||"+" "*(l/2)+s+" "*(l/2)+"||\n"
        s_head += "="*self.shape[0]+"\n"
        for c in self.col_names:
            s_head += c + "\t"
        s_head = s_head[:-1] + "\n"
        return s_head
    def _get_s_message(self):
        s_message = ""
        for m in self.message:
            s_message += m + "\n"
        return s_message
    def _get_s_content(self):
        s_content = ""
        names = self.runs.keys()
        names.sort()
        for i in range(self.shape[1]-8-self.Nlines_jobs-(self.Nlines_jobs>0)):
            if i <= (len(names)-1):
                R = self.runs[names[i]]
                s_content += names[i] + "\t"
                for c in self.col_names[1:]:
                    s_content += R.attrs.get(c,"-") + "\t"
                s_content = s_content[:-1] + "\n"
            else:
                s_content += ("-\t"*len(self.col_names))[:-1] + "\n"
        return s_content
    def _get_s_jobs(self):
        s_jobs = ""
        p = subprocess.Popen(["bjobs"],stdout=subprocess.PIPE)
        p.wait()
        lines = p.stdout.readlines()
        self.Nlines_jobs = len(lines)
        for l in lines:
            s_jobs += l
        return s_jobs
    def refresh_terminal_shape(self):
        rows, columns = os.popen('stty size', 'r').read().split()
        self.shape = (int(columns),int(rows))
    def clear_screen(self):
        sys.stdout.write("\033[2J") # clear screen and goes to position 0      
        sys.stdout.write("\033[H")
        sys.stdout.flush()
    def print_screen(self):
        self.refresh_terminal_shape()
        s_head = self._get_s_head()
        s_message = self._get_s_message()
        s_jobs = self._get_s_jobs()
        s_content = self._get_s_content()
        self.clear_screen()
        sys.stdout.write(s_head)
        sys.stdout.write("-"*self.shape[0]+"\n")
        sys.stdout.write(s_content)
        sys.stdout.write("-"*self.shape[0]+"\n")
        if self.Nlines_jobs > 0:
            sys.stdout.write(s_jobs)
            sys.stdout.write("-"*self.shape[0]+"\n")
        sys.stdout.write(s_message)
        sys.stdout.flush()
        

