#ifndef COURSE_HPP
#define COURSE_HPP

#include <string>

struct CourseInfo {
    std::string courseName;
    int creditHours;

    CourseInfo(const std::string& name = "", int credits = 0)
        : courseName(name), creditHours(credits) {}
};

#endif
