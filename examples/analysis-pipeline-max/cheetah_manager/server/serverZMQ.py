import zmq
import multiprocessing
import time

class Server:
    def __init__(self,C):
        self.C = C
        self.pipe_end_worker, self.pipe_end_helper =  multiprocessing.Pipe()
        self.updateEvent = multiprocessing.Event()
        self.process = multiprocessing.Process(target=ServerHelper,
                                               args=(C,self.pipe_end_helper,self.updateEvent,))
        self.process.start()
    def recvReqs(self):
        self.updateEvent.set()
        return self.pipe_end_worker.recv()
    def answerReqs(self,rList):
        self.pipe_end_worker.send(rList)
    def terminate(self):
        self.updateEvent.set()
        foo = self.pipe_end_worker.recv()
        self.pipe_end_worker.send("REQ_TERMINATE")
        self.process.join()
        self.pipe_end_worker.close()
        #self.pipe_end_helper.close()

class ServerHelper:
    def __init__(self,C,pipe,event):
        # Worker
        self.workerPipe = pipe
        self.workerUpdateEvent = event
        # Client 
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.PAIR)
        self.socket.bind("tcp://*:%s" % C["zmq"]["port"])
        self.poller = zmq.Poller()
        self.poller.register(self.socket, zmq.POLLIN)
        self.start()
    def start(self):
        msgsToWorker = []
        while True: 
            # Worker communication
            if self.isWorkerRequest():
                self.sendToWorker(msgsToWorker)
                rList = self.recvFromWorker()
                if rList == "REQ_TERMINATE":
                    break
                if msgsToWorker != []:
                    # Client communication
                    self.sendToClient(rList)
                    msgsToWorker = []
                self.workerUpdateEvent.clear()
            # Client communication
            elif self.isClientRequest():
                msgsToWorker.append(self.recvFromClient())
                answeredClientRequest = False
    def isClientRequest(self,timeout_ms=1000):
        msg = dict(self.poller.poll(timeout_ms))
        return (len(msg) > 0)
    def recvFromClient(self):
        return self.socket.recv_json()
    def sendToClient(self,msg):
        self.socket.send_json(msg)#,zmq.NOBLOCK)
    def isWorkerRequest(self):
        return self.workerUpdateEvent.is_set()
    def sendToWorker(self,msg):
        self.workerPipe.send(msg)
    def recvFromWorker(self):
        return self.workerPipe.recv()

