#include "Trie.h"

Trie::Trie() : root(make_unique<RootNode>()) {}

Trie::Trie(Trie &&other)
    : root(move(other.root)) {}

Trie &Trie::operator=(Trie &&other)
{
    if (this != &other)
    {
        root = move(other.root);
    }
    return *this;
}

Trie::~Trie()
{
}

const string Trie::splitFirstAS(const string &ASPath)
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

phmap::parallel_flat_hash_map<string, unsigned int, phmap::priv::hash_default_hash<string>,
                              phmap::priv::hash_default_eq<string>,
                              allocator<pair<const string, unsigned int> >, SUBMAP_N, shared_mutex>
    Trie::num;
phmap::parallel_flat_hash_map<unsigned int, string, phmap::priv::hash_default_hash<unsigned int>,
                              phmap::priv::hash_default_eq<unsigned int>,
                              allocator<pair<const unsigned int, string> >, SUBMAP_N, shared_mutex>
    Trie::stringMap;
unsigned int Trie::cnt = 0;
shared_mutex Trie::mtx;

unsigned int Trie::getKey(const string &str)
{
    {
        shared_lock<shared_mutex> read_lock(mtx);
        if (auto it = num.find(str); it != num.end())
        {
            return it->second;
        }
    }

    unique_lock<shared_mutex> write_lock(mtx);
    auto [insert_it, inserted] = num.try_emplace(str, ++cnt);
    if (inserted)
    {
        stringMap.try_emplace(cnt, str);
    }
    return insert_it->second;
}

const string &Trie::getValue(const unsigned int &key)
{
    try
    {
        return stringMap.at(key);
    }
    catch (const std::out_of_range &e)
    {
        cerr << "Trie::getValue Error:" << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}

void Trie::insert(shared_ptr<Route> route)
{
    TrieNode *tmpChild = nullptr;
    for (auto &child : root->children)
    {
        if (getValue(child->origin) == route->origin && child->localPref == route->localPref)
        {
            tmpChild = child.get();
            break;
        }
    }

    if (tmpChild == nullptr)
    {
        auto newChild = make_unique<TrieNode>();

        newChild->localPref = route->localPref;
        newChild->origin = getKey(route->origin);
        newChild->records.emplace_front(route->ASPath, route->community);

        root->children.emplace_front(move(newChild));
    }
    else
    {
        tmpChild->records.emplace_front(route->ASPath, route->community);
    }
}

void Trie::showAllRoute(string prefix)
{
    if (root->children.empty())
    {
        cout << "No route" << endl;
        return;
    }

    string route;
    for (auto &child : root->children)
    {
        route = "Route(prefix=" + prefix + ", ";
        route += "origin=" + getValue(child->origin) + ", ";
        route += "localPref=" + to_string(child->localPref) + ", ";
        for (auto &record : child->records)
        {
            const string &ASPath = record.first;
            const string &community = record.second;
            cout << route + "ASPath=" + ASPath + ", community=" + community + ")" << endl;
        }
    }
}

unsigned long long Trie::countRoute()
{
    unsigned long long n = 0;
    for (auto &child : root->children)
    {
        for (auto it = child->records.begin(); it != child->records.end(); ++it)
        {
            ++n;
        }
    }
    return n;
}

void Trie::findBestRoute(shared_ptr<Route> route, shared_ptr<Route> &bstRt)
{
    for (auto &child : root->children)
    {
        route->origin = getValue(child->origin);
        route->localPref = child->localPref;
        for (auto &record : child->records)
        {
            route->ASPath = record.first;
            route->community = record.second;
            route->preprocess();
            if (bstRt == nullptr || *route > bstRt)
            {
                bstRt = make_shared<Route>(*route);
            }
        }
    }
}

void Trie::findBestRoute(shared_ptr<Route> route, shared_ptr<Route> &bstRt, int flag)
{
    for (auto &child : root->children)
    {
        route->origin = getValue(child->origin);
        route->localPref = child->localPref;
        for (auto &record : child->records)
        {
            route->ASPath = record.first;
            route->community = record.second;
            route->preprocess();
            if (bstRt == nullptr || *route > bstRt)
            {
                bstRt = make_shared<Route>(*route);
            }
        }
    }
}

bool Trie::findRoute(const string &peerAS, shared_ptr<Route> &route)
{
    for (auto &child : root->children)
    {
        route->origin = getValue(child->origin);
        route->localPref = child->localPref;
        for (auto &record : child->records)
        {
            route->ASPath = record.first;
            if (peerAS == splitFirstAS(route->ASPath))
            {
                route->preprocess();
                return true;
            }
        }
    }
    return false;
}

bool Trie::erase(shared_ptr<Route> route)
{
    bool found = false;

    for (auto &child : root->children)
    {
        if (getValue(child->origin) == route->origin && child->localPref == route->localPref)
        {
            auto it = child->records.begin();
            auto last = child->records.before_begin();

            while (it != child->records.end())
            {
                if ((*it).first == route->ASPath)
                {
                    child->records.erase_after(last);
                    found = true;
                    break;
                }

                last = it;
                ++it;
            }

            if (found)
            {
                break;
            }
        }
    }

    if (!found)
        return false;

    root->children.remove_if([](const unique_ptr<TrieNode> &child)
                             { return child->records.empty(); });

    if (root->children.empty())
    {
        root.reset();
        return true;
    }

    return false;
}

bool Trie::eraseByPeerAS(const string &AS)
{
    bool found = false;

    for (auto &child : root->children)
    {
        auto it = child->records.begin();
        auto last = child->records.before_begin();

        while (it != child->records.end())
        {
            if (splitFirstAS((*it).first) == AS)
            {
                child->records.erase_after(last);
                found = true;
                break;
            }

            last = it;
            ++it;
        }

        if (found)
        {
            break;
        }
    }

    if (!found)
    {
        return false;
    }

    root->children.remove_if([](const unique_ptr<TrieNode> &child)
                             { return child->records.empty(); });

    if (root->children.empty())
    {
        root.reset();
        return true;
    }

    return false;
}

void Trie::clearRoute()
{
    for (auto &child : root->children)
    {
        child->records.clear();
    }

    root->children.clear();

    root.reset();
}

void Trie::store2DB(bsoncxx::builder::basic::array &routes)
{
    for (auto &child : root->children)
    {
        bsoncxx::builder::basic::document route{};
        route.append(bsoncxx::builder::basic::kvp("origin", Trie::getValue(child->origin)));
        route.append(bsoncxx::builder::basic::kvp("localPref", (int32_t)child->localPref));

        bsoncxx::builder::basic::array ASPath{};

        for (auto &record : child->records)
        {
            ASPath.append(record.first);
        }

        route.append(bsoncxx::builder::basic::kvp("ASPath", ASPath));
        routes.append(route);
    }

    clearRoute();
}
