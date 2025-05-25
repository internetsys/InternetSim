from cython.operator cimport dereference as deref, preincrement as inc

cdef class Extension:
    @staticmethod
    def loadROVSet(filePath):
        ROVSet.clear()
        
        with open(filePath, "r") as f:
            for line in f:
                if line.startswith("#"):
                    continue
                
                line = line.strip()
                ROVSet.insert(stoi(line.encode('utf-8')))
        
        print(f"Loaded {ROVSet.size()} ROV-SET")
    
    @staticmethod
    def loadInvalidSet(filePath):
        cdef pair[int, string] p
        invalidSet.clear()
       
        with open(filePath, "r") as f:
            for line in f:
                if line.startswith("#"):
                    continue

                line = line.strip()
                attackerAS, prefix = line.split()
                p = pair[int, string](stoi(attackerAS.encode('utf-8')), prefix.encode('utf-8'))            
                invalidSet.insert(p)

        print(f"Loaded {invalidSet.size()} Invalid-SET")

    @staticmethod
    def getInvalidSet():
        cdef unordered_set[pair[int, string], PairHash].iterator it = invalidSet.begin()
        cdef list pyList = []

        while it != invalidSet.end():
            p = deref(it)
            
            # pair<int, string> convert to Python tuple (int, str)
            pyTuple = (p.first, p.second.decode('utf-8'))
            pyList.append(pyTuple)
            inc(it)

        return pyList

    @staticmethod
    cdef bool ROV(int ASN, const string& prefix, const string& ASPath) noexcept nogil:
        cdef pair[int, string] p

        if (ROVSet.find(ASN) != ROVSet.end()):
            originAS = stoi(ASPath.substr(ASPath.rfind(' ') + 1))
            p = pair[int, string](originAS, prefix)

            if (invalidSet.find(p) != invalidSet.end()):
                return False
        
        return True

    @staticmethod
    def enableROV(int ASN):
        ROVSet.insert(ASN)

        