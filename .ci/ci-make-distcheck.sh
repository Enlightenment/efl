#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" != "release-ready" ] ; then
  exit 0
fi
travis_fold distcheck "make distcheck"
if [ "$DISTRO" != "" ] ; then
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
    --env CXX="ccache g++" \
    --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
    $(cat $HOME/cid) make distcheck || (sudo cat efl-*/_build/sub/src/test-suite.log; false)
else
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  make
fi
travis_endfold distcheck
