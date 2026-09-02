 @echo off
setlocal
set "SCRIPT_DIR=%~dp0"

REM Delete the old executable (if it exists)
echo Deleting old executable...
del /f /q "%SCRIPT_DIR%main.exe"

REM Rebuild the project
echo Rebuilding the project...
g++ "%SCRIPT_DIR%main.cpp" "%SCRIPT_DIR%admin.cpp" "%SCRIPT_DIR%Student.cpp" "%SCRIPT_DIR%instructor.cpp" "%SCRIPT_DIR%user.cpp" -o "%SCRIPT_DIR%main.exe"

REM Check if the build was successful
if exist "%SCRIPT_DIR%main.exe" (
    echo Build successful. Running the program...
    "%SCRIPT_DIR%main.exe"
    pause
) else (
    echo Build failed!
)
endlocal
