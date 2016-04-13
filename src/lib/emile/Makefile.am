
### Library
lib_LTLIBRARIES = libemile.la

installed_emilemainheadersdir = $(includedir)/emile-@VMAJ@
dist_installed_emilemainheaders_DATA = \
Emile.h \
emile_cipher.h \
emile_compress.h \
emile_image.h \
emile_base64.h

libemile_la_SOURCES = \
emile_private.h \
emile_main.c \
emile_compress.c \
emile_image.c \
emile_base64.c \
../../static_libs/rg_etc/rg_etc1.c \
../../static_libs/rg_etc/rg_etc2.c \
../../static_libs/rg_etc/rg_etc1.h \
../../static_libs/rg_etc/etc2_encoder.c

if ! ENABLE_LIBLZ4
libemile_la_SOURCES += \
../../static_libs/lz4/lz4.c \
../../static_libs/lz4/lz4.h \
../../static_libs/lz4/lz4hc.c \
../../static_libs/lz4/lz4hc.h
endif

if HAVE_CRYPTO_GNUTLS
libemile_la_SOURCES += emile_cipher_gnutls.c
else
if HAVE_CRYPTO_OPENSSL
libemile_la_SOURCES += emile_cipher_openssl.c
else
libemile_la_SOURCES += emile_cipher.c
endif
endif

libemile_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl \
-I$(top_srcdir)/src/static_libs/rg_etc \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/emile\" \
@EMILE_CFLAGS@ \
@NEON_CFLAGS@

if ! ENABLE_LIBLZ4
libemile_la_CPPFLAGS += \
-I$(top_srcdir)/src/static_libs/lz4
endif

libemile_la_LIBADD = @EMILE_SUBBUILD_LIBS@
libemile_la_DEPENDENCIES = @EMILE_SUBBUILD_INTERNAL_LIBS@
libemile_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@

if ENABLE_LIBLZ4
libemile_la_LIBADD += @LIBLZ4_LIBS@
endif
