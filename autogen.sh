#!/bin/sh

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.
(
  cd "$srcdir" &&
  (
    rm -rf autom4te.cache
    rm -f aclocal.m4 ltmain.sh

    autoreconf -vif
  )

  W=0

  rm -f config.cache-env.tmp
  echo "OLD_PARM=\"$@\"" >> config.cache-env.tmp
  echo "OLD_CFLAGS=\"$CFLAGS\"" >> config.cache-env.tmp
  echo "OLD_PATH=\"$PATH\"" >> config.cache-env.tmp
  echo "OLD_PKG_CONFIG_PATH=\"$PKG_CONFIG_PATH\"" >> config.cache-env.tmp
  echo "OLD_LDFLAGS=\"$LDFLAGS\"" >> config.cache-env.tmp

  cmp config.cache-env.tmp config.cache-env >> /dev/null
  if [ $? -ne 0 ]; then
    W=1;
  fi

  if [ $W -ne 0 ]; then
    echo "Cleaning configure cache...";
    rm -f config.cache config.cache-env
    mv config.cache-env.tmp config.cache-env
  else
    rm -f config.cache-env.tmp
  fi
)

if [ -z "$NOCONFIGURE" ]; then
  exec $srcdir/configure -C "$@"
fi
