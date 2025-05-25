#include "MongoManager.h"

string MongoManager::uri = "mongodb://localhost:27017";
unique_ptr<mongocxx::instance> MongoManager::instance = nullptr;
unique_ptr<mongocxx::pool> MongoManager::pool = nullptr;
once_flag MongoManager::flag;

void MongoManager::initialize()
{
    call_once(flag, [&]()
              {
        try {
            instance = make_unique<mongocxx::instance>();

            pool = make_unique<mongocxx::pool>(mongocxx::uri(uri));
            cout << "MongoDB initialized." << endl;

        } catch (const std::exception& e) {
            cerr << "MongoDB initialization failed: " << e.what() << endl;
            exit(EXIT_FAILURE);
        } });
}

mongocxx::pool &MongoManager::getPool()
{
    if (!pool)
    {
        cerr << "MongoManager::pool() called before init()" << endl;
        exit(EXIT_FAILURE);
    }
    return *pool;
}

mongocxx::pool::entry MongoManager::acquire()
{
    return getPool().acquire();
}
