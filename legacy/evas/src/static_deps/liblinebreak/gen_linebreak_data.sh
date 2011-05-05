#!/bin/sh
if [ ! -f "LineBreak.txt" ]; then
   wget http://unicode.org/Public/UNIDATA/LineBreak.txt
fi

sed -n -f LineBreak1.sed LineBreak.txt > tmp.txt
sed -f LineBreak2.sed tmp.txt | ./filter_dup > tmp.c
head -2 LineBreak.txt > tmp.txt
cat linebreakdata1.tmpl tmp.txt linebreakdata2.tmpl tmp.c linebreakdata3.tmpl > linebreakdata.c
rm tmp.txt tmp.c

