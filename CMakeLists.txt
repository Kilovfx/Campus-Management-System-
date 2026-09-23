#minumem version to run ==
cmake_minimum_required(VERSION 3.15)

#project name ==
project(CampusManagement LANGUAGES CXX)

#set C++ version to run
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#set MySQL path
set(MYSQL_DIR "C:/Program Files/MySQL/MySQL Server 8.0")


#find mysql.h and (libmysql.lib or mysql.lib)
find_path(

    MYSQL_INCLUDE_DIR
    mysql.h
    PATHS "${MYSQL_DIR}/include"

)


find_library(

    MYSQL_LIBRARY
    NAMES mysql libmysql
    PATHS "${MYSQL_DIR}/lib"

)

# Windows links against the import library above, but still needs the
# corresponding DLL at runtime.  Copy it next to each executable so running
# from the build directory does not depend on the user's PATH.
if(WIN32)
    find_file(
        MYSQL_RUNTIME_LIBRARY
        NAMES libmysql.dll
        PATHS "${MYSQL_DIR}/lib" "${MYSQL_DIR}/bin"
        NO_DEFAULT_PATH
    )
endif()

if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL was not found")
endif()

if(WIN32 AND NOT MYSQL_RUNTIME_LIBRARY)
    message(FATAL_ERROR "MySQL runtime DLL (libmysql.dll) was not found")
endif()


# Shared source files
set(

    COMMON_SOURCES
    src/database.cpp
    src/UserLoader.cpp
    src/Password.cpp
    src/PasswordHash.cpp
    src/blowfish.cpp
    src/user.cpp
    src/admin.cpp
    src/instructor.cpp 
    src/Student.cpp 

)



# -----------------------------
# Main Application - Start
# -----------------------------

add_executable(

    Campus_app
    src/main.cpp
    ${COMMON_SOURCES}

)

target_include_directories(

    Campus_app
    PRIVATE

    include
    ${MYSQL_INCLUDE_DIR}

)

target_link_libraries(

    Campus_app
    PRIVATE
    ${MYSQL_LIBRARY}

)

if(WIN32)
    add_custom_command(TARGET Campus_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MYSQL_RUNTIME_LIBRARY}"
                "$<TARGET_FILE_DIR:Campus_app>/libmysql.dll"
    )
endif()

# -----------------------------
# Main Application -- END
# -----------------------------



# -----------------------------
# Test Application -- Start
# -----------------------------
add_executable(

    Test_app
    test_src/test_main.cpp
    ${COMMON_SOURCES}

)


target_include_directories(

    Test_app
    PRIVATE
    
    include
    ${MYSQL_INCLUDE_DIR}
)

target_link_libraries(

    Test_app
    PRIVATE
    ${MYSQL_LIBRARY}
)

if(WIN32)
    add_custom_command(TARGET Test_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MYSQL_RUNTIME_LIBRARY}"
                "$<TARGET_FILE_DIR:Test_app>/libmysql.dll"
    )
endif()

# -----------------------------
# Test Application -- END
# -----------------------------
