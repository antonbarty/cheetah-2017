#!/usr/bin/env python
import sys,time
from PySide import QtGui,QtCore
import argparse,configobj
import clientZMQ
import runTableWidget

class CheManClient(QtGui.QMainWindow):
    def __init__(self,args):
        QtGui.QMainWindow.__init__(self)
        self.resize(800, 500)
        #self.move(300, 300)
        self.setWindowTitle('Cheetah Manager Client')

        self.init = True

        self.C = configobj.ConfigObj(args.filename)

        self.tabs = QtGui.QTabWidget()

        self.runTable = runTableWidget.RunTableWidget(self.C)
        self.tabs.addTab(self.runTable,"Run Table")
        
        self.setCentralWidget(self.tabs)

        self.client = clientZMQ.Client(self.C)
        self.client.newRList.connect(self.runTable.update)
        self.runTable.changed.connect(self.client.sendChangeRunRequest)

        self.client.sendUpdateRequest("REQ_FULL_LIST")
        
        self.updateReqTimer = QtCore.QTimer(self)
        self.updateReqTimer.setInterval(1000)
        self.updateReqTimer.timeout.connect(self.client.sendUpdateRequest)
        self.updateReqTimer.start()

        self.updateRecvTimer = QtCore.QTimer(self)
        self.updateRecvTimer.setInterval(1000)
        self.updateRecvTimer.timeout.connect(self.client.recvUpdate)
        self.updateRecvTimer.start()
        
        print "Done with init"


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
