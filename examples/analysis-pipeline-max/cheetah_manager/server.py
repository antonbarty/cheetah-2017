import zmq

class Server:
    def __init__(self,C):
        self._context = zmq.Context()
        self._socket = self._context.socket(zmq.PAIR)
        self._socket.bind("tcp://*:%s" % C["zmq"]["port"])
    def recv(self,timeout_ms=10000):
        poller = zmq.Poller()
        poller.register(self._socket, zmq.POLLIN)
        msg = dict(poller.poll(timeout_ms))
        if len(msg) > 0:
            return self._socket.recv()
        else:
            return None
    def send(self,msg):
        self._socket.send(msg)
