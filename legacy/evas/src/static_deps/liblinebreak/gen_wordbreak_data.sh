#!/bin/sh
FNAME="WordBreakProperty.txt";
if [ ! -f ${FNAME} ]; then
   wget http://www.unicode.org/Public/UNIDATA/auxiliary/WordBreakProperty.txt
fi

# 
sed -n 's/\(^[0-9A-F.]\+\)/\1/p' ${FNAME} > tmp.txt
sed -i 's/^\([0-9A-F]\+\)\s\+/\1..\1/' tmp.txt
sort --numeric-sort tmp.txt > tmp2.txt
./sort_numeric_hex.py tmp2.txt > tmp.txt
rm tmp2.txt
sed -i -n 's/^\([0-9A-F]\+\)..\([0-9A-F]\+\)\s*;\s*\([A-Za-z]\+\).*$/\t{0x\1, 0x\2, WBP_\3},/p' tmp.txt 

echo "/* The content of this file is generated from:" > wordbreakdata.x
head -2 ${FNAME} >> wordbreakdata.x
echo "*/" >> wordbreakdata.x
echo '#include "linebreak.h"' >> wordbreakdata.x
echo '#include "wordbreakdef.h"' >> wordbreakdata.x
echo "static struct WordBreakProperties wb_prop_default[] = {" >> wordbreakdata.x
cat tmp.txt >> wordbreakdata.x
echo "	{0xFFFFFFFF, 0xFFFFFFFF, WBP_Undefined}" >> wordbreakdata.x
echo "};" >> wordbreakdata.x
rm tmp.txt

