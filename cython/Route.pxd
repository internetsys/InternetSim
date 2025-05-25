from libcpp.string cimport string
from libcpp cimport bool
from libcpp.memory cimport make_shared, shared_ptr

cdef extern from "../cpp/Route.cpp":
    pass

cdef extern from "../cpp/Route.h":
    cdef cppclass Route:
        string prefix, ASPath, origin, community
        unsigned short localPref, MED
        unsigned char pathLength, originPriority

        Route() noexcept nogil

        Route(const string&, const unsigned short&, const string&, const string&, const string& community) noexcept nogil

        bool operator==(Route&)

        bool operator!=(Route&)

        bool operator>(shared_ptr[Route])

        bool operator>(Route&)

        bool operator<(shared_ptr[Route])

        bool operator<(Route&)
