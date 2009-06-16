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

if BUILD_LOADER_GIF
if !EVAS_STATIC_BUILD_GIF
gif_subdir = gif
endif
endif

if BUILD_LOADER_JPEG
if !EVAS_STATIC_BUILD_JPEG
jpeg_subdir = jpeg
endif
endif

if BUILD_LOADER_PMAPS
if !EVAS_STATIC_BUILD_PMAPS
pmaps_subdir = pmaps
endif
endif

if BUILD_LOADER_PNG
if !EVAS_STATIC_BUILD_PNG
png_subdir = png
endif
endif

if BUILD_LOADER_SVG
if !EVAS_STATIC_BUILD_SVG
svg_subdir = svg
endif
endif

if BUILD_LOADER_TIFF
if !EVAS_STATIC_BUILD_TIFF
tiff_subdir = tiff
endif
endif

if BUILD_LOADER_XPM
if !EVAS_STATIC_BUILD_XPM
xpm_subdir = xpm
endif
endif


SUBDIRS = $(edb_subdir) $(eet_subdir) $(gif_subdir) $(jpeg_subdir) $(pmaps_subdir) $(png_subdir) $(svg_subdir) $(tiff_subdir) $(xpm_subdir)
DIST_SUBDIRS = edb eet gif jpeg pmaps png svg tiff xpm
