#include "RouteTable.h"

RouteTable::RouteTable(int ASN)
{
    this->ASN = ASN;
}

RouteTable::~RouteTable()
{
}

shared_ptr<Route> RouteTable::insertRoute(shared_ptr<Route> route)
{
    string prefix = route->prefix;
    string peerAS = Trie::splitFirstAS(route->ASPath);
    shared_ptr<Route> rt;
    unsigned int prefixKey = Trie::getKey(prefix);

    try
    {
        if (adjRIB.find(prefixKey) == adjRIB.end())
        {
            adjRIB[prefixKey] = Trie();
            adjRIB[prefixKey].insert(route);
            bestRoute[prefixKey] = *route;
            return route;
        }
        else
        {
            rt = make_shared<Route>();
            rt->prefix = prefix; // Add prefix to complete

            if (adjRIB[prefixKey].findRoute(peerAS, rt))
            {
                if (adjRIB[prefixKey].erase(rt))
                {
                    adjRIB.erase(prefixKey);
                    adjRIB[prefixKey] = Trie(); // Reconstruct the Trie, because it will be inserted soon
                }

                adjRIB[prefixKey].insert(route);

                if (bestRoute[prefixKey] == *rt)
                {
                    shared_ptr<Route> bstRt;
                    adjRIB[prefixKey].findBestRoute(rt, bstRt);

                    bestRoute[prefixKey] = *bstRt;
                    return bstRt;
                }
                else if (*route > bestRoute[prefixKey])
                {
                    bestRoute[prefixKey] = *route;
                    return route;
                }
            }
            else
            {
                adjRIB[prefixKey].insert(route);

                if (*route > bestRoute[prefixKey])
                {
                    bestRoute[prefixKey] = *route;
                    return route;
                }
            }

            return nullptr;
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Error in insertRoute: " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}

shared_ptr<Route> RouteTable::getBestRoute(const string &prefix)
{
    unsigned int prefixKey = Trie::getKey(prefix);
    if (bestRoute.find(prefixKey) == bestRoute.end())
    {
        return nullptr;
    }
    return make_shared<Route>(bestRoute[prefixKey]);
}

shared_ptr<Route> RouteTable::updateBestRoute(shared_ptr<Route> route)
{
    shared_ptr<Route> rt, bstRt;
    string prefix = route->prefix;
    unsigned int prefixKey = Trie::getKey(prefix);
    bstRt = nullptr;

    try
    {
        if (route->ASPath == "")
        {
            if (needUpdateBest.find(prefixKey) != needUpdateBest.end())
            {
                needUpdateBest.erase(prefixKey);
            }
            else
            {
                return route; // Indicates that it has been updated
            }

            if (adjRIB.find(prefixKey) == adjRIB.end())
                return nullptr;

            rt = make_shared<Route>();
            rt->prefix = prefix;
            adjRIB[prefixKey].findBestRoute(rt, bstRt);

            if (bstRt == nullptr)
            {
                bestRoute.erase(prefixKey);
                return nullptr;
            }
            else if (*bstRt < bestRoute[prefixKey])
            {
                bestRoute[prefixKey] = *bstRt;
                return bstRt;
            }

            return nullptr;
        }
        else
        {
            if (adjRIB.find(prefixKey) == adjRIB.end())
            {
                if (needUpdateBest.find(prefixKey) != needUpdateBest.end())
                {
                    needUpdateBest.erase(prefixKey);
                }
                adjRIB[prefixKey] = Trie();
                adjRIB[prefixKey].insert(route);
                bestRoute[prefixKey] = *route;
                return route;
            }
            else
            {
                string peerAS = Trie::splitFirstAS(route->ASPath);
                rt = make_shared<Route>();
                rt->prefix = prefix; // Add prefix to complete

                bool flag = false;

                if (needUpdateBest.find(prefixKey) != needUpdateBest.end()) // If it has not been updated before
                {
                    needUpdateBest.erase(prefixKey);
                    adjRIB[prefixKey].findBestRoute(rt, bstRt);

                    if (*bstRt < bestRoute[prefixKey])
                    {
                        bestRoute[prefixKey] = *bstRt;
                        flag = true;
                    }
                }

                if (adjRIB[prefixKey].findRoute(peerAS, rt))
                {
                    if (adjRIB[prefixKey].erase(rt))
                    {
                        adjRIB.erase(prefixKey);
                        adjRIB[prefixKey] = Trie();
                    }

                    adjRIB[prefixKey].insert(route);

                    if (bestRoute[prefixKey] == *rt)
                    {
                        bstRt = nullptr;
                        adjRIB[prefixKey].findBestRoute(rt, bstRt);

                        bestRoute[prefixKey] = *bstRt;
                        return bstRt;
                    }
                    else if (*route > bestRoute[prefixKey])
                    {
                        bestRoute[prefixKey] = *route;
                        return route;
                    }
                }
                else
                {
                    adjRIB[prefixKey].insert(route);

                    if (*route > bestRoute[prefixKey])
                    {
                        bestRoute[prefixKey] = *route;
                        return route;
                    }
                }

                if (flag)
                {
                    return make_shared<Route>(bestRoute[prefixKey]);
                }

                return nullptr;
            }
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Error in updateBestRoute: " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}

bool RouteTable::withdrawRoute(const string &peerAS, const string &prefix)
{
    unsigned int prefixKey = Trie::getKey(prefix);
    shared_ptr<Route> route;
    route = make_shared<Route>();

    needUpdateBest.insert(prefixKey);

    if (adjRIB.find(prefixKey) != adjRIB.end() && adjRIB[prefixKey].findRoute(peerAS, route))
    {
        if (adjRIB[prefixKey].eraseByPeerAS(peerAS))
        {
            adjRIB.erase(prefixKey);
        }

        if (peerAS == Trie::splitFirstAS(bestRoute[prefixKey].ASPath)) // The route will be propagated to other ASes
        {
            return true;
        }
        return false;
    }

    return false;
}

void RouteTable::deleteRoute(shared_ptr<Route> route)
{
    unsigned int prefixKey = Trie::getKey(route->prefix);

    needUpdateBest.insert(prefixKey);

    if (adjRIB[prefixKey].erase(route))
    {
        adjRIB.erase(prefixKey);
        bestRoute.erase(prefixKey);
    }
    else if (*route == bestRoute[prefixKey])
    {
        shared_ptr<Route> rt, bstRt;
        rt = make_shared<Route>();
        rt->prefix = route->prefix;

        adjRIB[prefixKey].findBestRoute(rt, bstRt);
        bestRoute[prefixKey] = *bstRt;
    }
}

void RouteTable::deleteRouteByPeerAS(const string &peerAS, const string &prefix)
{
    if (prefix != "")
    {
        unsigned int prefixKey = Trie::getKey(prefix);

        if (adjRIB.find(prefixKey) != adjRIB.end())
        {
            if (adjRIB[prefixKey].eraseByPeerAS(peerAS))
            {
                adjRIB.erase(prefixKey);
                bestRoute.erase(prefixKey);
            }
            else if (peerAS == Trie::splitFirstAS(bestRoute[prefixKey].ASPath))
            {
                shared_ptr<Route> rt, bstRt;
                rt = make_shared<Route>();
                rt->prefix = prefix;
                adjRIB[prefixKey].findBestRoute(rt, bstRt);
                bestRoute[prefixKey] = *bstRt;
            }
        }
    }
    else
    {
        for (auto &item : adjRIB)
        {
            if (adjRIB[item.first].eraseByPeerAS(peerAS))
            {
                adjRIB.erase(item.first);
                bestRoute.erase(item.first);
            }
            else if (peerAS == Trie::splitFirstAS(bestRoute[item.first].ASPath))
            {
                shared_ptr<Route> rt, bstRt;
                rt = make_shared<Route>();
                rt->prefix = Trie::getValue(item.first);
                adjRIB[item.first].findBestRoute(rt, bstRt);
                bestRoute[item.first] = *bstRt;
            }
        }
    }
}

void RouteTable::showRoute(const string &prefix)
{
    if (Trie::num.find(prefix) == Trie::num.end() || adjRIB.find(Trie::num[prefix]) == adjRIB.end())
    {
        printf("No such route\n");
        return;
    }
    adjRIB[Trie::num[prefix]].showAllRoute(prefix);
}

void RouteTable::showRouteInDB(const string &prefix)
{
    MongoManager::initialize();

    try
    {
        auto client = MongoManager::acquire();
        auto collection = (*client)["RouteTable"][to_string(this->ASN % 2000)];

        bsoncxx::builder::basic::document queryBuilder;
        queryBuilder.append(bsoncxx::builder::basic::kvp("AS", (int32_t)this->ASN));
        queryBuilder.append(bsoncxx::builder::basic::kvp("prefix", prefix));

        auto query = queryBuilder.extract();

        auto cursor = collection.find(query.view());

        if (cursor.begin() == cursor.end())
        {
            cout << "No route" << endl;
            return;
        }
        else
        {
            string item;
            for (auto &&doc : cursor)
            {
                bsoncxx::document::view view = doc;
                bsoncxx::document::element routes = view["route"];
                bsoncxx::array::view routesView = routes.get_array().value;

                for (const bsoncxx::array::element &route : routesView)
                {
                    bsoncxx::document::view routeView = route.get_document().value;
                    item = "Route(prefix=" + prefix + ", ";
                    item += "origin=" + string(routeView["origin"].get_string().value) + ", ";
                    item += "localPref=" + to_string(routeView["localPref"].get_int32().value) + ", ";

                    auto ASPathView = routeView["ASPath"].get_array().value;

                    vector<string_view> ASPath;
                    for (const auto &element : ASPathView)
                    {
                        ASPath.push_back(element.get_string().value);
                    }
                    for (auto it = ASPath.rbegin(); it != ASPath.rend(); ++it)
                    {
                        cout << item + "ASPath=" + string(*it) + ", community=)" << endl;
                    }
                }
            }
        }
    }
    catch (const mongocxx::exception &e)
    {
        cout << "RouteTable::showRouteInDB Error: " << e.what() << endl;
    }
}

char *RouteTable::strtokX(char *str, const char *delim, char **savePtr)
{
    char *end;

    // If the str is empty, continue processing the last string
    if (str == NULL)
    {
        str = *savePtr;
    }
    if (!str)
    {
        return NULL;
    }

    // Skip characters in delim
    while (*str && strchr(delim, *str))
    {
        str++;
    }

    // The end of the string is reached
    if (!*str)
    {
        return NULL;
    }

    // Find the delimiter or end of string
    end = str;
    while (*end && !strchr(delim, *end))
    {
        end++;
    }

    // The end of the string, set savePtr to NULL
    if (!*end)
    {
        *savePtr = NULL;
    }
    else
    {
        // Replace the delimiter with '\0' and move savePtr
        *end = '\0';
        *savePtr = end + 1;
    }

    return str;
}

char *RouteTable::toBinaryIP(const char *prefix)
{
    size_t subnet = 32;
    char *IP = (char *)malloc(strlen(prefix) + 1);
    char *copyPrefix = strdup(prefix);
    char *savePtr = NULL;

    if (strchr(prefix, '/') == NULL)
    {
        strcpy(IP, prefix);
    }
    else
    {
        strcpy(IP, strtokX(copyPrefix, "/", &savePtr));
        subnet = atoi(strtokX(NULL, "/", &savePtr));
    }

    char *binaryIP = (char *)malloc(subnet + 1);
    binaryIP[0] = '\0';

    // convert IP address to binary
    char *token = strtokX(IP, ".", &savePtr);
    size_t l = 0;

    while (token != NULL)
    {
        size_t octet = atoi(token);
        for (size_t bit = 128; bit > 0 && l < subnet; bit >>= 1)
        {
            strcat(binaryIP, (octet & bit) ? "1" : "0");
            l++;
        }

        if (l == subnet)
        {
            break;
        }

        token = strtokX(NULL, ".", &savePtr);
    }

    // padding with 0s if necessary
    if (strlen(binaryIP) < subnet)
    {
        l = strlen(binaryIP);
        for (size_t i = l; i < subnet; ++i)
        {
            strcat(binaryIP, "0");
        }
    }
    binaryIP[subnet] = '\0';

    free(IP);
    free(copyPrefix);
    return binaryIP;
}

const int RouteTable::matchLen(const string &prefix, const string &subPre)
{
    char *binaryPreIP = toBinaryIP(prefix.c_str());
    char *binarySubPreIP = toBinaryIP(subPre.c_str());

    if (strlen(binaryPreIP) > strlen(binarySubPreIP))
    {
        free(binaryPreIP);
        free(binarySubPreIP);
        return 0;
    }

    int l = strlen(binaryPreIP);

    for (int i = 0; i < l; ++i)
        if (binaryPreIP[i] != binarySubPreIP[i])
        {
            free(binaryPreIP);
            free(binarySubPreIP);
            return 0;
        }

    return l;
}

void RouteTable::showBestRoute(const string &subPre)
{
    int maxLen = 0;
    shared_ptr<Route> route, bstRt;
    route = make_shared<Route>();

    for (auto &item : adjRIB)
    {
        int len = matchLen(Trie::stringMap[item.first], subPre);
        if (len > maxLen)
        {
            maxLen = len;
            adjRIB[item.first].findBestRoute(route, bstRt);
        }
    }

    printf("Route(prefix=%s, origin=%s, localPref=%d, ASPath=%s, community=%s)\n",
           bstRt->prefix.c_str(),
           bstRt->origin.c_str(),
           bstRt->localPref,
           bstRt->ASPath.c_str(),
           bstRt->community.c_str());
}

vector<shared_ptr<Route> > RouteTable::getRoutes(const string &prefix)
{
    vector<shared_ptr<Route> > routes;
    if (prefix != "")
    {
        unsigned int prefixKey = Trie::getKey(prefix);
        if (adjRIB.find(prefixKey) != adjRIB.end())
        {
            shared_ptr<Route> route, bstRt;
            route = make_shared<Route>();
            adjRIB[prefixKey].findBestRoute(route, bstRt);
            bstRt->prefix = prefix;
            routes.push_back(bstRt);
        }
    }
    else
    {
        for (auto &item : adjRIB)
        {
            shared_ptr<Route> route, bstRt;
            route = make_shared<Route>();
            adjRIB[item.first].findBestRoute(route, bstRt);
            bstRt->prefix = Trie::getValue(item.first);
            routes.push_back(bstRt);
        }
    }
    return routes;
}

int RouteTable::getOriginAS(const string &prefix)
{
    unsigned int prefixKey = Trie::getKey(prefix);

    if (adjRIB.find(prefixKey) == adjRIB.end())
    {
        return -1;
    }

    shared_ptr<Route> route, bstRt;
    route = make_shared<Route>();
    adjRIB[prefixKey].findBestRoute(route, bstRt, 1);
    if (bstRt == nullptr)
    {
        return -1;
    }

    if (bstRt->ASPath == "")
        return this->ASN; // Self-announced route

    return stoi(bstRt->ASPath.substr(bstRt->ASPath.rfind(' ') + 1));
}

unsigned long long RouteTable::countRoute()
{
    unsigned long long n = 0;
    for (auto &item : adjRIB)
    {
        n += adjRIB[item.first].countRoute();
    }
    return n;
}

unordered_map<int, bool> RouteTable::indexCreationFlag;
mutex RouteTable::indexCreationMutex;

void RouteTable::store2DB()
{
    MongoManager::initialize();

    vector<bsoncxx::document::value> documents;

    for (const auto &item : adjRIB)
    {
        bsoncxx::builder::basic::document doc{};
        doc.append(bsoncxx::builder::basic::kvp("AS", (int32_t)this->ASN));
        doc.append(bsoncxx::builder::basic::kvp("prefix", Trie::getValue(item.first)));
        bsoncxx::builder::basic::array routes{};
        adjRIB[item.first].store2DB(routes);
        doc.append(bsoncxx::builder::basic::kvp("route", routes));

        documents.push_back(doc.extract());
    }

    adjRIB.clear();
    bestRoute.clear();

    if (!documents.empty())
    {
        auto client = MongoManager::acquire();
        int shardIndex = this->ASN % 2000;
        auto collection = (*client)["RouteTable"][to_string(shardIndex)];

        bool indexCreated = false;

        {
            lock_guard<mutex> lock(indexCreationMutex);
            if (indexCreationFlag.count(shardIndex))
            {
                indexCreated = indexCreationFlag[shardIndex];
            }
        }

        if (!indexCreated)
        {

            bsoncxx::builder::basic::document indexBuilder;
            indexBuilder.append(bsoncxx::builder::basic::kvp("AS", 1)); // ascending index
            indexBuilder.append(bsoncxx::builder::basic::kvp("prefix", 1));

            try
            {
                lock_guard<mutex> lock(indexCreationMutex);
                collection.create_index(indexBuilder.extract());
                indexCreationFlag[shardIndex] = true;
            }
            catch (const mongocxx::exception &e)
            {
                cerr << "RouteTable::store2DB: Failed to create index of " << shardIndex << ": " << e.what() << endl;
            }
        }

        try
        {
            collection.insert_many(documents);
        }
        catch (const mongocxx::exception &e)
        {
            cerr << "RouteTable::store2DB Error:" << e.what() << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void RouteTable::fetchFromDB(const vector<string> &prefixes)
{
    MongoManager::initialize();

    try
    {
        auto client = MongoManager::acquire();
        auto collection = (*client)["RouteTable"][to_string(this->ASN % 2000)];

        bsoncxx::builder::basic::document queryBuilder;
        queryBuilder.append(bsoncxx::builder::basic::kvp("AS", (int32_t)this->ASN));

        bsoncxx::builder::basic::array prefixArray;

        for (const auto &prefix : prefixes)
        {
            prefixArray.append(prefix);
        }
        queryBuilder.append(bsoncxx::builder::basic::kvp("prefix", bsoncxx::builder::basic::make_document(
                                                                       bsoncxx::builder::basic::kvp("$in", prefixArray))));
        auto query = queryBuilder.extract();

        auto cursor = collection.find(query.view());
        shared_ptr<Route> rt = make_shared<Route>();

        for (auto &&doc : cursor)
        {
            bsoncxx::document::view view = doc;
            bsoncxx::document::element routes = view["route"];
            bsoncxx::array::view routesView = routes.get_array().value;

            rt->prefix = view["prefix"].get_string().value;

            for (const bsoncxx::array::element &route : routesView)
            {
                bsoncxx::document::view routeView = route.get_document().value;
                rt->origin = routeView["origin"].get_string().value;
                rt->localPref = routeView["localPref"].get_int32().value;
                auto ASPathView = routeView["ASPath"].get_array().value;

                vector<string_view> ASPath;
                for (const auto &element : ASPathView)
                {
                    ASPath.push_back(element.get_string().value);
                }

                for (auto it = ASPath.rbegin(); it != ASPath.rend(); ++it)
                {
                    rt->ASPath = *it;
                    rt->preprocess(); // Recalculate pathLength
                    insertRoute(rt);
                }
            }
        }
    }
    catch (const mongocxx::exception &e)
    {
        cerr << "RouteTable::fetchFromDB Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}
