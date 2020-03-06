@echo off
setlocal EnableDelayedExpansion

call :main || (echo Build configure failed.)
exit /B %errorlevel%

:check_env_vars
    @echo Checking if necessarry environment variables were set...

    set envfile=env.bat
    if exist %envfile% (
        @echo - Found %envfile% file.
        call %envfile%
    ) else (
        @echo - File %envfile% doesn't exists. Relying on previously set environment variables...
    )
    set envfile=

    set all_set=1
    if not defined OPENSSL_DIR set all_set=0
    if not defined REGEX_INCLUDE_DIR set all_set=0
    if not defined REGEX_DIR set all_set=0

    if %all_set%==1 (
        @echo - Using OpenSSL: %OPENSSL_DIR%
        @echo - Using Regex Include Directory: %REGEX_INCLUDE_DIR%
        @echo - Using Regex Lib Directory: %REGEX_DIR%
    ) else (
        @echo At least one of the following variables were not set:
        @echo     - OPENSSL_DIR: %OPENSSL_DIR%
        @echo     - REGEX_INCLUDE_DIR: %REGEX_INCLUDE_DIR%
        @echo     - REGEX_DIR: %REGEX_DIR%
        @echo Please define them using by creating a "env.bat" file containing:
        @echo     @set OPENSSL_DIR=^<your OpenSSL directory^>
        @echo     @set REGEX_INCLUDE_DIR=^<your pcre/include directory^>
        @echo     @set REGEX_DIR=^<your pcre/lib directory^>
        exit /B 1
    )
    set all_set=
exit /B 0


