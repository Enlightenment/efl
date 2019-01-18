#!/bin/sh

set -e

if [ "$DISTRO" != "" ] ; then
  docker exec $(cat $HOME/cid)  ccache -s
else
  ccache -s
fi
