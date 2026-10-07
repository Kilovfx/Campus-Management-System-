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
    int attempts = 0;
    const int maxAttempts = 6;

    while (true) {

        cout << "Enter username: ";
        cin >> inputUsername;

        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            inputUsername.clear();
        }

        cout << "Enter password: ";
        inputPassword = encryptpass();

        user* account = nullptr;

        for (auto* userAccount : users) {
            if (userAccount->getusername() == inputUsername &&
                userAccount->getRole() == "Student") {
                account = userAccount;
                break;
            }
        }

        //check if account manually locked by Admin
        if (account != nullptr && account->isUserLocked(db)) {
            cout << "Account is locked. Login is not available.\n";
            account->addLog(db, "Student attempted to login while account was locked");
            return;
        }


        if (account != nullptr && account->authenticate(inputUsername, inputPassword)) {
            user::recordLoginAttempts(db, inputUsername , true);
            account->resetFailedAttempts(db);
            attempts = 0;
            this->username = inputUsername;
            cout << "User : " << username << " logged in successfully!\n";
            account->addLog(db, "Student logged in");
            return;
        }

        user::recordLoginAttempts(db,inputUsername,false);
        if(account != nullptr){

            bool locked = account->increaseFailedAttempts(db);

                if(locked){
                    cout << "Student account locked after too many failed attempts.\n";
                    account->addLog(db,"Student account locked after too many failed attempts");
                    return;
                }
        }

        attempts++;

        //Unknown username reaches 6 attempts
        if(attempts >= maxAttempts) {
            cout <<"Login failed after 6 attempts.\n";
            return;
        }

        //Brute-force protiection 
        if(attempts >= 3){
            int waitTime = 15 * (attempts - 2);
            cout << "You have made "<< attempts<< " incorrect attempts, Please wait for " << waitTime << " seconds .. \n";
            this_thread::sleep_for(chrono::seconds(waitTime));
            cout << "You can try now.\n";

        }

        cout << "Invalid credentials. Please try again.\n";
    }
}


void student::logout(Database& db) {
    cout << username << " logged out.\n";
    addLog(db,"Student logged out.");
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

void student::ShowAcademicSummary(Database &db){
    int totalCourses = static_cast<int>(enrolledCourses.size());
    int completed = 0;
    int notGraded = 0;
    int totalCredits = 0;
    double totalPoints = 0;

    for(const auto& course : enrolledCourses){
        totalCredits += course.creditHours;

        if(course.grade == -1){
            notGraded++;
        }
        else {
            completed++;
            totalPoints = totalPoints + ConvertGradeToGPA(course.grade) * course.creditHours;
        }
    }

    double GPA = 0;

    if(totalCredits > 0){
        GPA = totalPoints / totalCredits;
    }
    cout << "\n-----------------------------\n";
    cout << "Academic Summary\n";
    cout << "-----------------------------\n";
    cout << "Student: " << first_name <<" "<< last_name << endl;
    cout << "Major: " << major << endl;
    cout << "Total Courses: " << totalCourses << endl;
    cout << "Completed: " << completed << endl;
    cout << "Not Graded: " << notGraded << endl;
    cout << "Total Credits: " << totalCredits << endl;
    cout << "GPA: " << fixed << setprecision(2) << GPA << " / 5.00" << endl;
    cout << "-----------------------------\n";

    addLog(db, "Viewed academic summary");
}

void student::viewCourses(Database& db) {
    cout << "Courses for " << username << ":\n";
    for (const auto& course : enrolledCourses) {
        cout << course.courseName << " (" << course.creditHours << " credits)" << endl;
    }
    addLog(db,"Viewed courses");
}

void student::ShowGrades(Database &db){

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
         addLog(db, "Viewed His Grades");
     }
}


void student::showMenu(std::vector<user*>& users,Database& db) {
    int choice;
    do {
        cout << "\nStudent Menu:\n";
        cout << "1. View Courses\n";
        cout << "2. Show Grades\n";
        cout << "3. Show Profile\n";
        cout << "4. Academic Summary\n";
        cout << "5. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cout << "\n";
        
        if(cin.fail() || choice < 1 || choice > 5 || hasExtraInputOnLine()) {
            cin.clear();  // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
            cout << "Invalid input! Please enter a number between 1 and 5.\n";
            continue;  // ask for input again
        }

        switch (choice) {
            case 1:
                viewCourses(db);
                break;
            case 2:
                ShowGrades(db);
                break;
            case 3:
                showprofile(db);
                break;
            case 4:
                ShowAcademicSummary(db);
                break;
            case 5:
                logout(db);
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 5);
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
