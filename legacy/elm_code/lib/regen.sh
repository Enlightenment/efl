#!/bin/sh

INCLUDE="-I /usr/local/share/eolian/include/eo-1 -I /usr/local/share/eolian/include/elementary-1 -I /usr/local/share/eolian/include/evas-1 -I /usr/local/share/eolian/include/efl-1"

eolian_gen $INCLUDE --gh --eo -o elm_code_widget2.eo.h elm_code_widget2.eo
eolian_gen $INCLUDE --gc --eo -o elm_code_widget2.eo.c elm_code_widget2.eo
eolian_gen $INCLUDE --gi --eo -o elm_code_widget2.c elm_code_widget2.eo
