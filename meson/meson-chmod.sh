#!/bin/sh
chmod "$1" "${DESTDIR}/$2" && touch "${DESTDIR}/$2"
