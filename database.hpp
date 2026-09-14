#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <iostream>
#include <mysql.h>


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

    MYSQL* getConnection();
};

#endif