:setup_flags
    @echo ------------------------------
    @echo Setting up build flags...

    :: ---------------------------------
    :: Compilers
    set CC=clang-cl
    @echo - C Compiler: %CC%
    set CXX=clang-cl
    @echo - C++ Compiler: %CXX%

    :: ---------------------------------
    :: Windows terminal specific options
    set CFLAGS=-fansi-escape-codes -fcolor-diagnostics %CFLAGS%

    :: ---------------------------------
    :: Ignored warnings
    :: TODO: Re-enable warnings one by one and solve them.

    :: Compiler-specific
        set CFLAGS=-Wno-reserved-id-macro %CFLAGS%
        set CFLAGS=-Wno-deprecated-declarations %CFLAGS%
        set CFLAGS=-Wno-gnu-zero-variadic-macro-arguments %CFLAGS%
        set CFLAGS=-Wno-nonportable-system-include-path %CFLAGS%
    :: Syntax/Parsing
        set CFLAGS=-Wno-implicit-function-declaration %CFLAGS%
        set CFLAGS=-Wno-missing-prototypes %CFLAGS%
        set CFLAGS=-Wno-unreachable-code %CFLAGS%
        set CFLAGS=-Wno-unreachable-code-return %CFLAGS%
        set CFLAGS=-Wno-extra-semi %CFLAGS%
        set CFLAGS=-Wno-extra-semi-stmt %CFLAGS%
        :: Declaration not visible outside function:
        set CFLAGS=-Wno-visibility %CFLAGS%
        :: A global variable declared in a .c file is not static and hasn't
        :: been declarated with `extern` anywhere.
        set CFLAGS=-Wno-missing-variable-declarations %CFLAGS%
    :: Architectural
        set CFLAGS=-Wno-cast-align %CFLAGS%
        set CFLAGS=-Wno-shorten-64-to-32 %CFLAGS%
    :: Type system
        :: When a switch-case misses one possible enum vlaue
        set CFLAGS=-Wno-switch-enum %CFLAGS%
        :: Discarding `const` qualifier in cast (`(T)const_value`
        set CFLAGS=-Wno-cast-qual %CFLAGS%
        set CFLAGS=-Wno-pedantic %CFLAGS%
        set CFLAGS=-Wno-double-promotion %CFLAGS%
        set CFLAGS=-Wno-float-conversion %CFLAGS%
        set CFLAGS=-Wno-implicit-float-conversion %CFLAGS%
        set CFLAGS=-Wno-int-conversion %CFLAGS%
        :: Implicit conversion like int -> short, short -> char or long -> char
        set CFLAGS=-Wno-implicit-int-conversion %CFLAGS%
        set CFLAGS=-Wno-sign-conversion %CFLAGS%
        set CFLAGS=-Wno-bad-function-cast %CFLAGS%
        :: Assign enum to a value not in range defined by the enum type
        set CFLAGS=-Wno-assign-enum %CFLAGS%
        :: uchar_value > 255 is always false
        set CFLAGS=-Wno-tautological-type-limit-compare %CFLAGS%
        :: Comparing float with == is unsafe (since floats not necessarily will
        :: have that specific value)
        set CFLAGS=-Wno-float-equal %CFLAGS%
    :: Pointer-related
        set CFLAGS=-Wno-pointer-integer-compare %CFLAGS%
    :: Others
        :: Using an undefined macro (which will be evaluated to 0)
        set CFLAGS=-Wno-undef %CFLAGS%
        set CFLAGS=-Wno-documentation %CFLAGS%
        set CFLAGS=-Wno-documentation-unknown-command %CFLAGS%
        set CFLAGS=-Wno-unused-macros %CFLAGS%
        set CFLAGS=-Wno-unused-parameter %CFLAGS%

    :: ------------------------------------
    :: Default flags for native compilation
    set CFLAGS=-Wno-language-extension-token %CFLAGS%

    @echo - Using CFLAGS=%CFLAGS%

    :: ------------------------------------------------------
    set MESONFLAGS=^
            -Dopenssl_dir=%OPENSSL_DIR%^
            -Dregex_include_dir=%REGEX_INCLUDE_DIR%^
            -Dregex_dir=%REGEX_DIR%^
            -Dcrypto=openssl^
            -Dnls=false^
            -Dsystemd=false^
            -Dglib=false^
            -Dgstreamer=false^
            -Ddbus=false^
            -Daudio=false^
            -Davahi=false^
            -Dv4l2=false^
            -Delua=false^
            -Dx11=false^
            -Dphysics=false^
            -Deeze=false^
            -Dpulseaudio=false^
            -Dharfbuzz=false^
            -Dfribidi=false^
            -Dfontconfig=false^
            -Dedje-sound-and-video=false^
            -Dlibmount=false^
            "-Devas-loaders-disabler=gst,pdf,ps,raw,svg,rsvg,xcf,bmp,dds,eet,generic,gif,ico,jp2k,jpeg,pmaps,png,psd,tga,tgv,tiff,wbmp,webp,xpm,json"^
            -Dopengl=none^
            "-Demotion-loaders-disabler=gstreamer1,libvlc,xine"^
            -Dbuild-tests=false^
            -Dbuild-examples=false^
            -Dbindings=^
            --native-file native-file-windows.txt

    if exist build (
        @echo "- Build directory ("build") already exists. Old config will be wiped with `--wipe`."
        set MESONFLAGS=%MESONFLAGS% --wipe
    ) else (
        @echo No Creating new build directory.
    )

    set NLM=^


    set NL=^^^%NLM%%NLM%%NLM%%NLM%
    @echo Here %NL%we go
    @echo Meson flags: %MESONFLAGS:        =!NL!%
    @echo ------------------------------
exit /B 0

:generate_build
    @echo ------------------------------
    @echo Generating build...
    set vcvars64="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"

    if not defined DevEnvDir (
        call %vcvars64%
    )
    meson build %MESONFLAGS:            = %
exit /B 0

:save_old_vars
    @echo ------------------------------
    set __OLD_vcvars64=%vcvars64%
    set __OLD_CC=%CC%
    set __OLD_CXX=%CXX%
    set __OLD_CFLAGS=%CFLAGS%
    set __OLD_MESONFLAGS=%MESONFLAGS%
exit /B 0

:restore_old_vars
    @echo ------------------------------
    set vcvars64=%__OLD_vcvars64%
    set CC=%__OLD_CC%
    set CXX=%__OLD_CXX%
    set CFLAGS=%__OLD_CFLAGS%
    set MESONFLAGS=%__OLD_MESONFLAGS%

    set __OLD_vcvars64=
    set __OLD_CC=
    set __OLD_CXX=
    set __OLD_CFLAGS=
    set __OLD_MESONFLAGS=
exit /B 0

:main
    call :save_old_vars
    call :check_env_vars || (echo Environment Variables check failed) && exit /B 1
    call :setup_flags
    call :generate_build
    call :restore_old_vars
