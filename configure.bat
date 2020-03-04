@Rem ---------------------------------
@Rem Windows terminal specific options

set CC=clang-cl
set CXX=clang-cl

@Rem ---------------------------------
@Rem Windows terminal specific options
set CFLAGS=-fansi-escape-codes -fcolor-diagnostics %CFLAGS%

@Rem ---------------------------------
@Rem Default flags
set CFLAGS=-Wno-language-extension-token %CFLAGS%

meson build ^
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
        --wipe^
        --native-file native-file-windows.txt
