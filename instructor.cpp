#include "instructor.hpp"
#include "student.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <map>
#include <limits>
#ifdef _WIN32
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

instructor::instructor(string username, string password, string role, int ID, string major):user(username, password, role, ID, major,false) {}

void instructor::login(vector<user*>& users) {
    string inputUsername, inputPassword;
    int unknownAccountAttempts = 0;
    user* account = nullptr;

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
                userAccount->getRole() == "Instructor") {
                account = userAccount;
                break;
            }
        }

        if (account != nullptr && !account->isActive()) {
            cout << "Account is locked. Login is not available.\n";
            return;
        }

        cout << "Enter password: ";
        inputPassword = getPassword();

        if (account != nullptr && authenticate(inputUsername, inputPassword, users)) {
            break;
        }

        if (account != nullptr) {
            account->increasedFailedAttempts();
            if (!account->isActive()) {
                cout << "Account locked after 6 incorrect attempts try again 1 min later.\n";
                account->addLog("Instructor account locked after too many failed login attempts");
                return;
            }
        } else {
            ++unknownAccountAttempts;
            if (unknownAccountAttempts >= 3) {
                cout << "Login attempts exceeded\n";
                return;
            }
        }

        cout << "Invalid credentials. Please try again.\n";
    }

    if (account != nullptr) {
        this->username = inputUsername;
        this->password = inputPassword;
        cout << username << " logged in successfully as an Instructor.\n";
        addLog("Instructor logged in");
    } else {
        cout << "Invalid credentials. Login failed.\n";
    }
}

bool instructor::authenticate(const string& username, const string& password, vector<user*>& users) {
    for (const auto& user : users) {
        if (user->getusername() == username && user->getpassword() == password && user->getRole() == "Instructor") {
            return true;
        }
    }
    return false;
}

void instructor::logout() {
    cout << username << " logged out.\n";
}

void instructor::showprofile() {
    cout << "Instructor Profile\n";
    cout << "Username: " << username << endl;
    cout << "Role: " << role << endl;
    cout << "ID: " << ID << endl;
    // Date of account creation
    char* dt = ctime(&creationDate);
    cout << "Account created on: " << dt << endl;
}



static const map<string, vector<string>>& getCourseCatalog() {
    static const map<string, vector<string>> courseCatalog = {
        {"Computer Science", {"C++", "Data Structures", "Databases", "Operating Systems", "Networks"}},
        {"Electrical Engineering", {"Circuit Analysis", "Electromagnetics", "Digital Systems", "Control Systems"}},
        {"Business Administration", {"Accounting", "Finance", "Organizational Behavior", "Business Ethics"}},
        {"Mechanical Engineering", {"Thermodynamics", "Fluid Mechanics", "Dynamics", "Materials Science"}},
        {"Civil Engineering", {"Statics", "Structural Analysis", "Geotechnical Engineering", "Hydraulics"}},
        {"Pharmacy", {"Pharmacology", "Pharmaceutics", "Medicinal Chemistry", "Clinical Pharmacy"}},
        {"Marketing", {"Principles of Marketing", "Consumer Behavior", "Digital Marketing", "Brand Management"}}
    };

    return courseCatalog;
}

void instructor::listAllMajors() {
    const auto& courseCatalog = getCourseCatalog();
    cout << "\n--- Available Majors ---\n";
    for (const auto& entry : courseCatalog) {
        cout << "- " << entry.first << "\n";
    }
}



