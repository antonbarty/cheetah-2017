import sys
from PySide import QtGui
import argparse,configobj
import client

class CheManClient(QtGui.QMainWindow):
    def __init__(self,args):
        QtGui.QMainWindow.__init__(self)
        self.resize(250, 150)
        self.move(300, 300)
        self.setWindowTitle('Cheetah Manager Client')

        self.C = configobj.ConfigObj(args.filename)

        self.client = client.Client(self.C)



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
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
