

class TerminalTable:
    def __init__(self):
        self.shape = get_terminal_shape()
        self.col_names = ["Run","Type","Status","#Frames","#Hits","HRate"]
        header = ""
        for c in self.col_names: header += c + "\t"
        self.title = ["# Cheetah queue status",("-"*self.shape[1]),"",("-"*self.shape[1])]
        self.put_message("Initializing...")
        self.run_dict = {}
    def set_message(self,message):
        self.message = [("-"*self.shape[1]),message]
    def add_run(self,run_name,run):
        self.run_dict[run_name] = run
        
    def print_table_terminal():
        rs = runs_types.keys()
        rs.sort()
        for i in range(term_shape[0]-5):
            if i <= (len(rs)-1):
                d = runs_info.get(rs[i],{})
	    #print runs_info,rs[i]
                print rs[i] + "\t" + runs_types[rs[i]] + "\t" + d.get("status","-") + "\t" + d.get("#frames","-") + "\t" + d.get("#hits","-") + "\t" + d.get("hrate","-")
            else:
                print "-\t-\t-\t-\t-\t-" 
        print ("-"*term_shape[1])
        

