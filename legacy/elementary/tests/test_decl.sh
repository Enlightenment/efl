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
    ass=$(sed '2q;d' eapi.decl)

    echo $i | grep -qe "^elm_object"
    if [ $? == 0 ];then
        rm eapi.decl
        return
    fi

    echo $i | grep -qe "^elm_widget"
    if [ $? == 0 ];then

        echo $ass | grep -qe "elm_widget\w\+(\(const \)\?Evas_Object \*"
        if [ $? == 0 ];then
            tail -n9 eapi.decl | grep -q "\(API_ENTRY\|_elm_widget_is\)"
            if [ $? != 0 ];then
                echo -e "\e[31;1mMISSING CHECKER\e[m\t $i"
                ret=1
            fi
            return
        fi

        echo $ass | grep -qe "elm_widget\w\+(\(const \)\?Elm_ \*"
        if [ $? == 0 ];then
            tail -n9 eapi.decl | grep -qe "ELM_WIDGET_ITEM_CHECK"
            if [ $? != 0 ];then
                echo -e "\e[31;1mMISSING CHECKER\e[m\t $i"
                ret=1
            fi
            return
        fi
    fi

    echo $ass | grep -qe "elm_\w\+_add"
    if [ $? == 0 ];then
        echo -e "\e[32;1m???\e[m\t $i"
        return
    fi

    echo $ass | grep -qe "elm_\w\+(\(const \)\?Evas_Object \*"
    if [ $? == 0 ];then
        tail -n9 eapi.decl | grep -q "ELM_CHECK_WIDTYPE"
        if [ $? != 0 ];then
            echo -e "\e[31;1mMISSING CHECKER\e[m\t $i"
            ret=1
        fi
        return
    fi

    echo $ass | grep -qe "elm_\w\+(\(const \)\?Elm_\w\+_Item \*"
    if [ $? == 0 ];then
        tail -n9 eapi.decl | grep -qe "ELM_\w\+_ITEM_.*CHECK"
        if [ $? != 0 ];then
            echo -e "\e[31;1mMISSING CHECKER\e[m\t $i"
            ret=1
        fi
        return
    fi

    rm eapi.decl
}

grep -e "^ *EAPI.*(.*)" src/lib/Elementary.h.in | grep -oe 'elm_\w\+' >eapi.list

for i in $(cat eapi.list); do
    check $i $(ls -1 src/lib/*.c | grep -v elm_widget)
done

grep -e "^ *EAPI.*(.*)" src/lib/elm_widget.h | grep -oe '_\?elm_\w\+' >eapi.list

for i in $(cat eapi.list); do
    check $i src/lib/elm_widget.c
done

rm -f eapi.decl
rm eapi.list
exit $ret
