#!/bin/sh

wget https://scan.coverity.com/download/linux64 --post-data="token=$COVERITY_SCAN_TOKEN&project=Enlightenment+Foundation+Libraries" -O coverity_tool.tgz
tar xzf coverity_tool.tgz
