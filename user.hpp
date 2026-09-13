#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <limits>

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

// Struct to store log entry with timestamp
struct LogEntry {
    string username;
    string message;
    time_t timestamp;
};

class user{
    protected:

    string username; // username for users
    string password; // password in string 'cause there is some characters
    string role; //roles for users 
    string major;
    int ID;
    vector <LogEntry> logs; //to count logs entry and veiw logs
    time_t creationDate;

    int failedAttempts = 0; // Track failed login attempts
    bool accountLocked = false; // Track if the account is locked
    time_t lockedUntil = 0; // Time when the temporary lock expires
    
    public:

    user(string username,string password,string role,int ID,string major,bool locked);
    void addLog(string logEntry);
    void veiwLog();
    bool isActive();
    void increasedFailedAttempts();
    void resetFailedAttempts();
    void lockAccount(int durationMinutes);

    virtual void login(std::vector<user*>& users) = 0; // login message for the user 
    virtual void logout(); // logout message for the user

    virtual string getusername(); 
    virtual string getpassword();
    virtual string getRole();
    virtual string getMajor();
    virtual int getID();

    virtual void showprofile();
    void setRole(string& newRole);
    void setPassword(const string& newPassword);
    void setMajor(string& newMajor);
    virtual string encryptpass();
    virtual ~user() {}

};
#endif