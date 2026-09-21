#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <iostream>
#include <mysql.h>
#include <string>

using namespace std;

class Database
{
private:
    MYSQL* conn;

public:

    Database();
    ~Database();
    bool connect();
    void disconnect();
    int getLastInsertID();
    MYSQL* getConnection();
    bool executeQuery(string query);
    string escapeString(const string& value);
    MYSQL_RES* executeSelect(string query);

};

#endif



