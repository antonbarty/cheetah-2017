import os,time
import PySide
from PySide import QtGui,QtCore
import numpy as np


class Messages(QtGui.QPlainTextEdit):
    def __init__(self,C):
        QtGui.QPlainTextEdit.__init__(self)
        self.C = C
    def update(self,rList):
        for rAttr in rList:
            if "Messages" in rAttr:
                self.insertPlainText(rAttr["Messages"])
