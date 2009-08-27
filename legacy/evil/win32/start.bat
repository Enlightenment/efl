@echo off

rem Set external libraries directory.
set EXT_DIR=%cd%\..\..\..\extern

if not exist %EXT_DIR% (
	set EXT_DIR=
	echo ERROR: External libs dir is not set.
	pause
	goto END
)

rem Setup common Win32 environment variables

rem Add installation directory pathes.
set INCLUDE=%EXT_DIR%\include;%INCLUDE%
set LIB=%EXT_DIR%\lib;%LIB%

set INCLUDE=%cd%\common;%cd%\..\src\lib;%INCLUDE%

rem Check for basic requirements for Visual Studio 2008
if "%VS90COMNTOOLS%" == "" (
	echo ERROR: Microsoft Visual Studio 2008 is not installed.
	pause
	goto NOVS9
)

set SolutionDirectory=%cd%\vs9
set DebugOutputDirectory=%SolutionDirectory%\out
set ReleaseOutputDirectory=%SolutionDirectory%\out
set DebugLibraryDirectory=%SolutionDirectory%\out
set ReleaseLibraryDirectory=%SolutionDirectory%\out
set TemporaryDirectory=%SolutionDirectory%\temp

rem Setting environment for using Microsoft Visual Studio 2008 x86 tools.
call "%VS90COMNTOOLS%vsvars32.bat"

vs9\evil.sln

goto END

:NOVS9

rem Check for basic requirements for Visual Studio 2005
if "%VS80COMNTOOLS%" == "" (
	echo ERROR: Microsoft Visual Studio 2005 is not installed.
	pause
	goto END
)

set SolutionDirectory=%cd%\vs8
set DebugOutputDirectory=%SolutionDirectory%\out
set ReleaseOutputDirectory=%SolutionDirectory%\out
set DebugLibraryDirectory=%SolutionDirectory%\out
set ReleaseLibraryDirectory=%SolutionDirectory%\out
set TemporaryDirectory=%SolutionDirectory%\temp

rem Setting environment for using Microsoft Visual Studio 2005 x86 tools.
call "%VS80COMNTOOLS%vsvars32.bat"

vs8\evil.sln

:END
