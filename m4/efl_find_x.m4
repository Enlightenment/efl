# efl_find.x.m4 - Macros to locate X11.                       -*- Autoconf -*-
# EFL_FIND_X(VARIABLE-PREFIX, [headers = "X11/Xlib.h"],
#            [libs-and-functions = "X11 XOpenDisplay"],
#            [action-if-found], [action-if-not-found])
# checks for X11 using, in order:
# 1) Xorg pkg-config files (using enviroment variables EFL_X11_CFLAGS
#                           and EFL_X11_LIBS if set, but only if x11.pc exists)
# 2) command line options: --x-includes=dir, --x-libraries=dir
#                          assume there is an X11 in the given directories
# 3) XMKMF environment variable if set
# 4) xmkmf executable if found
# 5) list of "standard" directories
#
# 2-5 is handled by A_PATH_X
#
# If a X11 is found, [action-if-success] is run and VARIABLE_cflags and
# VARIABLE_libs and VARIABLE_libdirs are defined and substituted.
# VARIABLE_libs will contain all of the libs listed in libs-and-functions.
# VARIABLE_libdirs will contain all -Lpath:s found in VARIABLE_libs
#
# headers is a list of headers to look for. libs-and-functions is a list of
# library and function pairs to look for.
# Each lib and function is checked in pairs, example:
# EFL_FIND_X([EVAS_X11], [X11/X.h], [X11 XCreateImage Xext XShmCreateImage])
# will look for XCreateImage in X11 and XShmCreateImage in Xext and include
# both -lX11 and -lXext in VARIABLE_libs
#
# action-if-found is only called if X11, all headers, all libraries and
# all functions are found.
# You can call EFL_FIND_X multiple times with different lists of headers, libs
# and functions.

