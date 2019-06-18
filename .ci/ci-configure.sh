#!/bin/sh

set -e

. .ci/travis.sh

if [ "$DISTRO" != "" ] ; then
  # Normal build test of all targets
  OPTS=" -Decore-imf-loaders-disabler=scim,ibus -Davahi=false -Dbindings=luajit"

  WAYLAND_LINUX_COPTS=" -Dwl=true -Ddrm=true -Dopengl=es-egl"

  ENABLED_LINUX_COPTS=" -Dharfbuzz=true -Dhyphen=true"

  DISABLED_LINUX_COPTS=" -Dsystemd=false"

  RELEASE_READY_LINUX_COPTS=" --buildtype=release"

  MINGW_COPTS="--cross-file .ci/cross_toolchain.txt -Davahi=false -Deeze=false -Dsystemd=false \
  -Dpulseaudio=false -Dx11=false -Dopengl=none -Dlibmount=false \
  -Devas-loaders-disabler=pdf,ps,raw,svg -Devas-modules=static -Dbindings=luajit \
  -Dbuild-examples=false -Dbuild-tests=false"

  if [ "$1" = "options-enabled" ]; then
    OPTS="$OPTS $ENABLED_LINUX_COPTS $WAYLAND_LINUX_COPTS"
  fi

  if [ "$1" = "options-disabled" ]; then
    OPTS="$OPTS $DISABLED_LINUX_COPTS"
  fi

  if [ "$1" = "wayland" ]; then
    OPTS="$OPTS $WAYLAND_LINUX_COPTS"
  fi

  if [ "$1" = "release-ready" ]; then
    OPTS="$OPTS $RELEASE_READY_LINUX_COPTS"
  fi

  if [ "$1" = "mingw" ]; then
    OPTS="$OPTS $MINGW_COPTS"
    docker exec $(cat $HOME/cid) sh -c '.ci/bootstrap-efl-native-for-cross.sh'
  fi
  if [ "$1" = "mingw" ]; then
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env PKG_CONFIG_PATH="/ewpi-64-install/lib/pkgconfig/" \
       $(cat $HOME/cid) sh -c "mkdir build && meson build $OPTS"
  else
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
      --env CXX="ccache g++" --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
      --env LD="ld.gold" $(cat $HOME/cid) sh -c "mkdir build && meson build $OPTS"
  fi
else
  # Prepare OSX env for build
  mkdir -p ~/Library/LaunchAgents
  ln -sfv /usr/local/opt/d-bus/*.plist ~/Library/LaunchAgents
  launchctl load ~/Library/LaunchAgents/org.freedesktop.dbus-session.plist
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"

  export CFLAGS="-I/usr/local/opt/openssl/include -frewrite-includes $CFLAGS"
  export LDFLAGS="-L/usr/local/opt/openssl/lib $LDFLAGS"
  LIBFFI_VER=$(brew list --versions libffi|head -n1|cut -d' ' -f2)
  export PKG_CONFIG_PATH="/usr/local/opt/openssl/lib/pkgconfig:/usr/local/Cellar/libffi/$LIBFFI_VER/lib/pkgconfig"
  mkdir build && meson build -Decore-imf-loaders-disabler=scim,ibus -Dx11=false -Davahi=false -Dbindings=luajit -Deeze=false -Dsystemd=false -Dnls=false -Dcocoa=true -Demotion-loaders-disabler=gstreamer,gstreamer1,libvlc,xine
fi
