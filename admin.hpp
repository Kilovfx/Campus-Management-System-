#ifndef ADMIN_HPP
#define ADMIN_HPP

#include "user.hpp"
#include <iostream>
#include <string>
#include <memory>

using namespace std; 

class student;
class instructor;

class admin : public user{
    private:
    static int nextID;
    enum{
        CREATE_USER = 1,
        DELETE_USER,
        ASSIGN_ROLE,
        CHANGE_PASSWORD,
        CHANGE_MAJOR,
        ASSIGN_COURSE,
        LOCK_UNLOCK_USER,
        VIEW_USERS,
        VIEW_LOGS,
        SHOW_PROFILE,
        LOGOUT
    };
    
    public:
    admin(string username, string password, string role, int ID);

    // message for log in & log out 
    void login(vector<user*>& users) override; 
    void logout() override;
    void lockUser(vector<user*>& users);
    void unlockUser(vector<user*>& users);
    void showprofile() override;
    void ViewAllLog(vector<user*>& users);
    void ListAll(vector<user*>& users);
    void Showmeniu(vector<user*>& users);
    bool authenticate(string username,string password);
    void createuser(vector<user*>& users,string username,string password,string role,string major);
    void deleteuser(vector<user*>& users,string username,string password); 
    void asignrole(vector<user*>& users,string username,string newRole);
    void changepassword(vector<user*>& users);
    void changeMajor(vector<user*>& users);
    void assignCourseForInstructor(vector<user*>& users);
    string encryptpass() override;


};
#endif
