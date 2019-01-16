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

travis_fold check "ninja test"
if [ "$DISTRO" != "" ] ; then
   # disable them for this distros, after meson 0.49 is out, this can be removed
   # https://github.com/mesonbuild/meson/commit/253c581412d7f2b09af353dd83d943454bd555be
   if [ "$DISTRO" != "Ubuntu1810" ] && [ "$DISTRO" != "Debian96" ]; then
     for tries in $(seq 1 ${NUM_TRIES}); do
         (docker exec --env EINA_LOG_BACKTRACE="0" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) meson test -t 120 -C build --wrapper dbus-launch ) && break
         docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cat build/meson-logs/testlog-dbus-launch.txt
         if [ $tries != ${NUM_TRIES} ] ; then echo "tests failed, trying again!" ; fi
         false
     done
      docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ninja -C build install
      docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) sh -c 'cd /exactness-elm-data; git pull'
      docker exec --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) sh -c 'cd /exactness-elm-data; git checkout origin/devs/stefan/init-shots-docker-travis-ci -b docker'
      docker exec --env EIO_MONITOR_POLL=1 --env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64  $(cat $HOME/cid) exactness -j 20 -b /exactness-elm-data/default-profile -p /exactness-elm-data/default-profile/tests.txt
   fi
fi
ret=$?
travis_endfold check

exit $ret
