##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_define([v_maj], [1])
m4_define([v_min], [10])
m4_define([v_mic], [0])
dnl m4_define([v_rev], m4_esyscmd([(git rev-list --count HEAD 2>/dev/null || echo 0) | tr -d '\n']))
##--   When released, remove the dnl on the below line
dnl m4_undefine([v_rev])
##--   When doing snapshots - change soname. remove dnl on below line
m4_define([relname], [release])
dnl m4_define([v_rel], [-release relname])
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], [m4_define([v_ver], [v_maj.v_min.v_mic.v_rev])], [m4_define([v_ver], [v_maj.v_min.v_mic])])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], v_mic)
m4_define([lt_age], v_min)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##

AC_INIT([evas_generic_loaders], [v_ver-beta3], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
m4_ifdef([v_rev], , [m4_define([v_rev], [0])])
m4_ifdef([v_rel], , [m4_define([v_rel], [])])
AC_DEFINE_UNQUOTED(VMAJ, [v_maj], [Major version])
AC_DEFINE_UNQUOTED(VMIN, [v_min], [Minor version])
AC_DEFINE_UNQUOTED(VMIC, [v_mic], [Micro version])
AC_DEFINE_UNQUOTED(VREV, [v_rev], [Revison])
version_info="lt_cur:lt_rev:lt_age"
release_info="v_rel"
AC_SUBST(version_info)
AC_SUBST(release_info)
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##--##
VMAJ=v_maj
AC_SUBST(VMAJ)

### Needed information

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

### Checks for libraries
AC_PROG_CC
AC_PROG_CXX
PKG_PROG_PKG_CONFIG

### Checks for libraries

# Eina library
PKG_CHECK_MODULES(EINA, [eina >= 1.6.99])

AC_ARG_ENABLE([poppler],
   [AC_HELP_STRING([--disable-poppler], [disable poppler support  @<:@default==enabled@:>@])],
   [want_poppler=$enableval],
   [want_poppler="yes"])

if test "x${want_poppler}" = "xyes" ; then
   PKG_CHECK_EXISTS([poppler >= 0.12 poppler < 0.20],
      [
       poppler_pc="poppler >= 0.12 poppler < 0.20"
       have_poppler="yes"
      ],
      [have_poppler="no"])

   if test "x${have_poppler}" = "xno" ; then
      PKG_CHECK_EXISTS([poppler >= 0.20],
         [
          poppler_pc="poppler >= 0.20"
          AC_DEFINE(HAVE_POPPLER_020, 1, [Have poppler 0.20])
          have_poppler="yes"
         ],
         [have_poppler="no"])
   fi

   if test "x${have_poppler}" = "xyes" ; then
      PKG_CHECK_MODULES([POPPLER], [${poppler_pc}],
         [have_poppler="yes"],
         [have_poppler="no"])
   fi

   if test "x${have_poppler}" = "xyes" ; then
      AC_LANG(C++)
      save_CPPFLAGS=$CPPFLAGS
      CPPFLAGS="${CPPFLAGS} ${POPPLER_CFLAGS}"
      save_CXXFLAGS=$CXXFLAGS
      CXXFLAGS="${CXXFLAGS} ${POPPLER_CFLAGS}"
      AC_CHECK_HEADERS([GlobalParams.h],
		       [have_poppler="yes"],
		       [have_poppler="no"])
      CXXFLAGS=$save_CXXFLAGS
      CPPFLAGS=$save_CPPFLAGS
      AC_LANG(C)
   fi
else
   have_poppler="no"
fi

AC_ARG_ENABLE([spectre],
   [AC_HELP_STRING([--disable-spectre], [disable spectre support  @<:@default==enabled@:>@])],
   [want_spectre=$enableval],
   [want_spectre="yes"])

if test "x$want_spectre" = "xyes" ; then
   PKG_CHECK_MODULES([SPECTRE], [libspectre], [have_ps="yes"], [have_ps="no"])
else
   have_ps=no
fi


AC_ARG_ENABLE([libraw],
   [AC_HELP_STRING([--disable-libraw], [disable libraw support  @<:@default==enabled@:>@])],
   [want_libraw=$enableval],
   [want_libraw="yes"])

if test "x$want_libraw" = "xyes" ; then
   PKG_CHECK_MODULES([LIBRAW], [libraw], [have_raw="yes"], [have_raw="no"])
else
   have_raw=no
fi

AC_ARG_ENABLE([svg],
   [AC_HELP_STRING([--disable-svg], [disable svg support  @<:@default==enabled@:>@])],
   [want_svg=$enableval],
   [want_svg="yes"]
)

have_svg_2_36="no"
if test "x$want_svg" = "xyes" ; then
   PKG_CHECK_MODULES([SVG],  [librsvg-2.0 >= 2.14.0 cairo >= 1.0.0], [have_svg="yes"], [have_svg="no"])
   PKG_CHECK_MODULES(
      [SVG_2_36], [librsvg-2.0 >= 2.36.0 cairo >= 1.0.0],
      [
       have_svg_2_36="yes"
       AC_DEFINE(HAVE_SVG_2_36, 1, [Have librsvg >= 2.36])
      ],
      [have_svg_2_36="no"])
else
   have_svg=no
fi
GST_REQS=0.10.13
GSTPLUG_REQS=0.10.13
GST_MAJORMINOR=0.10

AC_ARG_ENABLE([gstreamer],
   [AC_HELP_STRING([--disable-gstreamer], [disable gstreamer support  @<:@default==enabled@:>@])],
   [want_gstreamer=$enableval],
   [want_gstreamer="yes"])

if test "x$want_gstreamer" = "xyes" ; then
   PKG_CHECK_MODULES([GSTREAMER],
      [gstreamer-$GST_MAJORMINOR >= $GST_REQS
       gstreamer-plugins-base-$GST_MAJORMINOR >= $GSTPLUG_REQS
      ],
      [have_gst="yes"],
      [have_gst="no"])
else
   have_gst=no
fi
AM_CONDITIONAL([HAVE_GST], [test "x${have_gst}" = "xyes"])
AM_CONDITIONAL([HAVE_PDF], [test "x${have_poppler}" = "xyes"])
AM_CONDITIONAL([HAVE_PS],  [test "x${have_ps}" = "xyes"])
AM_CONDITIONAL([HAVE_RAW], [test "x${have_raw}" = "xyes"])
AM_CONDITIONAL([HAVE_SVG], [test "x${have_svg}" = "xyes"])

### Checks for header files

AC_CHECK_HEADERS(netinet/in.h unistd.h)

AC_CHECK_HEADER([zlib.h],
   [dummy="yes"],
   [AC_MSG_ERROR("Cannot find zlib.h. Make sure your CFLAGS environment variable contains include lines for the location of this file")])

SHM_OPEN_LIBS=""
AC_MSG_CHECKING([whether shm_open() is present])
LIBS_save=${LIBS}
LIBS="${LIBS} -lrt"
AC_LINK_IFELSE(
   [AC_LANG_PROGRAM(
      [[
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
      ]],
      [[
int fd;
fd = shm_open("/", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
shm_unlink("/");
      ]])],
   [
    have_shm_open="yes"
    AC_DEFINE(HAVE_SHM_OPEN, 1, [Have shm_open() call])
    SHM_OPEN_LIBS="-lrt"
   ],
   [have_shm_open="no"])
LIBS=${LIBS_save}
AC_MSG_RESULT([${have_shm_open}])
AC_SUBST(SHM_OPEN_LIBS)

### Checks for compiler characteristics
AM_PROG_CC_C_O
AC_C_INLINE
AC_C_BIGENDIAN
AC_C___ATTRIBUTE__

### Checks for library functions

AC_CONFIG_FILES([
Makefile
src/Makefile
src/bin/Makefile
src/bin/common/Makefile
src/bin/gst/Makefile
src/bin/pdf/Makefile
src/bin/ps/Makefile
src/bin/raw/Makefile
src/bin/xcf/Makefile
src/bin/svg/Makefile
pkgbuild/PKGBUILD
])

AC_OUTPUT

#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo
echo "Configuration Options Summary:"
echo
echo "Loaders:"
echo "  XCF..................: yes"
echo "  Gstreamer............: ${have_gst}"
echo "  PDF..................: ${have_poppler}"
echo "  PS...................: ${have_ps}"
echo "  RAW..................: ${have_raw}"
echo "  SVG..................: ${have_svg}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo
