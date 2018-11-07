#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold make make
if [ "$BUILDSYSTEM" = "ninja" ] ; then
  if [ "$DISTRO" != "" ] ; then
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build
  else
    export PATH="$(brew --prefix gettext)/bin:$PATH"
    ninja -C build
  fi
else
  if [ "$DISTRO" != "" ] ; then
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make
  else
    export PATH="$(brew --prefix gettext)/bin:$PATH"
    make
  fi
fi
travis_endfold make
