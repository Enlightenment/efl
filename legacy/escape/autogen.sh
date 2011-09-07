#!/bin/sh

rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh

touch README
touch ABOUT-NLS

echo "Running aclocal..." ; aclocal $ACLOCAL_FLAGS -I m4 || exit 1
echo "Running autoheader..." ; autoheader || exit 1
echo "Running autoconf..." ; autoconf || exit 1
echo "Running libtoolize..." ; (libtoolize --copy --automake || glibtoolize --automake) || exit 1
echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1

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

if [ -z "$NOCONFIGURE" ]; then
        AR="powerpc64-ps3-elf-ar" CC="powerpc64-ps3-elf-gcc" RANLIB="powerpc64-ps3-elf-ranlib" CFLAGS="-O2 -Wall -I$PSL1GHT/ppu/include -I$PS3DEV/portlibs/ppu/include" CXXFLAGS="-I$PSL1GHT/ppu/include -I$PS3DEV/portlibs/ppu/include"  LDFLAGS="-L$PSL1GHT/ppu/lib -L$PS3DEV/portlibs/ppu/lib" PKG_CONFIG_LIBDIR="$PSL1GHT/ppu/lib/pkgconfig" PKG_CONFIG_PATH="$PS3DEV/portlibs/ppu/lib/pkgconfig" ./configure   --prefix="$PS3DEV/portlibs/ppu"   --host=powerpc64-ps3-elf    --includedir="$PS3DEV/portlibs/ppu/include"   --libdir="$PS3DEV/portlibs/ppu/lib" -C "$@"

fi
