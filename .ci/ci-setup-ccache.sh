#!/bin/sh

set -e

if [ "$DISTRO" != "" ] ; then
  docker exec $(cat $HOME/cid)  sh -c ".ci/docker-ccache-setup.sh $1"
  docker exec $(cat $HOME/cid)  ccache -pz
else
  cp .ci/ccache.conf ~/.ccache
  echo "base_dir = $pwd" >> ~/.ccache/ccache.conf
  ccache -pz
fi
