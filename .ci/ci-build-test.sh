#!/bin/sh

set -e
. .ci/travis.sh
travis_fold compile_test compile_test
if [ "$DISTRO" != "" ] ; then
  if [ "$1" = "coverity" ] ; then
    exit 0
  fi
  if [ "$1" = "mingw" ] ; then
    exit 0
  fi
  docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) .ci/build-example.sh
else
  exit 0
  #FIXME: we don't install efl_ui.pc on osx?
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  .ci/build-example.sh
fi
travis_endfold compile_test
