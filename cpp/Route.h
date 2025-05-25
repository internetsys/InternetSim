#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <iostream>

using namespace std;

class Route
{
public:
    string prefix, ASPath, origin, community;
    unsigned short localPref;
    unsigned char pathLength, originPriority;

    Route();

    Route(const string &prefix, const unsigned short &localPref, const string &ASPath, const string &origin, const string &community = "");

    ~Route();

    static const string splitFirstAS(const string &ASPath);

    void preprocess();

    bool operator==(Route &route);

    bool operator!=(Route &route);

    bool operator>(shared_ptr<Route> route);

    bool operator>(Route &route);

    bool operator<(shared_ptr<Route> route);

    bool operator<(Route &route);
};

struct RouteCmp
{
    bool operator()(const shared_ptr<Route> lhs, const shared_ptr<Route> rhs) const;
};

#endif