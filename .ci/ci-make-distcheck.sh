#!/bin/sh

set -e
. .ci/travis.sh

if [ "$1" != "release-ready" ] ; then
  exit 0
fi

travis_fold distcheck "ninja dist"
if [ "$DISTRO" != "" ] ; then
  docker exec --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
    --env CXX="ccache g++" \
    --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
    --env LD="ld.gold" $(cat $HOME/cid) dbus-launch ninja -C build dist || \
    (sudo cat efl-*/_build/sub/src/test-suite.log; false)
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  ninja -C build dist
fi
travis_endfold distcheck
