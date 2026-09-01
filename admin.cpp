#include "admin.hpp"
#include "Student.hpp"
#include "instructor.hpp" 
#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
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

int admin::nextID = 3170;

admin::admin(string username, string password, string role, int ID) : user(username, password, role, nextID, "",false) {
    this->username = username;
    this->password = password;
    this->role = role;
    this->ID = nextID;
    nextID++;
}

bool admin::authenticate(string username,string password){
    return (username == this->username && password == this->password); // check for the credentials
}

void admin::showprofile(){
    cout<<"\nAdmin Profile\n";
    cout << "Username: " << username << endl;
    cout << "ID: " << nextID << endl;
    cout << "Role: "<< role << endl;

    // Example: Display current date (or the date of account creation if available)
    time_t now = time(0);
    char* dt = ctime(&now);
    cout << "Account created on: " << dt << endl;

}




string admin::getPassword() {
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
    
    string username,password;
    int attempts = 0;
    const int maxAttempts = 3;
    const int waitTime = 10;
    
    cout<<"enter the username: ";
    cin>>username;
    if (hasExtraInputOnLine()) {
        cout << "Invalid input: spaces are not allowed in usernames.\n";
        username.clear();
    }
    cout<<"enter the password: ";
    password = getPassword();
    
    while(!authenticate(username,password)){
        attempts++;
        
        if(attempts >= maxAttempts) {
            cout << "You have made " << attempts << " incorrect attempts. Please wait for " << waitTime << " seconds...\n";
            this_thread::sleep_for(chrono::seconds(waitTime));
            attempts = 0;
            cout << "You can now try again.\n";
        }
        
        cout<<"Invalid credentials. Please try again.\n";
        cout<<"enter the username: ";
        cin>>username;
        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            username.clear();
        }
        cout<<"enter the password: ";
        password = getPassword();
    }

    if(authenticate(username,password)){
    cout << "User : " << username <<" logged in successfully!\n";
    this-> username = username;
    this-> password = password;
    addLog("Admin logged in"); // count the logs for the admin 
    }
    else{
        cout<<"Invalid credentials. Exiting program.\n";
        exit(1);
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

        if(user->getRole() == "Student"){
            string major = user->getMajor();

            cout << " | Major: [" << major << "]";
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
    cout << "input the user you want to unlock : ";
    cin >> unlockedusername;

    if(isAdminRole(unlockedusername)) {
        cout << "Cannot unlock admin account!\n";
        if (hasExtraInputOnLine()) {
            cout << "Invalid input: spaces are not allowed in usernames.\n";
            unlockedusername.clear();
        }
    }

    user* selectedUser = nullptr;
    for (auto u : users) {
        if (u->getusername() == unlockedusername) {
            selectedUser = u;
            if(selectedUser->isActive()){
                cout <<" account already unlocked!\n";
            }
            break;
        }
    }

    if (selectedUser == nullptr) {
        cout << "User not found.\n";
    } else {
        if (!selectedUser->isActive()) {
            selectedUser->resetFailedAttempts(); // Reset failed attempts
            cout << "User " << unlockedusername << " has been unlocked.\n";
            addLog("Unlocked user: " + unlockedusername);
        } 
    }
}



void admin::createuser(std::vector<user*>& users, std::string username, std::string password, std::string role, std::string major) {
    user* newUser = nullptr;

    try {
        // Check if username already exists
        for (const auto& user : users) {
            if (user->getusername() == username) {
                std::cout << "Error: Username '" << username << "' already exists. Please use a different username.\n";
                addLog("Failed to create user: " + username + " - Username already exists");
                return;
            }
        }

        if (role == "Instructor") {
            newUser = new instructor(username, password, role, nextID, major);
            nextID++;
        } 

        else if (role == "Student") {
            if (!isMajorAllowed(major)) {
                cout << "Invalid major. Please choose from the list below:\n";
                printAllowedMajors();
                return;
            }
            newUser = new student(username, password, role, nextID, major);
            nextID++;
        } 

        else {
            std::cout << "Invalid role specified\n";
            return;
        }
        

        users.push_back(newUser);  // Add the new user to the vector
        std::cout << "User " << username << " with role " << role << " created successfully with ID: " << newUser->getID() << "\n";
        addLog("Created user: " + username + " with role: " + role);
    }

    catch (const std::bad_alloc& e) {
        std::cout << "Memory allocation failed: " << e.what() << std::endl;
 }
}





//function for deleting user by the admin using the khaled method [vector + iterator techneique]
void admin::deleteuser(vector<user*>& users,string username,string password){
    // List all users
    cout << "\n--- List of Users ---\n";
    static int userCount = 0;
    for (const auto& user : users) {
        userCount++;
        cout << userCount << ". Username: " << user->getusername() << " | Role: " << user->getRole() << " | ID: " << user->getID() << endl;
    }
    
    
    if (users.empty()) {
        cout << "No users found in the system.\n";
        return;
    }
    
    // Ask user to choose which user to delete
    int choice;
    cout << "\nEnter the number of the user to delete (1-" << userCount << "): ";
    cin >> choice;

    while(cin.fail() || choice < 1 || choice > userCount || hasExtraInputOnLine()) {
        if(cin.fail()) {
            cin.clear();  // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
        }
        cout << "Invalid choice. Please enter a number between 1 and " << userCount << ": ";
        cin >> choice;
    }
    
    // Get the selected user
    string selectedUsername = users[choice - 1]->getusername();
    
    // Prevent deleting the currently logged-in admin
    if (selectedUsername == this->username) {
        cout << "Cannot delete the currently logged-in admin account.\n";
        addLog("Attempted to delete own admin account (prevented)");
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
    
    // Delete the user
    auto it = users.begin();
    bool found = false;
    while(it < users.end()) {
        if((*it)->getusername() == selectedUsername) {
            delete *it;
            users.erase(it);
            cout << "User: " << selectedUsername << " Deleted Successfully\n";
            addLog("Deleted user: " + selectedUsername);
            found = true;
            break;
        } else {
            ++it;
        }
    }
    
    if(!found) {
        cout << "User: " << selectedUsername << " Not Found\n";
        return;
    }
}





void admin::asignrole(vector<user*>& users,string username,string newRole){

    auto itCheck = users.begin();
    while (itCheck != users.end()){
        if ((*itCheck)->getusername() == username){
            if((*itCheck)->getRole() == newRole){
                cout << "User: "<< username <<" already has the role: "<< newRole <<"\n";
                return;
            }
            break;
        }
        ++itCheck;
    }

    auto itAdmin = users.begin();
    while(itAdmin != users.end()){
        if((*itAdmin)->getusername() == username) {
            if(isAdminRole((*itAdmin)->getRole())){
                cout << "Cannot change role of Admin user: "<< username <<"\n";
                return;
            }
            break;
        }
        ++itAdmin;
    }

    auto it = users.begin();
    bool found = false;
    while (it != users.end()){
        if ((*it)->getusername() == username){
            (*it)->setRole(newRole);
            cout <<"Role of user: "<< username << " has been updated to : "<< newRole<<"\n";
            found = true;
            addLog("Assigned new role to user: " + username);
            break;
        }
        ++it;
    }
    if(!found){
        cout << "User : "<< username <<"Not Found\n";
    }
}

void admin::changepassword(vector<user*>& users){
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
            if(retry != 'y' && retry != 'Y') {
                addLog("Cancelled password change operation for non-existent user: " + targetUsername);
                return;
            }
        } else {
            userFound = true;
        }
    }
    
    // Verify admin's own password first
    string adminPassword;
    cout << "Enter your admin password to confirm: ";
    adminPassword = getPassword();
    
    if(adminPassword != this->password) {
        cout << "Invalid password!\n";
        addLog("Failed password change attempt for user: " + targetUsername + " (invalid admin password)");
        return;
    }
    
    // Now change the target user's password
    string newpassword, confirmpassword;
    while (true)
    {
        cout << "Enter the new password for " << targetUsername << ": ";
        newpassword = getPassword();

        if (newpassword == targetUser->getpassword()) {
            cout << "New password cannot be the same as the current password. Please try again.\n";
            continue;
        }

        cout << "Enter again to confirm: ";
        confirmpassword = getPassword();
        
        if(newpassword == confirmpassword){
            targetUser->setPassword(newpassword);
            cout << "Password for user " << targetUsername << " has been changed successfully\n";
            addLog("Changed password for user: " + targetUsername);
            break;
        }
        else{
            cout << "Passwords don't match! Please try again.\n";
        }
    }
}

