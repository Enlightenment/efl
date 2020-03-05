@set vcvars64="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
@set CLICOLOR_FORCE=1
%vcvars64% && ninja --verbose -C build
