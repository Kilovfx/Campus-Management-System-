#include <iostream>
#include <vector>
#include <fstream>
#include <limits>
#include "user.hpp"
#include "database.hpp"
#include "student.hpp"
#include "admin.hpp"
#include "instructor.hpp"
#include "Password.hpp"

using namespace std;

int main() {

    Database db;

    if(!db.connect()){
        cout <<" Database connection failed!\n";
    }
    else{
        cout <<"Database connection successful!\n";
    }
    
    vector<user*> users;  // قائمة لتخزين المستخدمين
    admin* admin1 = new admin("admin", Hashpassword("123"), "admin", 1350);  // إنشاء حساب admin
    users.push_back(admin1);
    
    // Temporary test accounts
    instructor* instructor1 = new instructor("ahmed", Hashpassword("123"), "Instructor", 5001,
                                              "Ahmed", "Example", "Computer Science");
    users.push_back(instructor1);
    
    student* student1 = new student("salem", Hashpassword("123"), "Student", 3001,
                                     "Salem", "Example", "Computer Science");
    users.push_back(student1);

    int choice;
    string input;

    do {
        cout << "\n--- Main Menu ---\n";
        cout << "1. Login as Admin\n";
        cout << "2. Login as Instructor\n";
        cout << "3. Login as Student\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        getline(cin, input);
        cout << "\n";

        if (input.empty() || input.find(' ') != string::npos || input.find('\t') != string::npos) {
            cout << "Invalid input! Please enter a number between 1 and 4.\n";
            continue;
        }

        try {
            size_t index = 0;
            choice = stoi(input, &index);
            if (index != input.size() || choice < 1 || choice > 4) {
                throw invalid_argument("invalid menu choice");
            }
        } catch (const exception&) {
            cout << "Invalid input! Please enter a number between 1 and 4.\n";
            continue;
        }

        string username, password, major;
        switch (choice) {
            case 1: {
                admin1->login(users);  // تسجيل دخول كـ Admin
                admin1->Showmeniu(users);  // عرض خيارات admin
                break;
            }
            case 2: {
                // Create a temporary instructor object for login validation
                instructor instructor1("", "", "Instructor", 56789, "", "", major);
                instructor1.login(users);
                if (instructor1.getusername() != "") {  // Check if login was successful
                    // Find the actual instructor in the users vector and use that
                    for (auto& user : users) {
                        if (user->getRole() == "Instructor" && user->getusername() == instructor1.getusername()) {
                            instructor* actualInstructor = dynamic_cast<instructor*>(user);
                            if (actualInstructor != nullptr) {
                                actualInstructor->showMenu(users);
                            }
                            break;
                        }
                    }
                }
                break;
            }
            case 3: {
                // Create a temporary student object for login validation
                student student1("", "", "Student", 1235, "", "", major);
                student1.login(users);
                if (student1.getusername() != "") {  // Check if login was successful
                    // Find the actual student in the users vector and use that
                    for (auto& user : users) {
                        if (user->getRole() == "Student" && user->getusername() == student1.getusername()) {
                            student* actualStudent = dynamic_cast<student*>(user);
                            if (actualStudent != nullptr) {
                                actualStudent->showMenu(users);
                            }
                            break;
                        }
                    }
                }
                break;
            }
            case 4: {
                cout << "Exiting program...\n";
                break;
            }
            default:
                cout << "Invalid choice, try again.\n";
        }
    } while (choice != 4);

    // Clean up remaining users (the admin pointer might have been deleted)
    for (auto& user : users) {
        // Only delete if it's not already deleted
        if (user != admin1) {  // admin1 might have been deleted already
            delete user;
        }
    }
    if (admin1 != nullptr) {
        delete admin1;
    }

    return 0;
}
