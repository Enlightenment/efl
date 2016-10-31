#!/bin/sh

echo $LD_LIBRARY_PATH
EINA_LOG_LEVEL=8 MONO_LOG_LEVEL=debug $MONO --config $MONO_CONFIG $MONO_BUILDPATH/src/tests/eo_mono/eo_mono.exe

