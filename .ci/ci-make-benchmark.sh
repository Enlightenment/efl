#!/bin/bash

set -e

if [ "$1" != "default" ] ; then
  exit 0
fi
travis_fold start "ninja-benchmark"
travis_time_start "ninja-benchmark"
if [ "$DISTRO" != "" ] ; then
  docker exec --env EIO_MONITOR_POLL=1 --env ELM_ENGINE=buffer $(cat $HOME/cid) ninja benchmark -C build
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  ninja benchmark -C build
fi
travis_time_finish "ninja-benchmark"
travis_fold end "ninja-benchmark"
