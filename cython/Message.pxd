from libcpp.string cimport string
from libcpp.pair cimport pair
from libcpp.memory cimport make_shared, shared_ptr
from libcpp.vector cimport vector

cdef extern from "../cpp/Message.cpp":
    pass

cdef extern from "../cpp/Message.h":
    cdef cppclass Message:
        string prefix, ASPath, origin, community
        pair[int, char] peer
        int lastAS
        char* type
        vector[string] prefixes

        Message() noexcept nogil

        Message(char*, const pair[int, char]&, const int&, const string&, const string&, const string&) noexcept nogil
        
        Message(char*, const pair[int, char]&, const int&, const string&, const string&, const string&, const string&) noexcept nogil

        Message(char*, const pair[int, char]&, const int&, const string&) noexcept nogil

        Message(char*, const int&, const string&) noexcept nogil

        Message(char*, const int&) noexcept nogil

        Message(char*) noexcept nogil

        Message(char*, const int&, const vector[string]&) noexcept nogil

