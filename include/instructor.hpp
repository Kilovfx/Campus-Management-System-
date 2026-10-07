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
    void logout(Database& db) override;
    void showprofile(Database& db) override;
    bool addCourseDatabase(Database &db,student* selectedStudent,string CourseName);
    bool addCourseLogic(Database &db,student* selectedStudent,string CourseName);
    void addCourseMenu(vector<user*>& users,Database &db);

    bool addGradeDatabase(Database &db,student* selectedStudent,string courseName,int grade);
    void addGrade(vector<user*>& users,Database &db); 


    void removeCourse(vector<user*>& users,Database& db);
    bool removeCourseDatabase(Database &db,student* selectedStudent,string courseName);

    void viewStudents(vector<user*>& users);
    void listAllMajors();
    void viewCoursesForMajor(const string& major);

    void viewStudentCourses(vector<user*>& users,Database &db);
    vector<CourseInfo> viewStudentCoursesDatabase(Database &db,student* selectedStudent);

    void showMenu(vector<user*>& users,Database &db);
    void assignCourse(const CourseInfo& course);
    void clearCourses();
    const vector<CourseInfo>& getCourses() const;
    vector<CourseInfo> getCoursesForMajor(Database& db, string major);
    string encryptpass() override;
};

#endif
