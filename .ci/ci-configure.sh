#!/bin/sh

set -e

. .ci/travis.sh

CI_BUILD_TYPE="$1"

DEFAULT_LINUX_COPTS="--prefix=/usr/ --with-tests=regular --disable-cxx-bindings --disable-dependency-tracking -C"

WAYLAND_LINUX_COPTS=" --enable-wayland --enable-elput --enable-drm \
--enable-wayland-ivi-shell --enable-gl-drm --with-opengl=es --enable-egl"

MISC_LINUX_COPTS=" --enable-harfbuzz --enable-liblz4 --enable-image-loader-webp --enable-xinput22 \
--enable-multisense --enable-lua-old --enable-xpresent --enable-hyphen \
--enable-pixman --enable-pixman-font --enable-pixman-rect --enable-pixman-line \
--enable-pixman-poly --enable-pixman-image --enable-pixman-image-scale-sample \
--enable-image-loader-generic --enable-libuv --enable-tile-rotate --enable-vnc-server \
--enable-sdl --enable-fb --enable-v4l2 --enable-cserve \
--enable-ecore-wayland --enable-ecore-drm --enable-cancel-ok --with-crypto=gnutls \
--enable-debug --disable-gstreamer1 --enable-gstreamer"

MISC_DISABLED_LINUX_COPTS=" --disable-neon --disable-libeeze --disable-systemd --disable-magic-debug \
--disable-valgrind --disable-gstreamer1 \
--disable-fontconfig --disable-fribidi --disable-poppler --disable-spectre --disable-libraw \
--disable-librsvg --disable-xcf --disable-libmount --disable-tslib --disable-audio \
--disable-pulseaudio --disable-avahi --disable-xinput2 --disable-xim --disable-scim \
--disable-ibus --disable-physics --disable-quick-launch --disable-elua"

RELEASE_READY_LINUX_COPTS=" --with-profile=release"

patch -p1 < .ci/efl.m4.diff
sed -i.orig 's/AC_INIT\(.*\)efl_version-[a-zA-Z0-9]\+/AC_INIT\1efl_version/g' configure.ac

if [ "$DISTRO" != "" ] ; then
  # Normal build test of all targets
  OPTS="$DEFAULT_LINUX_COPTS"

  if [ "$1" = "wayland" ]; then
    OPTS="$OPTS $WAYLAND_LINUX_COPTS"
  fi

  if [ "$1" = "misc" ]; then
    OPTS="$OPTS $MISC_LINUX_COPTS"
  fi

  if [ "$1" = "misc-disabled" ]; then
    OPTS="$OPTS $MISC_DISABLED_LINUX_COPTS"
  fi

  if [ "$1" = "release-ready" ]; then
    OPTS="$OPTS $RELEASE_READY_LINUX_COPTS"
  fi
  docker exec $(cat $HOME/cid) sh -c 'rm -f ~/.ccache/ccache.conf'
  travis_fold autoreconf autoreconf
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
    --env CXX="ccache g++" --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
    --env LD="ld.gold" $(cat $HOME/cid) sh -c "LIBTOOLIZE_OPTIONS='--no-warn' autoreconf -iv"
  travis_endfold autoreconf
  travis_fold configure "configure $OPTS"
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
    --env CXX="ccache g++" --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
    --env LD="ld.gold" $(cat $HOME/cid) sh -c ".ci/configure.sh $OPTS"
  travis_endfold configure
else
  OSX_COPTS="--disable-cxx-bindings --with-tests=regular --disable-dependency-tracking -C"

  # Prepare OSX env for build
  mkdir -p ~/Library/LaunchAgents
  ln -sfv /usr/local/opt/d-bus/*.plist ~/Library/LaunchAgents
  launchctl load ~/Library/LaunchAgents/org.freedesktop.dbus-session.plist
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"

  export CFLAGS="-I/usr/local/opt/openssl/include -frewrite-includes $CFLAGS"
  export LDFLAGS="-L/usr/local/opt/openssl/lib $LDFLAGS"
  export PKG_CONFIG_PATH="/usr/local/opt/openssl/lib/pkgconfig"

  # Normal build test of all targets
  rm -f ~/.ccache/ccache.conf
  travis_fold autoreconf autoreconf
  LIBTOOLIZE_OPTIONS="--no-warn" autoreconf -iv
  travis_endfold autoreconf
  travis_fold configure "configure $OSX_COPTS"
  .ci/configure.sh $OSX_COPTS
  travis_endfold configure
fi
