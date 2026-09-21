#ifndef USERLOADER_HPP
#define USERLOADER_HPP

#include <vector>
#include "user.hpp"
#include "Database.hpp"

using namespace std;

vector<user*> loadUsers(Database& db);

#endif