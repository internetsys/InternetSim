from libcpp.string cimport string
from libcpp.unordered_map cimport unordered_map
from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.memory cimport shared_ptr
from libcpp.vector cimport vector

from Route cimport Route

cdef extern from "../cpp/RouteTable.cpp":
    pass

cdef extern from "../cpp/RouteTable.h":
    cdef cppclass RouteTable:

        RouteTable(int ASN) noexcept

        shared_ptr[Route] insertRoute(shared_ptr[Route] route) noexcept nogil

        bool withdrawRoute(const string&, const string&) noexcept nogil

        shared_ptr[Route] updateBestRoute(shared_ptr[Route]) noexcept nogil

        shared_ptr[Route] getBestRoute(const string&) noexcept nogil
        
        void deleteRoute(shared_ptr[Route]) noexcept nogil

        void deleteRouteByPeerAS(const string &) noexcept nogil

        void deleteRouteByPeerAS(const string &, const string &) noexcept nogil

        vector[shared_ptr[Route]] getRoutes(const string &) noexcept nogil

        @staticmethod
        const int matchLen(const string &, const string &) noexcept nogil

        void showRoute(const string&)

        void showRouteInDB(const string&)

        void showBestRoute(const string&)

        void store2DB() noexcept nogil

        void fetchFromDB(const vector[string]&) noexcept nogil

        int getOriginAS(const string&)

        unsigned long long countRoute()

