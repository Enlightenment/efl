
dnl use: ARG_ENABLE_EVAS_VG_LOADER(loader, default_value)

AC_DEFUN([ARG_ENABLE_EVAS_VG_LOADER],
[dnl
m4_pushdef([DOWN], m4_tolower([$1]))dnl

AC_ARG_ENABLE([vg-loader-[]DOWN],
   [AC_HELP_STRING([--enable-vg-loader-[]DOWN], [enable $1 vg loader. @<:@default=$2@:>@])],
   [
      if test "x${enableval}" = "xyes" ; then
         want_evas_vg_loader_[]DOWN="yes"
      else
         if test "x${enableval}" = "xstatic" ; then
            want_evas_vg_loader_[]DOWN="static"
         else
            if test "x${enableval}" = "xauto" ; then
               want_evas_vg_loader_[]DOWN="auto"
            else
               want_evas_vg_loader_[]DOWN="no"
            fi
         fi
      fi
   ],
   [want_evas_vg_loader_[]DOWN="$2"])
m4_popdef([DOWN])dnl
])

dnl use: EVAS_CHECK_VG_LOADER_DEP_SVG(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_VG_LOADER_DEP_SVG],
[

have_dep="yes"
evas_vg_loader_[]$1[]_cflags=""
evas_vg_loader_[]$1[]_libs=""

AC_SUBST([evas_vg_loader_$1_cflags])
AC_SUBST([evas_vg_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_VG_LOADER_DEP_EET(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_VG_LOADER_DEP_EET],
[

have_dep="yes"
evas_vg_loader_[]$1[]_cflags=""
evas_vg_loader_[]$1[]_libs=""

AC_SUBST([evas_vg_loader_$1_cflags])
AC_SUBST([evas_vg_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: ARG_ENABLE_EVAS_IMAGE_LOADER(loader, default_value)

AC_DEFUN([ARG_ENABLE_EVAS_IMAGE_LOADER],
[dnl
m4_pushdef([DOWN], m4_tolower([$1]))dnl

AC_ARG_ENABLE([image-loader-[]DOWN],
   [AC_HELP_STRING([--enable-image-loader-[]DOWN], [enable $1 image loader. @<:@default=$2@:>@])],
   [
      if test "x${enableval}" = "xyes" ; then
         want_evas_image_loader_[]DOWN="yes"
      else
         if test "x${enableval}" = "xstatic" ; then
            want_evas_image_loader_[]DOWN="static"
         else
            if test "x${enableval}" = "xauto" ; then
               want_evas_image_loader_[]DOWN="auto"
            else
               want_evas_image_loader_[]DOWN="no"
            fi
         fi
      fi
   ],
   [want_evas_image_loader_[]DOWN="$2"])
m4_popdef([DOWN])dnl
])

dnl use: EVAS_CHECK_LOADER_DEP_BMP(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_BMP],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_DDS(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_DDS],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_EET(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_EET],
[

requirement=""
have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

dnl Eet is required
have_dep="yes"

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_GENERIC(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_GENERIC],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_GIF(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

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

if test "x$2" = "xstatic"  && test "x${have_dep}" = "xyes" ; then
   requirements_libs_evas="${evas_image_loader_[]$1[]_libs} ${requirements_libs_evas}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_ICO(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_ICO],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_JPEG(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_JPEG],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_CHECK_HEADER([jpeglib.h], [have_dep="yes"])

if test "x${have_dep}"  = "xyes" ; then
   AC_CHECK_LIB([jpeg],
      [jpeg_CreateDecompress],
      [
       evas_image_loader_[]$1[]_libs="-ljpeg"
       AC_COMPILE_IFELSE(
          [AC_LANG_PROGRAM(
              [[
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
              ]],
              [[
struct jpeg_decompress_struct decomp;
decomp.region_x = 0;
              ]])],
          [have_jpeg_region="yes"],
          [have_jpeg_region="no"])
      ],
      [have_dep="no"]
   )
   if test "x${have_jpeg_region}" = "xyes" ; then
     AC_DEFINE(BUILD_LOADER_JPEG_REGION, [1], [JPEG Region Decode Support])
   fi
fi

if test "x$2" = "xstatic"  && test "x${have_dep}" = "xyes" ; then
   requirements_libs_evas="${evas_image_loader_[]$1[]_libs} ${requirements_libs_evas}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_JP2K(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_JP2K],
[

requirement=""
have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

PKG_CHECK_EXISTS([libopenjp2 >= 2.0],
   [
    have_dep="yes"
    requirement="libopenjp2 >= 2.0"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   fi
   PKG_CHECK_MODULES([JP2K], [${requirement}])
   evas_image_loader_[]$1[]_cflags="${JP2K_CFLAGS}"
   evas_image_loader_[]$1[]_libs="${JP2K_LIBS}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_PMAPS(loader, want_static[[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_PMAPS],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_PNG(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_PNG],
[

requirement=""
have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

dnl libpng.pc is the latest version of libpng that ahs been installed.
dnl We check it first.
PKG_CHECK_EXISTS([libpng >= 1.2.10],
   [
    have_dep="yes"
    requirement="libpng >= 1.2.10"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xno" ; then
   PKG_CHECK_EXISTS([libpng15],
      [
       have_dep="yes"
       requirement="libpng15"
      ],
      [have_dep="no"])
fi

if test "x${have_dep}" = "xno" ; then
   PKG_CHECK_EXISTS([libpng14],
      [
       have_dep="yes"
       requirement="libpng14"
      ],
      [have_dep="no"])
fi

if test "x${have_dep}" = "xno" ; then
   PKG_CHECK_EXISTS([libpng12 >= 1.2.10],
      [
       have_dep="yes"
       requirement="libpng12 >= 1.2.10"
      ],
      [have_dep="no"])
fi

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   fi
   PKG_CHECK_MODULES([PNG], [${requirement}])
   evas_image_loader_[]$1[]_cflags="${PNG_CFLAGS}"
   evas_image_loader_[]$1[]_libs="${PNG_LIBS}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_PSD(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_PSD],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_TGV(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_TGV],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_SVG(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_SVG],
[

requirement=""
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""
version_esvg="0.0.18"
version_ender="0.0.6"

PKG_CHECK_EXISTS([esvg >= ${version_esvg} ender >= ${version_ender}],
   [
    have_dep="yes"
    requirement="esvg >= ${version_esvg} ender >= ${version_ender}"
   ],
   [have_dep="no"])

if test "x${have_dep}" = "xyes" ; then
   if test "x$2" = "xstatic" ; then
      requirements_pc_evas="${requirement} ${requirements_pc_evas}"
      requirements_pc_deps_evas="${requirement} ${requirements_pc_deps_evas}"
   fi
   PKG_CHECK_MODULES([SVG], [${requirement}])
   evas_image_loader_[]$1[]_cflags="${SVG_CFLAGS}"
   evas_image_loader_[]$1[]_libs="${SVG_LIBS}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_TGA(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_TGA],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_TIFF(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

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

if test "x$2" = "xstatic"  && test "x${have_dep}" = "xyes" ; then
   requirements_libs_evas="${evas_image_loader_[]$1[]_libs} ${requirements_libs_evas}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_WBMP(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_WBMP],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_WEBP(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_WEBP],
[

have_dep="no"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_CHECK_HEADER([webp/decode.h], [have_dep="yes"])

if test "x${have_dep}"  = "xyes" ; then
   AC_CHECK_LIB([webp],
      [WebPDecodeRGBA],
      [
       evas_image_loader_[]$1[]_libs="-lwebp"
      ],
      [have_dep="no"]
   )
fi

if test "x$2" = "xstatic"  && test "x${have_dep}" = "xyes" ; then
   requirements_libs_evas="${evas_image_loader_[]$1[]_libs} ${requirements_libs_evas}"
fi

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_LOADER_DEP_XPM(loader, want_static[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])

AC_DEFUN([EVAS_CHECK_LOADER_DEP_XPM],
[

have_dep="yes"
evas_image_loader_[]$1[]_cflags=""
evas_image_loader_[]$1[]_libs=""

AC_SUBST([evas_image_loader_$1_cflags])
AC_SUBST([evas_image_loader_$1_libs])

AS_IF([test "x${have_dep}" = "xyes"], [$3], [$4])

])

dnl use: EVAS_CHECK_IMAGE_LOADER(loader, want_loader, macro)
AC_DEFUN([EVAS_CHECK_IMAGE_LOADER],
[dnl
m4_pushdef([UP], m4_toupper([$1]))dnl
m4_pushdef([DOWN], m4_tolower([$1]))dnl

want_loader="$2"
want_static_loader="no"
have_loader="no"
have_evas_image_loader_[]DOWN="no"

AC_MSG_CHECKING([whether to enable $1 image loader])
AC_MSG_RESULT([${want_loader}])

if test "x${want_loader}" = "xyes" -o "x${want_loader}" = "xstatic" -o "x${want_loader}" = "xauto"; then
   m4_default([EVAS_CHECK_LOADER_DEP_]m4_defn([UP]))(DOWN, ${want_loader}, [have_loader="yes"], [have_loader="no"])
fi

if test "x${have_loader}" = "xno"; then
   if test "x${want_loader}" = "xyes" -o "x${want_loader}" = "xstatic"; then
      AC_MSG_ERROR([$1 dependencies not found])
   fi
fi

AC_MSG_CHECKING([whether $1 image loader will be built])
AC_MSG_RESULT([${have_loader}])

if test "x${have_loader}" = "xyes" ; then
   if test "x${want_loader}" = "xstatic" ; then
      have_evas_image_loader_[]DOWN="static"
      want_static_loader="yes"
   else
      have_evas_image_loader_[]DOWN="yes"
   fi
fi

if test "x${have_loader}" = "xyes" ; then
   AC_DEFINE(BUILD_LOADER_[]UP, [1], [UP Image Loader Support])
fi

AM_CONDITIONAL(BUILD_LOADER_[]UP, [test "x${have_loader}" = "xyes"])

if test "x${want_static_loader}" = "xyes" ; then
   AC_DEFINE(EVAS_STATIC_BUILD_[]UP, [1], [Build $1 image loader inside libevas])
   have_static_module="yes"
fi

EFL_ADD_FEATURE([EVAS_LOADER], DOWN, [${have_evas_image_loader_]DOWN[}])dnl
AM_CONDITIONAL(EVAS_STATIC_BUILD_[]UP, [test "x${want_static_loader}" = "xyes"])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])

dnl use: EVAS_CHECK_VG_LOADER(loader, want_loader, macro)
AC_DEFUN([EVAS_CHECK_VG_LOADER],
[dnl
m4_pushdef([UP], m4_toupper([$1]))dnl
m4_pushdef([DOWN], m4_tolower([$1]))dnl

want_loader="$2"
want_static_loader="no"
have_loader="no"
have_evas_vg_loader_[]DOWN="no"

AC_MSG_CHECKING([whether to enable $1 vg loader])
AC_MSG_RESULT([${want_loader}])

if test "x${want_loader}" = "xyes" -o "x${want_loader}" = "xstatic" -o "x${want_loader}" = "xauto"; then
   m4_default([EVAS_CHECK_VG_LOADER_DEP_]m4_defn([UP]))(DOWN, ${want_loader}, [have_loader="yes"], [have_loader="no"])
fi

if test "x${have_loader}" = "xno"; then
   if test "x${want_loader}" = "xyes" -o "x${want_loader}" = "xstatic"; then
      AC_MSG_ERROR([$1 dependencies not found])
   fi
fi

AC_MSG_CHECKING([whether $1 vg loader will be built])
AC_MSG_RESULT([${have_loader}])

if test "x${have_loader}" = "xyes" ; then
   if test "x${want_loader}" = "xstatic" ; then
      have_evas_vg_loader_[]DOWN="static"
      want_static_loader="yes"
   else
      have_evas_vg_loader_[]DOWN="yes"
   fi
fi

if test "x${have_loader}" = "xyes" ; then
   AC_DEFINE(BUILD_VG_LOADER_[]UP, [1], [UP Image Loader Support])
fi

AM_CONDITIONAL(BUILD_VG_LOADER_[]UP, [test "x${have_loader}" = "xyes"])

if test "x${want_static_loader}" = "xyes" ; then
   AC_DEFINE(EVAS_STATIC_BUILD_VG_[]UP, [1], [Build $1 vg loader inside libevas])
   have_static_module="yes"
fi

EFL_ADD_FEATURE([EVAS_VG_LOADER], DOWN, [${have_evas_vg_loader_]DOWN[}])dnl
AM_CONDITIONAL(EVAS_STATIC_BUILD_VG_[]UP, [test "x${want_static_loader}" = "xyes"])dnl
m4_popdef([UP])dnl
m4_popdef([DOWN])dnl
])
