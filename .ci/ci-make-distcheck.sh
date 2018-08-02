#!/bin/bash

set -e


. .ci/travis.sh

if [ "$1" != "release-ready" ] ; then
  exit 0
fi

# skip distcheck if distcheck was skipped
if [ -n "$DISTCHECK_SKIPPED" ] ; then
  exit 0
fi

travis_fold distcheck "make distcheck"
if [ "$DISTRO" != "" ] ; then
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
    --env CXX="ccache g++" \
    --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
    --env LD="ld.gold" $(cat $HOME/cid) bash -c .ci/distcheck.sh
else
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  .ci/distcheck.sh
fi

rm -f ~/cachedir/last-distcheck.txt
git rev-parse HEAD > ~/cachedir/last-distcheck.txt

travis_endfold distcheck
