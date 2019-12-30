#!/bin/sh

set -e
. .ci/travis.sh

if [ "$1" = "release-ready" ] || [ "$1" = "coverity" ] ; then
  exit 0
fi

travis_fold install "ninja install"
if [ "$1" = "asan" ]; then
  docker exec --env EIO_MONITOR_POLL=1 --env ASAN_OPTIONS=abort_on_error=0 --env LSAN_OPTIONS=suppressions=/src/.ci/asan-ignore-leaks.supp $(cat $HOME/cid) ninja -C build install
  exit $?
fi
if [ "$DISTRO" != "" ] ; then
  docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build install
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  ninja -C build install
else
  sudo ninja -C build install
fi
travis_endfold install
