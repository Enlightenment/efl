#!/bin/sh

awk '{printf("\"%s\\n\"\n", $0);}' $1
