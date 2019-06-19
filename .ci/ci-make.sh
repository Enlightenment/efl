#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold ninja ninja
if [ "$DISTRO" != "" ] ; then
  docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build
else
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  ninja -C build
fi
travis_endfold ninja
