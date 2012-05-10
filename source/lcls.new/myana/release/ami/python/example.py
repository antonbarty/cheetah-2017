import pyami

class AmiScalar(pyami.Entry):
    def __init__(self,name):
        pyami.Entry.__init__(self,name)
    
class AmiAcqiris(pyami.Entry):
    def __init__(self,detid,channel):
        pyami.Entry.__init__(self,detid,channel)
    
eth_lo = 0x7f000001
eth_mc = 0xefff2604
CxiAcq = 0x18000200

pyami.connect(eth_lo,eth_lo,eth_mc)

x = AmiScalar("ProcTime")
x.get()

x = AmiAcqiris(CxiAcq,1)
x.get()
