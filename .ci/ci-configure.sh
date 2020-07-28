#!/bin/bash

set -e

travis_fold start "meson"
travis_time_start "meson"
if [ "$DISTRO" != "" ] ; then
  # Why do we need to disable the imf loaders here?
  OPTS=" -Decore-imf-loaders-disabler=scim,ibus"

  MONO_LINUX_COPTS=" -Dbindings=cxx,mono -Dmono-beta=true"

  WAYLAND_LINUX_COPTS=" -Dwl=true -Ddrm=true -Dopengl=es-egl -Dwl-deprecated=true -Ddrm-deprecated=true"

  # TODO:
  # - No libelogind package in fedora 30 repo
  # - Ibus
  ENABLED_LINUX_COPTS=" -Dfb=true -Dsdl=true -Dbuffer=true -Dbuild-id=travis-build \
  -Ddebug-threads=true -Dglib=true -Dg-mainloop=true -Dxpresent=true -Dxinput22=true \
  -Devas-loaders-disabler=json,avif -Decore-imf-loaders-disabler= \
  -Dharfbuzz=true -Dpixman=true -Dhyphen=true -Defl-one=true \
  -Dvnc-server=true -Delua=true -Dbindings=lua,cxx,mono -Delogind=false -Dinstall-eo-files=true -Dphysics=true"

  # Enabled png, jpeg evas loader for in tree edje file builds
  DISABLED_LINUX_COPTS=" -Daudio=false -Davahi=false -Dx11=false -Dphysics=false -Deeze=false \
  -Dopengl=none -Deina-magic-debug=false -Dbuild-examples=false -Dbuild-tests=false \
  -Dcrypto=gnutls -Dglib=false -Dgstreamer=false -Dsystemd=false -Dpulseaudio=false \
  -Dnetwork-backend=connman -Dxinput2=false -Dtslib=false \
  -Devas-loaders-disabler=gst,pdf,ps,raw,svg,xcf,bmp,dds,eet,generic,gif,ico,jp2k,json,pmaps,psd,tga,tgv,tiff,wbmp,webp,xpm,avif \
  -Decore-imf-loaders-disabler=xim,ibus,scim \
  -Dfribidi=false -Dfontconfig=false \
  -Dedje-sound-and-video=false -Dembedded-lz4=false -Dlibmount=false -Dv4l2=false \
  -Delua=false -Dnls=false -Dbindings= -Dlua-interpreter=luajit -Dnative-arch-optimization=false"
  #evas_filter_parser.c:(.text+0xc59): undefined reference to `lua_getglobal' with interpreter lua

  RELEASE_READY_LINUX_COPTS=" --buildtype=release"

  MINGW_COPTS="--cross-file .ci/cross_toolchain.txt -Davahi=false -Deeze=false -Dsystemd=false \
  -Dpulseaudio=false -Dx11=false -Dopengl=none -Dlibmount=false \
  -Devas-loaders-disabler=json,pdf,ps,raw,svg,rsvg,avif \
  -Dharfbuzz=true -Dpixman=true -Dembedded-lz4=false "

  if [ "$1" = "default" ]; then
    OPTS="$OPTS $MONO_LINUX_COPTS"
  elif [ "$1" = "options-enabled" ]; then
    OPTS="$OPTS $ENABLED_LINUX_COPTS $WAYLAND_LINUX_COPTS"
  elif [ "$1" = "options-disabled" ]; then
    OPTS="$OPTS $DISABLED_LINUX_COPTS"
  elif [ "$1" = "wayland" ]; then
    OPTS="$OPTS $WAYLAND_LINUX_COPTS"
  elif [ "$1" = "release-ready" ]; then
    OPTS="$OPTS $RELEASE_READY_LINUX_COPTS"
  elif [ "$1" = "coverity" ]; then
    travis_fold start "cov-download"
    travis_time_start "cov-download"
    OPTS="$OPTS $WAYLAND_LINUX_COPTS"
    docker exec --env COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN $(cat $HOME/cid) sh -c '.ci/coverity-tools-install.sh'
    travis_time_finish "cov-download"
    travis_fold end "cov-download"
  elif [ "$1" = "mingw" ]; then
    travis_fold start "cross-native"
    travis_time_start "cross-native"
    OPTS="$OPTS $MINGW_COPTS"
    docker exec $(cat $HOME/cid) sh -c '.ci/bootstrap-efl-native-for-cross.sh'
    travis_time_finish "cross-native"
    travis_fold end "cross-native"
  fi

  if [ "$1" = "asan" ]; then
    docker exec --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
      --env CXX="ccache g++" --env CFLAGS="-O0 -g" --env CXXFLAGS="-O0 -g" \
      --env LD="ld.gold" $(cat $HOME/cid) sh -c "mkdir build && meson build $OPTS -Db_sanitize=address"
  elif [ "$1" = "mingw" ]; then
    docker exec --env EIO_MONITOR_POLL=1 --env PKG_CONFIG_PATH="/ewpi-64-install/lib/pkgconfig/" \
       $(cat $HOME/cid) sh -c "mkdir build && meson build $OPTS"
  elif [ "$1" = "coverity" ]; then
    docker exec --env EIO_MONITOR_POLL=1 --env CFLAGS="-fdirectives-only"  --env CC="gcc" --env CXX="g++"\
    --env CXXFLAGS="-fdirectives-only" $(cat $HOME/cid) sh -c "mkdir build && meson build $OPTS"
  else
    docker exec --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
      --env CXX="ccache g++" --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
      --env LD="ld.gold" $(cat $HOME/cid) sh -c "mkdir build && meson build $OPTS"
  fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  # Prepare OSX env for build
  mkdir -p ~/Library/LaunchAgents
  ln -sfv /usr/local/opt/d-bus/*.plist ~/Library/LaunchAgents
  launchctl load ~/Library/LaunchAgents/org.freedesktop.dbus-session.plist
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"

  export CFLAGS="-I/usr/local/opt/openssl/include -frewrite-includes $CFLAGS"
  export LDFLAGS="-L/usr/local/opt/openssl/lib $LDFLAGS"
  LIBFFI_VER=$(brew list --versions libffi|head -n1|cut -d' ' -f2)
  export CC="ccache gcc"
  #force using system zlib, which doesn't have a pc file provided because that's the smartest thing possible
  zlib_vers=$(grep ZLIB_VERSION /usr/include/zlib.h|head -n1|awk '{print $3}'|cut -d'"' -f2)
  sed -iE "s/REPLACE_THIS/$zlib_vers/" .ci/zlib.pc
  export PKG_CONFIG_PATH="/usr/local/opt/openssl/lib/pkgconfig:/usr/local/Cellar/libffi/$LIBFFI_VER/lib/pkgconfig:$(pwd)/.ci"
  mkdir build && meson build -Dopengl=full -Decore-imf-loaders-disabler=scim,ibus -Dx11=false -Davahi=false -Deeze=false -Dsystemd=false -Dnls=false -Dcocoa=true -Dgstreamer=false -Dbuild-tests=false
else # Native Ubuntu Linux Travis builds (non-docker)
  OPTS=" -Decore-imf-loaders-disabler=scim,ibus"

  if [ "$TRAVIS_CPU_ARCH" = "ppc64le" ]; then
      OPTS="$OPTS -Dbindings="
  elif [ "$TRAVIS_CPU_ARCH" = "s390x" ] ; then
      OPTS="$OPTS -Dbindings= -Delua=false -Dlua-interpreter=lua"
  fi
  if [ "$1" = "codecov" ]; then
      OPTS="$OPTS -Db_coverage=true"
  fi
  mkdir build && meson build $OPTS
fi
travis_time_finish "meson"
travis_fold end "meson"
