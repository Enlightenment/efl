#!/bin/bash

set -e

travis_fold start "bionic-deps"
travis_time_start "bionic-deps"

sudo apt-get update -y
sudo apt-get install -y build-essential autoconf automake autopoint doxygen check libharfbuzz-dev libpng-dev libudev-dev libwebp-dev libssl-dev libfribidi-dev libcogl-gles2-dev libgif-dev libtiff5-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libdbus-1-dev libmount-dev libblkid-dev libpulse-dev libxrandr-dev libxtst-dev libxcursor-dev libxcomposite-dev libxinerama-dev libxkbfile-dev libbullet-dev libsndfile1-dev libraw-dev libspectre-dev libpoppler-cpp-dev libpam0g-dev liblz4-dev faenza-icon-theme gettext git imagemagick libasound2-dev libbluetooth-dev libfontconfig1-dev libfreetype6-dev libibus-1.0-dev libiconv-hook-dev libjpeg-dev libjpeg-turbo8-dev libpoppler-dev libpoppler-private-dev libproxy-dev librsvg2-dev libscim-dev libsystemd-dev libtool libudisks2-dev libunibreak-dev libxcb-keysyms1-dev libxss-dev linux-tools-common libcurl4-openssl-dev systemd ccache git binutils-gold python3-pip ninja-build dbus-x11 libavahi-client-dev python3-setuptools libopenjp2-7-dev libc6-dev libpcre3-dev
# s390x on Ubuntu Bionic on Travis does not have a luajit package and fails install, use lua5.1
if [ "$TRAVIS_CPU_ARCH" = "s390x" ] ; then
  sudo apt-get install -y liblua5.1-dev
else
  sudo apt-get install -y luajit libluajit-5.1-dev
fi
sudo pip3 install meson==0.54.3

travis_time_finish "bionic-deps"
travis_fold end "bionic-deps"
