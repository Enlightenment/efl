dnl use: ECORE_CHECK_POLL(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_CHECK_POLL],
[
_ecore_want_poll=$1
_ecore_have_poll="no"

AC_ARG_ENABLE(poll,
   [AC_HELP_STRING([--disable-poll], [disable poll in the ecore_file module])],
   [
    if test "x${enableval}" = "xyes" ; then
       _ecore_want_poll="yes"
    else
       _ecore_want_poll="no"
    fi
   ])

AC_MSG_CHECKING(whether polling is to be used for filemonitoring)
AC_MSG_RESULT(${_ecore_want_poll})

if test "x${_ecore_want_poll}" = "xyes" ; then
   AC_DEFINE([HAVE_POLL], [1], [ File monitoring with polling ])
   _ecore_have_poll="yes"
fi

if test "x${_ecore_have_poll}" = "xyes" ; then
   m4_default([$2], [:])
else
   m4_default([$3], [:])
fi
])

dnl use: ECORE_CHECK_INOTIFY(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_CHECK_INOTIFY],
[
_ecore_want_inotify=$1
_ecore_have_inotify="no"

dnl We need to check if the right inotify version is accessible
_ecore_want_inotify="yes"
AC_ARG_ENABLE(inotify,
   [AC_HELP_STRING([--disable-inotify], [disable inotify in the ecore_file module])],
   [
    if test "x${enableval}" = "xyes" ; then
       _ecore_want_inotify="yes"
    else
       _ecore_want_inotify="no"
    fi
   ])

AC_MSG_CHECKING(whether inotify is to be used for filemonitoring)
AC_MSG_RESULT($_ecore_want_inotify)

dnl It is hard to find a good test on how to check the correct
dnl inotify version. They changed the headers a lot.
dnl in kernel 2.6.13 __NR_inotify_init was added to the defined syscalls
dnl in asm/unistd.h and IN_MOVE_SELF was added to linux/inotify.h
dnl so with this check you need a very new kernel and kernel-headers!

if test "x${_ecore_want_inotify}" = "xyes" ; then
   AC_CHECK_LIB([c], [inotify_init],
      [
       AC_DEFINE(HAVE_INOTIFY, 1, [ File monitoring with Inotify ])
       AC_DEFINE(HAVE_SYS_INOTIFY, 1, [ File monitoring with Inotify - sys/inotify.h ])
       _ecore_have_inotify="yes"
      ],
      [
       AC_TRY_COMPILE(
          [
           #include <asm/unistd.h>
           #include <linux/inotify.h>
          ],
          [int a = __NR_inotify_init; int b = IN_MOVE_SELF;],
          [
           AC_DEFINE([HAVE_INOTIFY], [1], [ File monitoring with Inotify ])
           _ecore_have_inotify="yes"
          ],
          [_ecore_have_inotify="no"])
      ])
fi

if test "x$_ecore_have_inotify" = "xyes" ; then
   m4_default([$2], [:])
else
   m4_default([$3], [:])
fi
])

dnl use: ECORE_CHECK_CURL(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_CHECK_CURL],
[
_ecore_want_curl=$1
_ecore_have_curl="no"

AC_ARG_ENABLE([curl],
   [AC_HELP_STRING([--disable-curl], [disable curl support])],
   [
    if test "x${enableval}" = "xyes" ; then
       _ecore_want_curl="yes"
    else
       _ecore_want_curl="no"
    fi
   ])

if test "x${_ecore_want_curl}" = "xyes" ; then
   PKG_CHECK_MODULES(CURL, libcurl,
      [
       AC_DEFINE(HAVE_CURL, 1, [ Downloading with CURL ])
       _ecore_have_curl="yes"
      ],
      [_ecore_have_curl="no"])
fi

if test "x$_ecore_have_curl" = "xyes" ; then
   m4_default([$2], [:])
else
   m4_default([$3], [:])
fi
])

