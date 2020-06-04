#!/bin/bash

set -e

if [ "$1" = "codecov" ] || [ "$1" = "coverity" ] || [ "$1" = "mingw" ] || [ "$1" = "release-ready" ]; then
  exit 0
fi

travis_fold start "app-compile-test"
travis_time_start "app-compile-test"
if [ "$DISTRO" != "" ] ; then
  docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) .ci/build-example.sh
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  exit 0
  #FIXME: we don't install efl_ui.pc on osx?
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  .ci/build-example.sh
fi
travis_time_finish "app-compile-test"
travis_fold end "app-compile-test"
