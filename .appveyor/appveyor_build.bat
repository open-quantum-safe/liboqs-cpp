@echo off
IF %COMPILER%==msvc2019 (
    @echo on
    CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    cd %APPVEYOR_BUILD_FOLDER%
    mkdir build
    cd build
    cmake .. -DLIBOQS_INCLUDE_DIR="C:\%LIBOQS_INCLUDE_DIR%" -DLIBOQS_LIB_DIR="C:\%LIBOQS_LIB_DIR%"
    msbuild -verbosity:minimal oqs_cpp.sln
    %APPVEYOR_BUILD_FOLDER%\build\Debug\kem.exe
    %APPVEYOR_BUILD_FOLDER%\build\Debug\rand.exe
    %APPVEYOR_BUILD_FOLDER%\build\Debug\sig.exe
)
IF %COMPILER%==msys2 (
    @echo on
    SET "PATH=C:\msys64\mingw64\bin;%PATH%"
    cd %APPVEYOR_BUILD_FOLDER%
    bash -lc 'ls /c/%LIBOQS_INCLUDE_DIR%'
    mkdir build
    cd build
    bash -lc "cmake .. -DLIBOQS_INCLUDE_DIR=/c/%LIBOQS_INCLUDE_DIR% -DLIBOQS_LIB_DIR=/c/%LIBOQS_LIB_DIR% -GNinja && ninja"
    %APPVEYOR_BUILD_FOLDER%\build\kem.exe
    %APPVEYOR_BUILD_FOLDER%\build\rand.exe
    %APPVEYOR_BUILD_FOLDER%\build\sig.exe
)
