#ifndef INSTRUCTOR_HPP
#define INSTRUCTOR_HPP

#include "user.hpp"
#include "database.hpp"
#include "Course.hpp"
#include <iostream>
#include <map>
#include <vector>

using namespace std;


class instructor : public user {
    
private:
    vector<CourseInfo> courses; // List of courses the instructor is teaching
public:
    instructor(string username, string password, string role, int ID,
               string first_name, string last_name, string major);
    static const map<string, vector<CourseInfo>>& getCourseCatalog();
    void login(vector<user*>& users,Database &db) override;
    void viewMyCourses(Database& db);
    void logout() override;
    void showprofile() override;
    void addCourse(vector<user*>& users);
    void addGrade(vector<user*>& users); 
    void removeCourse(vector<user*>& users);
    void viewCourses(vector<user*>& users);
    void viewStudents(vector<user*>& users);
    void listAllMajors();
    void viewCoursesForMajor(const string& major);
    void viewStudentCourses(vector<user*>& users);
    void showMenu(vector<user*>& users,Database &db);
    void assignCourse(const CourseInfo& course);
    void clearCourses();
    const vector<CourseInfo>& getCourses() const;
    vector<CourseInfo> getCoursesForMajor(Database& db, string major);
    string encryptpass() override;
};

#endif
