#ifndef MONGOMANAGER_H
#define MONGOMANAGER_H

#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <memory>
#include <string>
#include <mutex>
#include <iostream>

using namespace std;

class MongoManager
{
private:
    static string uri;

    static unique_ptr<mongocxx::instance> instance;
    static unique_ptr<mongocxx::pool> pool;
    static once_flag flag;

    MongoManager() = delete;

public:
    static mongocxx::pool &getPool();

    static void initialize();

    static mongocxx::pool::entry acquire();
};

#endif