#include "student.hpp"
#include "instructor.hpp"
#include "Password.hpp"
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

instructor::instructor(string username, string password, string role, int ID,
                       string first_name, string last_name, string major)
    : user(username, password, role, ID, first_name, last_name, major, false) {}

const vector <CourseInfo>& instructor::getCourses() const {
    return courses;
};


void instructor::login(vector<user*>& users,Database &db) {
    string inputUsername, inputPassword;
    int attempts = 0;
    const int maxAttempt = 6;

    while (true) {

        cout << "Enter username: ";
        cin >> inputUsername;

        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            inputUsername.clear();
        }

        cout <<"Enter password: ";
        inputPassword = encryptpass();

        //search for instructor

        user* account = nullptr;

        for(auto* userAccount : users){
            if(userAccount->getusername() == inputUsername && userAccount->getRole() == "Instructor"){
                account = userAccount;
                break;
            }
        }

        //check if its manually locked by Admin
        if(account != nullptr && account->isUserLocked(db)){
            cout <<"Account is locked. Login is not available.\n";
            account->addLog(db, "Instructor attempted to login while account was locked");
            return;
        }

        //successful login

        if(account != nullptr && account->authenticate(inputUsername,inputPassword)){
            user::recordLoginAttempts(db, inputUsername,true);
            account->resetFailedAttempts(db);

            attempts = 0;
            this->username = inputUsername;

            cout << "User : " << username << " logged in successfully!\n";
            addLog(db,"Instructor logged in");
            return;
        }

        //record every failed attempts
        user::recordLoginAttempts(db,inputUsername,false);

        if (account != nullptr){
            bool locked = account->increaseFailedAttempts(db);

            if(locked) {
                cout << "Instructor account locked after too many failed attempts.\n";
                account->addLog(db,"Instructor account locked after too many failed attempts");
                return;
            }
        }

        attempts++;

        //unknown username reaches 6 attempts
        if(attempts >= maxAttempt){
            cout << "Login failed after 6 attempts.\n";
            return;
        }

        //brute force protection 
        if(attempts >= 3){

            int waittime = 15 * (attempts - 2);
            cout << "You have made " << attempts << " incorrect attempts, Please wait for " << waittime << " seconds .. \n";
            this_thread::sleep_for(chrono::seconds(waittime));

            cout << "You can try now.\n";
        }

        cout << "Invalid credentials. Please try again.\n";
    }
}

void instructor::logout(Database& db) {
    cout << username << " logged out.\n";
    addLog(db, "Instructor logged out");
}

void instructor::showprofile(Database& db) {
    cout << "Instructor Profile\n";
    cout << "----------------\n";
    cout << "Name: " << first_name << " " << last_name << endl;
    cout << "Username: " << username << endl;
    cout << "ID: " << ID << endl;
    cout << "Major: " << major << endl;
    // Date of account creation
    char* dt = ctime(&creationDate);
    cout << "Account created on: " << dt << endl;
}


void instructor::viewMyCourses(Database &db){

        string query =
        "SELECT c.course_name, c.credit_hours "
        "FROM courses c "
        "INNER JOIN instructor_courses ic "
        "ON ic.course_id = c.course_id "
        "WHERE ic.instructor_id = " + to_string(ID);


    MYSQL_RES* result = db.executeSelect(query);

    if(result == nullptr){
        cout <<"No courses found\n";
        return;
    }

    MYSQL_ROW row;

    cout <<"\n--- My Courses ---\n";

    bool hasCourse = false;

    while((row = mysql_fetch_row(result)) != nullptr)
    {
        hasCourse = true;

        cout << "- "
         << row[0]
         << " ("
         << row[1]
         << " credits)"
         << endl;
    }

    if(!hasCourse){
        cout <<"You have no assigned courses.\n";
    }

    mysql_free_result(result);
}

