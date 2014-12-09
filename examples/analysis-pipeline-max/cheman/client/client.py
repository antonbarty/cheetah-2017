#!/usr/bin/env python
import sys,time
from PySide import QtGui,QtCore
import argparse,configobj
import clientZMQ
import runTable
import messages

class CheManClient(QtGui.QMainWindow):
    def __init__(self,args):
        QtGui.QMainWindow.__init__(self)
        self.resize(800, 500)
        self.setWindowTitle('Cheetah Manager Client')

        self.init = True

        self.C = configobj.ConfigObj(args.filename)

        self.tabs = QtGui.QTabWidget()

        self.runTable = runTable.RunTableWidget(self.C)
        self.tabs.addTab(self.runTable,"Run Table")

        self.messages = messages.Messages(self.C)
        self.tabs.addTab(self.messages,"Log messages")
        
        self.setCentralWidget(self.tabs)

        self.client = clientZMQ.Client(self.C)
        
        self.updateReqTimer = QtCore.QTimer(self)
        self.updateReqTimer.setInterval(5000)
        self.updateRecvTimer = QtCore.QTimer(self)
        self.updateRecvTimer.setInterval(1000)

        # Connect signals
        self.client.newRList.connect(self.runTable.update)
        self.client.newRList.connect(self.messages.update)
        self.runTable.changed.connect(self.client.sendChangeRunRequest)
        self.updateReqTimer.timeout.connect(self.client.sendUpdateRequest)
        self.updateRecvTimer.timeout.connect(self.client.recvUpdate)
       
        # Initialize list by sending out request for full list
        self.client.sendUpdateRequest("REQ_FULL_LIST")
        # Starting timers
        self.updateReqTimer.start()
        self.updateRecvTimer.start()

    def terminate(self):
        self.updateReqTimer.stop()
        self.updateRecvTimer.stop()
        self.client.terminate()

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
    CMC.terminate()
    time.sleep(1)
    sys.exit(ret)

if __name__ == '__main__':
    main()
