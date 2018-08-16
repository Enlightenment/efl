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
    (docker exec --env EINA_LOG_BACKTRACE="0" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) sh -c 'cd build && meson test --wrap="dbus-run-session --"' --print-errorlogs) && break
    cat build/meson-logs/testlog-dbus-run-session.txt
    if [ $tries != 3 ] ; then echo "tests failed, trying again!" ; fi
    false
  done
#else
  #export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
  #for tries in 1 2 3 ; do
    #cd build && meson test --wrap="dbus-run-session --"' --print-errorlogs && break
    #if [ $tries != 3 ] ; then echo "tests failed, trying again!" ; fi
    #false
  #done
fi
ret=$?
travis_endfold check

exit $ret
