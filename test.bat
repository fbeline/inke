@echo off

echo Running CMake build...
cmake --build build

if errorlevel 1 (
    echo Failed to build project.
    exit /b 1
)

copy .\build\compile_commands.json .

ctest --output-on-failure --test-dir build

exit /b 0
