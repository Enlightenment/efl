#!/bin/sh
od --width=4 -t x4 -v $1 | \
awk '{ if (NF > 1) printf("0x%s, ", $2); L = L + 1; if (L > 5) { L = 0; printf("\n");}}'
