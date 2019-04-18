#!/bin/bash +e

set -o pipefail
export TEST_VAR=1
if ! ./configure $@ 2>&1 | tee -a configlog ; then
  if grep -q 'configure: error: changes in the environment can compromise the build' configlog ; then
    echo "clearing config.cache and retrying..."
    rm -f configlog config.cache
    ./configure $@
  fi
fi
