#include "Route.h"

Route::Route()
{
    this->localPref = 100;
    this->pathLength = 0;
    this->originPriority = 0;
}

Route::Route(const string &prefix, const unsigned short &localPref, const string &ASPath, const string &origin, const string &community)
{
    this->prefix = prefix;
    this->localPref = localPref;
    this->ASPath = ASPath;
    this->origin = origin;
    this->community = community;

    size_t len = this->ASPath.length();
    if (len == 0)
    {
        this->pathLength = 0;
    }
    else
    {
        this->pathLength = 1;
        for (size_t i = 0; i < len; i++)
            if (this->ASPath[i] == ' ')
                ++this->pathLength;
    }

    this->originPriority = (this->origin == "IGP") ? 3 : (this->origin == "EGP") ? 2
                                                                                 : 1;
}

Route::~Route()
{
}

const string Route::splitFirstAS(const string &ASPath)
{
    // Find the position of the first space
    size_t pos = ASPath.find(' ');

    // If no spaces are found, return the entire ASPath
    if (pos == string::npos)
    {
        return ASPath;
    }

    // Extract the first AS
    return ASPath.substr(0, pos);
}

void Route::preprocess()
{
    size_t len = this->ASPath.length();
    if (len == 0)
    {
        this->pathLength = 0;
    }
    else
    {
        this->pathLength = 1;
        for (size_t i = 0; i < len; i++)
            if (this->ASPath[i] == ' ')
                ++this->pathLength;
    }

    this->originPriority = (this->origin == "IGP") ? 3 : (this->origin == "EGP") ? 2
                                                                                 : 1;
}

bool Route::operator==(Route &route)
{
    if (this->pathLength == 0)
        this->preprocess();

    if (route.pathLength == 0)
        route.preprocess();

    return (this->prefix == route.prefix && this->localPref == route.localPref && this->ASPath == route.ASPath && this->origin == route.origin && this->community == route.community);
}

bool Route::operator!=(Route &route)
{
    return !(*this == route);
}

bool Route::operator>(shared_ptr<Route> route)
{
    if (this->pathLength == 0)
        this->preprocess();

    if (route->pathLength == 0)
        route->preprocess();

    if (this->pathLength == 0 && route->pathLength == 0)
        return false;

    if (this->localPref != route->localPref)
        return this->localPref > route->localPref;

    if (this->pathLength != route->pathLength)
        return this->pathLength < route->pathLength;

    if (this->originPriority != route->originPriority)
        return this->originPriority > route->originPriority;

    int peer1, peer2;

    try
    {
        peer1 = stoi(splitFirstAS(this->ASPath));
        peer2 = stoi(splitFirstAS(route->ASPath));
    }
    catch (const std::exception &e)
    {
        cerr << "Error in Route::operator>(shared_ptr<Route> route) : " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }

    return peer1 < peer2;
}

bool Route::operator<(shared_ptr<Route> route)
{
    if (this->pathLength == 0)
        this->preprocess();

    if (route->pathLength == 0)
        route->preprocess();

    if (this->pathLength == 0 && route->pathLength == 0)
        return false;

    if (this->localPref != route->localPref)
        return this->localPref < route->localPref;

    if (this->pathLength != route->pathLength)
        return this->pathLength > route->pathLength;

    if (this->originPriority != route->originPriority)
        return this->originPriority < route->originPriority;

    int peer1, peer2;
    try
    {
        peer1 = stoi(splitFirstAS(this->ASPath));
        peer2 = stoi(splitFirstAS(route->ASPath));
    }
    catch (const std::exception &e)
    {
        cerr << "Error in Route::operator<(shared_ptr<Route> route) : " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }

    return peer1 > peer2;
}

bool Route::operator>(Route &route)
{
    if (this->pathLength == 0)
        this->preprocess();

    if (route.pathLength == 0)
        route.preprocess();

    if (this->pathLength == 0 && route.pathLength == 0)
        return false;

    if (this->localPref != route.localPref)
        return this->localPref > route.localPref;

    if (this->pathLength != route.pathLength)
        return this->pathLength < route.pathLength;

    if (this->originPriority != route.originPriority)
        return this->originPriority > route.originPriority;

    int peer1, peer2;

    try
    {
        peer1 = stoi(splitFirstAS(this->ASPath));
        peer2 = stoi(splitFirstAS(route.ASPath));
    }
    catch (const std::exception &e)
    {
        cerr << "Error in Route::operator>(Route &route) : " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }

    return peer1 < peer2;
}

bool Route::operator<(Route &route)
{
    if (this->pathLength == 0)
        this->preprocess();

    if (route.pathLength == 0)
        route.preprocess();

    if (this->pathLength == 0 && route.pathLength == 0)
        return false;

    if (this->localPref != route.localPref)
        return this->localPref < route.localPref;

    if (this->pathLength != route.pathLength)
        return this->pathLength > route.pathLength;

    if (this->originPriority != route.originPriority)
        return this->originPriority < route.originPriority;

    int peer1, peer2;
    try
    {
        peer1 = stoi(splitFirstAS(this->ASPath));
        peer2 = stoi(splitFirstAS(route.ASPath));
    }
    catch (const std::exception &e)
    {
        cerr << "Error in Route::operator<(Route &route) : " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }

    return peer1 > peer2;
}

bool RouteCmp::operator()(const shared_ptr<Route> lhs, const shared_ptr<Route> rhs) const
{
    if (lhs->pathLength == 0)
        lhs->preprocess();

    if (rhs->pathLength == 0)
        rhs->preprocess();

    if (lhs->localPref != rhs->localPref)
        return lhs->localPref < rhs->localPref;

    if (lhs->pathLength != rhs->pathLength)
        return lhs->pathLength > rhs->pathLength;

    if (lhs->originPriority != rhs->originPriority)
        return lhs->originPriority < rhs->originPriority;

    int peer1, peer2;
    peer1 = stoi(Route::splitFirstAS(lhs->ASPath));
    peer2 = stoi(Route::splitFirstAS(rhs->ASPath));

    return peer1 > peer2;
}
