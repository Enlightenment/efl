#!/bin/sh
cd `dirname $0`

INCLUDE="-I /usr/local/share/eolian/include/eo-1 -I /usr/local/share/eolian/include/elementary-1 -I /usr/local/share/eolian/include/evas-1 -I /usr/local/share/eolian/include/efl-1"

eolian_gen $INCLUDE --gh --eo -o elm_code_widget.eo.h elm_code_widget.eo
eolian_gen $INCLUDE --gc --eo -o elm_code_widget.eo.c elm_code_widget.eo
eolian_gen $INCLUDE --gi --eo -o elm_code_widget.c elm_code_widget.eo
