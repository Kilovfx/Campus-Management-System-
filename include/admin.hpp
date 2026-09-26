#ifndef ADMIN_HPP
#define ADMIN_HPP

#include "user.hpp"
#include "database.hpp"
#include "instructor.hpp"
#include <iostream>
#include <string>
#include <memory>

using namespace std; 

class student;
class instructor;

class admin : public user{
    private:

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
    void Showmeniu(vector<user*>& users,Database& db);
    bool authenticate(string username,string password);
    void createuser(vector<user*>& users,Database& db, string username, string password, string role,string first_name, string last_name, string major);
    int  getMajorID(Database& db,string majorName);                
    void deleteuser(vector<user*>& users,Database& db,string username,string password); 
    void asignrole(vector<user*>& users,Database &db,string username,string newRole);
    bool changeUserRoleDatabase(Database &db,user* selectedUser,string newRole);
    bool changeMajorDatabase(Database &db,user* selectedUser,string newMajor);
    bool assignCourseDatabase(Database &db,user* selectedInstructor,string courseName);
    vector<CourseInfo> getCoursesForMajor(Database& db, string major);
    void changepassword(vector<user*>& users,Database& db);
    void changeMajor(vector<user*>& users,Database &db);
    void assignCourseForInstructor(vector<user*>& users,Database &db);
    string encryptpass() override;


};

#endif
