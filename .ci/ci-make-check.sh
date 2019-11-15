#!/bin/sh

. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi

if [ "$1" = "mingw" ] ; then
  exit 0
fi

if [ "$1" = "coverity" ] ; then
  exit 0
fi

#T7151
if [ "$1" = "options-enabled" ] || [ "$1" = "options-disabled" ] ; then
  exit 0
fi

NUM_TRIES=5

travis_fold check "ninja test"
if [ "$DISTRO" != "" ] ; then
  for tries in $(seq 1 ${NUM_TRIES}); do
    (docker exec --env EINA_LOG_BACKTRACE="0" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) meson test -t 120 -C build --wrapper dbus-launch ) && break
    docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cat build/meson-logs/testlog-dbus-launch.txt
    if [ $tries != ${NUM_TRIES} ] ; then echo "tests failed, trying again!" ; fi
      false
  done
fi
ret=$?
travis_endfold check

exit $ret
