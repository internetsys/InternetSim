#ifndef ROUTETABLE_H
#define ROUTETABLE_H

#include <cstring>
#include <cstdio>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <utility>

#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <bsoncxx/json.hpp>

#include "Route.h"
#include "Trie.h"
#include "MongoManager.h"

using namespace std;

class RouteTable
{
private:
    unordered_map<unsigned int, Trie> adjRIB;

    unordered_map<unsigned int, Route> bestRoute;

    unordered_set<unsigned int> needUpdateBest;

    int ASN;

    static mutex indexCreationMutex;

    static unordered_map<int, bool> indexCreationFlag;

public:
    RouteTable(int ASN);

    ~RouteTable();

    shared_ptr<Route> insertRoute(shared_ptr<Route> rt);

    bool withdrawRoute(const string &AS, const string &prefix);

    shared_ptr<Route> updateBestRoute(shared_ptr<Route> route);

    shared_ptr<Route> getBestRoute(const string &prefix);

    void deleteRoute(shared_ptr<Route> route);

    void deleteRouteByPeerAS(const string &peerAS, const string &prefix = "");

    void showRoute(const string &prefix);

    void showRouteInDB(const string &prefix);

    unsigned long long countRoute();

    static char *strtokX(char *str, const char *delim, char **savePtr);

    static char *toBinaryIP(const char *prefix);

    static const int matchLen(const string &prefix, const string &subPre);

    void showBestRoute(const string &prefix);

    void store2DB();

    void fetchFromDB(const vector<string> &prefixes);

    int getOriginAS(const string &prefix);

    vector<shared_ptr<Route> > getRoutes(const string &prefix = "");
};

#endif
