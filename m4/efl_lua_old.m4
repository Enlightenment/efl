dnl EFL_CHECK_LUA_OLD(EFL)
dnl checks for lua 5.1 or 5.2 in pkg-config (multiple names) and -llua directly
dnl will call EFL_ADD_LIBS() or EFL_DEPEND_PKG() as required.
dnl this is a strict call and will abort if lua is not found
dnl keep in mind that this is only executed if --enable-lua-old is set
AC_DEFUN([EFL_CHECK_LUA_OLD],
[dnl
requirement_lua=""
PKG_CHECK_EXISTS([lua >= 5.1.0], [requirement_lua="lua >= 5.1.0"],
   [PKG_CHECK_EXISTS([lua5.1 >= 5.1.0], [requirement_lua="lua5.1 >= 5.1.0"],
      [PKG_CHECK_EXISTS([lua-5.1 >= 5.1.0], [requirement_lua="lua-5.1 >= 5.1.0"],
         [PKG_CHECK_EXISTS([lua51 >= 5.1.0], [requirement_lua="lua51 >= 5.1.0"],
            [PKG_CHECK_EXISTS([lua5.2 >= 5.2.0], [requirement_lua="lua5.2 >= 5.2.0"],
               [PKG_CHECK_EXISTS([lua-5.2 >= 5.2.0], [requirement_lua="lua-5.2 >= 5.2.0"],
                  [PKG_CHECK_EXISTS([lua52 >= 5.2.0], [requirement_lua="lua52 >= 5.2.0"])])])])])])])

if test "x${requirement_lua}" = "x"; then
   AC_MSG_CHECKING([whether lua_newstate() is in liblua])
   AC_CHECK_LIB([lua], [lua_newstate],
      [have_lua="yes"
       EFL_ADD_LIBS([$1], [-llua])],
      [have_lua="no"])
   AC_MSG_RESULT([${have_lua}])
   if test "${have_lua}" = "no"; then
      AC_MSG_ERROR([Missing lua 5.1 or 5.2 support])
   fi
else
   EFL_DEPEND_PKG([$1], [LUA], [${requirement_lua}])
fi
])
