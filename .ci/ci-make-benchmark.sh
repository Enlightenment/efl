#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold benchmark "make benchmark"
if [ "$DISTRO" != "" ] ; then
  docker exec --env MAKEFLAGS="-j5" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make benchmark
else
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  make benchmark
fi
travis_endfold benchmark
