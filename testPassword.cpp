#include <iostream>
#include "Password.hpp"

using namespace std;

int main() {

    string hash = Hashpassword("123");
    cout << hash <<endl;

    return 0;


}