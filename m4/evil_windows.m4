dnl Copyright (C) 2011 Vincent Torri <vincent dot torri at gmail dot com>
dnl This code is public domain and can be freely used or copied.

dnl Macro that select the Windows version (XP (default), Vista, 7)

dnl Usage: EFL_SELECT_WINDOWS_VERSION()
dnl Update CPPFLAGS accordingly

AC_DEFUN([EFL_SELECT_WINDOWS_VERSION],
[

dnl configure option

AC_ARG_WITH([windows-version],
   [AC_HELP_STRING([--with-windows-version], [select the target Windows version (vista, win7, win8, win81 or win10) @<:@default=win7@:>@])],
   [_winver=${with_windows_version}],
   [_winver="win7"])

AC_MSG_CHECKING([which Windows version to target])
AC_MSG_RESULT([${_winver}])

case "${_winver}" in
  vista)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0600 -DWINVER=0x0600"
     _efl_windows_version="Windows Vista"
     ;;
  win7)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0601 -DWINVER=0x0601"
     _efl_windows_version="Windows 7"
     ;;
  win8)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0602 -DWINVER=0x0602"
     _efl_windows_version="Windows 8"
     ;;
  win81)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0603 -DWINVER=0x0603"
     _efl_windows_version="Windows 8.1"
     ;;
  win10)
     EFL_WINDOWS_VERSION_CFLAGS="-D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00"
     _efl_windows_version="Windows 10"
     ;;
  *)
     AC_MSG_ERROR([Wrong Windows version passed to configure. Run ./configure --help])
     ;;
esac

EFL_CFLAGS="${EFL_CFLAGS} ${EFL_WINDOWS_VERSION_CFLAGS}"
AC_SUBST([EFL_WINDOWS_VERSION_CFLAGS])
AC_SUBST([_efl_windows_version])
AC_MSG_NOTICE([Targetting ${_efl_windows_version}])

])
