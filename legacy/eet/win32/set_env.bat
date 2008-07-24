@echo off

rem Set external libraries directory.
set EXT_DIR=%cd%\..\..\..\..\extern

if not exist %EXT_DIR% (
	set EXT_DIR=
	echo ERROR: External libs dir is not set.
	pause
	goto END
)

rem Add installation directory pathes.
set INCLUDE=%EXT_DIR%\include;%INCLUDE%
set LIB=%EXT_DIR%\lib;%LIB%

rem Add Evil lib path
set EvilInclude=%cd%\..\..\..\proto\evil\src\lib
set EvilCommon=%cd%\..\..\..\proto\evil\win32\common
set EvilOut=%cd%\..\..\..\proto\evil\win32\%PROJECT_TYPE%\out

set INCLUDE=%EvilCommon%;%EvilInclude%;%EvilInclude%\dlfcn;%EvilInclude%\mman;%INCLUDE%
set LIB=%EvilOut%;%LIB%

:END
