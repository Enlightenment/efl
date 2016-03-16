#!/bin/sh

BASEDIR=$(dirname $0)
/usr/bin/env node --expose-gc $BASEDIR/eolian_js_suite.js
