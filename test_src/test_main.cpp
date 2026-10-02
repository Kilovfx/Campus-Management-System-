#include <iostream>
#include <vector>
#include <cassert>
#include "user.hpp"
#include "admin.hpp"
#include "database.hpp"
#include "UserLoader.hpp"
#include "Password.hpp"
#include "mysql.h"

using namespace std;

int main()
{
    Database db;
    vector<user*> users;

    if(!db.connect()){
        cout<<"Failed connection . . .\n";
        return 0;
    }

    cout<<"Database connected successfully\n";

    users = loadUsers(db);

    cout << "Loaded " << users.size() << " users.\n";

    for(auto* loadedUser : users){

        cout << "Username: "
             << loadedUser->getusername()
             << " | Role: "
             << loadedUser->getRole()
             << " | ID: "
             << loadedUser->getID()
             << "\n";
    }

       admin admin1("admin", "456", "admin", 0);

       cout << "\n--- Test View All Logs ---\n";
       admin1.ViewAllLog(db);

       cout << "\n--- Test View Log By ID  ---\n";
       bool logFound = admin1.ViewLogByID(db,2);
       assert(logFound == true);

       bool logNotFound = admin1.ViewLogByID(db,99999);
       assert(logNotFound == false);

       cout << "\n--- Test View Logs By User ID ---\n";

       bool userLogsFound = admin1.ViewLogsByUserID(db, 1350);
       assert(userLogsFound == true);
    
       bool noUserLogs = admin1.ViewLogsByUserID(db, 999999);
       assert(noUserLogs == false);

       cout << "\nAll tests passed successfully!\n";

       db.disconnect();

          return 0;
}








string testpasswordhash(string password){

    string hash = Hashpassword("123");
    cout << hash <<endl;
    return hash;

}


string testasignrole(vector<user*>& users,Database &db,string username,string newRole){
           admin admin1("admin", "456", "admin", 0);

        cout << "Before Test 1\n";

        cout << "\n===== Test 1: User does not exist =====\n";
        admin1.asignrole(users, db, "student1", "Instructor");

        cout << "After Test 1\n";


        cout << "\n===== Test 2: Admin role change =====\n";
        admin1.asignrole(users, db, "admin", "Student");

        cout << "After Test 2\n";


        cout << "\n===== Test 3: Same role =====\n";
        admin1.asignrole(users, db, "salem", "Student");

        cout << "After Test 3\n";


        cout << "\n===== Test 4: Student -> Instructor =====\n";
        admin1.asignrole(users, db, "saed", "Instructor");

        cout << "After Test 4\n";

        // reload from database
        for(auto* user : users){
            delete user;
        }

        users.clear();

        users = loadUsers(db);

         cout << "\n===== Users after tests =====\n";;
         admin1.ListAll(users,db);


        for(auto* user : users){
            delete user;
        }

        users.clear();

        return "";
}

string testchangemajor(vector<user*>& users,Database &db){
    // starting login to admin
        admin admin1("admin", "456", "admin", 0);
        int x = 1;
        while(x <= 4){
        
        cout << "\n===== Test (" << x << ") =====\n";
        admin1.changeMajor(users,db);

        cout << "After Test "<< x <<endl;


        //clear memory
        for(auto* user : users){
            delete user;
        }

        users.clear();

        //reload newest from database
        users = loadUsers(db);

         cout << "\n===== Users after tests =====\n";;
         admin1.ListAll(users,db);

        x++;

    }

        for(auto* user : users){
            delete user;
        }

        users.clear();

        return "";
}


void testAssignCourseForInstructor(vector<user*>& users, Database& db)
{
    admin admin1("admin", "456", "admin", 0);

    int testNumber = 1;

    while(testNumber <= 4)
    {
        cout << "\n===== Assign Course Test "
             << testNumber << " =====\n";

        admin1.assignCourseForInstructor(users, db);

        cout << "After Test " << testNumber << "\n";

        // Clear current users from memory
        for(auto* user : users)
        {
            delete user;
        }

        users.clear();

        // Reload users from database
        users = loadUsers(db);

        cout << "\n===== Users after test =====\n";
        admin1.ListAll(users,db);

        testNumber++;
    }

    // Final cleanup
    for(auto* user : users)
    {
        delete user;
    }

    users.clear();
}
