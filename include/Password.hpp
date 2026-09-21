#ifndef PASSWORD_HPP
#define PASSWORD_HPP


#include <string>

using namespace std;

string Hashpassword(string password);
bool verifypassword(string password,string hash);



#endif