@echo off

set __OLD_vcvars64=%vcvars64%
set vcvars64="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
set CLICOLOR_FORCE=1

set NINJAFLAGS=%*

set vcvars64="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
set CLICOLOR_FORCE=1

if not defined DevEnvDir (
    echo DevEnvDir not defined. Running vcvars64...
    call %vcvars64%
    @set DevEnvDir=%cd%
)

ninja %NINJAFLAGS% -C build

set vcvars64=%__OLD_vcvars64%
set __OLD_vcvars64=
exit /B %ERRORLEVEL%
