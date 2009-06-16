MAINTAINERCLEANFILES = Makefile.in

if BUILD_LOADER_EDB
if !EVAS_STATIC_BUILD_EDB
edb_subdir = edb
endif
endif

if BUILD_LOADER_EET
if !EVAS_STATIC_BUILD_EET
eet_subdir = eet
endif
endif

if BUILD_SAVER_JPEG
if !EVAS_STATIC_BUILD_JPEG
jpeg_subdir = jpeg
endif
endif

if BUILD_LOADER_PNG
if !EVAS_STATIC_BUILD_PNG
png_subdir = png
endif
endif

if BUILD_LOADER_TIFF
if !EVAS_STATIC_BUILD_TIFF
tiff_subdir = tiff
endif
endif

SUBDIRS = $(edb_subdir) $(eet_subdir) $(jpeg_subdir) $(png_subdir) $(tiff_subdir)
