#include "Hash.h"

template <typename T>
inline void hashCombine(size_t &seed, const T &val)
{
    seed ^= hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// auxiliary generic functions to create a hash value using a seed
template <typename T>
inline void hashVal(size_t &seed, const T &val)
{
    hashCombine(seed, val);
}

template <typename T, typename... Types>
inline void hashVal(size_t &seed, const T &val, const Types &...args)
{
    hashCombine(seed, val);
    hashVal(seed, args...);
}

template <typename... Types>
inline size_t hashVal(const Types &...args)
{
    size_t seed = 0;
    hashVal(seed, args...);
    return seed;
}

size_t RouteHash::operator()(const shared_ptr<Route> route) const
{
    return hashVal(route->prefix, route->localPref, route->ASPath, route->origin);
}

size_t PairHash::operator()(const pair<int, int> &p) const
{
    return hashVal(p.first, p.second);
}

size_t PairHash::operator()(const pair<int, string> &p) const
{
    return hashVal(p.first, p.second);
}
