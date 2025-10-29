#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <libpq-fe.h>

bool db_connect();
bool db_insert(const std::string& key, const std::string& value);
bool db_get(const std::string& key, std::string& value_out);
bool db_delete(const std::string& key);
void db_close();

#endif // DATABASE_H
