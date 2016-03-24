#!/bin/bash
ret=0

BLACKLIST="tests/test_setter_blacklist.txt"

check(){
    func=$(echo $1 | grep -oe '_\?elm_\w\+')
    base=${func%_set}
    cat $BLACKLIST | grep -q $func
    if [ $? == 0 ];then
	return
    fi
    echo $1 | grep -qe "_set(const \+[^\(char \+\*\)]"
    if [ $? == 0 ];then
        echo -e "\e[31;1mCONST\e[m\t $func"
        ret=1
    fi
    echo $1 | grep -qe "EAPI \+\(void\|Eina_Bool\)"
    if [ $? != 0 ];then
        echo -e "\e[31;1mUNKNOWN RETURN\e[m\t $func"
        ret=1
    fi
    echo $1 | grep -qe "\(cb_set\|all_set\)("
    if [ $? != 0 ];then
        grep -qe "EAPI.*\<${base}_get\>" $2
        if [ $? != 0 ];then
            echo -e "\e[31;1mMISSING GETTER\e[m\t $func"
            ret=1
        fi
        echo $1 | grep -qe "\(content\|icon\|end\)_set(\w\+ \+\*obj, \w\+ \+\*\w\+);"
        if [ $? == 0 ];then
            grep -qe "EAPI.*\<${base}_unset\>" $2
            if [ $? != 0 ];then
                echo -e "\e[31;1mMISSING UNSETTER\e[m\t $func"
                ret=1
            fi
        fi
    fi
}

HEADERS="src/lib/Elementary.h.in
src/lib/Elementary_Cursor.h
src/lib/elm_widget.h"

IFS=$'\n'

for header in $HEADERS; do
    grep -e "^ *EAPI.*(.*)" $header | grep -e 'elm_\w\+_set' >eapi.list

    for i in $(cat eapi.list); do
        check $i $header
    done
    rm eapi.list
done
exit $ret
