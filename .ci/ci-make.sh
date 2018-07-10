#!/bin/sh

set -e

if [ "$DISTRO" != "" ] ; then
  docker exec --env MAKEFLAGS="-j5" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) make
else
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  make
fi
