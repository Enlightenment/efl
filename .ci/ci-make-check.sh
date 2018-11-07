#!/bin/sh

. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi

if [ "$1" = "mingw" ] ; then
  exit 0
fi

#T7151
if [ "$1" = "options-enabled" ] || [ "$1" = "options-disabled" ] ; then
  exit 0
fi

NUM_TRIES=5

travis_fold check "make check-TESTS"
if [ "$BUILDSYSTEM" = "ninja" ] ; then
   if [ "$DISTRO" != "" ] ; then
     for tries in $(seq 1 ${NUM_TRIES}); do
          (docker exec --env EINA_LOG_BACKTRACE="0" --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build test) && break
          docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cat src/test-suite.log
          if [ $tries != ${NUM_TRIES} ] ; then echo "tests failed, trying again!" ; fi
          false
      done
   fi
else
  if [ "$DISTRO" != "" ] ; then
    for tries in $(seq 1 ${NUM_TRIES}); do
      (docker exec --env EINA_LOG_BACKTRACE="0" --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make -j2 -C src/ check-TESTS) && break
      docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cat src/test-suite.log
      if [ $tries != ${NUM_TRIES} ] ; then echo "tests failed, trying again!" ; fi
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
fi
ret=$?
travis_endfold check

exit $ret