dnl use: ECORE_CHECK_GNUTLS(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_CHECK_GNUTLS],
[
_ecore_want_gnutls=$1
_ecore_have_gnutls="no"

AC_ARG_ENABLE([gnutls],
   [AC_HELP_STRING([--disable-gnutls], [disable gnutls support])],
   [
    if test "x${enableval}" = "xyes" ; then
       _ecore_want_gnutls="yes"
    else
       _ecore_want_gnutls="no"
    fi
   ])

if test "x${_ecore_want_gnutls}" = "xyes" -o "x${_ecore_want_gnutls}" = "xauto" ; then
   PKG_CHECK_MODULES([TLS], [gnutls],
      [
       AC_DEFINE([USE_GNUTLS], [1], [Use GnuTLS])
       _ecore_have_gnutls="yes"
      ],
      [_ecore_have_gnutls="no"])
   # for ecore_con_ssl.c
   PKG_CHECK_MODULES([TLS2], [gnutls >= 2.0.0],
      [AC_DEFINE(USE_GNUTLS2, 1, [Use GnuTLS 2 or higher])],
      [dummy="no"])
fi

if test "x$_ecore_have_gnutls" = "xyes" ; then
  ifelse([$2], , :, [$2])
else
  ifelse([$3], , :, [$3])
fi
])

dnl use: ECORE_CHECK_OPENSSL(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_CHECK_OPENSSL],
[
_ecore_want_openssl=$1
_ecore_have_openssl="no"

AC_ARG_ENABLE(openssl,
  [AC_HELP_STRING([--disable-openssl], [disable openssl support])],
  [
    if test "x${enableval}" = "xyes" ; then
       _ecore_want_openssl="yes"
    else
       _ecore_want_openssl="no"
    fi
  ])

if test "x${_ecore_want_openssl}" = "xyes" -o "x${_ecore_want_openssl}" = "xauto"; then
   PKG_CHECK_MODULES([SSL],
      [openssl],
      [
       AC_DEFINE(USE_OPENSSL, 1, [Use OpenSSL])
       _ecore_have_openssl="yes"
      ],
      [_ecore_have_openssl="no"])
fi

if test "x$_ecore_have_openssl" = "xyes" ; then
   m4_default([$2], [:])
else
   m4_default([$3], [:])
fi
])

dnl use: ECORE_CHECK_TSLIB(default-enabled[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
AC_DEFUN([ECORE_CHECK_TSLIB],
[
_ecore_want_tslib=$1
_ecore_have_tslib="no"
TSLIB_LIBS=""
TSLIB_CFLAGS=""

if test "x${_ecore_want_tslib}" = "xyes" -o "x${_ecore_want_tslib}" = "xauto" ; then
   PKG_CHECK_MODULES(TSLIB, tslib-1.0,
     [
      AC_DEFINE(HAVE_TSLIB, 1, [Build Ecore_FB Touchscreen Code])
      _ecore_have_ts="yes"
     ],[
      PKG_CHECK_MODULES(TSLIB, tslib,
        [
         AC_DEFINE(HAVE_TSLIB, 1, [Build Ecore_FB Touchscreen Code])
         _ecore_have_ts="yes"
        ],[
         AC_CHECK_HEADER([tslib.h],
           [
            AC_CHECK_LIB([ts], [ts_open], 
              [
               TSLIB_LIBS="-lts" 
               TSLIB_CFLAGS=""
               AC_DEFINE(HAVE_TSLIB, 1, [Build Ecore_FB Touchscreen Code])
               _ecore_have_ts="yes"
              ],[
               AC_CHECK_LIB([tslib], [ts_open],
                 [
                  TSLIB_LIBS="-ltslib"
                  TSLIB_CFLAGS=""
                  AC_DEFINE(HAVE_TSLIB, 1, [Build Ecore_FB Touchscreen Code])
                  _ecore_have_ts="yes"
                 ],[
                  _ecore_have_ts="no"
                 ])
              ])
           ])
        ])
     ])
fi

AC_SUBST(TSLIB_LIBS)
AC_SUBST(TSLIB_CFLAGS)

if test "x$_ecore_have_tslib" = "xyes" ; then
   m4_default([$2], [:])
else
   m4_default([$3], [:])
fi
])
