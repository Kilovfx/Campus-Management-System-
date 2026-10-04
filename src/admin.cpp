#include "admin.hpp"
#include "Student.hpp"
#include "instructor.hpp" 
#include "Password.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include <cctype>
#include <limits>
#ifdef _WIN32
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif
#include <chrono>
#include <ctime>
#include <fstream>

using namespace std;

static const vector<string>& getAllowedMajors() {
    static const vector<string> majors = {
        "Computer Science",
        "Civil Engineering",
        "Electrical Engineering",
        "Business Administration",
        "Mechanical Engineering",
        "Cyber Security",
        "Pharmacy",
        "Marketing"

    };
    return majors;
}

static bool isMajorAllowed(const string& major) {
    const auto& majors = getAllowedMajors();
    return find(majors.begin(), majors.end(), major) != majors.end();
}

static void printAllowedMajors() {
    const auto& majors = getAllowedMajors();
    cout << "Available majors:\n";
    for (const auto& m : majors) {
        cout << "- " << m << "\n";
    }
}

static bool isAdminRole(const string& role) {
    string normalized = role;
    transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    return normalized == "admin";
}


admin::admin(string username, string password, string role, int ID)
    : user(username, password, role, ID, "", "", "", false) {
    this->username = username;
    this->password = password;
    this->role = role;
    this->ID = ID;
}

