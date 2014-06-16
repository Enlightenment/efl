#!/bin/sh

ELUA_BIN="$ELUA"

if [ ! "$ELUA_BIN" ]; then
    ELUA_BIN="${ELUA_INTREE}"
    export EFL_RUN_IN_TREE=1
fi

if [ -x "$ELUA_BIN" ]; then
    $ELUA_BIN :xgettext "$@"
else
    $XGETTEXT "$@"
fi

exit $?
