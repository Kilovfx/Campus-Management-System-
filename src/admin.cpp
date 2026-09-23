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
        "Civil Engineering",
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

bool admin::authenticate(string username,string password){
    return (username == this->username && verifypassword(password,this->password)); // check for the credentials
}

void admin::showprofile(){
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
void admin::login(vector<user*>& users){
    
    string username, password;
    int attempts = 0;
    const int maxAttempts = 3;
    const int waitTime = 10;

    while (true) {
        cout << "enter the username: ";
        cin >> username;
        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            username.clear();
        }

        cout << "enter the password: ";
        password = encryptpass();

        user* account = nullptr;
        for (auto* userAccount : users) {
            if (userAccount->getusername() == username &&
                userAccount->getRole() == "admin") {
                account = userAccount;
                break;
            }
        }

        if (account != nullptr &&
            verifypassword(password, account->getpassword())) {
            this->username = username;
            cout << "User : " << username << " logged in successfully!\n";
            addLog("Admin logged in");
            return;
        }

        attempts++;

        if(attempts >= maxAttempts) {
            cout << "You have made " << attempts << " incorrect attempts. Please wait for " << waitTime << " seconds...\n";
            this_thread::sleep_for(chrono::seconds(waitTime));
            attempts = 0;
            cout << "You can now try again.\n";
        }

        cout << "Invalid credentials. Please try again.\n";
    }
}




void admin::ViewAllLog(vector<user*>& users){
    for(auto& user : users){
        user->veiwLog();
        cout<<"\n";
    }
}


void admin::ListAll(vector<user*>& users){
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
    addLog("Listed all users");
}



//message logout for the admin
void admin::logout(){
    cout <<"Admin: " << username <<" logged out!";
    addLog("Admin logged out."); // count the logs for the admin 
}


void admin::lockUser(vector<user*>& users) {
    string lockedusername;
    int minute;
    ListAll(users);
    
    while (true) {
        cout << "Input the user you want to lock: ";
        cin >> lockedusername;
        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            continue;
        }

        user* selectedUser = nullptr;
        for (auto u : users) {
            if (u->getusername() == lockedusername) {
                selectedUser = u;
                break;
            }
        }

        if (selectedUser == nullptr) {
            cout << "User not found.\n";
            continue;
        }

        if (selectedUser == this || isAdminRole(selectedUser->getRole())) {
            cout << "Cannot lock an admin account.\n";
            continue;
        }

        if (!selectedUser->isActive()) {
            cout << "Account already locked.\n";
            continue;
        }

        cout << "Enter lock duration in minutes: ";
        cin >> minute;
        while (cin.fail() || minute <= 0 || hasExtraInputOnLine()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid duration. Enter a positive number of minutes: ";
            cin >> minute;
        }

        selectedUser->lockAccount(minute);
        cout << "User " << lockedusername << " has been locked.\n";
        addLog("Locked user: " + lockedusername);
        break;
    }
} 


void admin::unlockUser(vector<user*>& users) {
    string unlockedusername;

    ListAll(users);

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

        if (selectedUser->isActive()) {
            cout << "Account already unlocked!\n";
            continue;
        }

        selectedUser->resetFailedAttempts();

        cout << "User " << unlockedusername << " has been unlocked.\n";
        addLog("Unlocked user: " + unlockedusername);

        break;
    }
}



