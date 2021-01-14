@echo off
IF %COMPILER%==msvc2019 (
    ctest -V
)
IF %COMPILER%==msys2 (
    ctest -V
)