//check if admin handling error
bool admin::canLockUser(Database& db, string username){

    string Query =
    "SELECT role FROM users WHERE username='" +
    db.escapeString(username) + "'";

    MYSQL_RES* result = db.executeSelect(Query);

    if(result == nullptr){
        cout << "User was not found\n";
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if(row == nullptr){
        cout << "User was not found\n";
        return false;
    }

    string role = row[0];

    if(role == "Admin" || role == "admin"){
        cout << "Cannot lock admin account!\n";
        cout<<endl;
        return false;
    }

    return true;
}

bool admin::unlockuserDatabase(Database& db,string username){

    string findQuery = 
    "SELECT u.user_id, ac.locked "
    "FROM users u "
    "INNER JOIN account_security ac "
    "ON u.user_id = ac.user_id "
    "WHERE u.username='"
    + db.escapeString(username) + "'";

    MYSQL_RES* result = db.executeSelect(findQuery);

    if(result == nullptr) {
        cout <<"User not found.\n";
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if(row == nullptr){
        cout <<"User not found\n";
        mysql_free_result(result);
        return false;
    }

    int userID = stoi(row[0]);
    bool locked = stoi(row[1]);
    
    mysql_free_result(result);

    if(!locked) {
        cout <<"Account already unlocked.\n";
        return false;
    }

    string unlockQuery = 
    "UPDATE account_security SET "
    "locked=0, "
    "failed_attempts=0, "
    "locked_until=NULL "
    "WHERE user_id=" + to_string(userID);

    if(!db.executeQuery(unlockQuery)){
        cout <<"Failed unlocking user\n";
        return false;
    }

    return true;
}


bool admin::changeUserRoleDatabase(Database &db,user* selectedUser,string newRole){

    string oldRole = selectedUser->getRole();
    int ID = selectedUser->getID();

    if(oldRole == newRole){
        cout <<"User already has this role.\n";
        return false;
    }

    // Start transaction
    if(!db.executeQuery("START TRANSACTION"))
    {
        cout << "Failed starting transaction\n";
        return false;
    }

    // student -> instructor
    if(oldRole == "Student" && newRole == "Instructor"){

        string deletestudentCourses = 
        "DELETE FROM student_courses WHERE student_id="
        + to_string(ID);

        if(!db.executeQuery(deletestudentCourses)){

        cout<<"Failed to delete student courses\n";
        db.executeQuery("ROLLBACK");
        return false;

        }


        string InsertQuery = 
        "INSERT INTO instructors "
        "(instructor_id,first_name,last_name,major_id)"
        "SELECT student_id, first_name, last_name, major_id "
        "FROM students WHERE student_id=" 
        + to_string(ID);

        if(!db.executeQuery(InsertQuery) ||
           mysql_affected_rows(db.getConnection()) != 1){

        cout << "Failed creating instructor profile for student ID "
             << ID << " (student row may not exist)\n";
        db.executeQuery("ROLLBACK");
        return false;

        }

    

        string deleteStudent =
        "DELETE FROM students WHERE student_id="
        + to_string(ID);

        if(!db.executeQuery(deleteStudent) ||
           mysql_affected_rows(db.getConnection()) != 1){
            
            cout << "Failed deleting student profile for student ID "
                 << ID << "\n";
            db.executeQuery("ROLLBACK");
            return false;
        }

    }

    // instructor -> student
    else if(oldRole == "Instructor" && newRole == "Student"){

        string deleteInstructorCourses = 
        "DELETE FROM instructor_courses WHERE instructor_id="
        + to_string(ID);

        if(!db.executeQuery(deleteInstructorCourses)){

        cout<<"Failed to delete instructor courses\n";
        db.executeQuery("ROLLBACK");
        return false;

        }


        string InsertQuery = 
        "INSERT INTO students "
        "(student_id, first_name, last_name, major_id) "
        "SELECT instructor_id, first_name, last_name, major_id "
        "FROM instructors WHERE instructor_id="
        + to_string(ID);

        if(!db.executeQuery(InsertQuery) ||
           mysql_affected_rows(db.getConnection()) != 1){

        cout << "Failed creating student profile for instructor ID "
             << ID << " (instructor row may not exist)\n";
        db.executeQuery("ROLLBACK");
        return false;

        }

    

        string deleteInstructor =
        "DELETE FROM instructors WHERE instructor_id="
        + to_string(ID);

        if(!db.executeQuery(deleteInstructor) ||
           mysql_affected_rows(db.getConnection()) != 1){
            
            cout << "Failed deleting instructor profile for instructor ID "
                 << ID << "\n";
            db.executeQuery("ROLLBACK");
            return false;
        }

    }


    string Updateuser = 
    "UPDATE users SET role='" +
    db.escapeString(newRole) +
    "' WHERE user_id="
    + to_string(ID);

    if(!db.executeQuery(Updateuser))
    {
        cout << "Failed updating user role\n";
        db.executeQuery("ROLLBACK");
        return false;
    }

        // Save changes permanently
    if(!db.executeQuery("COMMIT"))
    {
        cout << "Failed committing transaction\n";
        db.executeQuery("ROLLBACK");
        return false;
    }

    return true;

}

vector<CourseInfo> admin::getCoursesForMajor(Database& db, string major){
    vector<CourseInfo> courses;
    string query =
    "SELECT c.course_name, c.credit_hours "
    "FROM courses c "
    "INNER JOIN majors m ON c.major_id = m.major_id "
    "WHERE m.major_name='" 
    + db.escapeString(major) + "'";

    MYSQL_RES* result = db.executeSelect(query);


    if(result == nullptr)
    {
        return courses;
    }


    MYSQL_ROW row;

    while((row = mysql_fetch_row(result)) != nullptr)
    {
        courses.push_back(
            CourseInfo(
                row[0],
                stoi(row[1])
            )
        );
    }


    mysql_free_result(result);

    return courses;
}

bool admin::lockUserDatabase(Database &db,string username, int minutes){

    string findQuery =
    "SELECT user_id, role FROM users WHERE username='"
    +db.escapeString(username) + "'";

    MYSQL_RES* result = db.executeSelect(findQuery);

    if(result == nullptr){
        cout << "User not found\n";
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if(row == nullptr){
        cout << "User does not exist\n";
        mysql_free_result(result);
        return false;
    }
    

    int userID = stoi(row[0]);
    string role = row[1];

    mysql_free_result(result);


    string lockQuery = 
    "UPDATE account_security SET "
    "locked=1, "
    "failed_attempts=0, "
    "locked_until=DATE_ADD(NOW(), INTERVAL "
    + to_string(minutes) +
    " MINUTE) "
    "WHERE user_id=" + to_string(userID);

    if(!db.executeQuery(lockQuery)){
        cout<<"Failed locking user\n";
        return false;
    }

    cout << "Rows affected: "
     << mysql_affected_rows(db.getConnection())
     << endl;

    return true;

}


bool admin::assignCourseDatabase(Database &db,user* selectedInstructor,string courseName){

     
    int InstructorID = selectedInstructor->getID();


    //check if instructor has two courses : 
    string countQuery = 
    "SELECT count(*) FROM instructor_courses WHERE instructor_id="
    + to_string(InstructorID);

    MYSQL_RES* countResult = db.executeSelect(countQuery);

    if(countResult == nullptr){
        cout <<"Failed to check instructor course \n";
        return false;
    }

    MYSQL_ROW countrow = mysql_fetch_row(countResult);
    
    if(countrow == nullptr){
        cout <<"This Instructor has no courses \n";
        mysql_free_result(countResult);
        return false;
    }

    int CountCourses = stoi(countrow[0]);
    mysql_free_result(countResult);

    if(CountCourses >= 2){
        cout <<"Instructor already has 2 Courses"<<endl;
        return false;
    }

    // get get Course ID 

    string getCourseIDQuery = 
    "SELECT course_id FROM courses WHERE course_name='"
    + db.escapeString(courseName) + "'";

    cout << "Searching course: [" << courseName << "]\n";
    MYSQL_RES* result = db.executeSelect(getCourseIDQuery);

    if(result == nullptr){
        cout << "Course not found\n";
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if(row == nullptr){
        cout << "Course not found\n";
        mysql_free_result(result);
        return false;
    }

    int courseID = stoi(row[0]);

    mysql_free_result(result);

    string dublicateQuery =
    "SELECT * FROM instructor_courses WHERE instructor_id="
    + to_string(InstructorID)
    + " AND course_id="
    + to_string(courseID);

    MYSQL_RES* dubilcateCheck = db.executeSelect(dublicateQuery);

    if(dubilcateCheck == nullptr){
        return false;
    }

    MYSQL_ROW dubilcateRow = mysql_fetch_row(dubilcateCheck);

    if(dubilcateRow != nullptr){

        cout << "Instructor already has this course\n";
        mysql_free_result(dubilcateCheck);
        return false;

    }

    mysql_free_result(dubilcateCheck);

    string insertQuery = 
    "INSERT INTO instructor_courses(instructor_id,course_id) VALUES("
    + to_string(InstructorID) 
    + ","
    + to_string(courseID) 
    + ")";

    if(!db.executeQuery(insertQuery)){
        cout << "Failed to assign course\n";
        return false;
    }

    return true;

}

bool admin::changeMajorDatabase(Database &db,user* selectedUser,string newMajor){

    int ID = selectedUser->getID();
    string role = selectedUser->getRole();

    string getmajorid = 
    "SELECT major_id FROM majors WHERE major_name='"
    + db.escapeString(newMajor) + "'";

    MYSQL_RES* result = db.executeSelect(getmajorid);

    if(result == nullptr){
        cout <<"Major not found\n";
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if(row == nullptr){
        cout <<"Major not found\n";
        mysql_free_result(result);
        return false;
    }

    int majorID = stoi(row[0]);

    mysql_free_result(result);

    //Update user table
    string updateUserQuery = 
    "UPDATE users SET major_id=" 
    + to_string(majorID) +
    " WHERE user_id="
    + to_string(ID);

    if(!db.executeQuery(updateUserQuery)){
    
        return false;
    }

    string updateProfile;

    if(role == "Student"){

        updateProfile =
        "UPDATE students SET major_id="
        + to_string(majorID) +
        " WHERE student_id="
        + to_string(ID);

    }

    else if(role == "Instructor"){

        updateProfile =
        "UPDATE instructors SET major_id="
        + to_string(majorID) +
        " WHERE instructor_id="
        + to_string(ID);

    }

    else {

        cout<<"Invalid role\n";
        return false;
    }

    if(!db.executeQuery(updateProfile)){
        return false;
    }

    return true;

}

bool admin::usernameexist(Database &db,string username){

    string userQuery = 
    "SELECT user_id FROM users WHERE username='" 
    + db.escapeString(username) + 
    "'LIMIT 1";

    MYSQL_RES* result = db.executeSelect(userQuery);

    if(result == nullptr){
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    bool exist = (row != nullptr);
    mysql_free_result(result);
    return exist;
    
}

bool admin::createaccountsecurity(Database &db,int userID){

    //for account_security table
    string securityQuery =
    "INSERT INTO account_security "
    "(user_id,failed_attempts,locked,locked_until,last_failed_attempt,last_login)"
    "VALUES ("
    + to_string(userID) +
    ",0,0,NULL,NULL,NULL)";

    if(!db.executeQuery(securityQuery)){
        cout <<"Failed creating account security\n";
        return false;
    }

    return true;
}

int admin::insertUser(Database &db,string username, string hashedpassword, string role, int majorID){

        string userQuery = 
        "INSERT INTO users "
        "(username,password_hash,role,major_id) VALUES('" +
            db.escapeString(username) + "','" +
            db.escapeString(hashedpassword) + "','" +
            db.escapeString(role) + "'," +
            to_string(majorID) + 

        ")" ;

        if(!db.executeQuery(userQuery)){
            return -1;
        }

    return db.getLastInsertID();    
}

bool admin::insertStudent(Database &db,int userID,string first_name,string last_name,int majorID){

        string studentQuery = 
        "INSERT INTO students"
        "(student_id,first_name,last_name,major_id) VALUES(" +
        to_string(userID) + ",'" +
        db.escapeString(first_name) + "','" +
        db.escapeString(last_name) + "',"  +
        to_string(majorID) + 

        ")";

    return db.executeQuery(studentQuery);
}

bool admin::insertInstructor(Database &db,int userID,string first_name,string last_name,int majorID){

    string instructorQuery = 
        "INSERT INTO instructors "
        "(instructor_id,first_name,last_name,major_id) VALUES(" +
        to_string(userID) + ",'" +
        db.escapeString(first_name) + "','" +
        db.escapeString(last_name) + "',"  +
        to_string(majorID) +
        ")";

    return db.executeQuery(instructorQuery);
}


void admin::showprofile(Database& db){
    cout<<"\nAdmin Profile\n";
    cout << "Username: " << username << endl;
    cout << "ID: " << ID << endl;
    cout << "Role: "<< role << endl;

    // Example: Display current date (or the date of account creation if available)
    time_t now = time(0);
    char* dt = ctime(&now);
    cout << "Account created on: " << dt << endl;

}

int admin::getMajorID(Database& db, string majorName)
{
    string majorQuery =
    "SELECT major_id FROM majors WHERE major_name='" 
    + db.escapeString(majorName) + "'";

    MYSQL_RES* result = db.executeSelect(majorQuery);

    if(result)
    {
        MYSQL_ROW row = mysql_fetch_row(result);

        if(row)
        {
            int id = stoi(row[0]);
            mysql_free_result(result);
            return id;
        }

        mysql_free_result(result);
    }

    return -1;
}


string admin::encryptpass() {
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



//message login for the admin
void admin::login(vector<user*>& users,Database &db){
    
    string username, password;
    int attempts = 0;
    const int maxAttempts = 6;
    

    while (true) {
        cout << "enter the username: ";
        cin >> username;

        //handle error 1
        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            username.clear();
        }

        //encrypted
        cout << "enter the password: ";
        password = encryptpass();

        //search for the admin
        user* account = nullptr;
        for (auto* userAccount : users) {
            if (userAccount->getusername() == username &&
                userAccount->getRole() == "admin") {
                account = userAccount;
                break;
            }
        }

        if(account !=nullptr && account->isUserLocked(db)){
            cout << "Account is locked. Login is not available.\n";
            return;
        }

        if (account != nullptr && account->authenticate(username,password)) {
            user::recordLoginAttempts(db,username,true);
            account->resetFailedAttempts(db);
            attempts = 0;
            this->username = username;
            cout << "User : " << username << " logged in successfully!\n";
            addLog(db,"Admin logged in");
            return;

        }
        //record every failed attempts
        user::recordLoginAttempts(db,username,false);

        
        //know admin but wrong password
        if(account !=nullptr){

            bool locked = account->increaseFailedAttempts(db);

            if(locked){
                cout << "Admin account locked after too many failed attempts.\n";
                account->addLog(db,"Admin account locked after too many failed attempts");
                return;
            }
        }


        attempts++;

        if(attempts >= maxAttempts){
            cout <<"Login failed after 6 attempts.\n";
            return;
        }

        if(attempts >= 3){

            int waittime = 15 * (attempts - 2);

            cout << "You have made " << attempts << " incorrect attempts, Please wait for " << waittime << " seconds .. \n";
            this_thread::sleep_for(chrono::seconds(waittime));

            cout <<"You can try now \n";
        }
            


        cout << "Invalid credentials. Please try again.\n";
    }



} 

void admin::ViewAllLog(Database& db){

    string Allquery =
    "SELECT l.log_id, l.user_id, u.username, l.message, l.log_time "
    "FROM `logs` l "
    "LEFT JOIN users u "
    "ON l.user_id = u.user_id "
    "ORDER BY l.log_id DESC";

    MYSQL_RES* result = db.executeSelect(Allquery);

    if(result == nullptr){
        cout << "Failed to retrieve logs.\n";
        return;       
    }

    MYSQL_ROW row;
    bool found = false;  

    cout << "\n--- All Logs ---\n";

    while((row = mysql_fetch_row(result)) != nullptr){

        found = true;

        cout << "Log ID: " << row[0]
             << " | User ID: " << row[1]
             << " | Username: "
             << (row[2] ? row[2] : "Deleted User")
             << " | Message: " << row[3]
             << " | Time: " << row[4]
             << endl;
    }  

    if(!found){
        cout << "No logs found.\n";
    }

    mysql_free_result(result);
}

bool admin::ViewLogByID(Database& db,int logID){

    string query =
    "SELECT l.log_id, l.user_id, u.username, "
    "l.message, l.log_time "
    "FROM `logs` l "
    "LEFT JOIN users u "
    "ON l.user_id = u.user_id "
    "WHERE l.log_id=" + to_string(logID);

    MYSQL_RES* result = db.executeSelect(query);

    if(result == nullptr){
        cout << "Failed to retrieve log.\n";
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if(row == nullptr){
        cout << "Log not found.\n";
        mysql_free_result(result);
        return false;
    }   


    cout << "\n--- Log ---\n";
    cout << "Log ID: " << row[0]
         << " | User ID: " << row[1]
         << " | Username: "
         << (row[2] ? row[2] : "Deleted User")
         << " | Message: " << row[3]
         << " | Time: " << row[4]
         << endl;  

    mysql_free_result(result);
    return true;
}

bool admin::ViewLogsByUserID(Database &db,int userID){

    string query =
    "SELECT l.log_id, l.user_id, u.username, "
    "l.message, l.log_time "
    "FROM `logs` l "
    "LEFT JOIN users u "
    "ON l.user_id = u.user_id "
    "WHERE l.user_id=" + to_string(userID) +
    " ORDER BY l.log_id DESC";

    MYSQL_RES* result = db.executeSelect(query);

    if(result == nullptr){
        cout << "Failed to retrieve logs.\n";
        return false;
    }

    MYSQL_ROW row;
    bool found = false;

    cout << "\n--- User Logs ---\n";

    while((row = mysql_fetch_row(result)) != nullptr){

        found = true;

        cout << "Log ID: " << row[0]
             << " | User ID: " << row[1]
             << " | Username: "
             << (row[2] ? row[2] : "Deleted User")
             << " | Message: " << row[3]
             << " | Time: " << row[4]
             << endl;
    }

    if(!found){
        cout << "No logs found for this user.\n";
    }

    mysql_free_result(result);

    return found;
}

void admin::ViewlogMenu(Database& db){

    int choice;

    while(true){

        cout << "\n--- View Logs ---\n";
        cout << "1. View all logs\n";
        cout << "2. Search by Log ID\n";
        cout << "3. Search by User ID\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        while(cin.fail() || choice < 0 || choice > 3 || hasExtraInputOnLine()){

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Invalid choice. Enter 0-3: ";
            cin >> choice;
        }

        if(choice == 0){
            break;
        }

        if(choice == 1){
            ViewAllLog(db);
        }

        else if(choice == 2){

            int logID;

            cout << "Enter Log ID: ";
            cin >> logID;

            while(cin.fail() || logID <= 0 || hasExtraInputOnLine()){

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                cout << "Invalid Log ID. Enter a positive number: ";
                cin >> logID;
            }

            ViewLogByID(db, logID);
        }

        else if(choice == 3){

            int userID;

            cout << "Enter User ID: ";
            cin >> userID;

            while(cin.fail() || userID <= 0 || hasExtraInputOnLine()){

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                cout << "Invalid User ID. Enter a positive number: ";
                cin >> userID;
            }

            ViewLogsByUserID(db, userID);
        }
    }
}






void admin::ListAll(vector<user*>& users,Database& db){
    for(auto& user : users){
        cout << "Username: " << user->getusername()
             << " | ID: " << user->getID()
             << " | Role: " << user->getRole();

        if(user->getRole() == "Student" || user->getRole() == "Instructor"){
            string major = user->getMajor();

            size_t courseCount = 0;
            if (user->getRole() == "Student") {
                student* selectedStudent = dynamic_cast<student*>(user);
                if (selectedStudent != nullptr) {
                    courseCount = selectedStudent->getEnrolledCourses().size();
                }
            } else {
                instructor* selectedInstructor = dynamic_cast<instructor*>(user);
                if (selectedInstructor != nullptr) {
                    courseCount = selectedInstructor->getCourses().size();
                }
            }

            cout << " | Major: [" << major << "]"
                 << " | Courses: " << courseCount;
        }

        cout << endl;
    }

    // ONE log only
    addLog(db,"Listed all users");
}



//message logout for the admin
void admin::logout(Database& db){
    cout <<"Admin: " << username <<" logged out!";
    addLog(db,"Admin logged out."); // count the logs for the admin 
}


void admin::lockUser(vector<user*>& users,Database &db) {
    string lockedusername;
    int minute;
    ListAll(users,db);
    
    while (true) {
        cout << "Input the user you want to lock: ";
        cin >> lockedusername;

        if(!canLockUser(db,lockedusername)){
            return;
        }

        //handle error for username input
        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            continue;
        }

        cout << "Enter lock duration in minutes: ";
        cin >> minute;

        //handle error for minute input
        while (cin.fail() || minute <= 0 || hasExtraInputOnLine()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid duration. Enter a positive number of minutes: ";
            cin >> minute;
        }

        if(lockUserDatabase(db,lockedusername, minute)){
            cout <<"User "<<lockedusername <<" locked successfully\n";
            addLog(db,"Locked user: "+lockedusername);
        }

        break;
    }
} 


void admin::unlockUser(vector<user*>& users,Database &db) {
    string unlockedusername;

    ListAll(users,db);

    while (true) {

        cout << "Input the user you want to unlock (0 to exit): ";
        cin >> unlockedusername;

        if (unlockedusername == "0") {
            cout << "Exiting unlock...\n";
            break;
        }

        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            continue;
        }

        user* selectedUser = nullptr;

        for (auto u : users) {
            if (u->getusername() == unlockedusername) {
                selectedUser = u;
                break;
            }
        }

        if (selectedUser == nullptr) {
            cout << "User not found.\n";
            continue;
        }

        if (selectedUser == this || isAdminRole(selectedUser->getRole())) {
            cout << "Cannot unlock admin account!\n";
            continue;
        }

        if (unlockuserDatabase(db,unlockedusername)){
            cout <<"User " << unlockedusername << " has been unlocked.\n";

            addLog(db,"Unlocked user: " + unlockedusername);
        }

        break;
    }
}



void admin::createuser(vector<user*>& users,Database& db, string username, string password, string role,string first_name, string last_name, string major) {
    user* newUser = nullptr;
    string hashedPassword = Hashpassword(password);

    //handling error existing input 1
    try {
        auto isBlank = [](const string& value) {
            return value.empty() || all_of(value.begin(), value.end(), [](unsigned char character) {
                return isspace(character) != 0;
            });
        };

        //handling error 2
        if (isBlank(first_name) || isBlank(last_name)) {
            cout << "Error: First name and last name cannot be empty.\n";
            addLog(db,"Failed to create user: " + username + " - Name is empty");
            return;
        }

        // Check if username already exists
        if(usernameexist(db,username)){
            cout <<"Username already exists\n";
            addLog(db,"Failed creating user: username already exists");
            return;
        }


        if(role!="Student" && role!="Instructor"){

            cout << "Invalid role\n";
            return;
        }



        //check if the major is allowed
        if(!isMajorAllowed(major)){
            cout <<"Major is not allowed\n";
            return;
        }


        int majorID = getMajorID(db,major);
        if(majorID == -1){
            cout << "Major ID not Found!\n";
            return;
        }

        /*
        STEP 1
        Insert into users table
        */

        int userID = insertUser(db,username,hashedPassword,role,majorID);

        if(userID == -1){
            cout <<"Failed creating user\n";
            return;
        }

        if(!createaccountsecurity(db,userID)){

            cout <<"Failed creating account security\n";
            return;
        }

        /*
        STEP 3
        Insert into student/instructor table
            */


        if (role == "Student"){

        if(!insertStudent(db,userID,first_name,last_name,majorID)){
            cout <<"Failed to create student\n";
            return;
        }

        newUser = new student(username,hashedPassword,role,userID,first_name,last_name,major);

        }

        else if(role == "Instructor"){

        if(!insertInstructor(db,userID,first_name,last_name,majorID)){
            cout <<"Failed to create instructor\n";
            return;
        }               

            newUser = new instructor(username,hashedPassword,role,userID,first_name,last_name,major);

        }

        
        if (newUser != nullptr) {

            users.push_back(newUser);  // Add the new user to the vector
            cout << "User " << username << " with role " << role << " created successfully with ID: " << userID << "\n";
            addLog(db,"Created user: " + username + " with role: " + role);
            
        }
          
    }

    catch(const exception& e){
        cout <<"Error: " << e.what() <<endl;
    }
}


bool admin::deleteUserDatabase(Database& db, int userID, string role){
    // Delete related student courses
    if(role == "Student"){

        string query =
        "DELETE FROM student_courses WHERE student_id="
        + to_string(userID);

        if(!db.executeQuery(query)){
            cout << "Failed deleting student courses.\n";
            return false;
        }

        // Delete student profile
        query =
        "DELETE FROM students WHERE student_id="
        + to_string(userID);

        if(!db.executeQuery(query)){
            cout << "Failed deleting student profile.\n";
            return false;
        }
    }

    // Delete related instructor courses
    else if(role == "Instructor"){

        string query =
        "DELETE FROM instructor_courses WHERE instructor_id="
        + to_string(userID);

        if(!db.executeQuery(query)){
            cout << "Failed deleting instructor courses.\n";
            return false;
        }

        // Delete instructor profile
        query =
        "DELETE FROM instructors WHERE instructor_id="
        + to_string(userID);

        if(!db.executeQuery(query)){
            cout << "Failed deleting instructor profile.\n";
            return false;
        }
    }

    // Delete account security
    string securityQuery =
    "DELETE FROM account_security WHERE user_id="
    + to_string(userID);

    if(!db.executeQuery(securityQuery)){
        cout << "Failed deleting account security.\n";
        return false;
    }

    // Delete from users table
    string userQuery =
    "DELETE FROM users WHERE user_id="
    + to_string(userID);

    if(!db.executeQuery(userQuery)){
        cout << "Failed deleting user profile.\n";
        return false;
    }

    return true;
}



//function for deleting user by the admin using the khaled method [vector + iterator techneique]
void admin::deleteuser(vector<user*>& users,Database& db,string username,string password){
    cout << "\n--- Delete User ---\n";
    cout << "1. List all users\n";
    cout << "2. Delete by username\n";
    cout << "3. Delete by ID\n";
    cout << "0. Cancel\n";

    int deleteChoice;

    cout << "Enter your choice: ";
    cin >> deleteChoice;

    while(cin.fail() || deleteChoice < 0 || deleteChoice > 3 || hasExtraInputOnLine())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Invalid choice. Enter 0-3: ";
        cin >> deleteChoice;
    }


    if(deleteChoice == 0)
    {
        cout << "Delete cancelled.\n";
        return;
    }


    user* selectedUser = nullptr;


    // Option 1: List users
    if(deleteChoice == 1)
    {
        cout << "\n--- Users List ---\n";

        for(auto* user : users)
        {
            cout << "Username: " << user->getusername()
                << " | ID: " << user->getID()
                << " | Role: " << user->getRole()
                << endl;
        }

        string input;

        cout << "\nEnter username or ID to delete: ";
        cin >> input;


        for(auto* user : users)
        {
            // Search by username
            if(user->getusername() == input)
            {
                selectedUser = user;
                break;
            }


            // Search by ID
            try
            {
                int id = stoi(input);

                if(user->getID() == id)
                {
                    selectedUser = user;
                    break;
                }
            }
            catch(...)
            {
                // input was not a number, ignore
            }
        }
    }


    // Option 2: Username
    else if(deleteChoice == 2)
    {
        string username;

        cout << "Enter username: ";
        cin >> username;


        for(auto* user : users)
        {
            if(user->getusername() == username)
            {
                selectedUser = user;
                break;
            }
        }
    }


    // Option 3: ID
    else if(deleteChoice == 3)
    {
        int id;

        cout << "Enter user ID: ";
        cin >> id;


        for(auto* user : users)
        {
            if(user->getID() == id)
            {
                selectedUser = user;
                break;
            }
        }
    }



    if(selectedUser == nullptr)
    {
        cout << "User not found.\n";
        return;
    }
    
    // Get the selected user
    string selectedUsername = selectedUser->getusername();
    int selectedID = selectedUser->getID();
    string selectedRole = selectedUser->getRole();
    
    // Prevent deleting the currently logged-in admin
    if (selectedUsername == this->username) {
        cout << "Cannot delete the currently logged-in admin account.\n";
        addLog(db,"Attempted to delete own admin account (prevented)");
        return;
    }

    if(isAdminRole(selectedRole)){
        cout <<"Cannot delete another admin.\n";
        return;
    }


    
    // Ask for confirmation
    char confirm;
    string input;
    cout << "\nAre you sure you want to delete user '" << selectedUsername << "'? (y/n): ";
    cin >> input;
    
    while (input.length() != 1 || (input[0] != 'y' && input[0] != 'Y' && input[0] != 'n' && input[0] != 'N'))

    {
        cout << "Invalid input. Please enter only 'y' or 'n': ";
        cin >> input;
    }

    confirm = input[0];
    
    // Check if user cancelled the deletion
    if(confirm != 'y' && confirm != 'Y') {
        cout << "Deletion cancelled.\n";
        addLog(db,"Cancelled deletion of user: " + selectedUsername);
        return;
    }


    
    /*
        Delete profile table first
        because it has foreign key to users
    */

    if(!deleteUserDatabase(db,selectedID,selectedRole)){
        return;
    }
    bool found = false;
    auto it = users.begin();

    while(it != users.end()){
        if((*it)->getID() == selectedID){
            delete *it;
            users.erase(it);
            found = true;
            break;
        }
        ++it;
    }

    if(!found) {
    cout << "User: " << selectedUsername << " Not Found\n";
    return;
    }
    else {
        cout << "User: " << selectedUsername
            << "| ID: " << selectedID
            << " | Role: " << selectedRole
            << " has been deleted successfully! \n";

        addLog(db,"Deleted user: " + selectedUsername);
    }
}





void admin::asignrole(vector<user*>& users,Database &db,string username,string newRole){

    auto it = users.begin();
    while(it != users.end()){
        if((*it)->getusername() == username) {
            user* selectedUser = *it;
            if(isAdminRole((*it)->getRole())){
                cout << "Cannot change role of Admin user: "<< username <<"\n";
                return;
            }

            if(changeUserRoleDatabase(db,selectedUser,newRole)){
            selectedUser->setRole(newRole);
            cout << "Role of user: "
                     << username
                     << " has been updated to: "
                     << newRole << "\n";

                addLog(db,"Changed role for user "
                       + username +
                       " to " + newRole);
            }
         return;
        } 
        ++it;
    }
        cout << "User: " << username << " Not Found\n";
    
}

void admin::changepassword(vector<user*>& users,Database& db){
    // List all users first
    ListAll(users,db);
    cout << "\n";
    
    // Ask admin to choose which user's password to change
    string targetUsername;
    user* targetUser = nullptr;
    bool userFound = false;
    
    while(!userFound) {

        cout << "Enter the username to change password for: ";
        cin >> targetUsername;

        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            targetUsername.clear();
        }

        if(targetUsername.empty()) {
            cout << "Username cannot be empty. Please try again.\n";
            continue;
        }
        
        // Check if user exists
        targetUser = nullptr;
        for(auto& u : users) {
            if(u->getusername() == targetUsername) {
                targetUser = u;
                break;
            }
        }

        if(targetUser == nullptr) {
            cout << "User: " << targetUsername << " Not Found\n";
            cout << "Do you want to try again? (y/n): ";
            char retry;
            cin >> retry;
            while(retry != 'y' && retry != 'Y' && retry != 'n' && retry !='N') {
                cout<<"wrong Input please Use (y/n): ";
                cin >> retry;

                if(retry == 'N' || retry == 'n'){
                    addLog(db,"Cancelled password change operation for non-existent user: " + targetUsername);
                    return;
                }
            }
        } else {
            userFound = true;
        }
    }
    
    // Verify admin's own password first
    string adminPassword;
    cout << "Enter your admin password to confirm: ";
    adminPassword = encryptpass();
    
    if(!verifypassword(adminPassword,this->password)) {
        cout << "Invalid password!\n";
        addLog(db,"Failed password change attempt for user: " + targetUsername + " (invalid admin password)");
        return;
    }
    
    // Now change the target user's password
    string newpassword, confirmpassword;
    while (true)
    {
        cout << "Enter the new password for " << targetUsername << ": ";
        newpassword = encryptpass();

        if (verifypassword(newpassword ,targetUser->getpassword())) {
            cout << "New password cannot be the same as the current password. Please try again.\n";
            continue;
        }

        cout << "Enter again to confirm: ";
        confirmpassword = encryptpass();
        
        if(newpassword != confirmpassword){
            cout <<"Passwords don't match! Please try again.\n"<<endl;
            continue;
        }
        
        string hashedpassword = Hashpassword(newpassword);
        string query =
            "UPDATE users SET password_hash='" +
            db.escapeString(hashedpassword) +
            "' WHERE user_id=" +
            to_string(targetUser->getID());

        if(!db.executeQuery(query)){

            cout << "Failed to update password in the database.\n";
            addLog(db,"Failed database password update for user: "
                   + targetUsername);
            return;

        }

        targetUser->setPassword(hashedpassword);

        cout << "Password for user "
             << targetUsername
             << " has been changed successfully.\n";

        addLog(db,"Changed password for user: " + targetUsername);
        break;
        
    }
}

void admin::changeMajor(vector<user*>& users,Database &db){
    int roleChoice;
    cout << "1. Change a student's major\n";
    cout << "2. Change an instructor's major\n";
    cout << "Enter your choice: ";
    cin >> roleChoice;

    while (cin.fail() || (roleChoice != 1 && roleChoice != 2) ||
           hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter 1 for Student or 2 for Instructor: ";
        cin >> roleChoice;
    }

    const string selectedRole = roleChoice == 1 ? "Student" : "Instructor";
    cout << "\n--- List of " << selectedRole << "s ---\n";
    vector<user*> matchingUsers;

    for (const auto& user : users) {
        if (user->getRole() == selectedRole) {
            matchingUsers.push_back(user);
            cout << matchingUsers.size() << ". " << user->getusername()
                 << " (ID: " << user->getID() << ", Major: "
                 << user->getMajor() << ")\n";
        }
    }

    if (matchingUsers.empty()) {
        cout << "No " << selectedRole << "s found in the system.\n";
        return;
    }

    int choice;
    cout << "\nSelect a " << selectedRole << " to change major (1-"
         << matchingUsers.size() << "): ";
    cin >> choice;

    while (cin.fail() || choice < 1 ||
           choice > static_cast<int>(matchingUsers.size()) ||
           hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 1 and "
             << matchingUsers.size() << ": ";
        cin >> choice;
    }

    user* selectedUser = matchingUsers[choice - 1];

    printAllowedMajors();
    string newMajor;
    const string currentMajor = selectedUser->getMajor();
    while (true) {
        cout << "Enter the new major for " << selectedUser->getusername() << ": ";
        getline(cin, newMajor);
        if (newMajor == currentMajor) {
            cout << "New major cannot be the same as the current major. Please choose a different major.\n";
            continue;
        }
        if (isMajorAllowed(newMajor)) {
            break;
        }
        cout << "Invalid major. Please choose from the list below:\n";
        printAllowedMajors();
    } 

    if(changeMajorDatabase(db, selectedUser, newMajor))
{
    if(selectedUser->getRole() == "Instructor")
    {
        instructor* selectedInstructor =
        dynamic_cast<instructor*>(selectedUser);

        if(selectedInstructor != nullptr)
        {
            selectedInstructor->clearCourses();
        }
    }


    selectedUser->setMajor(newMajor);


    cout << "Major updated successfully for "
         << selectedUser->getusername()
         << "\n";


    addLog(db,"Changed major for "
           + selectedUser->getusername()
           + " to "
           + newMajor);
 }
    else
    {
        cout << "Failed updating major\n";
    }

}


void admin::assignCourseForInstructor(vector<user*>& users,Database &db)
{
    vector<instructor*> instructors;

    cout << "\n--- Instructors List ---\n";

    for (const auto& user : users)
    {
        if (user->getRole() == "Instructor")
        {
            instructor* currentInstructor =
                dynamic_cast<instructor*>(user);

            if (currentInstructor != nullptr)
            {
                instructors.push_back(currentInstructor);

                cout << instructors.size()
                     << ". " << currentInstructor->getusername()
                     << " (ID: " << currentInstructor->getID()
                     << ", Major: " << currentInstructor->getMajor()
                     << ")\n";
            }
        }
    }

    if (instructors.empty())
    {
        cout << "No instructors found.\n";
        return;
    }

    int instructorChoice;
    instructor* selectedInstructor = nullptr;

    while (selectedInstructor == nullptr)
    {
        cout << "\nSelect an instructor (1-"
             << instructors.size()
             << ") or 0 to cancel: ";

        cin >> instructorChoice;

        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid choice. Please enter a number.\n";
            continue;
        }

        if (instructorChoice == 0)
            return;

        if (instructorChoice < 1 ||
            instructorChoice > static_cast<int>(instructors.size()))
        {
            cout << "Invalid choice. Please choose an instructor from the list.\n";
            continue;
        }

        selectedInstructor = instructors[instructorChoice - 1];
    }

        vector<CourseInfo> courses =
        getCoursesForMajor(db, selectedInstructor->getMajor());


        if(courses.empty())
        {
            cout << "No courses found for this major.\n";
            return;
        }

        const auto& currentCourses =
            selectedInstructor->getCourses();

        if (currentCourses.size() >= 2)
        {
            cout << "This instructor already has 2 courses.\n";
            return;
        }

        cout << "\n--- Courses for "
            << selectedInstructor->getMajor()
            << " ---\n";

        for(int i = 0; i < courses.size(); i++)
        {
            cout << i + 1 << ". "
            << courses[i].courseName
            << " (" << courses[i].creditHours
            << " credits)\n";
        }

        int courseChoice;

        while (true)
        {
            cout << "\nSelect a course (1-"
                 << courses.size()
                 << ") or 0 to cancel: ";

            cin >> courseChoice;

            if (cin.fail())
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid choice. Please enter a number.\n";
                continue;
            }

            if (courseChoice == 0)
                return;

            if (courseChoice < 1 ||
                courseChoice > static_cast<int>(courses.size()))
            {
                cout << "Invalid choice. Please choose a course from the list.\n";
                continue;
            }

            CourseInfo selectedCourse = courses[courseChoice - 1];

            // Save to database first
            if(assignCourseDatabase(db, selectedInstructor, selectedCourse.courseName))
            {
                // Update memory only if database succeeded
                selectedInstructor->assignCourse(selectedCourse);

                addLog(db,"Assigned course " + selectedCourse.courseName +
                " to instructor " + selectedInstructor->getusername());
                cout << "Course assigned successfully.\n";

            }
            else {
                   cout << "Failed to assign course.\n";
                 }
            break;
        }
}


