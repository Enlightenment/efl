dnl AC_ABSTRACT_SOCKET_TEST(ACTION_IF_FOUND, ACTION_IF_NOT_FOUND)
dnl test if a system supports the abstract socket namespace
dnl by rephorm
AC_DEFUN([AC_ABSTRACT_SOCKET_TEST], [
AC_MSG_CHECKING(abstract sockets)
AC_LANG_PUSH(C)
AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[
// headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
]],
[[
// main fn
#define ABS_SUN_LEN(s, path) (strlen(path) + 1 + (size_t)(((struct sockaddr_un *)NULL)->sun_path))
  int fd;
  struct sockaddr_un sock;
  char *tmp;
  char *name = "/ecore/dbus/abstract/test";

  sock.sun_family = AF_UNIX;
  snprintf(sock.sun_path, sizeof(sock.sun_path), ".%s", name);
  sock.sun_path[0] = '\0';
	fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (bind(fd, (struct sockaddr *)&sock, ABS_SUN_LEN(&sock, name)) < 0)
  {
    printf("Failed to bind to abstract socket.\n");
    exit(1);
  }

  printf ("connected\n");
  exit(0);
]])],
[$1],
[$2])
])

