#! /bin/sh

abort () {
    echo "$1 not found or command failed. Aborting!"
    exit 1
}

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

THEDIR="`pwd`"
cd "$srcdir"
DIE=0

set -x
aclocal || abort "aclocal"
libtoolize --ltdl --force --copy || abort "libtoolize"
autoheader || abort "autoheader"
automake --foreign --add-missing || abort "automake"
autoconf || abort "autoconf"

if test -z "$*"; then
        echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
fi

cd "$THEDIR"

$srcdir/configure "$@" || abort "configure"

set +x

echo "Now type:"
echo
echo "make"
echo "make install"
echo
echo "have fun."

