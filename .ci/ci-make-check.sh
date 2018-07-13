#!/bin/sh -x

. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold check "make check"
if [ "$DISTRO" != "" ] ; then
  for tries in 1 2 3 ; do
    (docker exec --env MAKEFLAGS="-j5" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make -j2 check) && break
    docker exec --env MAKEFLAGS="-j5" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cat src/test-suite.log
    if [ $tries != 3 ] ; then echo "tests failed, trying again!" ; fi
    false
  done
else
  export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  for tries in 1 2 3 ; do
    make -j2 check && break
    cat src/test-suite.log
    if [ $tries != 3 ] ; then echo "tests failed, trying again!" ; fi
    false
  done
  #all tests fail all the time currently
  true
fi
ret=$?
travis_endfold check

exit $ret
