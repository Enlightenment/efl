#!/bin/sh

set -e

. .ci/travis.sh

CI_BUILD_TYPE="$1"

#FIXME: disable cxx by default
DEFAULT_LINUX_COPTS="--prefix=/usr -Decore_wl2=false -Decore_drm2=false -Delput=false -Dopengl=full -Decore_avahi=false -Davahi=false -Decore-imf-loaders-disabler=scim,ibus"

WAYLAND_LINUX_COPTS=" -Decore_wl2=true -Decore_drm2=true -Delput=true -Dopengl=es-egl"

MISC_LINUX_COPTS=" -Dharfbuzz=true  \
-Dxpresent=true \
-Dlibuv=true \
-Decore_fb=true \
-Dcrypto=gnutls
"

MISC_DISABLED_LINUX_COPTS=" \
-Dgstreamer=false \
"

#RELEASE_READY_LINUX_COPTS=" --with-profile=release"

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
  travis_fold configure "configure $OPTS"
  docker exec --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
    --env CXX="ccache g++" --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
    --env LD="ld.gold" $(cat $HOME/cid) sh -c ".ci/configure.sh $OPTS"
  travis_endfold configure
else
  #FIXME: disable cxx by default
  OSX_COPTS="-Decore_wl2=false -Decore_drm2=false -Delput=false -Decore_x=false -Decore_cocoa=true -Dopengl=full -Dsystemd=false -Decore_avahi=false -Davahi=false -Deeze=false -Decore-imf-loaders-disabler=scim,ibus,xim"

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
  travis_fold configure "configure $OSX_COPTS"
  .ci/configure.sh $OSX_COPTS
  travis_endfold configure
fi