void admin::createuser(std::vector<user*>& users,Database& db, std::string username, std::string password,
                       std::string role, std::string first_name, std::string last_name,
                       std::string major) {
    user* newUser = nullptr;
    string hashedPassword = Hashpassword(password);
    int userID = -1;

    try {
        auto isBlank = [](const string& value) {
            return value.empty() || all_of(value.begin(), value.end(), [](unsigned char character) {
                return isspace(character) != 0;
            });
        };

        if (isBlank(first_name) || isBlank(last_name)) {
            cout << "Error: First name and last name cannot be empty.\n";
            addLog("Failed to create user: " + username + " - Name is empty");
            return;
        }

        // Check if username already exists
        for (const auto& user : users) {
            if (user->getusername() == username) {
                std::cout << "Error: Username '" << username << "' already exists. Please use a different username.\n";
                addLog("Failed to create user: " + username + " - Username already exists");
                return;
            }
        }

        const string usernameQuery =
            "SELECT user_id FROM users WHERE username='" +
            db.escapeString(username) + "' LIMIT 1";
        MYSQL_RES* usernameResult = db.executeSelect(usernameQuery);
        if (usernameResult == nullptr) {
            cout << "Failed to check whether the username already exists.\n";
            addLog("Failed to create user: " + username + " - Username lookup failed");
            return;
        }

        const bool usernameExists = mysql_num_rows(usernameResult) > 0;
        mysql_free_result(usernameResult);
        if (usernameExists) {
            cout << "Error: Username '" << username
                 << "' already exists. Please use a different username.\n";
            addLog("Failed to create user: " + username + " - Username already exists");
            return;
        }

        if (role == "Student" || role == "Instructor") {
            if (!isMajorAllowed(major)) {
                cout << "Invalid major. Please choose from the list below:\n";
                printAllowedMajors();
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

            string userQuery = 
            "INSERT INTO users "
            "(username,password_hash,role,major_id) VALUES('" +
                db.escapeString(username) + "','" +
                db.escapeString(hashedPassword) + "','" +
                role + "'," +
                to_string(majorID) + 
        
            ")" ;

            if(!db.executeQuery(userQuery)){
                cout <<"Failed creating user . \n";
                return;
            }

            // The ID is generated by this INSERT, so read it only afterwards.
            userID = db.getLastInsertID();
            if (userID < 0) {
                cout << "Failed to retrieve the new user's ID.\n";
                return;
            }

            /*
            STEP 3
            Insert into student/instructor table
             */


            if (role == "Student"){

                string studentQuery = 
                "INSERT INTO students"
                "(student_id,first_name,last_name,major_id) VALUES(" +
                to_string(userID) + ",'" +
                db.escapeString(first_name) + "','" +
                db.escapeString(last_name) + "',"  +
                to_string(majorID) + 
        
            ")";
            if(!db.executeQuery(studentQuery)){
                cout << "Failed creating student profile!\n";
                return;
            }

            newUser = new student(username,hashedPassword,role,userID,first_name,last_name,major);

            }
            if(role == "Instructor"){

                string instructorQuery = 
                "INSERT INTO instructors"
                "(instructor_id,first_name,last_name,major_id) VALUES('" +
                to_string(userID) + "','" +
                db.escapeString(first_name) + "','" +
                db.escapeString(last_name) + "','"  +
                to_string(majorID) + 
        
            ")";

             if(!db.executeQuery(instructorQuery)){
                cout << "Failed creating instructor profile.\n";
                return;
            }

                newUser = new instructor(username,hashedPassword,role,userID,first_name,last_name,major);

            }

        } else {
            cout << "Invalid role. Please enter either Student or Instructor.\n";
            return;
        }

        if (newUser == nullptr) {
            cout << "Failed to create the user profile.\n";
            return;
        }

        /*
            STEP 4
            Add object to memory
            */


        users.push_back(newUser);  // Add the new user to the vector
        std::cout << "User " << username << " with role " << role << " created successfully with ID: " << userID << "\n";
        addLog("Created user: " + username + " with role: " + role);
    }

    catch (const std::bad_alloc& e) {
        std::cout << "Memory allocation failed: " << e.what() << std::endl;
 }
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
        addLog("Attempted to delete own admin account (prevented)");
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
        addLog("Cancelled deletion of user: " + selectedUsername);
        return;
    }


    
    /*
        Delete profile table first
        because it has foreign key to users
    */

    if(selectedRole == "Student"){
        string query = 
        "DELETE FROM students WHERE student_id ="
        + to_string(selectedID);

        if(!db.executeQuery(query)){
            cout <<"Failed deleting student profile.\n";
            return;
        }
    }

    else if (selectedRole == "Instructor"){
        string query = 
        "DELETE FROM instructors WHERE instructor_id ="
        + to_string(selectedID);

        if(!db.executeQuery(query)){
            cout <<"Failed deleting instructor profile.\n";
            return;          
        }
    }

    // Delete from users table
    string userQuery =
    "DELETE FROM users WHERE user_id="
    + to_string(selectedID);
        if(!db.executeQuery(userQuery)){
            cout <<"Failed deleting user profile.\n";
            return;
        }

     // Delete from vector
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
    } else {
        cout <<"User: " << selectedUsername << "| ID: " << selectedID << " | Role: " << selectedRole << " has been deleted successfully! \n";
    }
}





