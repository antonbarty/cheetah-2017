import gdata.spreadsheet.service


class GoogleTable:
    def __init__(self,email,password,spreadsheet_key,worksheet_key):
        self.email = email
        self.password = password
        self.speadsheet_key = spreadsheet_key
        self.worksheet_key = worksheet_key
        self.spr_client = gdata.spreadsheet.service.SpreadsheetsService()
        self.spr_client.email = email
        self.spr_client.password = password
        self.spr_client.source = 'AMO-LC69'
        self.spr_client.ProgrammaticLogin()
    def write_table(self):
        self.write_row(0,["Run","Type","Status","#Frames","#Hits","Hit rate [%]"])
        rs = runs_info.keys()
        rs.sort()
        for i,r in zip(range(len(rs)),rs):
            d = runs_info[rs[i]]
            self.write_row(i+1,[rs[i],d["type"],d["status"],d.get("#frames",""),d.get("#hits",""),d.get("hrate","")])
    def write_row(self,row,values): 
        for col,value in zip(range(len(values)),values):
            entry = self.spr_client.UpdateCell(row=row+1, col=col+1, inputValue=value,
                                               key=spreadsheet_key, wksht_id=worksheet_id)
    def read_row(self,row):
        cells = []
        col = 0
        while True:
            c = self.read_cell(row,col)
            if c != None:
                cells.append(c)
                col += 1
            else:
                break
        return cells
    def read_col(col):
        cells = []
        row = 0
        while True:
            c = self.read_cell(row,col)
            if c != None:
                cells.append(c)
                row += 1
            else:
                break
        return cells
    def read_cell(row,col):
        s = self.spr_client.GetCellsFeed(key=spreadsheet_key, wksht_id=worksheet_id,cell="R%iC%i" % (row+1,col+1)).content.text
        return s
