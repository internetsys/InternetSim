import re
from enum import Enum, auto
from queue import Queue

from Topology import Topology
from Scheduler import Scheduler
from AS import AS
from Interpreter import Interpreter
from Extension import Extension


class CommandType(Enum):
    ANNOUNCE = auto()
    WITHDRAW = auto()
    SIMULATE = auto()
    CHANGE_ROUTING_POLICY = auto()
    CHANGE_NON_BGP_POLICY = auto()
    SHOW_PEERS = auto()
    SHOW_ROUTES = auto()
    REFRESH_ROUTE = auto()


class Engine:
    __slots__ = ("threadNum", "batchSize", "taskQueue", "scheduler", "command", "prefixInMem", "prefixInDB")

    def __init__(self, config):

        self.threadNum = int(config["Setting"]["threadNum"])
        self.batchSize = int(config["Setting"]["batchSize"])  # -1 means do not enable offloading

        self.taskQueue = Queue()
        self.scheduler = Scheduler()

        self.prefixInMem = set()
        self.prefixInDB = set()

        self.command = [
            ("AS (\d+) announce (.+);", CommandType.ANNOUNCE),
            ("AS (\d+) withdraw (.+);", CommandType.WITHDRAW),
            ("Simulate;", CommandType.SIMULATE),
            ("Change routing policy:", CommandType.CHANGE_ROUTING_POLICY),
            ("Change non-BGP policy:", CommandType.CHANGE_NON_BGP_POLICY),
            ("AS (\d+) show peers;", CommandType.SHOW_PEERS),
            ("AS (\d+) show routes (.+);", CommandType.SHOW_ROUTES),
            ("refresh route;", CommandType.REFRESH_ROUTE),
        ]

    def parseDescriptionFile(self, descriptionFilePath, topology):
        with open(descriptionFilePath, "r") as f:

            policyFlag = None  # Mark the type of the strategy block, None means no read-in
            policy = ""

            for line in f:

                if line.startswith("#") or not line.strip():
                    continue

                line = re.sub(r" +", " ", line.strip(" "))

                flag = False  # Check whether the command is legal

                if policyFlag:
                    if line.startswith("end;"):
                        line = line.strip()
                        policy += line.strip(";")

                        interpreter = Interpreter()

                        if policyFlag == CommandType.CHANGE_ROUTING_POLICY:
                            peers = interpreter.changeRoutingPolicy(topology, policy)
                            for x, y in peers:
                                self.taskQueue.put(("Refresh", y, x))

                            self.simulate(topology, "refresh")

                        elif policyFlag == CommandType.CHANGE_NON_BGP_POLICY:
                            ASes = interpreter.changeNonBGPPolicy(topology, policy)
                            
                            self.simulate(topology, "change", {"ROV ASes" : ASes})

                        policyFlag = None
                        policy = ""

                    else:
                        policy += line

                    continue

                for command in self.command:

                    result = re.match(command[0], line)

                    if result:

                        flag = True

                        if command[1] == CommandType.ANNOUNCE:

                            ASN = int(result.group(1))
                            prefix = result.group(2)

                            if ASN not in topology.ASes:
                                raise ValueError(f"Invalid AS number: {ASN}")

                            self.taskQueue.put(("Announce", ASN, prefix))

                        elif command[1] == CommandType.WITHDRAW:

                            ASN = int(result.group(1))
                            prefix = result.group(2)

                            if ASN not in topology.ASes:
                                raise ValueError(f"Invalid AS number: {ASN}")

                            self.taskQueue.put(("Withdraw", ASN, prefix))

                        elif command[1] == CommandType.SIMULATE:
                            self.simulate(topology)

                        elif command[1] == CommandType.CHANGE_ROUTING_POLICY:

                            policyFlag = CommandType.CHANGE_ROUTING_POLICY

                        elif command[1] == CommandType.CHANGE_NON_BGP_POLICY:

                            policyFlag = CommandType.CHANGE_NON_BGP_POLICY

                        elif command[1] == CommandType.SHOW_PEERS:

                            ASN = int(result.group(1))

                            if ASN not in topology.ASes:
                                raise ValueError(f"Invalid AS number: {ASN}")

                            peers = topology.ASes[ASN].getPeers()
                            ASNlist = []

                            for peer in peers:
                                ASNlist.append(peer[0])

                            print(f"AS {ASN}'s peers: {ASNlist}")

                        elif command[1] == CommandType.SHOW_ROUTES:

                            ASN = int(result.group(1))
                            prefix = result.group(2)

                            if ASN not in topology.ASes:
                                raise ValueError(f"Invalid AS number: {ASN}")

                            print(f"AS {ASN}'s route to {prefix}:")

                            if (self.batchSize==-1):
                                topology.ASes[ASN].showRoute(prefix)
                            
                            else:

                                if prefix not in self.prefixInDB:
                                    topology.ASes[ASN].showRoute(prefix)
                                
                                else:

                                    topology.ASes[ASN].showRouteInDB(prefix)

                if not flag:
                    raise SyntaxError(f"Invalid command: {line}")

    def simulate(self, topology, type=None, args=None):

        print("Simulation start")
        
        if (type == "change"):

            invalidSet = Extension.getInvalidSet()
            tmpQueue = Queue()
            ASes = args["ROV ASes"]

            if (self.batchSize != -1):
                
                needFetch = set()
                for attackerAS, prefix in invalidSet:
                    if (prefix in self.prefixInDB):
                        needFetch.add(prefix)
                    
                    else:
                        for AS in ASes:
                            if attackerAS == topology.ASes[AS].getOriginAS(prefix):
                                self.taskQueue.put(("Withdraw", AS, str(attackerAS), prefix))

                while not self.taskQueue.empty():
                    task = self.taskQueue.get()
                    tmpQueue.put(("UpdateBest", task[1], task[3]))
                    self.scheduler.push(task)
                
                self.scheduler.process(self.threadNum)

                while not tmpQueue.empty():
                    self.scheduler.push(tmpQueue.get())

                self.scheduler.process(self.threadNum)

                while (needFetch):

                    for AS in topology.ASes:
                        self.scheduler.push(("Store", AS))

                    self.prefixInDB.update(self.prefixInMem)

                    self.scheduler.process(self.threadNum)

                    k = 0
                    prefixes = []
                    while (k < self.batchSize):
                        prefixes.append(needFetch.pop())
                        k += 1
                        if (not needFetch):
                            break
                    
                    for AS in topology.ASes:
                        self.scheduler.push(("Fetch", AS, prefixes))

                    self.scheduler.process(self.threadNum)

                    self.prefixInMem = set(prefixes)
                    self.prefixInDB -= self.prefixInMem
                    
                    for attackerAS, prefix in invalidSet:
                        if (prefix in prefixes):
                            for AS in ASes:
                                if attackerAS == topology.ASes[AS].getOriginAS(prefix):
                                    self.taskQueue.put(("Withdraw", AS, str(attackerAS), prefix))
                            
                    while not self.taskQueue.empty():
                        task = self.taskQueue.get()
                        tmpQueue.put(("UpdateBest", task[1], task[3]))
                        self.scheduler.push(task)
                    
                    self.scheduler.process(self.threadNum)

                    while not tmpQueue.empty():
                        self.scheduler.push(tmpQueue.get())

                    self.scheduler.process(self.threadNum)
                
            else:
                                
                for attackerAS, prefix in invalidSet:
                    for AS in ASes:
                        if attackerAS == topology.ASes[AS].getOriginAS(prefix):
                            self.taskQueue.put(("Withdraw", AS, str(attackerAS), prefix))
                
                while not self.taskQueue.empty():
                    task = self.taskQueue.get()
                    tmpQueue.put(("UpdateBest", task[1], task[3]))
                    self.scheduler.push(task)

                self.scheduler.process(self.threadNum)

                while not tmpQueue.empty():
                    self.scheduler.push(tmpQueue.get())

                self.scheduler.process(self.threadNum)
                
        elif (type == "refresh"):

            if (self.batchSize != -1):
                needFetch = set(self.prefixInDB)
                
                for task in self.taskQueue.queue:
                    self.scheduler.push(task)
                
                self.scheduler.process(self.threadNum)

                while (needFetch):

                    for AS in topology.ASes:
                        self.scheduler.push(("Store", AS))

                    self.prefixInDB.update(self.prefixInMem)

                    self.scheduler.process(self.threadNum)

                    k = 0
                    prefixes = []
                    while (k < self.batchSize):
                        prefixes.append(needFetch.pop())
                        k += 1
                        if (not needFetch):
                            break
                    
                    for AS in topology.ASes:
                        self.scheduler.push(("Fetch", AS, prefixes))

                    self.scheduler.process(self.threadNum)

                    self.prefixInMem = set(prefixes)
                    self.prefixInDB -= self.prefixInMem
                    
                    for task in self.taskQueue.queue:
                        self.scheduler.push(task)
                    
                    self.scheduler.process(self.threadNum)

            else:
                while not self.taskQueue.empty():
                    self.scheduler.push(self.taskQueue.get())

                self.scheduler.process(self.threadNum)
        else:

            if (self.batchSize != -1):
                needFetch = set()
                tmpQueue = Queue()
                
                while not self.taskQueue.empty():
                    
                    task = self.taskQueue.get()
                    tmpQueue.put(task)
                    prefix = task[2]
                    
                    if (prefix not in self.prefixInDB):
                        if prefix not in self.prefixInMem:
                            self.prefixInMem.add(prefix)
                    else:
                        self.prefixInDB.remove(prefix)
                        needFetch.add(prefix)

                    if (len(self.prefixInMem) + len(needFetch)==self.batchSize) or self.taskQueue.empty():

                        if (len(needFetch) > 0):
                            for AS in topology.ASes:
                                self.scheduler.push(("Fetch", AS, needFetch))

                            self.prefixInMem.update(needFetch)
                            needFetch.clear()
                            self.scheduler.process(self.threadNum)
                        
                        while not tmpQueue.empty():
                            self.scheduler.push(tmpQueue.get())

                        self.scheduler.process(self.threadNum)

                        if len(self.prefixInMem)==self.batchSize:
                            for AS in topology.ASes:
                                self.scheduler.push(("Store", AS))

                            self.prefixInDB.update(self.prefixInMem)
                            self.prefixInMem.clear()

                            self.scheduler.process(self.threadNum)
            
            else:

                while not self.taskQueue.empty():
                    
                    self.scheduler.push(self.taskQueue.get())

                self.scheduler.process(self.threadNum)

        print("Simulation complete")

    def run(self, config, topology):
        self.parseDescriptionFile(config["Path"]["descriptionFilePath"], topology)
