import os,time
import PySide
from PySide import QtGui,QtCore
import numpy as np

THIS_DIR = os.path.dirname(os.path.realpath(__file__))
HEADER = ["Type","Command","Status","Process Rate","No. Frames","Hit Ratio"]
RUN_TYPES = ["","Data","Dark"]

class RunTableWidget(QtGui.QTableWidget):
    changed = QtCore.Signal(dict)
    def __init__(self,C):
        QtGui.QTableWidget.__init__(self)
        self.C = C
        self.runs = []
        self.indices = []
        self.names = []
        self.setColumnCount(len(HEADER))
        self.setHorizontalHeaderLabels(HEADER)
    def addRun(self,rAttr):
        #t0 = time.time()
        i = int(rAttr["Name"][1:])
        N = len(self.runs)
        R = RunRow(self,self.C,rAttr)
        self.runs.append(R)
        self.setRowCount(N+1)
        self.setVerticalHeaderLabels(self.names)
        self.indices.append(i)
        self.names.append(rAttr["Name"])
        self.setCellWidget(N,0,R.typeCBox)
        self.setCellWidget(N,1,R.cmdPButton)
        self.setCellWidget(N,2,R.statusLabel)
        self.setCellWidget(N,3,R.pRateLabel)
        self.setCellWidget(N,4,R.nFramesLabel)
        self.setCellWidget(N,5,R.hitRatioLabel)
        #print rAttr["Name"],N,self.names
        #print "Adding run took ", (time.time()-t0)
    def update(self,rList):
        #print "update",rList
        for rAttr in rList:
            if rAttr["Name"] not in self.names:
                self.addRun(rAttr)
            else:
                i = self.names.index(rAttr["Name"])
                self.runs[i].update(rAttr)
        #print "Done with update"

class RunRow(QtCore.QObject):
    def __init__(self,parent,C,rAttr):
        self.parent = parent
        self.C = C
        # Read run info into dict
        self.rAttr = rAttr
        self.rAttr["Type"] = rAttr.get("Type","-")
        self.rAttr["Status"] = rAttr.get("Status","-")
        self.rAttr["Process Rate"] = rAttr.get("Process Rate","-")
        self.rAttr["No. Hits"] = rAttr.get("No. Hits","-")
        self.rAttr["No. Frames"] = rAttr.get("No. Frames","-")
        self.rAttr["Hit Ratio"] = rAttr.get("Hit Ratio","-")
        # Initialize items and widgets
        #self.nameLEdit = QtGui.QLineEdit(self.parent,rAttr["Name"])
        #self.nameLEdit.setReadOnly(True)
        self.typeCBox = QtGui.QComboBox(self.parent)
        self.typeCBox.addItems(RUN_TYPES)
        self.typeCBox.setCurrentIndex(RUN_TYPES.index(self.rAttr["Type"]))
        self.cmdPButton = QtGui.QPushButton("")
        self.statusLabel = QtGui.QLabel(rAttr["Status"])
        self.statusLabel.setAlignment(QtCore.Qt.AlignCenter)
        s = rAttr["Status"]
        if s in ["Invalid","Waiting","-"]:
            self.statusLabel.setStyleSheet("background-color: rgb(100, 100, 100);")
        elif s in ["Started","Postponed","Running"]:
            self.statusLabel.setStyleSheet("background-color: rgb(0, 0, 255);")
        elif s == "Finished":
            self.statusLabel.setStyleSheet("background-color: rgb(0, 255, 0);")
        #self.statusTItem.setReadOnly(True)
        self.wheel = QtGui.QMovie(THIS_DIR+"/loader.gif")
        self.wheel.start()
        self.pRateLabel = QtGui.QLabel(rAttr["Process Rate"])
        self.pRateLabel.setAlignment(QtCore.Qt.AlignCenter)
        #self.pRateLabel.setReadOnly(True)
        self.nFramesLabel = QtGui.QLabel(rAttr["No. Frames"])
        self.nFramesLabel.setAlignment(QtCore.Qt.AlignCenter)
        #self.nFramesLabel.setReadOnly(True)
        self.hitRatioLabel = QtGui.QLabel(rAttr["Hit Ratio"])
        self.hitRatioLabel.setAlignment(QtCore.Qt.AlignCenter)
        #self.hitRatioLabel.setReadOnly(True)
        # Connect signals
        self.connectSignals()
        # update all items and widgets with info in run dict
        self.update(rAttr)
    def disconnectSignals(self):
        self.typeCBox.currentIndexChanged.disconnect(self.emitChange)
        self.cmdPButton.released.disconnect(self.emitCmd)
    def connectSignals(self):
        self.typeCBox.currentIndexChanged.connect(self.emitChange)
        self.cmdPButton.released.connect(self.emitCmd)       
    def update(self,rAttr):
        self.disconnectSignals()
        if rAttr == {}:
            return
        self.rAttr = rAttr
        #self.nameLabel.setText(rAttr["Name"])
        self.typeCBox.setCurrentIndex(self.typeCBox.findText(rAttr["Type"]))
        s = self.rAttr["Status"]
        if s in ["Started","Postponed","Running","Finished"]:
            self.cmdPButton.setText("Delete")
        elif s in ["Waiting","Invalid"]:
            self.cmdPButton.setText("Start")
        self.cmdPButton.setEnabled(s not in ["Invalid"])
        self.statusLabel.setText(rAttr.get("Status","-"))
        if s in ["Invalid","Waiting"]:
            self.statusLabel.setStyleSheet("background-color: rgb(100, 100, 100);")
        elif s in ["Started","Postponed","Running"]:
            self.statusLabel.setStyleSheet("background-color: rgb(0, 0, 255);")
        elif s == "Finished":
            self.statusLabel.setStyleSheet("background-color: rgb(0, 255, 0);")
        self.pRateLabel.setText(rAttr.get("Process Rate","-"))
        self.nFramesLabel.setText(rAttr.get("No. Frames","-"))
        self.hitRatioLabel.setText(rAttr.get("Hit Ratio","-"))
        self.connectSignals()
    def emitChange(self):
        rAttr = {}
        rAttr["Name"] = self.rAttr["Name"]
        rAttr["Type"] = self.typeCBox.currentText()
        self.parent.changed.emit(rAttr)
    def emitCmd(self):
        rAttr = {}
        rAttr["Name"] = self.rAttr["Name"]
        rAttr["Type"] = self.typeCBox.currentText()
        rAttr["Cmd"] = self.cmdPButton.text()
        self.statusLabel.setText("")
        self.statusLabel.setMovie(self.wheel)
        self.cmdPButton.setEnabled(False)
        self.parent.changed.emit(rAttr)
        
#class ComboBoxNoWheel(QtGui.QComboBox):
#    def wheelEvent (self, event):
#        event.ignore()
