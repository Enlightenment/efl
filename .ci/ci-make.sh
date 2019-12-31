#!/bin/sh

set -e
. .ci/travis.sh

if [ "$1" = "release-ready" ] ; then
  exit 0
fi

travis_fold ninja ninja
if [ "$1" = "asan" ]; then
  docker exec --env EIO_MONITOR_POLL=1 --env ASAN_OPTIONS=abort_on_error=0 --env LSAN_OPTIONS=suppressions=/src/.ci/asan-ignore-leaks.supp $(cat $HOME/cid) ninja -C build
  exit $?
fi

if [ "$DISTRO" != "" ] ; then
  if [ "$1" = "coverity" ] ; then
    docker exec --env EIO_MONITOR_POLL=1 --env PATH="/src/cov-analysis-linux64-2019.03/bin:$PATH" $(cat $HOME/cid) sh -c "cov-build --dir cov-int ninja -C build"
    docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) sh -c "tar caf efl-$(git rev-parse --short HEAD).xz cov-int"
    docker exec --env EIO_MONITOR_POLL=1 --env COVERITY_SCAN_TOKEN=$COVERITY_SCAN_TOKEN $(cat $HOME/cid) sh -c ".ci/coverity-upload.sh"
  else
    docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build
  fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  ninja -C build
else
  ninja -C build
fi
travis_endfold ninja
