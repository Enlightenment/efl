#!/bin/bash

set -e

if [ "$1" != "exactness" ] ; then
  exit 0
fi

travis_fold start "exactness"
travis_time_start "exactness"
if [ "$DISTRO" != "" ] ; then
  docker exec  --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) sh -c 'git -C /exactness-elm-data pull'
  docker exec  --env EIO_MONITOR_POLL=1 --env LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib64 --env EINA_LOG_LEVELS_GLOB=eina_*:0,ecore*:0,efreet*:0,eldbus:0,elementary:0 $(cat $HOME/cid) exactness -j 20 -b /exactness-elm-data/default-profile -p /exactness-elm-data/default-profile/ci-fedora32-tests.txt
fi
travis_time_finish "exactness"
travis_fold end "exactness"
