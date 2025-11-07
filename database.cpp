#include "database.h"
#include <iostream>
#include <cstdlib>

static const int POOL_SIZE = 10;

static DBConnectionPool* db_pool = nullptr;
static std::string global_conninfo = "dbname=kvdb user=kvuser password=kvpass host=127.0.0.1";

std::string get_conninfo_from_env() {
    const char* dbname = std::getenv("DB_NAME");
    const char* dbuser = std::getenv("DB_USER");
    const char* dbpass = std::getenv("DB_PASS");
    const char* dbhost = std::getenv("DB_HOST");

    std::string conninfo =
        "dbname=" + std::string(dbname ? dbname : "kvdb") +
        " user=" + std::string(dbuser ? dbuser : "kvuser") +
        " password=" + std::string(dbpass ? dbpass : "kvpass") +
        " host=" + std::string(dbhost ? dbhost : "127.0.0.1");
    return conninfo;
}


DBConnectionPool::DBConnectionPool(int size, const std::string &conninfo) {
    for (int i = 0; i < size; i++) {
        PGconn* conn = PQconnectdb(conninfo.c_str());
        if (PQstatus(conn) != CONNECTION_OK) {
            std::cerr << "DB Pool Connection Failed: " << PQerrorMessage(conn) << std::endl;
            PQfinish(conn);
        } else {
            pool_.push(conn);
        }
    }
}

DBConnectionPool::~DBConnectionPool() {
    while (!pool_.empty()) {
        PQfinish(pool_.front());
        pool_.pop();
    }
}

PGconn* DBConnectionPool::acquire() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [&]() { return !pool_.empty(); });

    PGconn* conn = pool_.front();
    pool_.pop();
    return conn;
}

void DBConnectionPool::release(PGconn* conn) {
    std::lock_guard<std::mutex> lock(mtx_);
    pool_.push(conn);
    cv_.notify_one();
}

bool db_insert(const std::string &key, const std::string &value) {
    PGconn* conn = db_pool->acquire();

    std::string query =
        "INSERT INTO kv_store (key, value) VALUES ('" + key + "', '" + value + "') "
        "ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;";

    PGresult* res = PQexec(conn, query.c_str());
    bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    PQclear(res);

    db_pool->release(conn);
    return ok;
}

bool db_get(const std::string &key, std::string &value_out) {
    PGconn* conn = db_pool->acquire();

    std::string query =
        "SELECT value FROM kv_store WHERE key = '" + key + "';";
    PGresult* res = PQexec(conn, query.c_str());

    bool ok = PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0;

    if (ok)
        value_out = PQgetvalue(res, 0, 0);

    PQclear(res);
    db_pool->release(conn);
    return ok;
}

bool db_delete(const std::string &key) {
    PGconn* conn = db_pool->acquire();

    std::string query =
        "DELETE FROM kv_store WHERE key = '" + key + "';";
    PGresult* res = PQexec(conn, query.c_str());

    bool ok = PQresultStatus(res) == PGRES_COMMAND_OK;
    PQclear(res);

    db_pool->release(conn);
    return ok;
}

bool db_connect() {
    db_pool = new DBConnectionPool(POOL_SIZE, get_conninfo_from_env());
    std::cout << "DB Connection Pool Created (" << POOL_SIZE << " connections)" << std::endl;
    return true;
}

void db_close() {
    delete db_pool;
    db_pool = nullptr;
}

