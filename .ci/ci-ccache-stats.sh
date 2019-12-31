#!/bin/sh

set -e

if [ "$DISTRO" != "" ] ; then
  docker exec $(cat $HOME/cid)  ccache -s
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
  ccache -s
fi
