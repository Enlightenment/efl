DESCRIPTION = "Ecore is the core event abstraction layer for the \
enlightenment foundation libraries. It makes makes doing selections, drag \
and drop, event loops, timeouts and idle handlers fast, optimized, and \
convenient."
HOMEPAGE = "http://www.enlightenment.org"
MAINTAINER = "Carsten Haitzler (Rasterman) <raster@rasterman.com>"
SECTION = "e/libs"
PRIORITY = "optional"
include ecore.oe
inherit native
DEPENDS = "eet-native evas-native"

export EET_CONFIG = "${STAGING_BINDIR}/eet-config-native"
export EVAS_CONFIG = "${STAGING_BINDIR}/evas-config-native"

do_stage () {
  for p in ${parts}; do
    dir=`echo $p|tr A-Z a-z`
    install -m 0644 ${S}/src/lib/$dir/$p.h ${STAGING_INCDIR}/
    oe_libinstall -C src/lib/$dir lib$dir ${STAGING_LIBDIR}/
  done
  install -m 0644 ${S}/src/lib/ecore/Ecore_Data.h ${STAGING_INCDIR}/
  install -m 0644 ${S}/ecore.m4 ${STAGING_DATADIR}/aclocal/
}
