#ifndef COURSE_HPP
#define COURSE_HPP

#include <string>

using namespace std;

struct CourseInfo {
    string courseName;
    int creditHours;

    CourseInfo(string name, int credits)
        : courseName(name), creditHours(credits) {}
};

#endif