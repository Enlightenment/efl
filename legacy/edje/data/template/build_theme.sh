#!/bin/sh -e
THEME="default"
APPNAME=""
edje_cc -v -id ./images -fd ./fonts $THEME.edc $THEME.eet
if [ $? = "0" ]; then
    if [ "$APPNAME" = "" ]; then
	echo "Build was successful"
    else
	PREFIX=`dirname \`which $APPNAME\` | sed 's/bin//'`
	sudo cp $THEME.eet $PREFIX"share/$APPNAME/themes/"
	echo -n "Installed theme to "
	echo $PREFIX"share/$APPNAME/themes/"
    fi
else
    echo "Building failed"
fi
