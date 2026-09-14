@echo off

setlocal

set "SCRIPT_DIR=%~dp0"
set "MYSQL=C:\Program Files\MySQL\MySQL Server 8.0"
set "PATH=%MYSQL%\lib;%PATH%"

echo Deleting old executable...
del /f /q "%SCRIPT_DIR%main.exe"


echo Rebuilding the project...

g++ -std=c++14 "%SCRIPT_DIR%main.cpp" "%SCRIPT_DIR%Database.cpp" "%SCRIPT_DIR%admin.cpp" "%SCRIPT_DIR%Student.cpp" "%SCRIPT_DIR%instructor.cpp" "%SCRIPT_DIR%user.cpp" "%SCRIPT_DIR%password.cpp" "%SCRIPT_DIR%bcrypt.cpp" "%SCRIPT_DIR%blowfish.cpp" -I"%MYSQL%\include" -L"%MYSQL%\lib" -lmysql -o "%SCRIPT_DIR%main.exe"

if %errorlevel% equ 0 (

    echo Build successful. Running the program...

    "%SCRIPT_DIR%main.exe"

    pause

) else (

    echo Build failed!

    pause

)

endlocal