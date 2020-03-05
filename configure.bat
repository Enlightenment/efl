@echo off
@setlocal

call :main

:check_env_vars
    echo Running the function!
exit /B 0


:setup_flags
    @echo ------------------------------
    @echo Setting up build flags...

    :: ---------------------------------
    :: Compilers
    @set CC=clang-cl
    @echo C Compiler: %CC%
    @set CXX=clang-cl
    @echo C++ Compiler: %CXX%

    :: ---------------------------------
    :: Windows terminal specific options
    @set CFLAGS=-fansi-escape-codes -fcolor-diagnostics %CFLAGS%

    :: ------------------------------------
    :: Default flags for native compilation
    @set CFLAGS=-Wno-language-extension-token %CFLAGS%

    @echo Using CFLAGS=%CFLAGS%

    :: ------------------------------------------------------
    @set MESONFLAGS=^
     -Dopenssl_dir="C:/Users/Tiz/source/pkg/openssl/"^
     -Dregex_include_dir="C:/Users/Tiz/source/pkg/pcre-7.0/include/"^
     -Dregex_dir="C:/Users/Tiz/source/pkg/pcre-7.0/lib/"^
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

    @if exist build (
        @echo "Build directory ("build") already exists. Old config will be wiped with `--wipe`."
        @set MESONFLAGS=%MESONFLAGS% --wipe
    ) else (
        @echo No Creating new build directory.
    )
exit /B 0

:main
    @set vcvars64="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
    call :setup_flags

    @echo on
    @echo Running meson with flags: %MESONFLAGS%
    @echo
    @%vcvars64% && meson build %MESONFLAGS%
