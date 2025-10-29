#include "database.h"
#include <iostream>

static PGconn* conn = nullptr;

bool db_connect() {
    const char* conninfo = "dbname=kvdb user=kvuser password=kvpass host=127.0.0.1";

    conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "❌ DB Connection failed: " << PQerrorMessage(conn) << std::endl;
        return false;
    }

    std::cout << "✅ Database Connected Successfully!" << std::endl;
    return true;
}

std::string escape_string(const std::string& input) {
    char* escaped = PQescapeLiteral(conn, input.c_str(), input.length());
    std::string result(escaped);
    PQfreemem(escaped);
    return result;
}

bool db_insert(const std::string& key, const std::string& value) {
    std::string esc_key = escape_string(key);
    std::string esc_value = escape_string(value);

    std::string query =
        "INSERT INTO kv_store (key, value) VALUES (" + esc_key + ", " + esc_value + ") "
        "ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;";

    //std::cout<<"before exec query: "<<query<<std::endl;

    PGresult* res = PQexec(conn, query.c_str());

    //std::cout<<"after exec query: "<<PQcmdStatus(res)<<std::endl;

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "❌ INSERT Error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}


/*bool db_insert(const std::string& key, const std::string& value) {
    std::string query =
        "INSERT INTO kv_store (key, value) VALUES (" + key + ", '" + value + "') "
        "ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;";
        
    std::cout<<"before exec query: "<<query<<std::endl;

    PGresult* res = PQexec(conn, query.c_str());
    
    std::cout<<"after exec query: "<<query<<std::endl;

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "❌ INSERT Error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}*/

bool db_get(const std::string& key, std::string& value_out) {
    std::string query =
        "SELECT \"value\" FROM kv_store WHERE \"key\" = '" + key + "';";

    PGresult* res = PQexec(conn, query.c_str());
    ExecStatusType status = PQresultStatus(res);

    if (status != PGRES_TUPLES_OK) {
        std::cerr << "❌ SELECT Error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return false;
    }

    value_out = PQgetvalue(res, 0, 0);
    PQclear(res);
    return true;
}

bool db_delete(const std::string& key) {
    std::string query =
        "DELETE FROM kv_store WHERE \"key\" = '" + key + "';";

    PGresult* res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "❌ DELETE Error: " << PQerrorMessage(conn) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

void db_close() {
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}


/*#include "database.h"
#include <iostream>

static PGconn* conn = nullptr;

bool db_connect() {
    const char* conninfo = "dbname=kvdb user=kvuser password=kvpass host=127.0.0.1";

    conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << std::endl;
        return false;
    }
    return true;
}

bool db_insert(const std::string& key, const std::string& value) {
    std::string query = "INSERT INTO kv_store (key, value) VALUES ('" + key + "', '" + value + "') "
                        "ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;";
    
    PGresult* res = PQexec(conn, query.c_str());
    bool success = PQresultStatus(res) == PGRES_COMMAND_OK;
    
    PQclear(res);
    return success;
}

bool db_get(const std::string& key, std::string& value_out) {
    std::string query = "SELECT value FROM kv_store WHERE key = '" + key + "';";
    
    PGresult* res = PQexec(conn, query.c_str());
    bool success = PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0;

    if (success) {
        value_out = PQgetvalue(res, 0, 0);
    }
    
    PQclear(res);
    return success;
}

bool db_delete(const std::string& key) {
    std::string query = "DELETE FROM kv_store WHERE key = '" + key + "';";
    
    PGresult* res = PQexec(conn, query.c_str());
    bool success = PQresultStatus(res) == PGRES_COMMAND_OK;

    PQclear(res);
    return success;
}

void db_close() {
    if (conn) {
        PQfinish(conn);
        conn = nullptr;
    }
}
*/
