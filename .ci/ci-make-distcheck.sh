#!/bin/bash

set -e


. .ci/travis.sh

run_distcheck() {
   if [ "$DISTRO" != "" ] ; then
     docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
       --env CXX="ccache g++" \
       --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
       --env LD="ld.gold" $(cat $HOME/cid) bash -c .ci/distcheck.sh
   else
     export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
     .ci/distcheck.sh
   fi
}

if [ "$1" != "release-ready" ] ; then
  exit 0
fi
travis_fold distcheck "make distcheck"

num_buildfiles_changed=0
if [ -f ~/cachedir/last-distcheck.txt ] ; then
  prev_distcheck=$(cat ~/cachedir/last-distcheck.txt)
  if git show --oneline $prev_distcheck 2>&1 > /dev/null ; then
    num_buildfiles_changed=$(git diff --name-only $prev_distcheck HEAD|grep '\.am\|\.ac\|\.mk'|wc -l)
    manual_trigger=$(git log --grep 'cibuildme' $prev_distcheck..HEAD|wc -l)
  else
    manual_trigger=1
  fi
  if [ "$num_buildfiles_changed" != "0" -o "$manual_trigger" != "0" ]; then
    run_distcheck
  else
    echo "Skipping distcheck: not forced and no build files changed"
  fi
else
  run_distcheck
fi
rm -f ~/cachedir/last-distcheck.txt
git rev-parse HEAD > ~/cachedir/last-distcheck.txt

travis_endfold distcheck
