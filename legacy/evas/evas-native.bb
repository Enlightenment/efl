DESCRIPTION = "Evas is a clean display canvas API for several target display \
systems that can draw anti-aliased text, smooth super and sub-sampled scaled \
images, alpha-blend objects much and more."
HOMEPAGE = "http://www.enlightenment.org"
MAINTAINER = "Carsten Haitzler (Rasterman) <raster@rasterman.com>"
SECTION = "e/libs"
PRIORITY = "optional"
include evas.oe
inherit native
DEPENDS = "libpng-native jpeg-native eet-native freetype-native"

export FREETYPE_CONFIG = "${STAGING_BINDIR}/freetype-config-native"
export EET_CONFIG = "${STAGING_BINDIR}/eet-config-native"

do_stage () {
  for i in ${headers}; do
    install -m 0644 ${S}/src/lib/$i ${STAGING_INCDIR}/
  done
  oe_libinstall -C src/lib libevas ${STAGING_LIBDIR}/
}
