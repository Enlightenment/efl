#!/bin/bash

set -e

if [ "$1" = "release-ready" ] ; then
  exit 0
fi

travis_fold start "ninja"
travis_time_start "ninja"
if [ "$DISTRO" != "" ] ; then
  if [ "$1" = "coverity" ] ; then
    docker exec --env EIO_MONITOR_POLL=1 --env PATH="/src/cov-analysis-linux64-2019.03/bin:$PATH" $(cat $HOME/cid) sh -c "cov-build --dir cov-int ninja -C build"
    docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) sh -c "tar caf efl-$(git rev-parse --short HEAD).xz cov-int"
    docker exec --env EIO_MONITOR_POLL=1 --env COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN $(cat $HOME/cid) sh -c ".ci/coverity-upload.sh"
  else
    docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build
    if [ "$1" = "options-enabled" ]; then # we have efl-one on and want to check it after build
      docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) python scripts/test-efl-one.py build
    fi
  fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  latest_brew_python3_bin="$(ls -1d /usr/local/Cellar/python/3.*/bin | sort -n | tail -n1)"
  export PATH="${latest_brew_python3_bin}${PATH:+:}${PATH}"
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  ninja -C build
else
  ninja -C build
fi
travis_time_finish "ninja"
travis_fold end "ninja"
