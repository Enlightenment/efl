#!/bin/sh

set -e

if [ "$DISTRO" != "" ] ; then
  docker exec $(cat $HOME/cid)  sh -c ".ci/docker-ccache-setup.sh $1"
  docker exec $(cat $HOME/cid)  ccache -pz
else
  cp .ci/ccache.conf ~/.ccache
  ccache -o base_dir="$(pwd)"
  ccache -pz
fi
