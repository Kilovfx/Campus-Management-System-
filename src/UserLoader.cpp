#include "UserLoader.hpp"
#include "admin.hpp"
#include "instructor.hpp"
#include "Student.hpp"
#include <algorithm>
#include <cctype>

static string normalizeRole(string role) {
    const auto first = role.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }

    role = role.substr(first, role.find_last_not_of(" \t\r\n") - first + 1);
    transform(role.begin(), role.end(), role.begin(),
        [](unsigned char character) {
            return static_cast<char>(tolower(character));
        });

    if (role == "admin") {
        return "admin";
    }
    if (role == "student") {
        return "Student";
    }
    if (role == "instructor") {
        return "Instructor";
    }
    return "";
}

vector<user*> loadUsers(Database& db) {
    vector<user*> users;

    if (db.getConnection() == nullptr) {
        return users;
    }

    MYSQL_RES* userResult = db.executeSelect("SELECT * FROM users");
    if (userResult == nullptr) {
        return users;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(userResult)) != nullptr) {
        if (row[0] == nullptr || row[1] == nullptr ||
            row[2] == nullptr || row[3] == nullptr) {
            continue;
        }

        const int id = stoi(row[0]);
        const string username = row[1];
        const string password = row[2];
        const string role = normalizeRole(row[3]);

        if (role == "admin") {
            users.push_back(new admin(username, password, role, id));
            continue;
        }

        if (role == "Student") {
            string firstName;
            string lastName;
            string majorName;

            const string profileQuery =
                "SELECT s.first_name, s.last_name, m.major_name "
                "FROM students s "
                "LEFT JOIN majors m ON s.major_id = m.major_id "
                "WHERE s.student_id = " + to_string(id);
            MYSQL_RES* profileResult = db.executeSelect(profileQuery);

            if (profileResult != nullptr) {
                MYSQL_ROW profileRow = mysql_fetch_row(profileResult);
                if (profileRow != nullptr) {
                    firstName = profileRow[0] == nullptr ? "" : profileRow[0];
                    lastName = profileRow[1] == nullptr ? "" : profileRow[1];
                    majorName = profileRow[2] == nullptr ? "" : profileRow[2];
                }
                mysql_free_result(profileResult);
            }

            student* loadedStudent = new student(
                username, password, role, id, firstName, lastName, majorName
            );
            users.push_back(loadedStudent);

            const string courseQuery =
                "SELECT c.course_name, c.credit_hours, sc.grade "
                "FROM student_courses sc "
                "INNER JOIN courses c ON sc.course_id = c.course_id "
                "WHERE sc.student_id = " + to_string(id);
            MYSQL_RES* courseResult = db.executeSelect(courseQuery);

            if (courseResult != nullptr) {
                MYSQL_ROW courseRow;
                while ((courseRow = mysql_fetch_row(courseResult)) != nullptr) {
                    if (courseRow[0] == nullptr || courseRow[1] == nullptr) {
                        continue;
                    }

                    const int grade = courseRow[2] == nullptr
                        ? -1
                        : stoi(courseRow[2]);
                    loadedStudent->loadCourse(
                        courseRow[0], grade, stoi(courseRow[1])
                    );
                }
                mysql_free_result(courseResult);
            }
            continue;
        }

        if (role == "Instructor") {
            string firstName;
            string lastName;
            string majorName;

            const string profileQuery =
                "SELECT i.first_name, i.last_name, m.major_name "
                "FROM instructors i "
                "LEFT JOIN majors m ON i.major_id = m.major_id "
                "WHERE i.instructor_id = " + to_string(id);
            MYSQL_RES* profileResult = db.executeSelect(profileQuery);

            if (profileResult != nullptr) {
                MYSQL_ROW profileRow = mysql_fetch_row(profileResult);
                if (profileRow != nullptr) {
                    firstName = profileRow[0] == nullptr ? "" : profileRow[0];
                    lastName = profileRow[1] == nullptr ? "" : profileRow[1];
                    majorName = profileRow[2] == nullptr ? "" : profileRow[2];
                }
                mysql_free_result(profileResult);
            }

            instructor* loadedInstructor = new instructor(
                username, password, role, id, firstName, lastName, majorName
            );
            users.push_back(loadedInstructor);

            const string courseQuery =
                "SELECT c.course_name, c.credit_hours "
                "FROM instructor_courses ic "
                "INNER JOIN courses c ON ic.course_id = c.course_id "
                "WHERE ic.instructor_id = " + to_string(id);
            MYSQL_RES* courseResult = db.executeSelect(courseQuery);

            if (courseResult != nullptr) {
                MYSQL_ROW courseRow;
                while ((courseRow = mysql_fetch_row(courseResult)) != nullptr) {
                    if (courseRow[0] == nullptr || courseRow[1] == nullptr) {
                        continue;
                    }

                    loadedInstructor->assignCourse(
                        CourseInfo(courseRow[0], stoi(courseRow[1]))
                    );
                }
                mysql_free_result(courseResult);
            }
        }
    }

    mysql_free_result(userResult);
    return users;
}