const map<string, vector<CourseInfo>>& instructor::getCourseCatalog() {
    static const map<string, vector<CourseInfo>> courseCatalog = {
        {"Computer Science", {
            CourseInfo("C++", 3),
            CourseInfo("Data Structures", 3),
            CourseInfo("Databases", 3),
            CourseInfo("Operating Systems", 3),
            CourseInfo("Networks", 3)
        }},

        {"Electrical Engineering", {
            CourseInfo("Circuit Analysis", 3),
            CourseInfo("Electromagnetics", 3),
            CourseInfo("Digital Systems", 3),
            CourseInfo("Control Systems", 3)
        }},

        {"Business Administration", {
            CourseInfo("Accounting", 3),
            CourseInfo("Finance", 3),
            CourseInfo("Organizational Behavior", 3),
            CourseInfo("Business Ethics", 3)
        }},

        {"Mechanical Engineering", {
            CourseInfo("Thermodynamics", 3),
            CourseInfo("Fluid Mechanics", 3),
            CourseInfo("Dynamics", 3),
            CourseInfo("Materials Science", 3)
        }},

        {"Civil Engineering", {
            CourseInfo("Statics", 3),
            CourseInfo("Structural Analysis", 3),
            CourseInfo("Geotechnical Engineering", 3),
            CourseInfo("Hydraulics", 3)
        }},

        {"Cyber Security", {
            CourseInfo("Network Security", 3),
            CourseInfo("Ethical Hacking", 3),
            CourseInfo("Digital Forensics", 3),
            CourseInfo("Incident Response", 3)
        }},

        {"Pharmacy", {
            CourseInfo("Pharmacology", 3),
            CourseInfo("Pharmaceutics", 3),
            CourseInfo("Medicinal Chemistry", 3),
            CourseInfo("Clinical Pharmacy", 3)
        }},

        {"Marketing", {
            CourseInfo("Principles of Marketing", 3),
            CourseInfo("Consumer Behavior", 3),
            CourseInfo("Digital Marketing", 3),
            CourseInfo("Brand Management", 3)
        }}
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
    cout << "\n";
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
        cout << "- " << course.courseName << " (" << course.creditHours << " credits)\n";
    }
}

vector<CourseInfo> instructor::viewStudentCoursesDatabase(Database &db,student* selectedStudent){
    vector<CourseInfo> courses;

    if(selectedStudent == nullptr)
        return courses;

    int studentID = selectedStudent->getID();

    string Query =
    "SELECT c.course_name, c.credit_hours "
    "FROM student_courses sc "
    "INNER JOIN courses c "
    "ON sc.course_id = c.course_id "
    "WHERE sc.student_id = " + to_string(studentID);

    MYSQL_RES* result = db.executeSelect(Query);

    if(result == nullptr)
        return courses;

    MYSQL_ROW row;

    while((row = mysql_fetch_row(result)) != nullptr){

        courses.push_back(CourseInfo(row[0],stoi(row[1])));
    }

    mysql_free_result(result);
    return courses;
}


void instructor::viewStudentCourses(vector<user*>& users, Database& db)
{
    vector<student*> students;

    cout << "\n--- Students List ---\n";

    for (const auto& user : users)
    {
        if (user->getRole() == "Student")
        {
            student* currentStudent = dynamic_cast<student*>(user);

            if (currentStudent != nullptr)
            {
                students.push_back(currentStudent);

                cout << students.size() << ". "
                     << currentStudent->getusername()
                     << " (ID: " << currentStudent->getID()
                     << ", Major: " << currentStudent->getMajor() << ")\n";
            }
        }
    }

    if (students.empty())
    {
        cout << "No students found in the system.\n";
        return;
    }

    int choice;

    cout << "Select a student (1-" << students.size()
         << ") or 0 to cancel: ";

    cin >> choice;

    while (cin.fail() ||choice < 0 || choice > static_cast<int>(students.size()) || hasExtraInputOnLine())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Invalid choice. Select a student (1-"
             << students.size()
             << ") or 0 to cancel: ";

        cin >> choice;
    }

    if (choice == 0)
    {
        cout << "Operation cancelled.\n";
        return;
    }

    student* selectedStudent = students[choice - 1];

    // Get courses from database
    vector<CourseInfo> enrolledCourses = viewStudentCoursesDatabase(db, selectedStudent);

    cout << "\n--- Courses for Student: "<< selectedStudent->getusername() << " ---\n";

    if (enrolledCourses.empty())
    {
        cout << "No courses enrolled.\n";
    }
    else
    {
        for (const auto& course : enrolledCourses)
        {
            cout << "- " << course.courseName
                 << " (" << course.creditHours
                 << " credits)\n";
        }
    }
}

