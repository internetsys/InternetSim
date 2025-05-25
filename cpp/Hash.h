#ifndef HASH_H
#define HASH_H

#include <functional>

#include "Route.h"

using namespace std;

struct RouteHash
{
    size_t operator()(const shared_ptr<Route> route) const;
};

struct PairHash
{
    size_t operator()(const pair<int, int> &p) const;

    size_t operator()(const pair<int, string> &p) const;
};

#endif