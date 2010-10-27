#!/bin/bash
ret=0
check(){
    func=$(echo $1 | grep -oe 'elm_\w\+')
    echo $1 | grep -qe "_get([^\(const \)]\([^,\*)]\)\+\*"
    if [ $? == 0 ];then
        echo -e "\e[31;1mNOT CONST\e[m\t $func"
        ret=1
    fi

}

HEADERS="src/lib/Elementary.h.in
src/lib/Elementary_Cursor.h
src/lib/elm_widget.h"

IFS=$'\n'

for header in $HEADERS; do
    grep -e "^ *EAPI.*(.*)" $header | grep -e 'elm_\w\+_get' >eapi.list

    for i in $(cat eapi.list); do
        check $i $header
    done
    rm eapi.list
done
exit $ret
