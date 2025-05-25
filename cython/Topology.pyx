cdef class Topology:
    def __cinit__(self):
        self.ASes = {}
        self.ASNum = 0
        self.linkNum = 0
        self.policyNum = 0

    cpdef void addAS(self, int ASNx, int ASNy, char relationship):
        if ASNx not in self.ASes:
            self.ASes[ASNx] = AS(ASNx)
            self.ASNum += 1
        if ASNy not in self.ASes:
            self.ASes[ASNy] = AS(ASNy)
            self.ASNum += 1
            
        self.ASes[ASNx].addPeer((ASNy, relationship))
        self.ASes[ASNy].addPeer((ASNx, -relationship))
        self.linkNum += 1

    def countRoute(self):
        n = 0
        for ASN in self.ASes:
            n += self.ASes[ASN].countRoute()

        print("Total number of routes: ", n)

    def __str__(self):
        return f"Topology(ASes={self.ASes}, ASNum={self.ASNum}, linkNum={self.linkNum})"

