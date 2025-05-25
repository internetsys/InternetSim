from libcpp.queue cimport queue
from libcpp.memory cimport make_shared, shared_ptr
from libc.string cimport strcmp

from Message cimport Message
from AS cimport AS

cdef class Processor:
    cdef queue[shared_ptr[Message]] messageQueue
    
    cdef void addMessage(self, shared_ptr[Message]) noexcept nogil
    
    cdef int process(self)
