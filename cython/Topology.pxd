from AS cimport AS
from libcpp.unordered_map cimport unordered_map
from libcpp.pair cimport pair

cdef class Topology:
    cdef public dict ASes
    
    cdef public int ASNum, linkNum, policyNum

    cpdef void addAS(self, int, int, char)