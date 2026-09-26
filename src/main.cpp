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
#include "UserLoader.hpp"

using namespace std;

int main() {

    cout << "Program started\n";

    Database db;

    if (db.connect()) {
        cout << "Database connection successful!\n";
    } else {
        cout << "Database connection failed!\n";
        cout << "Please start MySQL and make sure the database is available before running the program.\n";
        return 0;
    }

    vector<user*> users;

    users = loadUsers(db);
    cout << "Loaded " << users.size() << " user account(s) from SQL.\n";
    if (users.empty()) {
        cout << "No valid user accounts were found in the users table.\n";
    }
    for (auto* loadedUser : users) {
        cout << "Loaded username: " << loadedUser->getusername()
             << " | role: " << loadedUser->getRole() << "\n";
    }
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
                admin admin1("", "", "admin", 0);
                admin1.login(users,db);
                if (admin1.getusername() != "") {
                    for (auto* user : users) {
                        if (user->getRole() == "admin" && user->getusername() == admin1.getusername()) {
                            admin* actualAdmin = dynamic_cast<admin*>(user);
                            if (actualAdmin != nullptr) {
                                actualAdmin->Showmeniu(users,db);
                            }
                            break;
                        }
                    }
                }
                break;
            }
            case 2: {
                instructor instructor1("", "", "Instructor", 56789, "", "", major);
                instructor1.login(users,db);
                if (instructor1.getusername() != "") {
                    for (auto* user : users) {
                        if (user->getRole() == "Instructor" && user->getusername() == instructor1.getusername()) {
                            instructor* actualInstructor = dynamic_cast<instructor*>(user);
                            if (actualInstructor != nullptr) {
                                actualInstructor->showMenu(users,db);
                            }
                            break;
                        }
                    }
                }
                break;
            }
            case 3: {
                student student1("", "", "Student", 1235, "", "", major);
                student1.login(users,db);
                if (student1.getusername() != "") {
                    for (auto* user : users) {
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

    for (auto* user : users) {
        delete user;
    }

    return 0;
}
