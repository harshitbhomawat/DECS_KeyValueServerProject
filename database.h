#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <libpq-fe.h>

class DBConnectionPool {
public:
    DBConnectionPool(int size, const std::string &conninfo);
    ~DBConnectionPool();

    PGconn* acquire();
    void release(PGconn* conn);

private:
    std::queue<PGconn*> pool_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

bool db_connect();
void db_close();

bool db_insert(const std::string &key, const std::string &value);
bool db_get(const std::string &key, std::string &value);
bool db_delete(const std::string &key);
bool db_flush();  //Only for testing.

#endif
