#!/bin/bash

set -e

travis_fold start "ccache-stats"
travis_time_start "ccache-stats"
if [ "$DISTRO" != "" ] ; then
  docker exec $(cat $HOME/cid)  ccache -s
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  ccache -s
fi
travis_time_finish "ccache-stats"
travis_fold end "ccache-stats"
