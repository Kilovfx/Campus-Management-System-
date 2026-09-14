#include "user.hpp"
#include <iostream>
#include <map>
#include <vector>

using namespace std;

struct CourseInfo {
    string courseName;
    int creditHours;

    CourseInfo(const string& name = "", int credits = 0)
        : courseName(name), creditHours(credits) {}
};

class instructor : public user {
    
private:
    vector<CourseInfo> courses; // List of courses the instructor is teaching
public:
    instructor(string username, string password, string role, int ID,
               string first_name, string last_name, string major);
    static const map<string, vector<CourseInfo>>& getCourseCatalog();
    void login(vector<user*>& users) override;
    bool authenticate(const string& username, const string& password, vector<user*>& users);
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
    void showMenu(vector<user*>& users);
    void assignCourse(const CourseInfo& course);
    void clearCourses();
    const vector<CourseInfo>& getCourses() const;
    string encryptpass() override;
};
