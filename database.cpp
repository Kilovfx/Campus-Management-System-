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
    

    else{
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

    }
    
    cout << "Database connection failed: "
         << mysql_error(initializedConnection) << endl;
    mysql_close(initializedConnection);
    conn = NULL;
    return false;
}

MYSQL_RES* Database::executeSelect(string query){
    if(mysql_query(conn,query.c_str())){
        cout << mysql_error(conn);
        return nullptr;
    }
    return mysql_store_result(conn);
}

bool Database::executeQuery(string query)
{
    if(mysql_query(conn, query.c_str()))
    {
        return false;
    }

    return true;
}



void Database::disconnect()
{
    if(conn)
    {
        mysql_close(conn);
        conn = NULL;
    }
}


MYSQL* Database::getConnection(){
    return conn;
}