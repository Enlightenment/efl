#!/bin/sh

echo $LD_LIBRARY_PATH
EINA_LOG_LEVEL=8 MONO_LOG_LEVEL=debug $MONO $MONO_BUILDPATH/src/tests/efl_mono/efl_mono${EXEEXT}

