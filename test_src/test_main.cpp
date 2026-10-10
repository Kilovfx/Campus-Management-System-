#include <iostream>
#include <gtest/gtest.h>
#include <vector>
#include <cassert>
#include "user.hpp"
#include "admin.hpp"
#include "database.hpp"
#include "UserLoader.hpp"
#include "Password.hpp"
#include "mysql.h"
#include "instructor.hpp"
#include "Course.hpp"
#include "Student.hpp"


class LoadUsersTest : public ::testing::Test {
protected:
    Database db;
    vector<user*> users;

    void SetUp() override {
        ASSERT_TRUE(db.connect("campus_managment_test"));
        users = loadUsers(db);
    }

    void TearDown() override {
        for (auto* u : users) delete u;
        users.clear();
        db.disconnect();
    }
};

TEST_F(LoadUsersTest, LoadsAllSeededUsers) {
    ASSERT_FALSE(users.empty());

    set<string> loaded;

    for (auto* u : users)
    {
    ASSERT_NE(u, nullptr);

    cout << "Loaded username: " << u->getusername() << endl;

        EXPECT_FALSE(u->getusername().empty());
        EXPECT_GT(u->getID(), 0);
        EXPECT_FALSE(u->getpassword().empty());

        const string role = u->getRole();
        EXPECT_TRUE(role == "admin" || role == "Student" || role == "Instructor");

        loaded.insert(u->getusername());
    }

    for (const char* expected : {"ci_admin", "ci_student", "ci_ahmed", "ci_instructor2"}) {
        EXPECT_EQ(loaded.count(expected), 1u) << "missing: " << expected;
    }
}

TEST_F(LoadUsersTest, RolesMatchDynamicType) {
    for (auto* u : users) {
        SCOPED_TRACE("user: " + u->getusername());

        if (u->getRole() == "admin")
            EXPECT_NE(dynamic_cast<admin*>(u), nullptr);
        else if (u->getRole() == "Student")
            EXPECT_NE(dynamic_cast<student*>(u), nullptr);
        else if (u->getRole() == "Instructor")
            EXPECT_NE(dynamic_cast<instructor*>(u), nullptr);
    }
}

TEST(StudentTest,ConvertGradesToGPA){
    student student1(
        "ci_student",
        "456",
        "Student",
        3001,
        "CI",
        "Student",
        "Marketing"
    );

    EXPECT_DOUBLE_EQ(student1.ConvertGradeToGPA(95), 5.0);
    EXPECT_DOUBLE_EQ(student1.ConvertGradeToGPA(90), 4.5);
    EXPECT_DOUBLE_EQ(student1.ConvertGradeToGPA(85), 4.0);
    EXPECT_DOUBLE_EQ(student1.ConvertGradeToGPA(60), 1.5);
    EXPECT_DOUBLE_EQ(student1.ConvertGradeToGPA(59), 0.0);
}
