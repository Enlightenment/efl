MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @EEZE_CFLAGS@ \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_LIB_DIR=\"$(libdir)\"

AM_CFLAGS = @EEZE_CFLAGS@

includesdir = $(includedir)/eeze-@VMAJ@

# Sensor modules
pkgdir = $(libdir)/eeze-sensor
if HAVE_EEZE_TIZEN
pkg_LTLIBRARIES = eeze-sensor-tizen.la
eeze_sensor_tizen_la_SOURCES = eeze_sensor_tizen.c
eeze_sensor_tizen_la_LDFLAGS = -no-undefined -module -avoid-version @TIZEN_SENSOR_LIBS@
eeze_sensor_tizen_la_LIBTOOLFLAGS = --tag=disable-static
else
pkg_LTLIBRARIES = eeze-sensor-fake.la
eeze_sensor_fake_la_SOURCES = eeze_sensor_fake.c
eeze_sensor_fake_la_LDFLAGS = -no-undefined -module -avoid-version
eeze_sensor_fake_la_LIBTOOLFLAGS = --tag=disable-static
endif


