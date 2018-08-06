#!/bin/sh

. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi

#T7151
if [ "$1" = "misc" ] || [ "$1" = "misc-disabled" ] ; then
  exit 0
fi

travis_fold check "make check-TESTS"
if [ "$DISTRO" != "" ] ; then
  for tries in 1 2 3 ; do
    (docker exec --env EINA_LOG_BACKTRACE="0" --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make -j2 -C src/ check-TESTS) && break
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cat src/test-suite.log
    if [ $tries != 3 ] ; then echo "tests failed, trying again!" ; fi
    false
  done
#else
  #export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  #for tries in 1 2 3 ; do
    #make -j2 -C src/ check-TESTS && break
    #cat src/test-suite.log
    #if [ $tries != 3 ] ; then echo "tests failed, trying again!" ; fi
    #false
  #done
fi
ret=$?
travis_endfold check

exit $ret
