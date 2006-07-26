MAINTAINERCLEANFILES = Makefile.in

if BUILD_ECORE_EVAS
ECORE_EVAS_LIB = $(top_builddir)/src/lib/ecore_evas/libecore_evas.la
else
ECORE_EVAS_LIB =
endif

if BUILD_ECORE_DIRECTFB
ECORE_DIRECTFB_LIB = $(top_builddir)/src/lib/ecore_directfb/libecore_directfb.la
else
ECORE_DIRECTFB_LIB =
endif

if BUILD_ECORE_X
ECORE_X_LIB = $(top_builddir)/src/lib/ecore_x/libecore_x.la
else
ECORE_X_LIB =
endif

if BUILD_ECORE_FB
ECORE_FB_LIB = $(top_builddir)/src/lib/ecore_fb/libecore_fb.la
else
ECORE_FB_LIB =
endif

if BUILD_ECORE_JOB
ECORE_JOB_LIB = $(top_builddir)/src/lib/ecore_job/libecore_job.la
else
ECORE_JOB_LIB =
endif

if BUILD_ECORE_CON
ECORE_CON_LIB = $(top_builddir)/src/lib/ecore_con/libecore_con.la
else
ECORE_CON_LIB =
endif

if BUILD_ECORE_IPC
ECORE_IPC_LIB = $(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la
else
ECORE_IPC_LIB =
endif

if BUILD_ECORE_TXT
ECORE_TXT_LIB = $(top_builddir)/src/lib/ecore_txt/libecore_txt.la
else
ECORE_TXT_LIB =
endif

if BUILD_ECORE_CONFIG
ECORE_CONFIG_LIB = $(top_builddir)/src/lib/ecore_config/libecore_config.la
else
ECORE_CONFIG_LIB =
endif

if BUILD_ECORE_DBUS
ECORE_DBUS_LIB = $(top_builddir)/src/lib/ecore_dbus/libecore_dbus.la
else
ECORE_DBUS_LIB =
endif

INCLUDES = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_evas \
-I$(top_srcdir)/src/lib/ecore_directfb \
-I$(top_srcdir)/src/lib/ecore_x \
-I$(top_srcdir)/src/lib/ecore_fb \
-I$(top_srcdir)/src/lib/ecore_job \
-I$(top_srcdir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore_ipc \
-I$(top_srcdir)/src/lib/ecore_txt @iconv_cflags@ \
-I$(top_srcdir)/src/lib/ecore_config \
-I$(top_srcdir)/src/lib/ecore_dbus \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_evas \
-I$(top_builddir)/src/lib/ecore_directfb \
-I$(top_builddir)/src/lib/ecore_x \
-I$(top_builddir)/src/lib/ecore_fb \
-I$(top_builddir)/src/lib/ecore_job \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_builddir)/src/lib/ecore_ipc \
-I$(top_builddir)/src/lib/ecore_txt @iconv_cflags@ \
-I$(top_builddir)/src/lib/ecore_config \
-I$(top_builddir)/src/lib/ecore_dbus \
@EVAS_CFLAGS@ \
@x_cflags@ \
@EET_CFLAGS@ \
-DPACKAGE_BIN_DIR=\"@PACKAGE_BIN_DIR@\" \
-DPACKAGE_LIB_DIR=\"@PACKAGE_LIB_DIR@\" \
-DPACKAGE_DATA_DIR=\"@PACKAGE_DATA_DIR@\"

bin_PROGRAMS = \
ecore_test \
ecore_evas_test \
ecore_config \
ecore_dbus_test \
ecore_dbus_hal_test

ecore_test_SOURCES = \
ecore_test.c

ecore_test_LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_X_LIB) \
$(ECORE_TXT_LIB) \
$(ECORE_JOB_LIB) \
$(ECORE_FB_LIB) \
$(ECORE_EVAS_LIB) \
$(ECORE_CON_LIB) \
$(ECORE_IPC_LIB) \
-lm @iconv_libs@

ecore_test_CFLAGS = \
$(CFLAGS) \
$(INCLUDES)

ecore_test_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_X_LIB) \
$(ECORE_TXT_LIB) \
$(ECORE_JOB_LIB) \
$(ECORE_FB_LIB) \
$(ECORE_EVAS_LIB) \
$(ECORE_CON_LIB) \
$(ECORE_IPC_LIB)

ecore_evas_test_SOURCES = \
ecore_evas_test.c \
ecore_evas_test_app.c \
ecore_evas_test_bg.c \
ecore_evas_test_calibrate.c \
ecore_evas_test.h

ecore_evas_test_LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_X_LIB) \
$(ECORE_TXT_LIB) \
$(ECORE_JOB_LIB) \
$(ECORE_FB_LIB) \
$(ECORE_EVAS_LIB) \
$(ECORE_CON_LIB) \
$(ECORE_IPC_LIB) \
-lm @iconv_libs@

ecore_evas_test_CFLAGS = \
$(CFLAGS) \
$(INCLUDES)

ecore_evas_test_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_JOB_LIB) \
$(ECORE_FB_LIB) \
$(ECORE_X_LIB) \
$(ECORE_EVAS_LIB) \
$(ECORE_CON_LIB) \
$(ECORE_IPC_LIB) \
$(ECORE_TXT_LIB)

ecore_config_SOURCES = \
ecore_config.c

ecore_config_LDADD = \
$(ECORE_CONFIG_LIB)

ecore_config_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_CONFIG_LIB)

ecore_dbus_test_SOURCES = \
ecore_dbus_test.c \
ecore_dbus_test.h

ecore_dbus_test_LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_DBUS_LIB)

ecore_dbus_test_CFLAGS = \
$(CFLAGS) \
$(INCLUDES)

ecore_dbus_test_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_DBUS_LIB)


ecore_dbus_hal_test_SOURCES = \
ecore_dbus_hal_test.c \
ecore_dbus_test.h

ecore_dbus_hal_test_LDADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_DBUS_LIB)

ecore_dbus_hal_test_CFLAGS = \
$(CFLAGS) \
$(INCLUDES)

ecore_dbus_hal_test_DEPENDENCIES = \
$(top_builddir)/src/lib/ecore/libecore.la \
$(ECORE_DBUS_LIB)

