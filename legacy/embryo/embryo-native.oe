DESCRIPTION = "Embryo is a tiny library designed to provide a virutal \
machine system for executing small pieces of code logic very quickly."
HOMEPAGE = "http://www.enlightenment.org"
MAINTAINER = "Carsten Haitzler (Rasterman) <raster@rasterman.com>"
SECTION = "e/libs"
PRIORITY = "optional"
include embryo.oe
inherit native
DEPENDS = ""

do_stage () {
  install -m 0755 src/bin/.libs/embryo ${STAGING_BINDIR}
  install -m 0755 src/bin/embryo_cc ${STAGING_BINDIR}
  oe_libinstall -C src/lib libembryo ${STAGING_LIBDIR}/
  install -m 0644 ${S}/src/lib/Embryo.h ${STAGING_INCDIR}/
}
