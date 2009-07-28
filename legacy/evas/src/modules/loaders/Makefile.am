MAINTAINERCLEANFILES = Makefile.in

SUBDIRS =

if BUILD_LOADER_EDB
if !EVAS_STATIC_BUILD_EDB
SUBDIRS += edb
endif
endif

if BUILD_LOADER_EET
if !EVAS_STATIC_BUILD_EET
SUBDIRS += eet
endif
endif

if BUILD_LOADER_GIF
if !EVAS_STATIC_BUILD_GIF
SUBDIRS += gif
endif
endif

if BUILD_LOADER_JPEG
if !EVAS_STATIC_BUILD_JPEG
SUBDIRS += jpeg
endif
endif

if BUILD_LOADER_PMAPS
if !EVAS_STATIC_BUILD_PMAPS
SUBDIRS += pmaps
endif
endif

if BUILD_LOADER_PNG
if !EVAS_STATIC_BUILD_PNG
SUBDIRS += png
endif
endif

if BUILD_LOADER_SVG
if !EVAS_STATIC_BUILD_SVG
SUBDIRS += svg
endif
endif

if BUILD_LOADER_TIFF
if !EVAS_STATIC_BUILD_TIFF
SUBDIRS += tiff
endif
endif

if BUILD_LOADER_XPM
if !EVAS_STATIC_BUILD_XPM
SUBDIRS += xpm
endif
endif


SUBDIRS = $(edb_subdir) $(eet_subdir) $(gif_subdir) $(jpeg_subdir) $(pmaps_subdir) $(png_subdir) $(svg_subdir) $(tiff_subdir) $(xpm_subdir)
DIST_SUBDIRS = edb eet gif jpeg pmaps png svg tiff xpm