bool instructor::addCourseDatabase(Database &db,student* selectedStudent,string CourseName){

    if(selectedStudent == nullptr)
        return false;

    int StudentID = selectedStudent->getID();

    string courseQuery = 
    "SELECT course_id "
    "FROM courses "
    "WHERE course_name='"
    + db.escapeString(CourseName) + "'";

    MYSQL_RES* courseResult = db.executeSelect(courseQuery);

    if(courseResult == nullptr)
        return false;

    MYSQL_ROW CourseRow = mysql_fetch_row(courseResult);

    if(CourseRow == nullptr){
        mysql_free_result(courseResult);
        return false;
    }

    int courseID = stoi(CourseRow[0]);
    mysql_free_result(courseResult);

    string dublicateQuery =
    "SELECT * "
    "FROM student_courses "
    "WHERE student_id=" + to_string(StudentID) +
    " AND course_id=" + to_string(courseID);

    MYSQL_RES* dublicateResult = db.executeSelect(dublicateQuery);

    if(dublicateResult == nullptr)
        return false;

    MYSQL_ROW dublicateRow = mysql_fetch_row(dublicateResult);

    if(dublicateRow != nullptr){
        mysql_free_result(dublicateResult);
        cout << "Student already has this course.\n";
        return false;
    }

    mysql_free_result(dublicateResult);

    string insertQuery = 
    "INSERT INTO student_courses "
    "(student_id, course_id, grade) "
    "VALUES(" + to_string(StudentID) + "," + to_string(courseID) + ",NULL)"; 

    if(!db.executeQuery(insertQuery)){
        cout << "Failed to add course to database.\n";
        return false;
    }

    return true;

}

bool instructor::addCourseLogic(Database &db,student* selectedStudent,string CourseName){

    if(selectedStudent == nullptr)
        return false;

    string studentMajor = selectedStudent->getMajor();

    const auto& courseCatalog = getCourseCatalog();

    auto majorIt = courseCatalog.find(studentMajor);

    if(majorIt == courseCatalog.end()){
        return false;
    }

    int creditHours = -1;

    for(const CourseInfo& course : majorIt->second){
        if (course.courseName == CourseName){

            creditHours = course.creditHours;
            break;
        }
    }


    if (creditHours == -1)
        return false;

    if (!addCourseDatabase(db,selectedStudent,CourseName)){
        addLog(db, "Failed to add course " + CourseName + " to student " + selectedStudent->getusername());
        return false;
    }

    //Update student in memory
    selectedStudent->addCourse(CourseName,creditHours);

    addLog(db, "Added course " + CourseName + " to student " + selectedStudent->getusername());
    return true;
}

