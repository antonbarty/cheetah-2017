import os,time
import PySide
from PySide import QtGui,QtCore
import numpy as np

THIS_DIR = os.path.dirname(os.path.realpath(__file__))
HEADER = ["Type","Test","Run","Status","Process Rate","No. Frames","Hit Ratio"]
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
        self.setCellWidget(N,1,R.cmdTestPButton)
        self.setCellWidget(N,2,R.cmdPButton)
        self.setCellWidget(N,3,R.statusLabel)
        self.setCellWidget(N,4,R.pRateLabel)
        self.setCellWidget(N,5,R.nFramesLabel)
        self.setCellWidget(N,6,R.hitRatioLabel)
        #print rAttr["Name"]#,N,self.names
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
        self.typeCBox = ComboBoxNoWheel(self.parent)
        self.typeCBox.addItems(RUN_TYPES)
        self.typeCBox.setCurrentIndex(RUN_TYPES.index(self.rAttr["Type"]))
        self.cmdPButton = PushButtonNoWheel("")
        self.cmdTestPButton = PushButtonNoWheel("Start Test")
        self.test_started = False
        self.statusLabel = QtGui.QLabel(rAttr["Status"])
        self.statusLabel.setAlignment(QtCore.Qt.AlignCenter)
        s = rAttr["Status"]
        if s in ["Invalid","-"]:
            self.statusLabel.setStyleSheet("background-color: rgb(150, 150, 150);")
        elif s in ["Ready"]:
            self.statusLabel.setStyleSheet("background-color: rgb(180, 180, 180);")
        elif s in ["Started","Waiting","Running"]:
            self.statusLabel.setStyleSheet("background-color: rgb(50, 50, 255);")
        elif s == "Finished":
            self.statusLabel.setStyleSheet("background-color: rgb(50, 255, 50);")
        self.wheel = QtGui.QMovie(THIS_DIR+"/loader.gif")
        self.wheel.start()
        self.pRateLabel = QtGui.QLabel(rAttr["Process Rate"])
        self.pRateLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.nFramesLabel = QtGui.QLabel(rAttr["No. Frames"])
        self.nFramesLabel.setAlignment(QtCore.Qt.AlignCenter)
        self.hitRatioLabel = QtGui.QLabel(rAttr["Hit Ratio"])
        self.hitRatioLabel.setAlignment(QtCore.Qt.AlignCenter)
        # Connect signals
        self.connectSignals()
        # update all items and widgets with info in run dict
        self.update(rAttr)
    def disconnectSignals(self):
        self.typeCBox.currentIndexChanged.disconnect(self.emitChange)
        self.cmdPButton.released.disconnect(self.emitCmd)
        self.cmdTestPButton.released.disconnect(self.emitCmdTest)
    def connectSignals(self):
        self.typeCBox.currentIndexChanged.connect(self.emitChange)
        self.cmdPButton.released.connect(self.emitCmd)       
        self.cmdTestPButton.released.connect(self.emitCmdTest)       
    def update(self,rAttr0):
        self.disconnectSignals()
        rAttr = dict(rAttr0)
        if rAttr == {}:
            return
        s = self.rAttr["Status"]
        if "Messages" in rAttr:
            M = rAttr.pop("Messages")
        else:
            M = ""
        self.rAttr = rAttr
        #self.nameLabel.setText(rAttr["Name"])
        self.typeCBox.setCurrentIndex(self.typeCBox.findText(rAttr["Type"]))
        if s in ["Started","Waiting","Running","Finished"]:
            self.cmdPButton.setText("Delete")
        elif s in ["Ready","Invalid"]:
            self.cmdPButton.setText("Start")
        self.cmdPButton.setEnabled(s not in ["Invalid"])
        if not self.test_started:
            self.cmdTestPButton.setEnabled(s not in ["Invalid"])
        else:
            if "Finished test run" in M:
                self.cmdTestPButton.setEnabled(s not in ["Invalid"])
                self.test_started = False
        self.statusLabel.setText(rAttr.get("Status","-"))
        if s in ["Invalid",""]:
            self.statusLabel.setStyleSheet("background-color: rgb(150, 150, 150);")
        elif s in ["Ready"]:
            self.statusLabel.setStyleSheet("background-color: rgb(180, 180, 180);")
        elif s in ["Started","Waiting","Running"]:
            self.statusLabel.setStyleSheet("background-color: rgb(50, 50, 255);")
        elif s == "Finished":
            self.statusLabel.setStyleSheet("background-color: rgb(50, 255, 50);")
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
        self.statusLabel.setStyleSheet("background-color: rgb(210, 210, 210);")
        self.statusLabel.setMovie(self.wheel)
        self.cmdPButton.setEnabled(False)
        self.parent.changed.emit(rAttr)
    def emitCmdTest(self):
        rAttr = {}
        rAttr["Name"] = self.rAttr["Name"]
        rAttr["Type"] = self.typeCBox.currentText()
        rAttr["Cmd"] = self.cmdTestPButton.text()
        self.test_started = True
        self.cmdTestPButton.setEnabled(False)
        self.parent.changed.emit(rAttr)

        
class ComboBoxNoWheel(QtGui.QComboBox):
    def wheelEvent (self, event):
        event.ignore()

class PushButtonNoWheel(QtGui.QPushButton):
    def wheelEvent (self, event):
        event.ignore()
