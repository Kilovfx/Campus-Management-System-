#include <iostream>
#include <vector>
#include <cassert>
#include "user.hpp"
#include "admin.hpp"
#include "database.hpp"
#include "UserLoader.hpp"
#include "Password.hpp"
#include "mysql.h"
#include "instructor.hpp"
#include "Course.hpp"
#include "Student.hpp"

using namespace std;

int main()
{
    Database db;
    vector<user*> users;

    if(!db.connect()){
        cout<<"Failed connection . . .\n";
        return 1;
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


    student student1(
        "ci_student",
        "456",
        "Student",
        3001,
        "CI",
        "Student",
        "Marketing"
);

    student1.addCourse("C++", 3);
    student1.addCourse("Database", 3);
    student1.addCourse("Networking", 3);

    student1.getEnrolledCourses()[0].grade = 95;
    student1.getEnrolledCourses()[1].grade = 85;

    student1.ShowAcademicSummary(db);


    //    admin admin1("admin", "456", "admin", 0);

    // // ==========================================
    // // Find an instructor
    // // ==========================================
       
    //     instructor* testInstructor = nullptr;

    //     for(auto* loadedUser : users)
    //     {
    //         if(loadedUser->getRole() == "Instructor")
    //         {
    //             instructor* currentInstructor =
    //                 dynamic_cast<instructor*>(loadedUser);

    //             if(currentInstructor != nullptr &&
    //             currentInstructor->getusername() == "ci_ahmed")
    //             {
    //                 testInstructor = currentInstructor;
    //                 break;
    //             }
    //         }
    //     }

    //    assert(testInstructor != nullptr);

    // // ==========================================
    // // Get courses
    // // ==========================================

    // vector<CourseInfo> courses = admin1.getCoursesForMajor(db,testInstructor->getMajor());
    //    assert(courses.size() >= 2);

    //    string firstCourse = courses[0].courseName;
    //    string secondCourse = courses[1].courseName;

    //    cout << "\nTest Instructor: "<< testInstructor->getusername() << endl;

    //     cout << "First Course: " << firstCourse << endl;

    //     cout << "Second Course: " << secondCourse << endl;

    //     // ==========================================
    //     // TEST 1
    //     // assignCourseDatabase()
    //     // ==========================================

    //     cout << "\n--- Test assignCourseDatabase ---\n";

    //     bool assignResult = admin1.assignCourseDatabase(db,testInstructor,firstCourse);

    //     assert(assignResult == true);

    //     cout << "assignCourseDatabase() passed.\n";

    //     // ==========================================
    //     // TEST 2
    //     // changeCourseDatabase()
    //     // ==========================================

    //     cout << "\n--- Test changeCourseDatabase ---\n";


    //     bool changeResult = admin1.changeCourseDatabase(db,testInstructor,firstCourse,secondCourse);

    //     assert(changeResult = true);

    //      cout << "changeCourseDatabase() passed.\n";

    //     // ==========================================
    //     // Finished
    //     // ==========================================

    //     cout << "\nAll tests passed successfully!\n";

    //     for(auto* userAccount : users)
    //     {
    //         delete userAccount;
    //     }
    
      users.clear();

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


