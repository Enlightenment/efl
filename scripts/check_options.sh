#!/bin/bash

BUILD_DIR=`mktemp -d`
INSTALL_DIR=`mktemp -d`

meson $@ --prefix ${INSTALL_DIR} ${BUILD_DIR}

ninja -C ${BUILD_DIR} all
ninja -C ${BUILD_DIR} install
#ninja -C ${BUILD_DIR} test