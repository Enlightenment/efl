dnl Copyright (C) 2011 Vincent Torri <vincent dot torri at gmail dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macro that select the Windows version (XP (default), Vista, 7)

dnl Usage: EFL_SELECT_WINDOWS_VERSION()
dnl Update CPPFLAGS accordingly

AC_DEFUN([EFL_SELECT_WINDOWS_VERSION],
[

dnl configure option

AC_ARG_WITH([windows-version],
   [AC_HELP_STRING([--with-windows-version], [select the target Windows version (xp, vista or win7) @<:@default=xp@:>@])],
   [
    if test "x${with_windows_version}" = "xvista" ; then
       _winver="vista"
    else
       if test "x${with_windows_version}" = "xwin7" ; then
          _winver="win7"
       else
          if test "x${with_windows_version}" = "ce" ; then
             _winver="ce"
          else
             _winver="xp"
          fi
       fi
    fi
   ],
   [_winver="xp"])

AC_MSG_CHECKING([which Windows version to target])
AC_MSG_RESULT([${_winver}])

case "${_winver}" in
  vista)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0600"
     _efl_windows_version="Windows Vista"
     ;;
  win7)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0601"
     _efl_windows_version="Windows 7"
     ;;
  *)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0501"
     _efl_windows_version="Windows XP"
     ;;
esac

EFL_CFLAGS="${EFL_CFLAGS} ${EFL_WINDOWS_VERSION_CFLAGS}"
AC_SUBST([EFL_WINDOWS_VERSION_CFLAGS])
AC_SUBST([_efl_windows_version])

])
