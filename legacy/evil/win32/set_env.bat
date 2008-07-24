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

set INCLUDE=%cd%\common;%cd%\..\src\lib;%INCLUDE%

:END
