@echo off
IF %COMPILER%==msvc2019 (
    %APPVEYOR_BUILD_FOLDER%\unit_tests\build\tests\Debug\oqs_cpp_testing.exe
)
IF %COMPILER%==msys2 (
    %APPVEYOR_BUILD_FOLDER%\unit_tests\build\tests\oqs_cpp_testing.exe
)
