#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold examples "make examples"
echo "TODO"

travis_endfold examples