void admin::changeMajor(vector<user*>& users){
    // List students only
    cout << "\n--- List of Students ---\n";
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
    cout << "\nSelect a student to change major (1-" << studentCount << "): ";
    cin >> choice;

    while (cin.fail() || choice < 1 || choice > studentCount || hasExtraInputOnLine()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 1 and " << studentCount << ": ";
        cin >> choice;
    }

    student* selectedStudent = studentList[choice - 1];

    cin.ignore();
    printAllowedMajors();
    string newMajor;
    const string currentMajor = selectedStudent->getMajor();
    while (true) {
        cout << "Enter the new major for " << selectedStudent->getusername() << ": ";
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

    selectedStudent->setMajor(newMajor);
    cout << "Major updated successfully for " << selectedStudent->getusername() << "\n";
    addLog("Changed major for student: " + selectedStudent->getusername());
}

void admin::Showmeniu(vector<user*>& users){
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
        if(cin.fail() || choice < 1 || choice > 10 || hasExtraInputOnLine()) {
            cin.clear();  // clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // ignore the invalid input
            cout << "Invalid input! Please enter a number between 1 and 10.\n";
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

            string username,password,role,major;
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

            if (role == "Student") {
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
            
            createuser(users,username,password,role,major);

            }
            break;

            case DELETE_USER: 
            {
                deleteuser(users, "", "");
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
                                asignrole(users,username,newRole);
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
                changepassword(users);  
            }
            break;

            case CHANGE_MAJOR:
            {
                changeMajor(users);
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
