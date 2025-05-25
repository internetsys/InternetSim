#ifndef TRIE_H
#define TRIE_H

#include <cstring>
#include <cstdlib>
#include <memory>
#include <string>
#include <iostream>
#include <shared_mutex>

#include <phmap.h>
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>

#include "TrieNode.h"
#include "Route.h"
#include "MongoManager.h"

#define SUBMAP_N 8 // 2^N submaps

using namespace std;

class Trie
{
private:
    unique_ptr<RootNode> root;

public:
    static phmap::parallel_flat_hash_map<string, unsigned int, phmap::priv::hash_default_hash<string>,
                                         phmap::priv::hash_default_eq<string>,
                                         allocator<pair<const string, unsigned int> >, SUBMAP_N, shared_mutex>
        num;
    static phmap::parallel_flat_hash_map<unsigned int, string, phmap::priv::hash_default_hash<unsigned int>,
                                         phmap::priv::hash_default_eq<unsigned int>,
                                         allocator<pair<const unsigned int, string> >, SUBMAP_N, shared_mutex>
        stringMap;
    static unsigned int cnt;
    static shared_mutex mtx;

    Trie();

    Trie(Trie &&other);

    Trie(const Trie &) = delete;

    Trie &operator=(Trie &&other);

    Trie &operator=(const Trie &) = delete;

    ~Trie();

    static const string splitFirstAS(const string &ASPath);

    static unsigned int getKey(const string &str);

    static const string &getValue(const unsigned int &key);

    static bool findVal(const string &str);

    void insert(shared_ptr<Route> route);

    void showAllRoute(string route);

    unsigned long long countRoute();

    void findBestRoute(shared_ptr<Route> route, shared_ptr<Route> &bstRt);

    void findBestRoute(shared_ptr<Route> route, shared_ptr<Route> &bstRt, int flag);

    bool findRoute(const string &peerAS, shared_ptr<Route> &route);

    bool erase(shared_ptr<Route> route);

    bool eraseByPeerAS(const string &AS);

    void clearRoute();

    void store2DB(bsoncxx::builder::basic::array &routes);
};

#endif