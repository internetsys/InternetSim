from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.string cimport string, stoi
from libcpp.unordered_set cimport unordered_set

cdef extern from "../cpp/Hash.cpp":
    pass

cdef extern from "../cpp/Hash.h":
    cdef cppclass PairHash:
        size_t operator()(const pair[int, string]&) const

cdef unordered_set[int] ROVSet
cdef unordered_set[pair[int, string], PairHash] invalidSet

cdef class Extension:
    @staticmethod
    cdef bool ROV(int, const string&, const string&) noexcept nogil

    # Add self-defined mechanism here
