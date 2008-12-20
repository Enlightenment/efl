dnl use: EVAS_CHECK_LOADER_DEP_EDB(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_EDB],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

PKG_CHECK_MODULES([EDB], [edb], [have_dep="yes"], [have_dep="no"])
evas_image_loader_[]$1[]_cflags="${EDB_CFLAGS}"
evas_image_loader_[]$1[]_libs="${EDB_LIBS}"

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_EET(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_EET],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

PKG_CHECK_MODULES([EET], [eet >= 1.0.1], [have_dep="yes"], [have_dep="no"])
evas_image_loader_[]$1[]_cflags="${EET_CFLAGS}"
evas_image_loader_[]$1[]_libs="${EET_LIBS}"

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_GIF(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_GIF],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_CHECK_HEADER([gif_lib.h], [have_dep="yes"])

if test "x${have_dep}"  = "xyes" ; then
   AC_CHECK_LIB([gif],
      [DGifOpenFileName],
      [
       evas_image_loader_[]$1[]_libs="-lgif"
      ],
      [have_dep="no"]
   )

   if test "x${have_dep}"  = "xno" ; then
      AC_CHECK_LIB([ungif],
         [DGifOpenFileName],
         [
          have_dep="yes"
          evas_image_loader_[]$1[]_libs="-lungif"
         ]
      )
   fi
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_JPEG(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_JPEG],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_CHECK_HEADER([jpeglib.h], [have_dep="yes"])

if test "x${have_dep}"  = "xyes" ; then
   AC_CHECK_LIB([jpeg],
      [jpeg_CreateDecompress],
      [evas_image_loader_[]$1[]_libs="-ljpeg"],
      [have_dep="no"]
   )
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_PMAPS(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_PMAPS],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_PNG(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_PNG],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

PKG_CHECK_EXISTS([libpng12],
   [PKG_CHECK_MODULES([PNG], [libpng12], [have_dep="yes"], [have_dep="no"])],
   [PKG_CHECK_EXISTS([libpng10],
       [PKG_CHECK_MODULES([PNG], [libpng10], [have_dep="yes"], [have_dep="no"])],
       [PKG_CHECK_MODULES([PNG], [libpng], [have_dep="yes"], [have_dep="no"])])]
)

evas_image_loader_[]$1[]_cflags="${PNG_CFLAGS}"
evas_image_loader_[]$1[]_libs="${PNG_LIBS}"

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_SVG(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_SVG],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

PKG_CHECK_MODULES([SVG], [librsvg-2.0 >= 2.14.0],
   [have_dep="yes"],
   [have_svg="no"]
)

if test "x${have_dep}" = "xyes" ; then
   evas_image_loader_[]$1[]_cflags="${SVG_CFLAGS}"
   evas_image_loader_[]$1[]_libs="${SVG_LIBS}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_TIFF(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_TIFF],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_CHECK_HEADER([tiffio.h], [have_dep="yes"])

if test "x${have_dep}"  = "xyes" ; then
   AC_CHECK_LIB([tiff],
      [TIFFReadScanline],
      [
       evas_image_loader_[]$1[]_libs="-ltiff"
      ],
      [have_dep="no"]
   )

   if test "x${have_dep}"  = "xno" ; then
      AC_CHECK_LIB([tiff],
         [TIFFReadScanline],
         [
          have_dep="yes"
          evas_image_loader_[]$1[]_libs="-ltiff -ljpeg -lz -lm"
         ]
      )
   fi

   if test "x${have_dep}"  = "xno" ; then
      AC_CHECK_LIB([tiff34],
         [TIFFReadScanline],
         [
          have_dep="yes"
          evas_image_loader_[]$1[]_libs="-ltiff34 -ljpeg -lz -lm"
         ]
      )
   fi
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_LOADER_DEP_XPM(loader[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_XPM],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

if test "x${have_dep}" = "xyes" ; then
  m4_default([$2], [:])
else
  m4_default([$3], [:])
fi

])

dnl use: EVAS_CHECK_IMAGE_LOADER(loader, want_loader, macro)


AC_DEFUN([EVAS_CHECK_IMAGE_LOADER],
[

m4_pushdef([UP], m4_toupper([[$1]]))
m4_pushdef([DOWN], m4_tolower([[$1]]))

want_loader="$2"
have_evas_image_loader_[]DOWN="no"

AC_ARG_ENABLE([image-loader-[]DOWN],
   [AC_HELP_STRING([--disable-image-loader-[]DOWN], [disable $1 image loader])],
   [want_loader=${enableval}]
)

AC_MSG_CHECKING([whether to enable $1 image loader])
AC_MSG_RESULT([${want_loader}])

if test "x${want_loader}" = "xyes" -o "x${want_loader}" = "xauto"; then
   m4_default([EVAS_CHECK_LOADER_DEP_]m4_defn([UP]))(DOWN, [have_evas_image_loader_[]DOWN="yes"], [have_evas_image_loader_[]DOWN="no"])
fi

if test "x${have_evas_image_loader_[]DOWN}" = "xno" -a "x${want_loader}" = "xyes" -a "x${use_strict}" = "xyes" ; then
   AC_MSG_ERROR([$1 dependencies not found (strict dependencies checking)])
fi

if test "x${have_evas_image_loader_[]DOWN}" = "xyes" ; then
   AC_DEFINE(BUILD_LOADER_[]UP, [1], [UP Image Loader Support])
fi

AM_CONDITIONAL(BUILD_LOADER_[]UP, [test "x${have_evas_image_loader_[]DOWN}" = "xyes"])

m4_popdef([UP])
m4_popdef([DOWN])

])

dnl use: EVAS_CHECK_FONT_LOADER(want)


AC_DEFUN([EVAS_CHECK_FONT_LOADER],
[

pushdef([UP], translit([$1], [a-z], [A-Z]))dnl
pushdef([DOWN], translit([$1], [A-Z], [a-z]))dnl

want_loader="$1"
have_evas_font_loader_eet="no"

AC_ARG_ENABLE([font-loader-eet],
   [AC_HELP_STRING([--disable-font-loader-eet],
       [disable EET font loader. [[default=enabled]]])],
   [want_loader=${enableval}]
)

AC_MSG_CHECKING([whether to enable Eet font loader])
AC_MSG_RESULT([${want_loader}])

if test "x$want_loader" = "xyes" -o "x$want_loader" = "xauto"; then
    PKG_CHECK_MODULES([EET], [eet >= 1.0.1], [have_evas_font_loader_eet="yes"], [have_evas_font_loader_eet="yes"])
fi

if test "x${have_evas_font_loader_eet}" = "xno" -a "x$want_loader" = "xyes" -a "x$use_strict" = "xyes" ; then
   AC_MSG_ERROR([Eet dependencies not found (strict dependencies checking)])
fi

if test "x$have_evas_font_loader_eet" = "xyes" ; then
   AC_DEFINE([BUILD_FONT_LOADER_EET], [1], [EET Font Loader Support])
fi

popdef([UP])
popdef([DOWN])

])
