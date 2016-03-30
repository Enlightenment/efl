#!/bin/sh

ROOTNS="efl"
GENPATH="dokuwiki/data/pages"
rm -rf "$GENPATH/$ROOTNS"
elua gendoc.lua -r "$GENPATH" -n "$ROOTNS"
