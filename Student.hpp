#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "user.hpp"  // Include the base class header
#include <string>
#include <vector>
#include <cmath>


struct CourseGrade {
    std::string courseName;
    double grade;
    double creditHours;

    CourseGrade(const std::string& name = "", double g = -1, double credits = 0)
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
    std::vector<CourseGrade>& getEnrolledCourses();
    const std::vector<CourseGrade>& getEnrolledCourses() const;
    void ShowGrades();
    void viewCourses();
    void showMenu(std::vector<user*>& users);
    std::string encryptpass() override;

    double ConvertGradeToGPA(double grade); 

private:
    std::vector<CourseGrade> enrolledCourses;  // List of courses the student is enrolled in
};

#endif
