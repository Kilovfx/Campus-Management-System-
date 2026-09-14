#include "Database.hpp"


Database::Database()
{
    conn = NULL;
}


Database::~Database()
{
    disconnect();
}


bool Database::connect()
{
    MYSQL* initializedConnection = mysql_init(NULL);

    if(initializedConnection == NULL)
    {
        cout << "MySQL initialization failed\n";
        return false;
    }


    conn = mysql_real_connect(
        initializedConnection,
        "localhost",
        "root",
        "4587103",
        "campus_managment",
        3306,
        NULL,
        0
    );


    if(conn)
    {
        cout << "Database connected\n";
        return true;
    }


    cout << "Database connection failed: "
         << mysql_error(initializedConnection) << endl;
    mysql_close(initializedConnection);
    conn = NULL;
    return false;
}


void Database::disconnect()
{
    if(conn)
    {
        mysql_close(conn);
        conn = NULL;
    }
}


MYSQL* Database::getConnection()
{
    return conn;
}