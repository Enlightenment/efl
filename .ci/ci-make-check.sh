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
      # disable them for this distros, after meson 0.49 is out, this can be removed
      # https://github.com/mesonbuild/meson/commit/253c581412d7f2b09af353dd83d943454bd555be
      if [ "$DISTRO" != "Ubuntu1810" ] && [ "$DISTRO" != "Debian96" ]; then
        for tries in $(seq 1 ${NUM_TRIES}); do
            (docker exec --env EINA_LOG_BACKTRACE="0" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) meson test -C build --wrapper dbus-launch ) && break
            docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cat src/test-suite.log
            if [ $tries != ${NUM_TRIES} ] ; then echo "tests failed, trying again!" ; fi
            false
        done
      fi
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
