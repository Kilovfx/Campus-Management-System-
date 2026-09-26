#include "user.hpp"
#include <iostream>
#ifdef _WIN32
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

using namespace std;

static const int accountLockDurationSeconds = 60;

user::user(string username, string password, string role, int ID,string first_name,string last_name,string major,bool locked) {
    this->username = username;
    this->password = password;
    this->role = role;
    this->ID = ID;
    this->first_name = first_name;
    this->last_name = last_name;
    this->major = major;  // Initialize major from the constructor parameter
    creationDate = time(0);  // Initialize creationDate with the current time
    failedAttempts = 0;
    this->accountLocked = locked;
}

//locked account functions

bool user::isActive(){
    if (accountLocked && time(nullptr) >= lockedUntil) {
        accountLocked = false;
        failedAttempts = 0;
        lockedUntil = 0;
    }
    return !accountLocked;
}

void user::increasedFailedAttempts(){
    failedAttempts++;
    if(failedAttempts >= 6){
        accountLocked = true;
        lockedUntil = time(nullptr) + accountLockDurationSeconds;
    }
}

void user::resetFailedAttempts(){
    failedAttempts = 0;
}

void user::lockAccount(int durationMinutes){
    accountLocked = true;
    lockedUntil = time(nullptr) + (durationMinutes * 60);
}

void user::login(vector<user*>& users,Database &db){
    cout<< username << " logged in successfully .\n";
}

void user::logout(){
    cout<< username << " logged out.\n";
}

string user::getusername(){
    return username;
}

string user::getpassword(){
    return password;
}

string user::getRole(){
    return role;
}

string user::getMajor(){
    return major;
}

int user::getID(){
    return ID;
}

void user::showprofile(){
    cout <<"Username : "<<username<<" ID: "<<ID<<" Role: "<<role<<endl;
    char* dt = ctime(&creationDate);
    cout << "Creation date of the Account : "<<dt<<endl;
}

void user::addLog(string logEntry){
    LogEntry entry;
    entry.username = username;
    entry.message = logEntry;
    entry.timestamp = time(0);
    logs.push_back(entry);
}

void user::veiwLog(){
    cout << "\n--- Logs for : "<< username <<" ---\n";
    cout<<"\n";
    cout << "Total log entries: " << logs.size() << "\n";
    if(logs.empty()) {
        cout << "No logs available.\n";
        return;
    }
    for(auto i = 0; i<logs.size(); ++i){
        string timestamp = ctime(&logs[i].timestamp);
        // Remove the newline from ctime()
        timestamp.pop_back();
        cout << "[" << timestamp << "] " << logs[i].message << " by " << logs[i].username << "\n";
    }
    cout<<"\n";
    cout << "--- End of logs ---\n";
};

void user::setRole(string& newRole){
    role = newRole;
}

void user::setPassword(const string& newPassword){
    password = newPassword;
}

void user::setMajor(string& newMajor){
    major = newMajor;
}


bool user::isUserLocked(Database &db){

    string query =
    "SELECT locked, locked_until "
    "FROM account_security "
    "WHERE user_id=" + to_string(ID);

     MYSQL_RES* result = db.executeSelect(query);

     if(result == nullptr)
        return false;

    MYSQL_ROW row = mysql_fetch_row(result);

    if(row == nullptr)
    {
        mysql_free_result(result);
        return false;
    }


    bool locked = stoi(row[0]);
    cout << "Database locked status: " << locked << endl;

    if(!locked)
    {
        mysql_free_result(result);
        return false;
    }

    // check if lock time expired
    string checkTime =
    "SELECT NOW() >= locked_until "
    "FROM account_security "
    "WHERE user_id=" + to_string(ID);

    MYSQL_RES* timeResult = db.executeSelect(checkTime);

    if(timeResult == nullptr){

        mysql_free_result(result);
        return locked;

    }

    MYSQL_ROW timeRow = mysql_fetch_row(timeResult);

    if(timeRow && stoi(timeRow[0]) == 1)
    {
        // unlock automatically
        string unlock =
        "UPDATE account_security SET "
        "locked=0, failed_attempts=0 "
        "WHERE user_id=" + to_string(ID);

        db.executeQuery(unlock);

        mysql_free_result(timeResult);
        mysql_free_result(result);

        return false;
    }

    mysql_free_result(timeResult);
    mysql_free_result(result);

    return true;
}




string user::encryptpass(){
    string password = "";
    char ch;

#ifdef _WIN32
    // Windows implementation using conio.h
    while (true) {
        ch = _getch();  // Read a single character without echoing
        if (ch == 13) {  // Enter key pressed (carriage return)
            cout << endl;
            break;
        } else if (ch == 8) {  // Backspace key
            if (password.length() > 0) {
                password.pop_back();
                cout << "\b \b";  // Erase the last '*' printed
            }
        } else {
            password.push_back(ch);  // Add character to password
            cout << "*";  // Print '*' instead of the actual character
        }
    }
#else
    // Unix/Linux implementation using termios.h
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);  // Get current terminal settings
    newt = oldt;
    newt.c_lflag &= ~ECHO;  // Turn off echoing of typed characters
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Apply new settings

    while (true) {
        ch = getchar();  // Read a single character
        if (ch == 10) {  // Enter key pressed (newline)
            break;
        } else if (ch == 127) {  // Backspace key
            if (password.length() > 0) {
                password.pop_back();
                cout << "\b \b";  // Erase the last '*' printed
            }
        } else {
            password.push_back(ch);  // Add character to password
            cout << "*";  // Print '*' instead of the actual character
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restore terminal settings
#endif
    return password;  // Return the password
}