void admin::Showmeniu(vector<user*>& users,Database& db){
    int choice;
    string input;
    do
    {
        cout<<"\n Admin Meniu: \n";
        cout<<CREATE_USER<<". Create user\n";
        cout<<DELETE_USER<<". Delete user\n";
        cout<<ASSIGN_ROLE<<". Assign role\n";
        cout<<CHANGE_PASSWORD<<". Change password\n";
        cout<<CHANGE_MAJOR<<". Change major\n";
        cout<<ASSIGN_COURSE<<". Assign course to instructor\n";
        cout<<LOCK_UNLOCK_USER<<". Lock/Unlock user\n";
        cout<<VIEW_USERS<<". View users\n";
        cout<<VIEW_LOGS<<". View logs\n";
        cout<<SHOW_PROFILE<<". Show profile\n";
        cout<<LOGOUT<<". LOGOUT\n";
        cout<<"\n";
        cout<<"Enter your choice: ";
        cin>>choice;
        cout<<"\n";
        
        // Check if there are extra characters after the number
        if(cin.fail() || choice < 1 || choice > 11 || hasExtraInputOnLine()) {
            cin.clear();  // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
            cout << "Invalid input! Please enter a number between 1 and 11.\n";
            continue;  // ask for input again
        }

        // getline(cin, input); // clear the newline character from the input buffer

        // if(input.empty()) {
        //     cout << "Invalid input! Please enter only the number corresponding to your choice.\n";
        //     continue;  // ask for input again
        // }

        switch (choice)
        {
            case CREATE_USER:
            {

            string username,password,role,first_name,last_name,major;
            bool ValidRole = false;

            cout << "Enter username: ";
            cin >> username;
            if (hasExtraInputOnLine()) {
                cout << "Invalid input: spaces are not allowed in usernames.\n";
                username.clear();
            }
            cout << "Enter password: ";
            cin >> password;
            cout << "Enter role ( Instructor / Student ): ";
            cin >> role;
            if (hasExtraInputOnLine()) {
                cout << "Invalid input: spaces are not allowed in roles.\n";
                role.clear();
            }

            while(!ValidRole) {
                // Convert role to proper case for comparison
                string lowerRole = role;
                transform(lowerRole.begin(), lowerRole.end(), lowerRole.begin(), ::tolower);
                
                if(lowerRole == "instructor") {
                    role = "Instructor";
                    ValidRole = true;
                } else if(lowerRole == "student") {
                    role = "Student";
                    ValidRole = true;
                } else {
                    cout << "Invalid role. Please enter either 'Instructor' or 'Student': ";
                    cin >> role;
                    if (hasExtraInputOnLine()) {
                        cout << "Invalid input: spaces are not allowed in roles.\n";
                        role.clear();
                    }
                    ValidRole = false; 
                }
            }

            if (role == "Student" || role == "Instructor") {
                while (true) {
                    cout << "Enter first name: ";
                    getline(cin, first_name);
                    if (!first_name.empty() &&
                        any_of(first_name.begin(), first_name.end(), [](unsigned char character) {
                            return !isspace(character);
                        })) {
                        break;
                    }
                    cout << "Invalid first name. Name cannot be empty.\n";
                }

                while (true) {
                    cout << "Enter last name: ";
                    getline(cin, last_name);
                    if (!last_name.empty() &&
                        any_of(last_name.begin(), last_name.end(), [](unsigned char character) {
                            return !isspace(character);
                        })) {
                        break;
                    }
                    cout << "Invalid last name. Name cannot be empty.\n";
                }

                printAllowedMajors();
                while (true) {
                    cout << "Enter the major: ";
                    getline(cin, major);
                    if (isMajorAllowed(major)) {
                        break;
                    }
                    cout << "Invalid major. Please choose from the list below:\n";
                    printAllowedMajors();
                }
            }
            
            createuser(users, db, username, password, role, first_name, last_name, major);

            }
            break;

            case DELETE_USER: 
            {
                deleteuser(users,db, "", "");
            }
            break;

            case ASSIGN_ROLE:
            {
                string username,newRole;
                bool userExists = false;
                bool validRole = false;
                bool retry = true;

                while(retry) {
                    const char Loading[] = {'/', '-', '\\', '|'};
                    int index = 0;
                    cout<<"Lisitng all the users :\n " <<endl;
                    for(int i = 0;i<10;i++){
                       cout<<"Loading : "<<Loading[index]<<"\r"; // \r is to overwrite the previous Loading charachters 
                       cout.flush(); // to ensure printing the result after the loading immedately 
                       this_thread::sleep_for(chrono::milliseconds(300)); // Sleep for 300ms before changing charachters
                       index++; // to move next charachters
                       if(index == 4) index = 0; // reset to redo the loading animation
                    }
                    ListAll(users,db);
                    cout << "\n";
                    cout << "Enter the username to modify: ";
                    cin >> username;
                    if (hasExtraInputOnLine()) {
                        cout << "Invalid input: spaces are not allowed in usernames.\n";
                        username.clear();
                    }
                    // Check if user exists
                    for(const auto& user : users) {
                        if(user->getusername() == username) {
                            if(isAdminRole(user->getRole())) {
                                cout << "Cannot change role of Admin user: "<< username <<"\n";
                                validRole = true;
                                break;
                            }
                            userExists = true;
                            break;
                        }
                    }
                    
                    if(!userExists) {
                        cout << "User: " << username << " Not Found\n";
                        cout << "Do you want to try again? (y/n): ";
                        char choice;
                        cin >> choice;
                        if(choice != 'y' && choice != 'Y') {
                            retry = false;
                        }
                    } else {
                        // User exists, proceed to get the new role
                        bool validRole = false;
                        while(!validRole) {
                            cout << "Enter the new role (Instructor/Student): ";
                            cin >> newRole;
                            if (hasExtraInputOnLine()) {
                                cout << "Invalid input: spaces are not allowed in roles.\n";
                                newRole.clear();
                            }

                            if(newRole == "Instructor" || newRole == "Student") {
                                validRole = true;
                                // Assign the new role
                                asignrole(users,db,username,newRole);
                                retry = false;
                            } else {
                                cout << "Invalid role. Please enter either 'Instructor' or 'Student'.\n";
                            }
                        }
                    }
                }
            }
            break;

            case CHANGE_PASSWORD:
            {
                changepassword(users,db);  
            }
            break;

            case CHANGE_MAJOR:
            {
                changeMajor(users,db);
            }
            break;

            case ASSIGN_COURSE:
            {
                assignCourseForInstructor(users,db);
            }
            break;

            case LOCK_UNLOCK_USER:
            {
                while(true){

                int lockChoice;
                cout << "1. Lock User\n";
                cout << "2. Unlock User\n";
                cout << "3. Exit\n";
                cout << "\n";
                cout << "Enter your choice (1 or 2): ";
                cin >> lockChoice;

                while(cin.fail() || lockChoice < 1 || lockChoice > 3 || hasExtraInputOnLine()) {
                    cin.clear();  // clear the error flag
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
                    cout << "Invalid choice. Please enter 1 to Lock, 2 to Unlock, or 3 to Exit: ";
                    cin >> lockChoice;
                }
                if(lockChoice == 1){
                    lockUser(users,db);
                    continue;
                } else if (lockChoice == 2) {
                    unlockUser(users,db);
                } else if (lockChoice == 3){
                        break;
                    } 
                break;
                }
            }
            break;
            case VIEW_USERS:
            {
                ListAll(users,db);
            }
            break;

            case VIEW_LOGS:
            {
                ViewlogMenu(db);
            }
            break;

            case SHOW_PROFILE:
            {
                showprofile(db);
            }
            break;

            case LOGOUT:
            {
                logout(db);
            }
            break;

            default:
                cout<<"Invalid choice, please try again. \n";

        }
    } while (choice != LOGOUT);

}
