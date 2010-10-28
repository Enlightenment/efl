#!/bin/bash
ret=0

check(){
    i=$1
    shift
    grep -h -B1 -10 -e "^$i(" $@ >eapi.decl
    if [ $? != 0 ]; then
        echo -e "\e[31;1mNOT IMPLEMENTED\e[m\t $i"
        ret=1
        return
    fi
    grep -qe "^--$" eapi.decl
    if [ $? == 0 ]; then
        echo -e "\e[31;1mMULTI IMPLEMENT\e[m\t $i"
        ret=1
        return
    fi

    head -1 eapi.decl | grep -qe '^EAPI'
    if [ $? != 0 ];then
        echo -e "\e[31;1mMISSING EAPI\e[m\t $i"
        ret=1
    fi

    func=$(echo $i | grep -oe 'elm_\w\+')

    sed '2q;d' eapi.decl | grep -qe "elm_widget\w\+(\(const \)\?Evas_Object \*"
    if [ $? == 0 ];then
        tail -n9 eapi.decl | grep -q "\(API_ENTRY\|_elm_widget_is\)"
        if [ $? != 0 ];then
            echo -e "\e[31;1mMISSING CHECKER\e[m\t $i"
            ret=1
        fi
        return
    fi

    sed '2q;d' eapi.decl | grep -qe "elm_object_\w\+("
    if [ $? == 0 ];then
        rm eapi.decl
        return
    fi

    sed '2q;d' eapi.decl | grep -qe "elm_\w\+_add("
    if [ $? == 0 ];then
        rm eapi.decl
        return
    fi

    sed '2q;d' eapi.decl | grep -qe "elm_\w\+(\(const \)\?Evas_Object \*"
    if [ $? == 0 ];then
        tail -n9 eapi.decl | grep -q "ELM_CHECK_WIDTYPE"
        if [ $? != 0 ];then
            echo -e "\e[31;1mMISSING CHECKER\e[m\t $i"
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

rm -f eapi.decl
rm eapi.list
exit $ret
