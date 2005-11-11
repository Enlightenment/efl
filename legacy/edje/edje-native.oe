DESCRIPTION = "Edje is a graphical layout and animation library for animated \
resizable, compressed and scalable themes."
HOMEPAGE = "http://www.enlightenment.org"
MAINTAINER = "Carsten Haitzler (Rasterman) <raster@rasterman.com>"
SECTION = "e/libs"
PRIORITY = "optional"
include edje.oe
inherit native
DEPENDS = "evas-native ecore-native embryo-native eet-native"

EXTRA_OECONF = "--enable-fb-only"

export EDB_CONFIG = "${STAGING_BINDIR}/edb-config-native"
export EET_CONFIG = "${STAGING_BINDIR}/eet-config-native"
export FREETYPE_CONFIG = "${STAGING_BINDIR}/freetype-config-native"
export EVAS_CONFIG = "${STAGING_BINDIR}/evas-config-native"
export ECORE_CONFIG = "${STAGING_BINDIR}/ecore-config-native"
export EMBRYO_CONFIG = "${STAGING_BINDIR}/embryo-config-native"

do_stage() {
  for i in edje edje_ls edje_cc; do
    install -m 0755 src/bin/$i ${STAGING_BINDIR}
  done
  oe_libinstall -C src/lib libedje ${STAGING_LIBDIR}/  
}

