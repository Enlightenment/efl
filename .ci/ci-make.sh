#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold ninja ninja
if [ "$DISTRO" != "" ] ; then
  docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) sh -c 'cd build && ninja'
else
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  cd build && ninja
fi
travis_endfold ninja