void instructor::viewCoursesForMajor(const string& major) {
    const auto& courseCatalog = getCourseCatalog();
    vector<string> majors;

    cout << "\n--- Available Majors ---\n";
    for (const auto& entry : courseCatalog) {
        majors.push_back(entry.first);
        cout << majors.size() << ". " << entry.first << "\n";
    }

    int choice;
    cout << "Select a major (1-" << majors.size() << ") or 0 to cancel: ";
    cin >> choice;
    while (cin.fail() || choice < 0 ||
           choice > static_cast<int>(majors.size()) ||
           hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid choice. Select a major (1-" << majors.size()
             << ") or 0 to cancel: ";
        cin >> choice;
    }

    if (choice == 0) {
        cout << "Operation cancelled.\n";
        return;
    }

    const string& selectedMajor = majors[choice - 1];
    const auto entry = courseCatalog.find(selectedMajor);
    if (entry == courseCatalog.end()) {
        cout << "No courses found for the major: " << selectedMajor << "\n";
        return;
    }

    cout << "\n--- Courses for Major: " << selectedMajor << " ---\n";
    for (const auto& course : entry->second) {
        cout << "- " << course << "\n";
    }
}


void instructor::viewStudentCourses(vector<user*>& users) {
    vector<student*> students;
    cout << "\n--- Students List ---\n";
    for (const auto& user : users) {
        if (user->getRole() == "Student") {
            student* currentStudent = dynamic_cast<student*>(user);
            if (currentStudent != nullptr) {
                students.push_back(currentStudent);
                cout << students.size() << ". " << currentStudent->getusername()
                     << " (ID: " << currentStudent->getID()
                     << ", Major: " << currentStudent->getMajor() << ")\n";
            }
        }
    }

    if (students.empty()) {
        cout << "No students found in the system.\n";
        return;
    }

    int choice;
    cout << "Select a student (1-" << students.size() << ") or 0 to cancel: ";
    cin >> choice;
    while (cin.fail() || choice < 0 ||
           choice > static_cast<int>(students.size()) ||
           hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid choice. Select a student (1-" << students.size()
             << ") or 0 to cancel: ";
        cin >> choice;
    }

    if (choice == 0) {
        cout << "Operation cancelled.\n";
        return;
    }

    student* selectedStudent = students[choice - 1];
    const auto& enrolledCourses = selectedStudent->getEnrolledCourses();
    cout << "\n--- Courses for Student: " << selectedStudent->getusername() << " ---\n";
    if (enrolledCourses.empty()) {
        cout << "No courses enrolled.\n";
    } else {
        for (const auto& course : enrolledCourses) {
            cout << "- " << course << "\n";
        }
    }
}

void instructor::addCourse(vector<user*>& users) {
    // Display all students
    cout << "\n--- Students List ---\n";
    vector<student*> studentList;
    int studentCount = 0;
    
    for (const auto& user : users) {
        if (user->getRole() == "Student") {
            studentCount++;
            student* std = dynamic_cast<student*>(user);
            if (std != nullptr) {
                studentList.push_back(std);
                cout << studentCount << ". " << user->getusername() << " (ID: " << user->getID() << ", Major: " << user->getMajor() << ")\n";
            }
        }
    }
    
    // Check if there are any students
    if (studentList.empty()) {
        cout << "No students found in the system.\n";
        return;
    }
    
    // Let instructor choose a student
    int choice;
    cout << "\nSelect a student to add the course to (1-" << studentCount << "): ";
    cin >> choice;
    
    // Validate input
    while (cin.fail() || choice < 1 || choice > studentCount || hasExtraInputOnLine())
    
    {
        cin.clear();  // clear the error flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
        cout << "Invalid input! Please enter a number between 1 and " << studentCount << ".\n";
        cout << "Select a student to add the course to (1-" << studentCount << "): ";
        cin >> choice;
        continue;  // ask for input again
    }
    
    
    // Add course to the selected student (restricted by major)
    student* selectedStudent = studentList[choice - 1];
    const string studentMajor = selectedStudent->getMajor();
    const auto& courseCatalog = getCourseCatalog();
    auto catalogIt = courseCatalog.find(studentMajor);

    // Validate if major exists in catalog
    if (catalogIt == courseCatalog.end()) {
        cout << "No course catalog found for major: " << studentMajor << "\n";
        return;
    }

    while (true) {
        // Display courses for the student's major
        cout << "\nCourses for " << studentMajor << ":\n";
        cout << "\n";
        for (size_t i = 0; i < catalogIt->second.size(); ++i) {
            cout << (i + 1) << ". " << catalogIt->second[i] << "\n";
        }

        // Prompt for course selection
        int courseChoice;
        cout << "\nSelect a course to add (1-" << catalogIt->second.size() << ") or 0 to finish: ";
        cin >> courseChoice;
        cout<<endl;
        // Validate course choice input
        while (cin.fail() || courseChoice < 0 || courseChoice > static_cast<int>(catalogIt->second.size()) || hasExtraInputOnLine()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number between 0 and " << catalogIt->second.size() << ": ";
            cin >> courseChoice;
        }

        // Check for finish
        if (courseChoice == 0) {
            cout << "Finished adding courses.\n";
            return;
        }

        // Get the selected course name
        string courseName = catalogIt->second[courseChoice - 1];
        if (selectedStudent->hasCourse(courseName)) {
            cout << "Student " << selectedStudent->getusername() << " already has the course: " << courseName << "\n";
            continue;
        }
        selectedStudent->addCourse(courseName);

        // Add course to instructor's course list if not already there (avoid duplicates)
        auto courseIt = find(courses.begin(), courses.end(), courseName);
        if (courseIt == courses.end()) {
            courses.push_back(courseName);
        }

        cout << "Course " << courseName << " added to student " << selectedStudent->getusername() << " successfully.\n";
    }
}



void instructor::removeCourse(vector<user*>& users) {
    cout << "\n--- Students List ---\n";
    vector<student*> studentList;
    int studentCount = 0;

    for (const auto& user : users) {
        if (user->getRole() == "Student") {
            student* std = dynamic_cast<student*>(user);
            if (std != nullptr) {
                studentList.push_back(std);
                studentCount++;
                cout << studentCount << ". " << std->getusername() << " (ID: " << std->getID() << ", Major: " << std->getMajor() << ")\n";
            }
        }
    }

    if (studentList.empty()) {
        cout << "No students found in the system.\n";
        return;
    }

    int choice;
    cout << "\nSelect a student to remove a course from (1-" << studentCount << "): ";
    cin >> choice;

    while (cin.fail() || choice < 1 || choice > studentCount || hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 1 and " << studentCount << ": ";
        cin >> choice;
    }

    student* selectedStudent = studentList[choice - 1];

    const auto& enrolledCourses = selectedStudent->getEnrolledCourses();
    if (enrolledCourses.empty()) {
        cout << "Student " << selectedStudent->getusername() << " has no courses to remove.\n";
        return;
    }

    cout << "\nCourses for " << selectedStudent->getusername() << ":\n";
    for (size_t i = 0; i < enrolledCourses.size(); ++i) {
        cout << (i + 1) << ". " << enrolledCourses[i] << "\n";
    }

    int courseChoice;
    cout << "Select a course to remove (1-" << enrolledCourses.size() << ") or 0 to cancel: ";
    cin >> courseChoice;

    while (cin.fail() || courseChoice < 0 || courseChoice > static_cast<int>(enrolledCourses.size()) || hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 0 and " << enrolledCourses.size() << ": ";
        cin >> courseChoice;
    }

    if (courseChoice == 0) {
        cout << "Operation cancelled.\n";
        return;
    }

    string courseName = enrolledCourses[courseChoice - 1];
    selectedStudent->removeCourse(courseName);

    bool courseStillAssigned = false;
    for (const auto& user : users) {
        if (user->getRole() == "Student") {
            student* std = dynamic_cast<student*>(user);
            if (std != nullptr && std->hasCourse(courseName)) {
                courseStillAssigned = true;
                break;
            }
        }
    }

    if (!courseStillAssigned) {
        auto it = find(courses.begin(), courses.end(), courseName);
        if (it != courses.end()) {
            courses.erase(it);
        }
    }

    cout << "Course " << courseName << " removed successfully from " << selectedStudent->getusername() << ".\n";
}

void instructor::viewCourses(vector<user*>& users) {
    const auto& courseCatalog = getCourseCatalog();
    if (courseCatalog.empty()) {
        cout << "No course catalog available.\n";
        return;
    }

    vector<string> majors;
    for (const auto& entry : courseCatalog) {
        majors.push_back(entry.first);
    }

    cout << "\nAvailable majors:\n";
    for (size_t i = 0; i < majors.size(); ++i) {
        cout << (i + 1) << ". " << majors[i] << "\n";
    }

    int majorChoice;
    cout << "Select a major to view courses (1-" << majors.size() << ") or 0 to cancel: ";
    cin >> majorChoice;

    while (cin.fail() || majorChoice < 0 || majorChoice > static_cast<int>(majors.size()) || hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 0 and " << majors.size() << ": ";
        cin >> majorChoice;
    }

    if (majorChoice == 0) {
        cout << "Operation cancelled.\n";
        return;
    }

    const string& selectedMajor = majors[majorChoice - 1];
    auto catalogIt = courseCatalog.find(selectedMajor);

    cout << "\nAvailable courses for " << selectedMajor << ":\n";
    for (const auto& course : catalogIt->second) {
        cout << "- " << course << "\n";
    }

    vector<string> taughtForMajor;
    for (const auto& course : courses) {
        if (find(catalogIt->second.begin(), catalogIt->second.end(), course) != catalogIt->second.end()) {
            taughtForMajor.push_back(course);
        }
    }

    cout << "\nCourses assigned to students for " << selectedMajor << ":\n";
    if (taughtForMajor.empty()) {
        cout << "None\n";
    } else {
        for (const auto& course : taughtForMajor) {
            int studentCount = 0;
            for (const auto& user : users) {
                if (user->getRole() == "Student") {
                    student* enrolledStudent = dynamic_cast<student*>(user);
                    if (enrolledStudent != nullptr && enrolledStudent->hasCourse(course)) {
                        ++studentCount;
                    }
                }
            }
            cout << "- " << course << " (" << studentCount << " "
                 << (studentCount == 1 ? "Student" : "Students") << ")\n";
        }
    }
}

void instructor::viewStudents(vector<user*>& users) {
    cout << "Students enrolled in the courses being taught by " << username << ":\n";
    for (const auto& user : users) {
        if (user->getRole() == "Student") {
            cout << "Name: " << user->getusername() << " | Major: " << user->getMajor() << " | ID: " << user->getID() << endl;
        }
    }
}

void instructor::showMenu(vector<user*>& users) {
    int choice;
    do {
        cout << "\nInstructor Menu:\n";
        cout << "1. Add Course\n";
        cout << "2. Remove Course\n";
        cout << "3. View Courses\n";
        cout << "4. View Courses for Major\n";
        cout << "5. View Student Courses\n";
        cout << "6. View Students\n";
        cout << "7. List All Majors\n";
        cout << "8. Show Profile\n";
        cout << "9. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;
        
        if(cin.fail() || choice < 1 || choice > 9 || hasExtraInputOnLine()) {
            cin.clear();  // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
            cout << "Invalid input! Please enter a number between 1 and 9.\n";
            continue;  // ask for input again
        }

        switch (choice) {
            case 1: {
                addCourse(users);
                break;
            }
            case 2: {
                removeCourse(users);
                break;
            }
            case 3:
                viewCourses(users);
                break;
            case 4:
                viewCoursesForMajor(users[0]->getMajor());
                break; 
            case 5:
                viewStudentCourses(users);
                break;   
            case 6:
                viewStudents(users);
                break;
            case 7:
                listAllMajors();
                break;
            case 8:
                showprofile();
                break;
            case 9:
                logout();
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 9);
}

string instructor::getPassword() {
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
