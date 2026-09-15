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
    if (conn == nullptr) {
        cout << "Database is not connected\n";
        return nullptr;
    }

    if(mysql_query(conn,query.c_str())){
        cout << mysql_error(conn) << endl;
        return nullptr;
    }
    return mysql_store_result(conn);
}

bool Database::executeQuery(string query)
{
    if (conn == nullptr) {
        cout << "Database is not connected\n";
        return false;
    }

    if(mysql_query(conn, query.c_str()))
    {
        cout << mysql_error(conn) << endl;
        return false;
    }

    return true;
}

// for SQL injections
string Database::escapeString(const string& value)
{
    if (conn == nullptr) {
        return value;
    }
    string escaped(value.size() * 2 + 1, '\0');
    unsigned long len = mysql_real_escape_string(
        conn, &escaped[0], value.c_str(), value.size());
    escaped.resize(len);
    return escaped;
}


int Database::getLastInsertID(){

    if(conn == nullptr){

        return -1;
    }

    return static_cast<int>(mysql_insert_id(conn));
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