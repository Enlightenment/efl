# # ============================================================================
# #  http://www.gnu.org/software/autoconf-archive/ax_cxx_compile_stdcxx_11.html
# # ============================================================================
# #
# # SYNOPSIS
# #
# #   AX_CXX_COMPILE_STDCXX_11([ext|noext],[mandatory|optional])
# #
# # DESCRIPTION
# #
# #   Check for baseline language coverage in the compiler for the C++11
# #   standard; if necessary, add switches to CXXFLAGS to enable support.
# #
# #   The first argument, if specified, indicates whether you insist on an
# #   extended mode (e.g. -std=gnu++11) or a strict conformance mode (e.g.
# #   -std=c++11).  If neither is specified, you get whatever works, with
# #   preference for an extended mode.
# #
# #   The second argument, if specified 'mandatory' or if left unspecified,
# #   indicates that baseline C++11 support is required and that the macro
# #   should error out if no mode with that support is found.  If specified
# #   'optional', then configuration proceeds regardless, after defining
# #   HAVE_CXX11 if and only if a supporting mode is found.
# #
# # LICENSE
# #
# #   Copyright (c) 2016 Felipe Magno de Almeida <felipe@expertisesolutions.com.br>
# #
# #   Copying and distribution of this file, with or without modification, are
# #   permitted in any medium without royalty provided the copyright notice
# #   and this notice are preserved. This file is offered as-is, without any
# #   warranty.

# AC_LANG(CSHARP)
# -----------
AC_LANG_DEFINE([CSHARP], [csharp], [MCS], [],
[ac_ext=cs
ac_compile='$MCS $MCSFLAGS conftest.$ac_ext >&AS_MESSAGE_LOG_FD'
])

AU_DEFUN([AC_LANG_CSHARP], [AC_LANG(CSHARP)])

m4_define([AC_LANG_PROGRAM(CSHARP)],
[$1
class MyClass
{
    static void Main(string[] args)
    {
        $2
    }
}])

AC_DEFUN([AC_LANG_COMPILER(CSHARP)],
[AC_REQUIRE([AC_PROG_MCS])])

AN_MAKEVAR([MCS], [AC_PROG_MCS])
AN_PROGRAM([mcs], [AC_PROG_MCS])
AC_DEFUN([AC_PROG_MCS],
[AC_LANG_PUSH(CSHARP)dnl
AC_ARG_VAR([MCS],     [MCS (C#) compiler command])dnl
AC_ARG_VAR([MCSFLAGS],  [MCS (C#) compiler flags])dnl
dnl _AC_ARG_VAR_LDFLAGS()dnl
m4_ifval([$1],
      [AC_CHECK_TOOLS(MCS, [$1])],
[AC_CHECK_TOOL(MCS, mcs)
if test -z "$MCS"; then
  if test -n "$ac_tool_prefix"; then
    AC_CHECK_PROG(MCS, [${ac_tool_prefix}mcs], [$ac_tool_prefix}mcs])
  fi
fi
if test -z "$MCS"; then
  AC_CHECK_PROG(MCS, mcs, mcs, , , false)
fi
if test -z "$MCS"; then
  HAVE_MCS=0
  AC_MSG_NOTICE([No C sharp compiler was found])
else
  HAVE_MCS=1
  AC_DEFINE(HAVE_MCS,1,
            [define if the MCS compiler is available])
fi
AC_SUBST(HAVE_MCS)
])

# Provide some information about the compiler.
_AS_ECHO_LOG([checking for _AC_LANG compiler version])
set X $ac_compile
ac_compiler=$[2]
_AC_DO_LIMIT([$ac_compiler --version >&AS_MESSAGE_LOG_FD])
m4_expand_once([_AC_COMPILER_EXEEXT])[]dnl
m4_expand_once([_AC_COMPILER_OBJEXT])[]dnl
AC_LANG_POP(CSHARP)dnl
])# AC_PROG_MCS


