import json

cdef class AS:
    def __cinit__(self, ASN):
        self.ASN = ASN
        peers[ASN] = ()
        routeTables[ASN] = new RouteTable(ASN)
        locks[ASN] = new mutex()

    @staticmethod
    def loadASSet(filePath):
        cdef set[int] ASMembers
        
        with open(filePath, "r") as f:
            data = json.load(f)
            for i in data:
                ASMembers.clear()
                for j in data[i]:
                    ASMembers.insert(stoi(j.encode('utf-8')))
                
                ASSet[i.encode('utf-8')] = ASMembers
        
        print(f"Loaded {ASSet.size()} AS-SETs")

    cpdef void addPeer(self, peer):
        cdef pair[int, char] p
        
        p.first = peer[0]
        p.second = peer[1]
        
        peers[self.ASN].push_back(p)

    def getPeers(self):
        return peers[self.ASN]

    def getFilter(self, peerASN, filterType):
        cdef:
            pair[int, int] p
            Filter* ptr

        p.first = self.ASN
        p.second = peerASN
        filter = []

        if filterType=='filter in':

            if (filterIn.find(p) == filterIn.end()):
                return []

            ptr = filterIn[p].nextFt

            while (ptr != NULL):

                match = ""
                action = ""

                if (ptr.matchType == ANY):
                    
                    match = "any"
                
                elif (ptr.matchType == COMMUNITY_CONTAINS):
                    
                    match = "community contains " + ptr.matchValue.decode('utf-8')
                
                elif (ptr.matchType == PREFIX_IS):
                    
                    match = "prefix is " + ptr.matchValue.decode('utf-8')
                
                elif (ptr.matchType == PREFIX_IN):
                    
                    match = "prefix in " + ptr.matchValue.decode('utf-8')
                
                elif (ptr.matchType == PATH):
                    
                    match = "path " + ptr.matchValue.decode('utf-8')

                for subAction in ptr.actions:
                    if (subAction.first == DENY):
                        
                        action = "deny;" + action
                    
                    elif (subAction.first == LOCAL_PREF):
                        
                        action = "local-pref " + subAction.second.decode('utf-8') + ";" + action
                    
                    elif (subAction.first == COMMUNITY_ADD):
                        
                        action = "community add " + subAction.second.decode('utf-8') + ";" + action
                    
                    elif (subAction.first == COMMUNITY_STRIP):
                        
                        action = "community strip;" + action
                    
                    elif (subAction.first == COMMUNITY_REMOVE):
                        
                        action = "community remove " + subAction.second.decode('utf-8') + ";" + action
                    
                    elif (subAction.first == PREPEND):
                        
                        action = "prepend " + subAction.second.decode('utf-8') + ";" + action
                    
                    
                filter.append((match, action[:-1]))

                ptr = ptr.nextFt

        else:
            if (filterOut.find(p) == filterOut.end()):
                return []

            ptr = filterOut[p].nextFt
            
            while (ptr != NULL):

                match = ""
                action = ""

                if (ptr.matchType == ANY):
                    
                    match = "any"
                
                elif (ptr.matchType == COMMUNITY_CONTAINS):
                    
                    match = "community contains " + ptr.matchValue.decode('utf-8')
                
                elif (ptr.matchType == PREFIX_IS):
                    
                    match = "prefix is " + ptr.matchValue.decode('utf-8')
                
                elif (ptr.matchType == PREFIX_IN):
                    
                    match = "prefix in " + ptr.matchValue.decode('utf-8')
                
                elif (ptr.matchType == PATH):
                    
                    match = "path " + ptr.matchValue.decode('utf-8')

                for subAction in ptr.actions:
                    if (subAction.first == DENY):
                        
                        action = "deny;" + action
                    
                    elif (subAction.first == LOCAL_PREF):
                        
                        action = "local-pref " + subAction.second.decode('utf-8') + ";" + action
                    
                    elif (subAction.first == COMMUNITY_ADD):
                        
                        action = "community add " + subAction.second.decode('utf-8') + ";" + action
                    
                    elif (subAction.first == COMMUNITY_STRIP):
                        
                        action = "community strip;" + action
                    
                    elif (subAction.first == COMMUNITY_REMOVE):
                        
                        action = "community remove " + subAction.second.decode('utf-8') + ";" + action
                    
                    elif (subAction.first == PREPEND):
                        
                        action = "prepend " + subAction.second.decode('utf-8') + ";" + action
                    
                    
                filter.append((match, action[:-1]))

                ptr = ptr.nextFt

        return filter

    def removeFilter(self, peerASN, filterType, index):
        cdef:
            pair[int, int] p
            Filter* ptr
            Filter* lastPtr

        index = int(index)
        p.first = self.ASN
        p.second = peerASN

        if filterType=='filter in':

            if (filterIn.find(p) == filterIn.end()):
                return False

            lastPtr = &filterIn[p]
            ptr = filterIn[p].nextFt

            while (ptr != NULL):

                if (index == 0):
                    lastPtr.nextFt = ptr.nextFt
                    del ptr
                    return True
                
                index -= 1
                lastPtr = ptr
                ptr = ptr.nextFt

        else:
            if (filterOut.find(p) == filterOut.end()):
                return False

            lastPtr = &filterOut[p]
            ptr = filterOut[p].nextFt

            while (ptr != NULL):

                if (index == 0):
                    lastPtr.nextFt = ptr.nextFt
                    del ptr
                    return True
                
                index -= 1
                lastPtr = ptr
                ptr = ptr.nextFt

        return False

    cpdef void addFilter(self, peerASN, filterType, match, action):
        cdef:
            pair[int, int] p
            Filter* ft
            Filter* ptr

        p.first = self.ASN
        p.second = peerASN
        match = match.encode('utf-8')
        action = action.encode('utf-8')

        if (filterType=='filter in'):
            
            if (filterIn.find(p) == filterIn.end()):
                
                filterIn[p] = Filter()
                filterIn[p].nextFt = new Filter(match, action)
            
            else:
                
                ft = new Filter(match, action)
                ptr = filterIn[p].nextFt
                
                if (ptr == NULL):
                    filterIn[p].nextFt = ft
                else:
                    while (ptr.nextFt != NULL):
                        ptr = ptr.nextFt
                    ptr.nextFt = ft
       
        else:
            
            if (filterOut.find(p) == filterOut.end()):
                
                filterOut[p] = Filter()
                filterOut[p].nextFt = new Filter(match, action)
            
            else:
                
                ft = new Filter(match, action)
                ptr = filterOut[p].nextFt
                
                if (ptr == NULL):
                    filterOut[p].nextFt = ft
                else:
                    while (ptr.nextFt != NULL):
                        ptr = ptr.nextFt
                    ptr.nextFt = ft            

    def __str__(self):
        return f"AS(ASN={self.ASN}, peers={peers[self.ASN]})"

    @staticmethod
    cdef void announcePrefix(int ASN, const string& prefix, Processor processor):
        cdef:
            shared_ptr[Message] message
            shared_ptr[Route] route
        
        route = make_shared[Route](Route(prefix, 100, "", "IGP", ""))
        locks[ASN].lock()
        routeTables[ASN].insertRoute(route)
        locks[ASN].unlock()

        for peer in peers[ASN]:
            
            message = make_shared[Message](Message("A", peer, ASN, prefix, to_string(ASN), "IGP"))
            processor.addMessage(message)

    @staticmethod
    cdef void withdrawPrefix(int ASN, const string& originAS, const string& prefix, Processor processor):
        cdef:
            shared_ptr[Message] message
            vector[shared_ptr[Route]] routes
            shared_ptr[Route] route
            Route* rt
        
        if (originAS != ""): # The origin AS is not itself
            locks[ASN].lock()
            routes = routeTables[ASN].getRoutes(prefix)

            for route in routes:
                rt = route.get()
                if (rt.ASPath.substr(rt.ASPath.rfind(' ') + 1) == originAS):
                    routeTables[ASN].deleteRoute(route)
            
            locks[ASN].unlock()
        
        else:
            locks[ASN].lock()
            
            route = make_shared[Route](Route(prefix, 100, "", "IGP", ""))
            routeTables[ASN].deleteRoute(route)
            
            locks[ASN].unlock()

        for peer in peers[ASN]:
            message = make_shared[Message](Message("W", peer, ASN, prefix))
            processor.addMessage(message)

    @staticmethod
    cdef void announceTo(int ASN, int peerASN, const string& prefix, Processor processor):
        cdef:
            shared_ptr[Message] message
            vector[shared_ptr[Route]] routes
            Route* rt
            pair[int, char] peer
            pair[int, char] lastPeer
            int lastAS

        for p in peers[ASN]:
            if (p.first == peerASN):
                peer = p
                break

        locks[ASN].lock()
        routes = routeTables[ASN].getRoutes(prefix)
        locks[ASN].unlock()
        
        for route in routes:
            
            rt = route.get()
            
            if (rt.ASPath == ""): # The origin AS is itself
                
                message = make_shared[Message](Message("A", peer, ASN, rt.prefix, to_string(ASN), "IGP"))
            
            else:

                lastAS = stoi(rt.ASPath.substr(0, rt.ASPath.find(' ')))
                if (lastAS == peerASN):
                    continue
                
                for p in peers[lastAS]:
                    if (p.first == ASN):
                        lastPeer = p
                        break
                
                route.get().prefix = rt.prefix
                
                message = AS.filterOut(ASN, peer, lastPeer, route)
            
            if (message == NULL):
                message = make_shared[Message](Message("X", peer, ASN, rt.prefix))

            processor.addMessage(message)

    @staticmethod
    cdef void store(int ASN, Processor processor) noexcept nogil:
        cdef:
            shared_ptr[Message] message

        message = make_shared[Message](Message("S", ASN))
        processor.addMessage(message)

    @staticmethod
    cdef void fetch(int ASN, const vector[string]& prefixes, Processor processor) noexcept nogil:
        cdef:
            shared_ptr[Message] message

        message = make_shared[Message](Message("F", ASN, prefixes))
        processor.addMessage(message)

    @staticmethod
    cdef char* strtokX(char* str, const char* delim, char** savePtr) noexcept nogil:
        cdef char* end
    
        # If the str is empty, processing the last string
        if str == NULL:
            str = savePtr[0]
        if not str:
            return NULL
    
        # Skip characters in delim
        while str[0] and strchr(delim, str[0]):
            str += 1
    
        # If the end of the string is reached, NULL is returned.
        if not str[0]:
            return NULL
    
        # Find the delimiter or end of string
        end = str
        while end[0] and not strchr(delim, end[0]):
            end += 1
    
        # If the end of the string is reached, set savePtr to NULL
        if not end[0]:
            savePtr[0] = NULL
        else:
            end[0] = '\0'
            savePtr[0] = end + 1
    
        return str

    @staticmethod
    cdef bool inASPath(const string& ASN, const string& ASPath) noexcept nogil:
        cdef:
            char* token
            const char* cASN = ASN.c_str()
            const char* cASPath = ASPath.c_str()
            char* savePtr = NULL
            char* tmp = strdup(cASPath)

        # Splitting the string
        token = AS.strtokX(tmp, " ", &savePtr)
        
        while token != NULL:
            if strcmp(token, cASN) == 0:
                free(tmp)
                return True
            
            token = AS.strtokX(NULL, " ", &savePtr)

        free(tmp)
        
        return False

    @staticmethod
    cdef bool inCommunity(const string& community, const string& communitySet) noexcept nogil:
        cdef:
            char* token
            const char* cCommunity = community.c_str()
            const char* cCommunitySet = communitySet.c_str()
            char* savePtr = NULL
            char* tmp = strdup(cCommunitySet)

        # Splitting the string
        token = AS.strtokX(tmp, " ", &savePtr)
        
        while token != NULL:
            if strcmp(token, cCommunity) == 0:
                free(tmp)
                return True
            
            token = AS.strtokX(NULL, " ", &savePtr)

        free(tmp)
        
        return False

    @staticmethod
    cdef string removeCommunity(const string& community, const string& target) noexcept nogil:
        cdef:
            char* token
            string result = string("")
            const char* cCommunity = community.c_str()
            const char* cTarget = target.c_str()
            char* savePtr = NULL
            char* tmp = strdup(cCommunity)

        # Splitting the string
        token = AS.strtokX(tmp, " ", &savePtr)
        
        while token != NULL:
            if strcmp(token, cTarget) != 0:
                if (result == ""):
                    result = string(token)
                else:
                    result += string(" ") + string(token)
            
            token = AS.strtokX(NULL, " ", &savePtr)

        free(tmp)
        
        return result

    @staticmethod
    cdef shared_ptr[Route] inAction(forward_list[pair[Action, string]]& actions, shared_ptr[Message] msg, int ASN) noexcept nogil:
        cdef:
            shared_ptr[Route] route 
            Message* message = msg.get()
            smatch result
            unsigned short localPref = 100
            string ASPath = message.ASPath
            string community = message.community
        
        for action in actions:

            if (action.first == ACCEPT):
                break

            if (action.first == DENY):
                
                return shared_ptr[Route]()

            elif (action.first == LOCAL_PREF):
                
                localPref = stoi(action.second)

            elif (action.first == COMMUNITY_ADD):
                
                if (community != ""):
                    community += string(" ")
                community += action.second

            elif (action.first == COMMUNITY_STRIP):
                
                community = string("")
            
            elif (action.first == COMMUNITY_REMOVE):
                
                community = AS.removeCommunity(community, action.second)

            elif (action.first == PREPEND):
                
                for i in range(stoi(action.second)):
                    ASPath = to_string(ASN) + string(" ") + ASPath

            else:
                printf("error: filterIn action not found!\n")
                
        route = make_shared[Route](Route(message.prefix, localPref, ASPath, "IGP", community))
        
        return route

    @staticmethod
    cdef shared_ptr[Message] outAction(forward_list[pair[Action, string]]& actions, shared_ptr[Route] rt, pair[int, char] peer, int ASN) noexcept nogil:
        cdef:
            shared_ptr[Message] message
            Route* route = rt.get()
            smatch result
            string ASPath = to_string(ASN) + string(" ") + route.ASPath
            string community = route.community
        
        for action in actions:

            if (action.first == ACCEPT):
                break

            elif (action.first == DENY):
                
                return shared_ptr[Message]()

            elif (action.first == LOCAL_PREF):
                
                localPref = stoi(action.second)

            elif (action.first == COMMUNITY_ADD):
                
                if (community != ""):
                    community += string(" ")
                community += action.second

            elif (action.first == COMMUNITY_STRIP):
                
                community = string("")
            
            elif (action.first == COMMUNITY_REMOVE):
                
                community = AS.removeCommunity(community, action.second)

            elif (action.first == PREPEND):
                
                for i in range(1, stoi(action.second)):
                    ASPath = to_string(ASN) + string(" ") + ASPath

            else:
                printf("error: filterOut action not found!\n")
        
        message = make_shared[Message](Message("A", peer, ASN, route.prefix, ASPath, "IGP", community))
        
        return message        

    @staticmethod
    cdef shared_ptr[Route] filterIn(int ASN, shared_ptr[Message] msg) noexcept nogil:
        cdef:
            shared_ptr[Route] route
            pair[int, int] p
            Filter* ptr
            Message* message = msg.get()
            smatch result
            int l = 0

        if (Extension.ROV(ASN, message.prefix, message.ASPath) == False):
            return shared_ptr[Route]()

        p.first = ASN
        p.second = message.lastAS

        if filterIn.find(p) != filterIn.end():

            ptr = filterIn[p].nextFt

            while (ptr != NULL):
                if (ptr.matchType == ANY):
                    
                    return AS.inAction(ptr.actions, msg, ASN)

                elif (ptr.matchType == AS_SET):

                    originAS = stoi(message.ASPath.substr(message.ASPath.rfind(' ') + 1))
                    if (ASSet[ptr.matchValue].find(originAS) != ASSet[ptr.matchValue].end()):
                        return AS.inAction(ptr.actions, msg, ASN)

                elif (ptr.matchType == PATH):
                    
                    if (regex_match(message.ASPath, regex(ptr.matchValue))):
                        return AS.inAction(ptr.actions, msg, ASN)

                elif (ptr.matchType == COMMUNITY_CONTAINS):
                    
                    if (AS.inCommunity(ptr.matchValue, message.community)):
                        return AS.inAction(ptr.actions, msg, ASN)
                     
                elif (ptr.matchType == PREFIX_IS):
                    
                    if (message.prefix == ptr.matchValue):
                        return AS.inAction(ptr.actions, msg, ASN)

                elif (ptr.matchType == PREFIX_IN):
                    
                    l = RouteTable.matchLen(ptr.matchValue, message.prefix)
                    if (l > 0):
                        return AS.inAction(ptr.actions, msg, ASN)
                    
                else:
                    printf("error: filterIn match not found!\n")

                ptr = ptr.nextFt

        # Use typical policy by default, localPref: customer > peer > provider
        if message.peer.second == -1:
            route = make_shared[Route](Route(message.prefix, 60, message.ASPath, "IGP", message.community))
        
        elif message.peer.second == 0:
            route = make_shared[Route](Route(message.prefix, 80, message.ASPath, "IGP", message.community))
        
        else:
            route = make_shared[Route](Route(message.prefix, 100, message.ASPath, "IGP", message.community)) 
        
        return route

    @staticmethod
    cdef shared_ptr[Message] filterOut(int ASN, pair[int, char] peer, pair[int, char] lastPeer, shared_ptr[Route] rt) noexcept nogil:
        cdef:
            shared_ptr[Message] message
            pair[int, int] p
            Filter* ptr
            Route* route = rt.get()
            smatch result
            string x
            int l = 0

        p.first = ASN
        p.second = peer.first

        if filterOut.find(p) != filterOut.end():

            ptr = filterOut[p].nextFt

            while (ptr != NULL):
                if (ptr.matchType == ANY):
                    
                    return AS.outAction(ptr.actions, rt, peer, ASN)

                elif (ptr.matchType == AS_SET):

                    originAS = stoi(route.ASPath.substr(route.ASPath.rfind(' ') + 1))
                    if (ASSet[ptr.matchValue].find(originAS) != ASSet[ptr.matchValue].end()):
                        return AS.outAction(ptr.actions, rt, peer, ASN)
                
                elif (ptr.matchType == PATH):
                    if (regex_match(route.ASPath, regex(ptr.matchValue))):
                        return AS.outAction(ptr.actions, rt, peer, ASN)

                elif (ptr.matchType == COMMUNITY_CONTAINS):
                    if (AS.inCommunity(ptr.matchValue, route.community)):
                        return AS.outAction(ptr.actions, rt, peer, ASN)
                     
                elif (ptr.matchType == PREFIX_IS):
                    if (route.prefix == ptr.matchValue):
                        return AS.outAction(ptr.actions, rt, peer, ASN)

                elif (ptr.matchType == PREFIX_IN):
                    l = RouteTable.matchLen(ptr.matchValue, route.prefix)
                    if (l > 0):
                        return AS.outAction(ptr.actions, rt, peer, ASN)
                    
                else:
                    printf("error: filterOut match not found!\n")

                ptr = ptr.nextFt

        # Use valley-free by default, from provider and peer not announce to provider and peer
        if (lastPeer.second < 1) and (peer.second > -1):
            message = make_shared[Message](Message("V"))
            
        else:
            message = make_shared[Message](Message("A", peer, ASN, route.prefix,  to_string(ASN) + string(' ') + route.ASPath, "IGP", route.community))
        
        return message

    @staticmethod
    cdef void announce(int ASN, pair[int, char] lastPeer, const string& prefix, shared_ptr[Route] route, Processor processor) noexcept nogil:
        cdef:
            shared_ptr[Message] message
        
        for peer in peers[ASN]:

            if (route == NULL):
                message = make_shared[Message](Message("X", peer, ASN, prefix))
            
            else:

                message = AS.filterOut(ASN, peer, lastPeer, route)

                if (message == NULL):
                    message = make_shared[Message](Message("X", peer, ASN, prefix))

                elif (strcmp(message.get().type, "V") == 0):
                    continue

            processor.addMessage(message)

    @staticmethod
    cdef void withdraw(int ASN, int lastAS, const string& prefix, Processor processor) noexcept nogil:
        cdef:
            shared_ptr[Message] message
        
        for peer in peers[ASN]:
            if (peer.first == lastAS):
                continue

            message = make_shared[Message](Message("W", peer, ASN, prefix))
            processor.addMessage(message)

    @staticmethod
    cdef void updateBestRoute(int ASN, pair[int, char] lastPeer, shared_ptr[Route] route, Processor processor) noexcept nogil:
        cdef:
            shared_ptr[Message] message
        
        for peer in peers[ASN]:
            if (peer.first == lastPeer.first):
                continue

            message = AS.filterOut(ASN, peer, lastPeer, route)
            
            if (message != NULL and strcmp(message.get().type, "V") != 0):
                message.get().type = "U"
                processor.addMessage(message)

    @staticmethod
    cdef void updateBest(int ASN, int lastAS, const string& prefix, Processor processor) noexcept nogil:
        cdef:
            shared_ptr[Message] message
            shared_ptr[Route] route
            Route* rt
            pair[int, char] lastPeer

        route = make_shared[Route]()
        route.get().prefix = prefix
        route.get().ASPath = string("")
        locks[ASN].lock()
        route = routeTables[ASN].updateBestRoute(route)
        locks[ASN].unlock()

        if (route == NULL):
            for p in peers[ASN]:
                if (p.first == lastAS):
                    continue

                message = make_shared[Message](Message("T", p, ASN, prefix)) # Only as a notification to update the best route
                processor.addMessage(message)
            
            return
        
        rt = route.get()
        if (rt.ASPath == ""):
            return

        lastAS = stoi(rt.ASPath.substr(0, rt.ASPath.find(' ')))
        
        for p in peers[lastAS]:
            if (p.first == ASN):
                lastPeer = p
                break
        
        for p in peers[ASN]:
            if (p.first == lastAS):
                continue

            message = AS.filterOut(ASN, p, lastPeer, route)
            
            if (message != NULL and strcmp(message.get().type, "V") != 0):
                message.get().type = "U"
                processor.addMessage(message)

            else:

                message = make_shared[Message](Message("T", p, ASN, prefix))
                processor.addMessage(message)

    @staticmethod
    cdef void receive(int ASN, shared_ptr[Message] msg, Processor processor) noexcept nogil:
        cdef:
            shared_ptr[Route] route 
            bool flag = False
            int lastAS
            Message* message = msg.get()
            Route* rt = NULL
        
        if strcmp(message.type, 'A') == 0:

            # Discard loop
            if AS.inASPath(to_string(ASN), message.ASPath):
                locks[ASN].lock()
                routeTables[ASN].deleteRouteByPeerAS(to_string(message.lastAS), message.prefix)
                locks[ASN].unlock()
                return

            route = AS.filterIn(ASN, msg)
            
            if (route == NULL):
                locks[ASN].lock()
                route = routeTables[ASN].getBestRoute(message.prefix)
                rt = route.get()
                routeTables[ASN].deleteRouteByPeerAS(to_string(message.lastAS), message.prefix)

                lastAS = stoi(rt.ASPath.substr(0, rt.ASPath.find(' ')))
                if (lastAS == message.lastAS):
                    route = routeTables[ASN].getBestRoute(message.prefix)
                    flag = True

                locks[ASN].unlock()

                if flag:
                    AS.announce(ASN, message.peer, message.prefix, route, processor)

                return

            locks[ASN].lock()
            route = routeTables[ASN].insertRoute(route)
            locks[ASN].unlock()

            if (route != NULL):
                AS.announce(ASN, message.peer, message.prefix, route, processor)

        elif (strcmp(message.type, 'X') == 0):
            
            locks[ASN].lock()
            route = routeTables[ASN].getBestRoute(message.prefix)
            
            if (route == NULL):
                locks[ASN].unlock()
                return

            rt = route.get()
            routeTables[ASN].deleteRouteByPeerAS(to_string(message.lastAS), message.prefix)

            if (rt.ASPath == ""):
                locks[ASN].unlock()
                return

            lastAS = stoi(rt.ASPath.substr(0, rt.ASPath.find(' ')))
            
            if (lastAS == message.lastAS):
                route = routeTables[ASN].getBestRoute(message.prefix)
                flag = True 

            locks[ASN].unlock()

            if flag:
                AS.announce(ASN, message.peer, message.prefix, route, processor)

        elif strcmp(message.type, 'W') == 0:
            
            locks[ASN].lock()
            flag = routeTables[ASN].withdrawRoute(to_string(message.lastAS), message.prefix)
            locks[ASN].unlock()

            if flag:
                AS.withdraw(ASN, message.lastAS, message.prefix, processor)

        elif strcmp(message.type, 'U') == 0:

            # Discard loop
            if AS.inASPath(to_string(ASN), message.ASPath):
                locks[ASN].lock()
                routeTables[ASN].deleteRouteByPeerAS(to_string(message.lastAS), message.prefix)
                locks[ASN].unlock()
                return

            route = AS.filterIn(ASN, msg)

            if (route == NULL):
                AS.updateBest(ASN, message.lastAS, message.prefix, processor)
                return
            
            locks[ASN].lock()
            route = routeTables[ASN].updateBestRoute(route)
            locks[ASN].unlock() 

            if (route != NULL):
                AS.updateBestRoute(ASN, message.peer, route, processor)

        elif strcmp(message.type, 'T') == 0:

            AS.updateBest(ASN, message.lastAS, message.prefix, processor)
        
        elif strcmp(message.type, 'S') == 0:

            locks[ASN].lock()
            routeTables[ASN].store2DB()
            locks[ASN].unlock()

        elif strcmp(message.type, 'F') == 0:

            locks[ASN].lock()
            routeTables[ASN].fetchFromDB(message.prefixes)
            locks[ASN].unlock()

    cpdef void showRoute(self, prefix):
        routeTables[self.ASN].showRoute(prefix.encode("utf-8"))

    cpdef void showRouteInDB(self, prefix):
        routeTables[self.ASN].showRouteInDB(prefix.encode("utf-8"))

    cpdef void showBestRoute(self, prefix):
        routeTables[self.ASN].showBestRoute(prefix.encode("utf-8"))

    cpdef void deleteRouteByPeerAS(self, peerAS):
        routeTables[self.ASN].deleteRouteByPeerAS(peerAS.encode("utf-8"))

    def countRoute(self):
        return routeTables[self.ASN].countRoute()

    def getOriginAS(self, prefix):
        return routeTables[self.ASN].getOriginAS(prefix.encode("utf-8"))
