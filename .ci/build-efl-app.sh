#!/bin/sh

set -e

cd /

#clone our examples from efl
git clone --depth=1 -b master http://git.enlightenment.org/tools/examples.git/

cd examples/apps/c/life/

#build the example
mkdir build
meson . ./build
ninja -C build all

#remove the folder again so its not left in the artifacts
cd /
rm -rf examples