void instructor::addCourseMenu(vector<user*>& users,Database &db) {
    // Display all students
    cout << "\n--- Students List ---\n";
    vector<student*> studentList;
    int studentCount = 0;
    
    for (const auto& user : users) {
        if (user->getRole() == "Student") {
            student* std = dynamic_cast<student*>(user);

            if(std != nullptr){
               studentList.push_back(std);
                studentCount++;

                cout << studentCount << ". "
                     << std->getusername()
                     << " (ID: " << std->getID()
                     << ", Major: " << std->getMajor() << ")\n"; 
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
    cout << "\nSelect a student to add the course to (1-"<< studentCount << ") or 0 to cancel: ";
    cin >> choice;
    
    // Validate input
    while (cin.fail() || choice < 0 || choice > studentCount || hasExtraInputOnLine())
    
    {
        cin.clear();  // clear the error flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
        cout << "Invalid input!" << endl;
        cout << "\nSelect a student to add the course to (1-"<< studentCount << ") or 0 to cancel: ";
        cin >> choice;
        continue;  // ask for input again

    }

    if (choice == 0)
    {
    cout << "Operation cancelled.\n";
    return;
    }
    
    
    
    // Add course to the selected student (restricted by major)
    student* selectedStudent = studentList[choice - 1];
    string studentMajor = selectedStudent->getMajor();

    const auto& courseCatalog = getCourseCatalog();

    auto majorIt = courseCatalog.find(studentMajor);

    if (majorIt == courseCatalog.end())
    {
        cout << "No course catalog found for major: "
             << studentMajor << "\n";
        return;
    }

    while (true)
    {
        // Display courses
        cout << "\nCourses for " << studentMajor << ":\n";

        for (size_t i = 0; i < majorIt->second.size(); ++i)
        {
            cout << i + 1 << ". "
                << majorIt->second[i].courseName
                << " (" << majorIt->second[i].creditHours
                << " credits)\n";
        }

        // Select course
        int courseChoice;

        cout << "\nSelect a course (1-"
            << majorIt->second.size()
            << ") or 0 to cancel: ";

        cin >> courseChoice;

        while (cin.fail() ||
            courseChoice < 0 ||
            courseChoice > static_cast<int>(majorIt->second.size()) ||
            hasExtraInputOnLine())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Invalid input! Please enter a number between 0 and "
                << majorIt->second.size() << ": ";

            cin >> courseChoice;
        }

        // Cancel / finish
        if (courseChoice == 0)
        {
            cout << "Finished adding courses.\n";
            return;
        }

        // Get selected course
        const CourseInfo& selectedCourse =
            majorIt->second[courseChoice - 1];

        string courseName = selectedCourse.courseName;

        // Call logic
        bool result = addCourseLogic(
            db,
            selectedStudent,
            courseName
        );

        if (result)
        {
            cout << "Course " << courseName
                << " added to student "
                << selectedStudent->getusername()
                << " successfully.\n";
        }
        else
        {
            cout << "Failed to add course "
                << courseName << ".\n";
        }
    }
}

bool instructor::removeCourseDatabase(Database &db,student* selectedStudent,string courseName){
    if(selectedStudent == nullptr)
        return false;

    int studentID = selectedStudent->getID();

    //get course id
    string courseQuery =
    "SELECT course_id "
    "FROM courses "
    "WHERE course_name='"
    + db.escapeString(courseName) + "'";

    MYSQL_RES* courseResult = db.executeSelect(courseQuery);

    if(courseResult == nullptr)
        return false;

    MYSQL_ROW courseRow = mysql_fetch_row(courseResult);

    if(courseRow == nullptr){
        mysql_free_result(courseResult);
        return false;
    }

    int courseID = stoi(courseRow[0]);
    mysql_free_result(courseResult);

    // Delete the student's course

    string DeleteQuery = 
    "DELETE FROM student_courses "
    "WHERE student_id=" + to_string(studentID) +
    " AND course_id=" + to_string(courseID);

    if(!db.executeQuery(DeleteQuery)){
        cout << "Failed to remove course from database.\n";
        return false;
    }

    if(mysql_affected_rows(db.getConnection()) != 1){
        cout << "Student does not have this course.\n";
        return false;
    }

    return true;
}

void instructor::removeCourse(vector<user*>& users,Database& db) {
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

    vector<CourseInfo> enrolledCourses = viewStudentCoursesDatabase(db,selectedStudent);

    if (enrolledCourses.empty()) {
        cout << "Student " << selectedStudent->getusername() << " has no courses to remove.\n";
        return;
    }

    //display Courses
    cout << "\nCourses for " << selectedStudent->getusername() << ":\n";
    
    for (size_t i = 0; i < enrolledCourses.size(); ++i) {
        cout << (i + 1) << ". " << enrolledCourses[i].courseName << " (" << enrolledCourses[i].creditHours << " credits)\n";
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

    string courseName = enrolledCourses[courseChoice - 1].courseName;
    
    if(!removeCourseDatabase(db,selectedStudent,courseName)){

        cout << "Failed to remove course "
         << courseName << ".\n";

        addLog(db, "Failed to remove course " + courseName + " from student " + selectedStudent->getusername());
        return;
    }
    
    auto& courses = selectedStudent->getEnrolledCourses();
    
    auto it = remove_if(courses.begin(),courses.end(),[&](const CourseGrade& course)
    {
        return course.courseName == courseName;
    }
);

    courses.erase(it, courses.end());

    cout << "Course " << courseName
        << " removed successfully from "
        << selectedStudent->getusername() << ".\n";

    addLog(db, "Removed course " + courseName + " from student " + selectedStudent->getusername());
}


void instructor::viewStudents(vector<user*>& users) {
    cout << "Students enrolled in the courses being taught by " << username << ":\n";
    for (const auto& user : users) {
        if (user->getRole() == "Student") {
            cout << "Name: " << user->getusername() << " | Major: " << user->getMajor() << " | ID: " << user->getID() << endl;
        }
    }
}

bool instructor::addGradeDatabase(Database &db,student* selectedStudent,string courseName,int grade){
    
    if(selectedStudent == nullptr)
        return false;

    if(grade < 0 || grade > 100)
        return false;

    int StudentID = selectedStudent->getID();

    string courseQuery = 
    "SELECT course_id "
    "FROM courses "
    "WHERE course_name='" + db.escapeString(courseName) + "'";

    MYSQL_RES* CourseResult = db.executeSelect(courseQuery);

    if(CourseResult == nullptr)
        return false;
    
    MYSQL_ROW CourseRow = mysql_fetch_row(CourseResult);

    if(CourseRow == nullptr){
        mysql_free_result(CourseResult);
        return false;
    }

    int CourseID = stoi(CourseRow[0]);

    mysql_free_result(CourseResult);

    //Update Grade 

    string UpdateQuery =
    "UPDATE student_courses "
    "SET grade=" + to_string(grade) +
    " WHERE student_id=" + to_string(StudentID) +
    " AND course_id=" + to_string(CourseID);

    if(!db.executeQuery(UpdateQuery)){

        cout << "Failed to update grade in database.\n";
        return false;
    }

    return true;

}

void instructor::addGrade(vector<user*>& users,Database &db) {
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
    cout << "\nSelect a student to add a grade for (1-" << studentCount << "): ";
    cin >> choice;

    while (cin.fail() || choice < 1 || choice > studentCount || hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 1 and " << studentCount << ": ";
        cin >> choice;
    }

    student* selectedStudent = studentList[choice - 1];
    auto& enrolledCourses = selectedStudent->getEnrolledCourses();
    if (enrolledCourses.empty()) {
        cout << "Student " << selectedStudent->getusername() << " has no courses to add a grade for.\n";
        return;
    }


    while (true) {
        // Display courses for the selected student.
        cout << "\nCourses for " << selectedStudent->getusername() << ":\n";
        for (size_t i = 0; i < enrolledCourses.size(); ++i) {
            cout << (i + 1) << ". "
                 << enrolledCourses[i].courseName
                 << " (" << enrolledCourses[i].creditHours
                 << " credits)";

            if (enrolledCourses[i].grade == -1) {
                cout << " | Grade: Not assigned";
            } else {
                cout << " | Grade: "
                     << enrolledCourses[i].grade
                     << "/100";
            }

            cout << "\n";
        }

        int courseChoice;
        cout << "Select a course to add a grade for (1-" << enrolledCourses.size() << ") or 0 to cancel: ";
        cin >> courseChoice;

        while (cin.fail() || courseChoice < 0 ||
               courseChoice > static_cast<int>(enrolledCourses.size()) ||
               hasExtraInputOnLine()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number between 0 and "
                 << enrolledCourses.size() << ": ";
            cin >> courseChoice;
        }

        if (courseChoice == 0) {
            cout << "Operation cancelled.\n";
            return;
        }

        string courseName = enrolledCourses[courseChoice - 1].courseName;
        int grade;

        cout << "Enter the grade for " << courseName << " (0-100): ";
        cin >> grade;

        while (cin.fail() || grade < 0 || grade > 100 || hasExtraInputOnLine()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a grade between 0 and 100: ";
            cin >> grade;
        }

        if(!addGradeDatabase(db,selectedStudent,courseName,grade)){
            cout << "Failed to update grade.\n";
            addLog(db,"Failed to add grade for " + courseName + " to student " + selectedStudent->getusername());
            continue;
        }

        enrolledCourses[courseChoice - 1].grade = grade;

        cout << "\nGrade added successfully!\n";
        cout << "Student: " << selectedStudent->getusername() << "\n";
        cout << "Course: "
             << enrolledCourses[courseChoice - 1].courseName << "\n";
        cout << "Grade: "
             << enrolledCourses[courseChoice - 1].grade
             << "/100\n";
        addLog(db,"Added grade " + to_string(grade) + " for " + courseName + " to student " + selectedStudent->getusername());
    }

}

void instructor::assignCourse(const CourseInfo& course) {
    if (courses.size() >= 2) {
        cout << "Instructor already has the maximum of 2 courses.\n";
        return;
    }

    for (const auto& existingCourse : courses) {
        if (existingCourse.courseName == course.courseName) {
            cout << "Instructor already has this course.\n";
            return;
        }
    }

    courses.push_back(course);

    cout << "Course " << course.courseName
         << " assigned to instructor successfully.\n";
}

void instructor::clearCourses() {
    courses.clear();
}



void instructor::showMenu(vector<user*>& users,Database &db) {
    int choice;
    do {
        cout << "\nInstructor Menu:\n";
        cout << "1. Add Course\n";
        cout << "2. Remove Course\n";
        cout << "3. add Grades to a Student\n";
        cout << "4. View Courses for Major\n";
        cout << "5. View Student Courses\n";
        cout << "6. View Students\n";
        cout << "7. List All Majors\n";
        cout << "8. Show Profile\n";
        cout << "9. View My Teaching Courses\n";
        cout << "10. Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;
        
        if(cin.fail() || choice < 1 || choice > 10 || hasExtraInputOnLine()) {
            cin.clear();  // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
            cout << "Invalid input! Please enter a number between 1 and 10.\n";
            continue;  // ask for input again
        }

        switch (choice) {
            case 1: {
                addCourseMenu(users,db);
                break;
            }
            case 2: {
                removeCourse(users,db);
                break;
            }
            case 3:{
                addGrade(users,db);
                break;
            }
            case 4:{
                viewCoursesForMajor(users[0]->getMajor());
                break; 
                }
            case 5:
                viewStudentCourses(users,db);
                break;   
            case 6:
                viewStudents(users);
                break;
            case 7:
                listAllMajors();
                break;
            case 8:
                showprofile(db);
                break;
            case 9:
                viewMyCourses(db);
                break;
            case 10:
                logout(db);
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 10);
}

string instructor::encryptpass() {
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
