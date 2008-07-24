@echo off
rem Warning: environment is set for Win32 platform.

set PROJECT_TYPE=vs8

pushd ..
call set_env.bat
popd

if "%EXT_DIR%" == "" goto END

rem Setup common Win32 environment variables

set SolutionDirectory=%cd%
set DebugOutputDirectory=%SolutionDirectory%\out
set ReleaseOutputDirectory=%SolutionDirectory%\out
set DebugLibraryDirectory=%SolutionDirectory%\out
set ReleaseLibraryDirectory=%SolutionDirectory%\out
set TemporaryDirectory=%SolutionDirectory%\temp

rem Check for basic requirements
if "%VS80COMNTOOLS%" == "" (
	echo ERROR: Microsoft Visual Studio 2005 is not installed.
	pause
	goto END
)

rem Setting environment for using Microsoft Visual Studio 2005 x86 tools.
call "%VS80COMNTOOLS%vsvars32.bat"

eet.sln

:END