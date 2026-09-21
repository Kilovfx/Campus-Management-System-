#include <iostream>
#include <vector>

#include "user.hpp"
#include "database.hpp"
#include "UserLoader.hpp"
#include "Password.hpp"
#include "mysql.h"

using namespace std;

int main()
{
    Database db;
    vector<user*> users;

    if(!db.connect()){
        cout<<"Failed connection . . .\n";
    }

    cout<<"Database connected successfully\n";
    users = loadUsers(db);
    cout << "Loaded " << users.size() << " users.\n";
    for(auto* loadedusers : users){
        cout << "Loaded username: " << loadedusers->getusername()
        << " | role: " << loadedusers->getRole() << "\n";
    }

    return 0;
}




string testpasswordhash(string password){
    string hash = Hashpassword("123");
    cout << hash <<endl;
    return hash;

}