AC_DEFUN([EFL_FIND_X],
[
  # Must print something as AC_CACHE_VAL writes (cached) if the value is cached
  AC_MSG_CHECKING([how to find X])
  efl_x11_need_result=1
  AC_CACHE_VAL(efl_cv_x11_cache,
    [
      # this is ugly and breaks that AC_CACHE_VAL may not have side effects
      # but I can't think of a better way right now
      efl_x11_need_result=0
      PKG_CHECK_EXISTS([x11],
        [
          AC_MSG_RESULT([use pkg-config])
          PKG_CHECK_MODULES([EFL_X11],[x11],
            [
              efl_cv_have_x11=yes
              efl_cv_x11_pkgconf=yes
              efl_cv_x11_cflags=$EFL_X11_CFLAGS
              efl_cv_x11_libs_pre=$EFL_X11_LIBS
              efl_cv_x11_libs_post=
            ])
        ],
        [
          AC_MSG_RESULT([use xmkmf])
          # Fallback to old AC_PATH_XTRA
          AC_PATH_X
          AC_PATH_XTRA
          if test "$no_x" = yes; then
            efl_cv_have_x11=no
          else
            efl_cv_have_x11=yes
            efl_cv_x11_pkgconf=no
            efl_cv_x11_cflags=$X_CFLAGS
            efl_cv_x11_libs_pre="$X_PRE_LIBS $X_LIBS"
            efl_cv_x11_libs_post=$X_EXTRA_LIBS
          fi
         ])
      # Record where we found X for the cache.
      if test "x$efl_cv_have_x11" = "xno"; then
        efl_cv_x11_cache="efl_cv_have_x11=no"
      else
        efl_cv_x11_cache="efl_cv_have_x11=yes\
          efl_cv_x11_pkgconf='$efl_cv_x11_pkgconf'\
          efl_cv_x11_cflags='$efl_cv_x11_cflags'\
          efl_cv_x11_libs_pre='$efl_cv_x11_libs_pre'\
          efl_cv_x11_libs_post='$efl_cv_x11_libs_post'"
      fi
    ])
  if test "x$efl_x11_need_result" = "x1"; then
    AC_MSG_RESULT([already found])
  fi
  eval "$efl_cv_x11_cache"

  if test "x$efl_cv_have_x11" = "xyes"; then
    ELF_X11_CFLAGS_save="$CFLAGS"
    ELF_X11_CPPFLAGS_save="$CPPFLAGS"
    CFLAGS="$CFLAGS $efl_cv_x11_cflags"
    CPPFLAGS="$CPPFLAGS $efl_cv_x11_cflags"
    efl_x11_found_all=1
    for efl_x11_header in ifelse([$2], , "X11/Xlib.h", [$2]); do
      AC_CHECK_HEADER([$efl_x11_header],,[
        efl_x11_found_all=0
        break])
    done
    CPPFLAGS="$ELF_X11_CPPFLAGS_save"
    CFLAGS="$ELF_X11_CFLAGS_save"

    if test "x$efl_x11_found_all" = "x1"; then
      EFL_X11_LIBS_save="$LIBS"
      if test "x$efl_cv_x11_pkgconf" = "xyes"; then
        efl_x11_modules="x11"
        efl_x11_lib=""
        for efl_x11_lib_function in ifelse([$3], , "X11 XOpenDisplay", [$3]); do
          if test -z "$efl_x11_lib"; then
            efl_x11_lib="$efl_x11_lib_function"
            case $efl_x11_lib in
              X11)
                ;;
              Xss)
                efl_x11_modules="$efl_x11_modules xscrnsaver"
                ;;
              *)
                efl_x11_lib=`echo $efl_x11_lib | tr '[A-Z]' '[a-z]'`
                efl_x11_modules="$efl_x11_modules $efl_x11_lib"
                ;;
            esac
          else
            efl_x11_lib=
          fi
        done
        efl_x11_modules="$efl_x11_modules x11-xcb"

        PKG_CHECK_EXISTS([$efl_x11_modules],
          [
            PKG_CHECK_MODULES([$1],[$efl_x11_modules],
              [
                efl_x11_cflags=$[]$1[]_CFLAGS
                efl_x11_libs=$[]$1[]_LIBS

                LIBS="$LIBS $[]$1[]_LIBS"
                efl_x11_lib=""
                for efl_x11_lib_function in ifelse([$3], , "X11 XOpenDisplay", [$3]); do
                  if test -z "$efl_x11_lib"; then
                    efl_x11_lib="$efl_x11_lib_function"
                  else
                    # This is a ugly way of using AC_CHECK_FUNC with different
                    # LIBS
                    eval "unset ac_cv_func_$efl_x11_lib_function"
                    AC_CHECK_FUNC([$efl_x11_lib_function],,
                      [
                        efl_x11_found_all=0
                        break])
                    efl_x11_lib=
                  fi
                done
              ])
           ],[efl_x11_found_all=0])
      else
        LIBS="$LIBS $efl_cv_x11_libs_pre"
        efl_x11_libs="$efl_cv_x11_libs_pre"
        efl_x11_lib=""
        for efl_x11_lib_function in ifelse([$3], , "X11 XOpenDisplay", [$3]); do
          if test -z "$efl_x11_lib"; then
            efl_x11_lib="$efl_x11_lib_function"
          else
            AC_CHECK_LIB([$efl_x11_lib], [$efl_x11_lib_function],,[
              efl_x11_found_all=0
              break],["$efl_cv_x11_libs_post"])
            efl_x11_libs="$efl_x11_libs -l$efl_x11_lib"
            efl_x11_lib=
          fi
        done
        if test -n "$efl_cv_x11_libs_post"; then
          efl_x11_libs="$efl_x11_libs $efl_cv_x11_libs_post"
        fi
      fi
      LIBS="$EFL_X11_LIBS_save"
    fi
  fi

  if test "x$efl_x11_found_all" = "x1"; then
    efl_x11_libdirs=""
    for efl_x11_option in "$efl_x11_libs"; do
      case $efl_x11_option in
        -L*)
          efl_x11_libdirs="$efl_x11_libdirs $efl_x11_option"
          ;;
        *)
          ;;
      esac
    done

    AC_SUBST([$1][_cflags],[$efl_cv_x11_cflags])
    AC_SUBST([$1][_libs],[$efl_x11_libs])
    AC_SUBST([$1][_libdirs],[$efl_x11_libdirs])

    ifelse([$4], , :, [$4])
  else
    ifelse([$5], , :, [$5])
  fi
])

