#!/usr/bin/env python
import sys,time
from PySide import QtGui,QtCore
import argparse,configobj
import clientZMQ
import runTableWidget

class CheManClient(QtGui.QMainWindow):
    newRequest = QtCore.Signal(dict)
    def __init__(self,args):
        QtGui.QMainWindow.__init__(self)
        self.resize(800, 500)
        #self.move(300, 300)
        self.setWindowTitle('Cheetah Manager Client')

        self.init = True

        self.C = configobj.ConfigObj(args.filename)

        self.runTable = runTableWidget.RunTableWidget(self,self.C)
        
        self.setCentralWidget(self.runTable)

        self.client = clientZMQ.Client(self.C)
        self.client.newRList.connect(self.runTable.update)
        self.runTable.changed.connect(self.sendRequest)
        self.newRequest.connect(self.client.sendRequest)

        self.timer = QtCore.QTimer(self)
        self.timer.setInterval(5000)
        self.timer.timeout.connect(self.sendRequest)
        self.timer.start()

        self.sendRequest({})

    def sendRequest(self,req=[]):
        self.newRequest.emit(req)

def main():
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-d', '--debug', dest='debuggingMode', action='store_true', help='debugging mode')
    parser.add_argument('filename', nargs="?", type=str, help='Configuration file to load', default="")
    args = parser.parse_args()
    
    if args.debuggingMode:
        # Set exception handler
        print "Running cheMan_client in debugging mode."
        sys.excepthook = exceptionHandler

    app = QtGui.QApplication(sys.argv)
    CMC = CheManClient(args)
    CMC.show()
    ret = app.exec_()
    CMC.client.terminate()
    time.sleep(1)
    sys.exit(ret)

if __name__ == '__main__':
    main()
