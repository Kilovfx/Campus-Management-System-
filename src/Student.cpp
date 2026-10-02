#include "student.hpp"
#include "Password.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <thread>
#include <chrono>
#include <iomanip>  // For std::setprecision
#ifdef _WIN32
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

using namespace std;

student::student(string username, string password, string role, int ID,
                 string first_name, string last_name, string major)
    : user(username, password, role, ID, first_name, last_name, major, false) {
    // Additional initialization for student-specific properties can be done here if needed
}

void student::loadCourse(string courseName, double grade, double creditHours)
{
    enrolledCourses.push_back(
        CourseGrade(courseName, grade, creditHours)
    );
}


void student::login(vector<user*>& users,Database &db) {
    string inputUsername, inputPassword;
    user* account = nullptr;
    int unknownAccountAttempts = 0;

    while (true) {
        cout << "Enter username: ";
        cin >> inputUsername;
        account = nullptr;
        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            inputUsername.clear();
        }

        for (auto* userAccount : users) {
            if (userAccount->getusername() == inputUsername &&
                userAccount->getRole() == "Student") {
                account = userAccount;
                break;
            }
        }

        if (account != nullptr && !account->isUserLocked(db)) {
            cout << "Account is locked. Login is not available.\n";
            return;
        }

        cout << "Enter password: ";
        inputPassword = encryptpass();

        if (account != nullptr && authenticate(inputUsername, inputPassword, users)) {
            break;
        }

        if (account != nullptr) {
            account->increaseFailedAttempts(db);
            if (!account->isUserLocked(db)) {
                cout << "Account locked after 6 incorrect attempts try again 1 min later.\n";
                account->addLog(db, "Student account locked after too many failed login attempts");
                return;
            }
        } else {
            ++unknownAccountAttempts;
            if (unknownAccountAttempts >= 3) {
                cout << "Login attempts exceeded.\n";
                return;
            }
        }

        cout << "Invalid credentials. Please try again.\n";
    }

    if (account != nullptr) {
        this->username = inputUsername;
        cout << username << " logged in successfully.\n";
        addLog(db, "Student logged in");
    } else {
        cout << "Invalid credentials. Login failed.\n";
    }
}




bool student::authenticate(const string& username, const string& password, vector<user*>& users) {
    for (const auto& u : users) {
        if (u->getusername() == username && verifypassword(password,u->getpassword()) && u->getRole() == "Student") {
            return true;
        }
    }
    return false;
}

void student::logout(Database& db) {
    cout << username << " logged out.\n";
}

void student::showprofile(Database& db) {
    cout << "Student Profile\n";
    cout << "----------------\n";
    cout << "Name: " << first_name << " " << last_name << endl;
    cout << "Username: " << username << endl;
    cout << "ID: " << ID << endl;
    cout << "Major: " << major << endl;
    // Date of account creation
    char* dt = ctime(&creationDate);
    cout << "Account created on: " << dt << endl;
    addLog(db,"Viewed profile");
}

void student::addCourse(string courseName, int creditHours) {
    if (hasCourse(courseName)) {
        cout << "Student " << username << " is already enrolled in: " << courseName << endl;
        return;
    }
    enrolledCourses.push_back(CourseGrade(courseName, -1, creditHours));
    cout << "Student " << username << " added to course: " << courseName << endl;
}

bool student::hasCourse(const string& courseName) const {
    for (const auto& course : enrolledCourses) {
        if (course.courseName == courseName) {
            return true;
        }
    }
    return false;
}

void student::removeCourse(string courseName,Database &db) {
    auto it = remove_if(enrolledCourses.begin(), enrolledCourses.end(),
        [&](const CourseGrade& course) { return course.courseName == courseName; });
    if (it != enrolledCourses.end()) {
        enrolledCourses.erase(it, enrolledCourses.end());
    }
}

const vector<CourseGrade>& student::getEnrolledCourses() const {
    return enrolledCourses;
}

std::vector<CourseGrade>& student::getEnrolledCourses() {
    return enrolledCourses;
}

void student::viewCourses(Database& db) {
    cout << "Courses for " << username << ":\n";
    for (const auto& course : enrolledCourses) {
        cout << course.courseName << " (" << course.creditHours << " credits)" << endl;
    }
    addLog(db,"Viewed courses");
}

void student::ShowGrades(){

     cout << "\n-----------------------------\n";
     cout << "Grades for: " << username << endl;
     cout << "ID: " << ID << endl;
     cout << "Major: " << major << endl;
     cout << "-----------------------------\n";

     if(enrolledCourses.empty()){
        cout << "No courses enrolled yet.\n";
        return;
     }

     double totalPoints = 0;
     int totalCredit = 0;

     for (auto &course : enrolledCourses) {
         cout << "Course: [ " << course.courseName << " | " << course.creditHours << " credits ]" << ", Grade: ";
         if (course.grade == -1) {
             cout << "Not graded yet";
         } else {
             cout << course.grade;
             totalPoints += ConvertGradeToGPA(course.grade) * course.creditHours;
             totalCredit += course.creditHours;
         }
            cout << endl;
     }
     if (totalCredit > 0) {
         double gpa = totalPoints / totalCredit;
         cout << "-----------------------------\n";
         cout << "Total GPA: " << fixed << setprecision(2) << gpa << endl;
     }
}


void student::showMenu(std::vector<user*>& users,Database& db) {
    int choice;
    do {
        cout << "\nStudent Menu:\n";
        cout << "1. View Courses\n";
        cout << "2. Show Grades\n";
        cout << "3. Show Profile\n";
        cout << "4. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cout << "\n";
        
        if(cin.fail() || choice < 1 || choice > 4 || hasExtraInputOnLine()) {
            cin.clear();  // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
            cout << "Invalid input! Please enter a number between 1 and 4.\n";
            continue;  // ask for input again
        }

        switch (choice) {
            case 1:
                viewCourses(db);
                break;
            case 2:
                ShowGrades();
                break;
            case 3:
                showprofile(db);
                break;
            case 4:
                logout(db);
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 4);
}

string student::encryptpass() {
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



double student::ConvertGradeToGPA(double grade) {
    if (grade >= 95)
        return 5.0;
    else if (grade >= 90)
        return 4.5;
    else if (grade >= 85)
        return 4.0;
    else if (grade >= 80)
        return 3.5;
    else if (grade >= 75)
        return 3.0;
    else if (grade >= 70)
        return 2.5;
    else if (grade >= 65)
        return 2.0;
    else if (grade >= 60)
        return 1.5;
    else
        return 0.0;
}
