#!/bin/sh

COPTS="--with-crypto=none --disable-pulseaudio --disable-cxx-bindings"
PARALLEL_JOBS=10

# Prepare OSX env for build
mkdir -p ~/Library/LaunchAgents
ln -sfv /usr/local/opt/d-bus/*.plist ~/Library/LaunchAgents
launchctl load ~/Library/LaunchAgents/org.freedesktop.dbus-session.plist
export PATH="$(brew --prefix gettext)/bin:$PATH"

# Normal build test of all targets
./autogen.sh $COPTS $@
make -j $PARALLEL_JOBS
make -j $PARALLEL_JOBS examples
#make -j $PARALLEL_JOBS benchmark
