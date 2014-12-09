import time
import PySide
from PySide import QtGui,QtCore
import zmq,multiprocessing

class Client(QtCore.QObject):
    newRList = QtCore.Signal(list)
    def __init__(self,C):
        QtCore.QObject.__init__(self)
        self.C = C
        self.pipe_end_worker, self.pipe_end_helper =  multiprocessing.Pipe()
        self.updateEvent = multiprocessing.Event()
        self.process = multiprocessing.Process(target=ClientHelper,
                                               args=(C,self.pipe_end_helper,))
        self.process.start()
    def sendUpdateRequest(self,cmd="REQ_UPDATED_LIST"):
        self.pipe_end_worker.send(cmd)
    def sendChangeRunRequest(self,r):
        self.pipe_end_worker.send(r)
    def recvUpdate(self):
        if self.pipe_end_worker.poll():
            t0 = time.time()
            rList = self.pipe_end_worker.recv()
            if rList != []:
                self.newRList.emit(rList)
    def terminate(self):
        print "Terminating (polling last update)"
        if self.pipe_end_worker.poll():
            self.pipe_end_worker.recv()
        print "Terminating (sending terminate signal to process)"
        self.pipe_end_worker.send("REQ_TERMINATE")
        print "Terminating (joining process)"
        self.process.join()
        print "Terminating (closing pipe)"
        self.pipe_end_worker.close()
        #self.pipe_end_helper.close()
        
class ClientHelper:
    def __init__(self,C,pipe):
        # Worker connection
        self.workerPipe = pipe
        # Server connection
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PAIR)
        self.socket.connect("tcp://%s:%s" % (C["zmq"].get("host","localhost"),C["zmq"]["port"]))
        self.start()
    def start(self):
        while True: 
            # Worker communication
            #t0 = time.time()
            msg = self.workerPipe.recv()
            print msg
            #print "Client loop 1", time.time()-t0, msg
            #t0 = time.time()
            if msg == "REQ_TERMINATE":
                #print "Client helper terminates"
                break
            #print "Client loop 2", time.time()-t0
            #t0 = time.time()
            self.socket.send_json(msg)
            #print "Client loop 3", time.time()-t0
            #t0 = time.time()
            self.workerPipe.send(self.socket.recv_json())
            #print "Client loop 4", time.time()-t0

