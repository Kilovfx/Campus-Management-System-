#include "password.hpp"
#include "PasswordHash.h"

string Hashpassword(string password)
{
    string hashedPassword = bcrypt::generateHash(password);
    return hashedPassword;
}

bool verifypassword(string password,string hash)
{
    return bcrypt::validatePassword(password,hash);
}
