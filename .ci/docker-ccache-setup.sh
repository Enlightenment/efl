#!/bin/sh

CI_BUILD_TYPE="$1"

cp .ci/ccache.conf ~/.ccache

sed -iE '/^base_dir/d' ~/.ccache/ccache.conf
echo "base_dir = $pwd" >> ~/.ccache/ccache.conf
