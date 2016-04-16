#!/bin/sh
if [ -z "$EFL_DOC_ROOT" ]; then
    export EFL_DOC_ROOT="dokuwiki/data/pages"
fi
elua gendoc.lua -r "$EFL_DOC_ROOT" "$@"
