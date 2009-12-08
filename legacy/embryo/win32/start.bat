@echo off

rem Set external libraries directory.
set EXT_DIR=%cd%\..\..\..\extern

if not exist %EXT_DIR% (
	set EXT_DIR=
	echo ERROR: External libs dir is not set.
	pause
	goto END
)

rem Check for basic requirements for Visual Studio 2008
if "%VS90COMNTOOLS%" == "" (
	echo ERROR: Microsoft Visual Studio 2008 is not installed.
	pause
	goto NOVS9
)

set PROJECT_TYPE=vs9
set VSCOMMONTOOLS=%VS90COMNTOOLS%vsvars32.bat

goto STARTVS

:NOVS9

rem Check for basic requirements for Visual Studio 2005
if "%VS80COMNTOOLS%" == "" (
	echo ERROR: Microsoft Visual Studio 2005 is not installed.
	pause
	goto END
)

set PROJECT_TYPE=vs8
set VSCOMMONTOOLS=%VS80COMNTOOLS%vsvars32.bat

:STARTVS

rem Setup common Win32 environment variables

rem Add Evil lib path
set EvilInclude=%cd%\..\..\evil\src\lib
set EvilCommon=%cd%\..\..\evil\win32\common
set EvilOut=%cd%\..\..\evil\win32\%PROJECT_TYPE%\out

set INCLUDE=%EvilCommon%;%EvilInclude%;%EvilInclude%\dlfcn;%INCLUDE%
set LIB=%EvilOut%;%LIB%

rem Add installation directory pathes.
set INCLUDE=%EXT_DIR%\include;%INCLUDE%
set LIB=%EXT_DIR%\lib;%LIB%

set INCLUDE=%cd%\common;%cd%\..\src\lib;%INCLUDE%

set SolutionDirectory=%cd%\%PROJECT_TYPE%
set DebugOutputDirectory=%SolutionDirectory%\out
set ReleaseOutputDirectory=%SolutionDirectory%\out
set DebugLibraryDirectory=%SolutionDirectory%\out
set ReleaseLibraryDirectory=%SolutionDirectory%\out
set TemporaryDirectory=%SolutionDirectory%\temp

rem Setting environment for using Microsoft Visual Studio x86 tools.
call "%VSCOMMONTOOLS%"

%PROJECT_TYPE%\embryo.sln

:END
