#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <limits>
#include "database.hpp"
#include "Password.hpp"

using namespace std;

inline bool hasExtraInputOnLine() {
    if (cin.peek() == '\n') {
        cin.get();
        return false;
    }
    if (cin.peek() == EOF) {
        return false;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return true;
}

class user{
    protected:

    string username; // username for users
    string password; // password in string 'cause there is some characters
    string first_name;
    string last_name;
    string role; //roles for users 
    string major;
    int ID;
    time_t creationDate;

    int failedAttempts = 0; // Track failed login attempts
    bool accountLocked = false; // Track if the account is locked
    time_t lockedUntil = 0; // Time when the temporary lock expires
    
    public:

    user(string username,string password,string role,int ID,string first_name,string last_name,string major,bool locked);
    void addLog(Database& db,string logEntry);
    bool isActive();
    bool increaseFailedAttempts(Database &db);
    void resetFailedAttempts(Database &db);
    static bool recordLoginAttempts(Database &db,string username, bool success);
    void lockAccount(int durationMinutes);

    virtual void login(std::vector<user*>& users,Database &db) = 0; // login message for the user 
    virtual void logout(Database& db); // logout message for the user

    static string getIPAddress();

    virtual string getusername(); 
    virtual string getpassword();
    virtual string getRole();
    virtual string getMajor();
    virtual int getID();

    virtual void showprofile(Database& db);
    void setRole(string& newRole);
    void setPassword(const string& newPassword);
    void setMajor(string& newMajor);
    virtual string encryptpass();
    virtual ~user() {}

    bool isUserLocked(Database &db);
    bool authenticate(const string& username, const string& passwrod);

};
#endif