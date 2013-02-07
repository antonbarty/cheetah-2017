class Filter:
    def __init__(self,name,lo,hi):
        self.expr = "%g<%s<%g" %(lo,name,hi)

    def and(self,b):
        Filter c
        c.expr = "(%s)&(%s)"%(self.expr,b.expr)
        return c

    def or(self,b):
        Filter c
        c.expr = "(%s)|(%s)"%(self.expr,b.expr)
        
