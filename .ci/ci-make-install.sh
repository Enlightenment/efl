#!/bin/bash

set -e

if [ "$1" = "release-ready" ] || [ "$1" = "coverity" ] ; then
  exit 0
fi

travis_fold start "ninja-install"
travis_time_start "ninja-install"
if [ "$DISTRO" != "" ] ; then
  docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build install
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  ninja -C build install
else
  sudo ninja -C build install
fi
travis_time_finish "ninja-install"
travis_fold end "ninja-install"
