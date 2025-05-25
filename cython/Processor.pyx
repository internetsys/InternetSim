from libc.stdio cimport printf

cdef class Processor:
    def __init__(self):
        pass

    cdef void addMessage(self, shared_ptr[Message] message) noexcept nogil:
        self.messageQueue.push(message)

    cdef int process(self):
        cdef:
            shared_ptr[Message] message
            char* storeType = 'S'
            char* fetchType = 'F'

        with nogil:
            while not self.messageQueue.empty():
                message = self.messageQueue.front()
                self.messageQueue.pop()

                if strcmp(message.get().type, storeType) == 0:
                    
                    AS.receive(message.get().lastAS, message, self)
                
                elif strcmp(message.get().type, fetchType) == 0:

                    AS.receive(message.get().lastAS, message, self)
                
                else:

                    AS.receive(message.get().peer.first, message, self)
                                            
        return 1
            
