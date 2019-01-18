#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold install "make install"
if [ "$BUILDSYSTEM" = "ninja" ] ; then
  if [ "$DISTRO" != "" ] ; then
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build install
  else
    export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
    ninja -C build install
  fi
else
  if [ "$DISTRO" != "" ] ; then
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make install
  else
    export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
    make install
  fi
fi
travis_endfold install
