from queue import Queue
from concurrent.futures import ThreadPoolExecutor, as_completed
from Processor cimport Processor
from AS cimport AS
from pymongo import MongoClient

cdef class Scheduler:
    def __cinit__(self):
        self.taskQueue = Queue()
        self.db = MongoClient("mongodb://localhost:27017/")["BGPData"]

    def getASPath(self, ASN, prefix):
        result = self.db[str(ASN % 2000)].find_one({"AS": ASN, "prefix": prefix}, {"AS Path": 1, "_id": 0})
        if (result):
            return result.get("AS Path")
        return None

    cpdef push(self, record):
        self.taskQueue.put(record)

    cpdef process(self, threadNum):
        futures = []
        
        with ThreadPoolExecutor(max_workers = threadNum) as executor:
            while not self.taskQueue.empty():
            
                record = self.taskQueue.get()

                processor = Processor()
                
                if record[0] == "Announce":
                    
                    AS.announcePrefix(record[1], record[2].encode('utf-8'), processor)
                    
                elif record[0] == "Withdraw":

                    if len(record) == 3:
                        AS.withdrawPrefix(record[1], ("").encode('utf-8'), record[2].encode('utf-8'), processor)
                    
                    elif len(record) == 4:
                        AS.withdrawPrefix(record[1], record[2].encode('utf-8'), record[3].encode('utf-8'), processor)

                elif record[0] == "UpdateBest":
                    
                    AS.updateBest(record[1], record[1], record[2].encode('utf-8'), processor)
                
                elif record[0] == "Refresh":

                    if len(record) == 3:
                        AS.announceTo(record[1], record[2], ("").encode('utf-8'), processor)
                    
                    else:
                        AS.announceTo(record[1], record[2], record[3].encode('utf-8'), processor)
                
                elif record[0] == "Store":

                    AS.store(record[1], processor)
                
                elif record[0] == "Fetch":
                    
                    prefixes=[prefix.encode('utf-8') for prefix in record[2]]
                    AS.fetch(record[1], prefixes, processor)
                
                future = executor.submit(processor.process)
                futures.append(future)
            
            for future in as_completed(futures):
                future.result()

            futures.clear()
