#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "user.hpp"  // Include the base class header
#include <string>
#include <vector>

struct CourseGrade {
    std::string courseName;
    int grade;
    int creditHours;

    CourseGrade(const std::string& name = "", int g = -1, int credits = 0)
        : courseName(name), grade(g), creditHours(credits) {}
};

class student : public user {
public:
    student(std::string username, std::string password, std::string role, int ID,string major);

    void login(std::vector<user*>& users) override;
    bool authenticate(const std::string& username, const std::string& password, std::vector<user*>& users);
    void logout() override;
    void showprofile() override;
    void addCourse(std::string courseName, int creditHours = 0);
    bool hasCourse(const std::string& courseName) const;
    void removeCourse(std::string courseName);
    const std::vector<CourseGrade>& getEnrolledCourses() const;
    void viewCourses();
    void showMenu(std::vector<user*>& users);
    std::string getPassword() override;

private:
    std::vector<CourseGrade> enrolledCourses;  // List of courses the student is enrolled in
};

#endif
