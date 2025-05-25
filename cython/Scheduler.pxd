from libcpp.unordered_set cimport unordered_set

cdef class Scheduler:
    cdef object taskQueue

    cdef object db

    cpdef push(self, record)

    cpdef process(self, threadNum)