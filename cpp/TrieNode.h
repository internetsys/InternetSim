#ifndef TRIENODE_H
#define TRIENODE_H

#include <memory>
#include <forward_list>

#include "Route.h"

using namespace std;

class TrieNode
{
public:
    forward_list<pair<string, string> > records;
    unsigned int origin;
    unsigned short localPref;

    TrieNode();

    ~TrieNode();
};

class RootNode
{
public:
    forward_list<unique_ptr<TrieNode> > children;

    RootNode();

    ~RootNode();
};

#endif
