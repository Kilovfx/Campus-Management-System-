#ifndef ADMIN_HPP
#define ADMIN_HPP

#include "user.hpp"
#include "database.hpp"
#include "Course.hpp"
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

    // loggin & loggout
    void login(vector<user*>& users,Database &db) override; 
    void logout() override;


    //lock user & unlock user
    void lockUser(vector<user*>& users,Database &db);
    void unlockUser(vector<user*>& users,Database &db);
    bool lockUserDatabase(Database &db,string username, int minutes);
    bool canLockUser(Database& db, string username);
    bool unlockuserDatabase(Database& db, string username);

    //show
    void showprofile() override;
    void Showmeniu(vector<user*>& users,Database& db);

    //veiw functions
    void ViewAllLog(Database& db);
    void ListAll(vector<user*>& users);
    
    // create user functions
    void createuser(vector<user*>& users,Database& db, string username, string password, string role,string first_name, string last_name, string major);
    bool usernameexist(Database &db,string username);
    int insertUser(Database &db,string username, string hashedpassword, string role, int majorID);
    bool createaccountsecurity(Database &db,int userID);
    bool insertStudent(Database &db,int userID,string first_name,string last_name,int majorID);
    bool insertInstructor(Database &db,int userID,string first_name,string last_name,int majorID);

    void deleteuser(vector<user*>& users,Database& db,string username,string password);
    bool deleteUserDatabase(Database& db, int userID, string role);

    void asignrole(vector<user*>& users,Database &db,string username,string newRole);
    bool changeUserRoleDatabase(Database &db,user* selectedUser,string newRole);
    

    void changepassword(vector<user*>& users,Database& db);
    string encryptpass() override;

    //change major
    void changeMajor(vector<user*>& users,Database &db);
    int  getMajorID(Database& db,string majorName);   
    bool changeMajorDatabase(Database &db,user* selectedUser,string newMajor);

    void assignCourseForInstructor(vector<user*>& users,Database &db);
    bool assignCourseDatabase(Database &db,user* selectedInstructor,string courseName);
    vector<CourseInfo> getCoursesForMajor(Database& db, string major);


};

#endif
