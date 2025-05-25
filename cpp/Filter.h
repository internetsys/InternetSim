#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <cstdio>
#include <forward_list>

using namespace std;

enum Match : char
{
    ANY,
    COMMUNITY_CONTAINS,
    PREFIX_IS,
    PREFIX_IN,
    PATH,
    AS_SET
};

enum Action : char
{
    ACCEPT,
    DENY,
    LOCAL_PREF,
    COMMUNITY_ADD,
    COMMUNITY_STRIP,
    COMMUNITY_REMOVE,
    PREPEND,
};

class Filter
{
public:
    Match matchType;
    string matchValue;
    forward_list<pair<Action, string> > actions;
    Filter *nextFt;

    Filter();

    Filter(const string &match, string &action);

    ~Filter();
};

#endif