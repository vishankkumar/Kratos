@echo off
rem Please do not modify this script

rem For any question please contact with us in:
rem  - https://github.com/KratosMultiphysics/Kratos

rem Optional parameters:
rem You can find a list will all the compiation options in INSTALL.md or here:
rem  - https://github.com/KratosMultiphysics/Kratos/wiki/Compilation-options

rem Set compiler
set CC=cl.exe
set CXX=cl.exe

rem Set variables
set KRATOS_SOURCE=~0,-1%/..
set KRATOS_BUILD=%KRATOS_SOURCE%/build
set KRATOS_APP_DIR=applications

rem Set basic configuration
if not defined KRATOS_BUILD_TYPE set KRATOS_BUILD_TYPE=Release
if not defined BOOST_ROOT set BOOST_ROOT=C:\Users\s4618455\Documents\KRATOS\boost_1_71_0
if not defined PYTHON_EXECUTABLE set PYTHON_EXECUTABLE=C:\Users\s4618455\AppData\Local\Programs\Python\Python38\python.exe

rem Set applications to compile
set KRATOS_APPLICATIONS=
#CALL :add_app %KRATOS_APP_DIR%\ExternalSolversApplication;
#CALL :add_app %KRATOS_APP_DIR%\StructuralMechanicsApplication;
#CALL :add_app %KRATOS_APP_DIR%\FluidDynamicsApplication;

rem Clean
del /F /Q "%KRATOS_BUILD%\%KRATOS_BUILD_TYPE%\cmake_install.cmake"
del /F /Q "%KRATOS_BUILD%\%KRATOS_BUILD_TYPE%\CMakeCache.txt"
del /F /Q "%KRATOS_BUILD%\%KRATOS_BUILD_TYPE%\CMakeFiles"

rem Configure
@echo on
cmake -G"Visual Studio 16 2019" -H"%KRATOS_SOURCE%" -B"%KRATOS_BUILD%\%KRATOS_BUILD_TYPE%"          ^
-DINCLUDE_FEAST=OFF                                                                                 ^
-DLAPACK_LIBRARIES="C:\Users\s4618455\Documents\KRATOS\external_libraries\liblapack.lib"                                         ^
-DBLAS_LIBRARIES="C:\Users\s4618455\Documents\KRATOS\external_libraries\libblas.lib"
-DKRATOS_BUILD_TESTING=OFF
-DCMAKE_INSTALL_PREFIX="C:\Users\s4618455\Documents\KRATOS_INSTALL"

rem Build
cmake --build "%KRATOS_BUILD%/%KRATOS_BUILD_TYPE%" --target install -- /property:configuration=%KRATOS_BUILD_TYPE% /p:Platform=x64
goto:eof

rem Function to add apps
:add_app
set KRATOS_APPLICATIONS=%KRATOS_APPLICATIONS%%1;
goto:eof
