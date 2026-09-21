#ifndef PASSWORD_HASH_H
#define PASSWORD_HASH_H

#include <string>

namespace bcrypt {

    std::string generateHash(const std::string & password , unsigned rounds = 10 );

    bool validatePassword(const std::string & password, const std::string & hash);

}

#endif // PASSWORD_HASH_H
