from libcpp cimport bool
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.memory cimport make_shared, shared_ptr
from libcpp.unordered_map cimport unordered_map
from libcpp.set cimport set
from libcpp.string cimport string, to_string, stoi
from libc.stdlib cimport malloc, free
from libc.string cimport  strdup, strchr, strcmp, strrchr
from libc.stdio cimport printf
from libc.stdint cimport uintptr_t
from libcpp.forward_list cimport forward_list

from Route cimport Route
from RouteTable cimport RouteTable
from Processor cimport Processor
from Message cimport Message
from Extension cimport Extension
cimport Scheduler

cdef extern from "<mutex>" namespace "std" nogil:
    cdef cppclass mutex:
        mutex() noexcept nogil

        void lock()
        void unlock()

cdef extern from "<regex>" namespace "std" nogil:
    cdef cppclass regex:
        regex(const string&) except +

    cdef cppclass smatch:
        const string str(uintptr_t n) const

    bint regex_search(const string&, smatch&, const regex&) except +

    bint regex_match(const string&, const regex&) except +

cdef extern from "../cpp/Filter.cpp":
    pass

cdef extern from "../cpp/Filter.h":
    enum Match:
        ANY,
        COMMUNITY_CONTAINS,
        PREFIX_IS,
        PREFIX_IN,
        PATH,
        AS_SET
    
    enum Action:
        ACCEPT,
        DENY,
        LOCAL_PREF,
        COMMUNITY_ADD,
        COMMUNITY_STRIP,
        COMMUNITY_REMOVE,
        PREPEND,
        MED

    cdef cppclass Filter:
        Match matchType
        string matchValue
        forward_list[pair[Action, string]] actions
        Filter* nextFt

        Filter() noexcept

        Filter(const string&, string&) noexcept

cdef extern from "../cpp/Hash.cpp":
    pass

cdef extern from "../cpp/Hash.h":
    cdef cppclass PairHash:
        size_t operator()(const pair[int, int] &) const

cdef unordered_map[int, RouteTable*] routeTables
cdef unordered_map[int, vector[pair[int, char]]] peers # -1 for customer, 0 for peer, 1 for provider
cdef unordered_map[pair[int, int], Filter, PairHash] filterIn
cdef unordered_map[pair[int, int], Filter, PairHash] filterOut
cdef unordered_map[int, mutex*] locks
cdef unordered_map[string, set[int]] ASSet


cdef class AS:
    cdef int ASN

    cdef Extension extension

    cpdef void addPeer(self, peer)

    cpdef void addFilter(self, peerASN, filterType, match, action)

    @staticmethod
    cdef void announcePrefix(int, const string&, Processor)
    
    @staticmethod
    cdef void withdrawPrefix(int, const string&, const string&, Processor)

    @staticmethod
    cdef void announceTo(int, int, const string&, Processor)

    @staticmethod
    cdef char* strtokX(char*, const char*, char**) noexcept nogil

    @staticmethod
    cdef bool inASPath(const string&, const string&) noexcept nogil

    @staticmethod
    cdef bool inCommunity(const string& community, const string& communitySet) noexcept nogil

    @staticmethod
    cdef string removeCommunity(const string&, const string&) noexcept nogil

    @staticmethod
    cdef shared_ptr[Route] inAction(forward_list[pair[Action, string]]&, shared_ptr[Message], int) noexcept nogil

    @staticmethod
    cdef shared_ptr[Message] outAction(forward_list[pair[Action, string]]&, shared_ptr[Route], pair[int, char], int) noexcept nogil

    @staticmethod
    cdef shared_ptr[Route] filterIn(int, shared_ptr[Message]) noexcept nogil

    @staticmethod
    cdef shared_ptr[Message] filterOut(int, pair[int, char], pair[int, char], shared_ptr[Route]) noexcept nogil

    @staticmethod
    cdef void announce(int, pair[int, char], const string&, shared_ptr[Route], Processor) noexcept nogil

    @staticmethod
    cdef void withdraw(int, int, const string&, Processor) noexcept nogil

    @staticmethod
    cdef void receive(int, shared_ptr[Message], Processor) noexcept nogil

    @staticmethod
    cdef void updateBestRoute(int, pair[int, char], shared_ptr[Route], Processor) noexcept nogil

    @staticmethod
    cdef void updateBest(int, int, const string&, Processor) noexcept nogil

    @staticmethod
    cdef void store(int, Processor) noexcept nogil

    @staticmethod
    cdef void fetch(int, const vector[string]&, Processor) noexcept nogil

    cpdef void showRoute(self, prefix)

    cpdef void showRouteInDB(self, prefix)

    cpdef void showBestRoute(self, prefix)

    cpdef void deleteRouteByPeerAS(self, peerAS)

