#!/bin/bash
ret=0

sed -n '/^EAPI /{n;p;}' src/lib/*.c >eapi.list

for i in $(cat eapi.list|grep -o "^\w\+"); do
    grep -q "\<$i\>" src/lib/elm_priv.h
    if [ $? == 0 ]; then
        echo -e "\e[31;1mEAPI in priv.h\e[m\t $i"
        ret=1
    fi

    echo $i | grep -qe '^_\?elm_widget'
    if [ $? == 0 ]; then
        grep -qe "EAPI.*\<$i\>" src/lib/elm_widget.h
        if [ $? != 0 ]; then
            echo -e "\e[31;1m WIDGET missing in widget.h\e[m\t $i"
            ret=1
        fi
        grep -qe "EAPI.*\<$i\>" src/lib/Elementary.h.in
        if [ $? == 0 ]; then
            echo -e "\e[31;1m WIDGET in Elm.h\e[m\t $i"
            ret=1
        fi
    else
        grep -qe "EAPI.*\<$i\>" src/lib/Elementary.h.in
        if [ $? != 0 ]; then
            echo -e "\e[31;1m ELM missing in Elm.h\e[m\t $i"
            ret=1
        fi
        grep -qe "EAPI.*\<$i\>" src/lib/elm_widget.h
        if [ $? == 0 ]; then
            echo -e "\e[31;1m ELM in widget.h\e[m\t $i"
            ret=1
        fi
    fi
done

rm eapi.list
exit $ret
