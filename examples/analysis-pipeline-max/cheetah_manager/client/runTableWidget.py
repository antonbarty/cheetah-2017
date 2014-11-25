import PySide
from PySide import QtGui,QtCore
import numpy as np

HEADER = ["Run ID","Type"]#,"Number of events","Number of hits","Hit ratio","Process rate","Status"]


class RunTableWidget(QtGui.QWidget):
    changed = QtCore.Signal(dict)
    def __init__(self,parent,C):
        QtGui.QWidget.__init__(self)
        self.vbox = QtGui.QVBoxLayout(self)
        self.hbox = QtGui.QHBoxLayout(self)
        sp = QtGui.QSizePolicy()
        sp.setHorizontalStretch(1)
        elements = []
        elements.append(QtGui.QLabel("Run ID"))
        elements.append(QtGui.QLabel("Type"))
        elements.append(QtGui.QLabel("Send Command"))
        elements.append(QtGui.QLabel("Status"))
        elements.append(QtGui.QLabel("Process Rate"))
        elements.append(QtGui.QLabel("No. Frames"))
        elements.append(QtGui.QLabel("Hit Ratio"))
        for e in elements:
            e.setSizePolicy(sp)
            e.setMinimumSize(150,25)
            self.hbox.addWidget(e)
        self.vbox.addLayout(self.hbox)
        self.vboxScroll = QtGui.QVBoxLayout()
        self.vboxScroll.setContentsMargins(0,0,11,0)
        self.scrollWidget = QtGui.QWidget()
        self.scrollWidget.setLayout(self.vboxScroll)
        self.scrollArea = QtGui.QScrollArea()
        self.scrollArea.setWidgetResizable(True)
        self.scrollArea.setFrameShape(QtGui.QFrame.NoFrame)
        self.scrollArea.setWidget(self.scrollWidget)
        self.scrollArea.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)
        self.scrollArea.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOn)
        self.vbox.addWidget(self.scrollArea)
        self.C = C
        self.runs = []
        self.indices = []
        self.names = []
    def addRun(self,rAttr):
        # where to insert?
        i = int(rAttr["Name"][1:])
        if self.indices == []:
            k_ins = 0
        elif len(self.indices) == 1:
            k_ins = int(i>self.indices[0])
        else:
            k_ins = -1
            a = np.array(self.indices)
            for j in range(len(self.indices)-1):
                if (i > self.indices[j+1]):
                    k = j
                    break
        R = RunLayout(self,self.C,rAttr)
        self.runs.insert(k_ins,R)
        self.indices.insert(k_ins,i)
        self.names.insert(k_ins,rAttr["Name"])
        self.vboxScroll.insertLayout(k_ins,R)
    def update(self,rList):
        print "update",rList
        for rAttr in rList:
            if rAttr["Name"] not in self.names:
                self.addRun(rAttr)
            else:
                i = self.names.index(rAttr["Name"])
                self.runs[i].update(rAttr)

class RunLayout(QtGui.QHBoxLayout):
    def __init__(self,parent,C,rAttr):
        QtGui.QHBoxLayout.__init__(self)
        self.parent = parent
        self.C = C
        elements = []
        self.nameLabel = QtGui.QLabel(rAttr["Name"])
        elements.append(self.nameLabel)
        self.typeCombo = QtGui.QComboBox()
        self.typeCombo.addItems(["","Data","Dark"])
        self.typeCombo.setCurrentIndex(0)
        elements.append(self.typeCombo)
        self.cmdButton = QtGui.QPushButton()
        elements.append(self.cmdButton)
        self.statusLabel = QtGui.QLabel(rAttr.get("Status","-"))
        elements.append(self.statusLabel)
        self.processRateLabel = QtGui.QLabel(rAttr.get("Process Rate","-"))
        elements.append(self.processRateLabel)
        self.nFramesLabel = QtGui.QLabel(rAttr.get("No. Frames","-"))
        elements.append(self.nFramesLabel)
        self.hitRatioLabel = QtGui.QLabel(rAttr.get("Hit Ratio","-"))
        elements.append(self.hitRatioLabel)
        sp = QtGui.QSizePolicy()
        sp.setHorizontalStretch(1)
        for e in elements:
            e.setSizePolicy(sp)
            e.setMinimumSize(150,25)
            self.addWidget(e)
        self.connectSignals()
        self.update(rAttr)
    def disconnectSignals(self):
        self.typeCombo.currentIndexChanged.disconnect(self.emitChange)
        self.cmdButton.released.disconnect(self.emitCmd)
    def connectSignals(self):
        self.typeCombo.currentIndexChanged.connect(self.emitChange)
        self.cmdButton.released.connect(self.emitCmd)       
    def update(self,rAttr):
        self.disconnectSignals()
        if rAttr == {}:
            return
        self.rAttr = rAttr
        self.nameLabel.setText(rAttr["Name"])
        self.typeCombo.setCurrentIndex(self.typeCombo.findText(rAttr["Type"]))
        s = self.rAttr["Status"]
        if s in ["Running","Finished"]:
            self.cmdButton.setText("Delete")
        elif s in ["Waiting","Postponed","Invalid"]:
            self.cmdButton.setText("Start")
        self.cmdButton.setEnabled(s not in ["Invalid","Postponed"])
        self.statusLabel.setText(rAttr.get("Status","-"))
        self.processRateLabel.setText(rAttr.get("Process Rate","-"))
        self.nFramesLabel.setText(rAttr.get("No. Frames","-"))
        self.hitRatioLabel.setText(rAttr.get("Hit Ratio","-"))
        self.connectSignals()
    def emitChange(self):
        rAttr = dict(self.rAttr)
        rAttr["Type"] = self.typeCombo.currentText()
        self.parent.changed.emit(rAttr)
    def emitCmd(self):
        rAttr["Cmd"] = self.cmdButton.text()
        self.emitChange()
