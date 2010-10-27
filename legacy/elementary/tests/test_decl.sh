#!/bin/bash
ret=0

check(){
    i=$1
    shift
    sed -n "/^$i(/{g;1!p;};h" $@ >eapi.decl
    lines=$(wc -l eapi.decl|cut -f1 -d' ')
    if [ $lines == 0 ]; then
        echo -e "\e[31;1mNOT IMPLEMENTED\e[m\t $i"
        ret=1
    elif [ $lines != 1 ]; then
        echo -e "\e[31;1mMULTI IMPLEMENTED\e[m\t $i"
        ret=1
    else
        cat eapi.decl | grep -qe '^EAPI'
        if [ $? != 0 ];then
            echo -e "\e[31;1mMISSING EAPI\e[m\t $i"
            ret=1
        fi
    fi
    rm eapi.decl
}

grep -e "^ *EAPI.*(.*)" src/lib/Elementary.h.in | grep -oe 'elm_\w\+' >eapi.list

for i in $(cat eapi.list); do
    check $i $(ls -1 src/lib/*.c | grep -v elm_widget | grep -v els_cursor)
done

grep -e "^ *EAPI.*(.*)" src/lib/Elementary_Cursor.h | grep -oe 'elm_\w\+' >eapi.list

for i in $(cat eapi.list); do
    check $i src/lib/els_cursor.c src/lib/elm_main.c
done

grep -e "^ *EAPI.*(.*)" src/lib/elm_widget.h | grep -oe '_\?elm_\w\+' >eapi.list

for i in $(cat eapi.list); do
    check $i src/lib/elm_widget.c
done

rm eapi.list
exit $ret
