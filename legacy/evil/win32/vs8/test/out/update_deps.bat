@echo off

set Target=%cd%
set e17=%cd%\..\..\..\..\..\..
set Libs=%e17%\libs
set Proto=%e17%\proto
set Extern=%e17%\..\extern


rem Copy proto
cd %Proto%\evil\win32\vs8\out
xcopy libdl.dll %Target%\ /Q /Y
if exist libdl.pdb xcopy libdl.pdb %Target%\ /Q /Y
xcopy libevil.dll %Target%\ /Q /Y
if exist libevil.pdb xcopy libevil.pdb %Target%\ /Q /Y