void admin::asignrole(vector<user*>& users, Database& db, string username, string newRole){

    auto it = find_if(users.begin(), users.end(), [&username](user* selectedUser) {
        return selectedUser->getusername() == username;
    });

    if (it == users.end()) {
        cout << "User: " << username << " Not Found\n";
        return;
    }

    user* selectedUser = *it;

    if (isAdminRole(selectedUser->getRole())) {
        cout << "Cannot change role of Admin user: " << username << "\n";
        return;
    }

    if (selectedUser->getRole() == newRole) {
        cout << "User: " << username << " already has the role: " << newRole << "\n";
        return;
    }

    const string query =
        "UPDATE users SET role='" + db.escapeString(newRole) +
        "' WHERE user_id=" + to_string(selectedUser->getID());

    if (!db.executeQuery(query)) {
        cout << "Failed to update the role of user: " << username << "\n";
        return;
    }

    selectedUser->setRole(newRole);
    cout << "Role of user: " << username << " has been updated to: " << newRole << "\n";
    addLog("Changed role for user " + username + " to " + newRole);
}

void admin::changepassword(vector<user*>& users,Database& db){
    // List all users first
    ListAll(users);
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
                    addLog("Cancelled password change operation for non-existent user: " + targetUsername);
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
        addLog("Failed password change attempt for user: " + targetUsername + " (invalid admin password)");
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
            addLog("Failed database password update for user: "
                   + targetUsername);
            return;

        }

        targetUser->setPassword(hashedpassword);

        cout << "Password for user "
             << targetUsername
             << " has been changed successfully.\n";

        addLog("Changed password for user: " + targetUsername);
        break;
        
    }
}

void admin::changeMajor(vector<user*>& users){
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

    size_t removedCourseCount = 0;
    if (selectedRole == "Instructor") {
        instructor* selectedInstructor = dynamic_cast<instructor*>(selectedUser);
        if (selectedInstructor != nullptr) {
            removedCourseCount = selectedInstructor->getCourses().size();
            selectedInstructor->clearCourses();
        }
    }

    selectedUser->setMajor(newMajor);
    cout << "Major updated successfully for " << selectedUser->getusername() << "\n";
    addLog("Changed major for " + selectedRole + " " +
           selectedUser->getusername() + " to " + newMajor);
    if (removedCourseCount > 0) {
        addLog("Removed " + to_string(removedCourseCount) +
               " assigned course(s) from instructor " +
               selectedUser->getusername() + " after major change");
    }
}


void admin::assignCourseForInstructor(vector<user*>& users)
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

        const auto& catalog =
            instructor::getCourseCatalog();

    auto majorIt = catalog.find(selectedInstructor->getMajor());

        if (majorIt == catalog.end())
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

        for (int i = 0; i < majorIt->second.size(); i++)
        {
            cout << i + 1 << ". "
                << majorIt->second[i].courseName
                << " (" << majorIt->second[i].creditHours
                << " credits)\n";
        }

        int courseChoice;

        while (true)
        {
            cout << "\nSelect a course (1-"
                 << majorIt->second.size()
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
                courseChoice > static_cast<int>(majorIt->second.size()))
            {
                cout << "Invalid choice. Please choose a course from the list.\n";
                continue;
            }

            CourseInfo selectedCourse =
                majorIt->second[courseChoice - 1];

            selectedInstructor->assignCourse(selectedCourse);
            addLog("Assigned course " + selectedCourse.courseName +
                   " to instructor " + selectedInstructor->getusername());
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
                    ListAll(users);
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
                                user* selectedUser = nullptr;
                                for (auto* user : users) {
                                    if (user->getusername() == username) {
                                        selectedUser = user;
                                        break;
                                    }
                                }

                                const bool sameRole =
                                    selectedUser != nullptr &&
                                    selectedUser->getRole() == newRole;

                                asignrole(users, db, username, newRole);

                                if (sameRole) {
                                    cout << "Do you want to try again? (y/n): ";
                                    char choice;
                                    cin >> choice;
                                    if (choice != 'y' && choice != 'Y') {
                                        retry = false;
                                    }
                                } else {
                                    retry = false;
                                }
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
                changeMajor(users);
            }
            break;

            case ASSIGN_COURSE:
            {
                assignCourseForInstructor(users);
            }
            break;

            case LOCK_UNLOCK_USER:
            {
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
                    lockUser(users);
                } else if (lockChoice == 2) {
                    unlockUser(users);
                }
                break;
            }
            case VIEW_USERS:
            {
                ListAll(users);
            }
            break;

            case VIEW_LOGS:
            {
                ViewAllLog(users);
            }
            break;

            case SHOW_PROFILE:
            {
                showprofile();
            }
            break;

            case LOGOUT:
            {
                logout();
            }
            break;

            default:
                cout<<"Invalid choice, please try again. \n";

        }
    } while (choice != LOGOUT);
}
