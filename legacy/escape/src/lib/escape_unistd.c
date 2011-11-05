#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <net/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/net.h>
#include <string.h>
#include <libiberty.h>
#include <sys/stat.h>

#include "Escape.h"

char *
escape_realpath(const char *path, char *resolved_path)
{
   char *real = lrealpath (path);

   if (real)
     {
        if (resolved_path)
          {
             memcpy (resolved_path, real, PATH_MAX);
             free (real);
             return resolved_path;
          }
        else
          {
             return real;
          }
     }

   return NULL;
}

int
escape_access(const char *pathname, int mode)
{
   struct stat stat_buf;

   if (stat(pathname, &stat_buf) != 0)
     return -1;

   if (mode == F_OK)
     return 0;
   if (mode == R_OK)
     {
        if (stat_buf.st_mode & S_IRUSR)
          return 0;
        errno = EACCES;
        return -1;
     }
   if (mode == W_OK)
     {
        if (stat_buf.st_mode & S_IWUSR)
          return 0;
        errno = EROFS;
        return -1;
     }
   if (mode == X_OK)
     {
        if (stat_buf.st_mode & S_IXUSR)
          return 0;
        errno = EACCES;
        return -1;
     }

   return 0;
}

EAPI ssize_t
escape_readlink(const char *path,
                char       *buf,
                size_t      bufsize)
{
   errno = EINVAL;
   return -1;
}

EAPI int
escape_symlink(const char *path1, const char *path2)
{
   errno = EINVAL;
   return -1;
}

/*
 * The code of the following functions has been kindly offered
 * by Tor Lillqvist.
 */
int
escape_pipe(int *fds)
{
   struct sockaddr_in saddr;
   int temp;
   int socket1 = -1;
   int socket2 = -1;
   fd_set read_set;
   fd_set write_set;
   int len;

   temp = socket (AF_INET, SOCK_STREAM, 0);

   if (temp == -1)
     goto out0;

   memset (&saddr, 0, sizeof (saddr));
   saddr.sin_family = AF_INET;
   saddr.sin_port = 0;
   saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

   if (bind (temp, (struct sockaddr *)&saddr, sizeof (saddr)))
     goto out0;

   if (listen (temp, 1) == -1)
     goto out0;

   len = sizeof (saddr);
   if (getsockname (temp, (struct sockaddr *)&saddr, &len))
     goto out0;

   socket1 = socket (AF_INET, SOCK_STREAM, 0);

   if (socket1 == -1)
     goto out0;

   if ((connect (socket1, (struct sockaddr *)&saddr, len) == -1) &&
       (errno != EAGAIN))
     goto out1;

   FD_ZERO (&read_set);
   FD_SET (temp, &read_set);

   if (select (0, &read_set, NULL, NULL, NULL) == -1)
     goto out1;

   if (!FD_ISSET (temp, &read_set))
     goto out1;

   socket2 = accept (temp, (struct sockaddr *)&saddr, &len);
   if (socket2 == -1)
     goto out1;

   FD_ZERO (&write_set);
   FD_SET (socket1, &write_set);

   if (select (0, NULL, &write_set, NULL, NULL) == -1)
     goto out2;

   if (!FD_ISSET (socket1, &write_set))
     goto out2;

   fds[0] = socket1;
   fds[1] = socket2;

   closesocket (temp);

   return 0;

out2:
   closesocket (socket2);
out1:
   closesocket (socket1);
out0:
   closesocket (temp);

   fds[0] = -1;
   fds[1] = -1;

   return -1;
}

#undef access
int
access(const char *pathname, int mode)
{
   return escape_access (pathname, mode);
}

