#include "elementary_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef HAVE_ENVIRON
# define _GNU_SOURCE 1
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#ifdef _WIN32
# include <direct.h> /* getcwd */
#endif

#ifdef HAVE_ENVIRON
extern char **environ;
#endif

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

int
main(int argc, char **argv)
{
   int sock, socket_unix_len, i, n;
   struct sockaddr_un socket_unix;
   char buf[PATH_MAX];
   struct stat st;
   char *exe;
   int we_are_elementary_run = 0;
   char *domain;
   char *cwd;
   char *rundir;

   int sargc, slen, envnum;
   unsigned char *sbuf = NULL, *pos;
   char **sargv = NULL;

   if (!getcwd(buf, sizeof(buf) - 1))
     {
        fprintf(stderr, "elementary_quicklaunch: currect working dir too big.\n");
        exit(-1);
     }
   cwd = strdup(buf);
   if (!(domain = getenv("ELM_QUICKLAUNCH_DOMAIN")))
     {
        domain = getenv("WAYLAND_DISPLAY");
        if (!domain) domain = getenv("DISPLAY");
        if (!domain) domain = "unknown";
     }
   rundir = getenv("XDG_RUNTIME_DIR");
   if (!rundir) rundir = "/tmp";
   snprintf(buf, sizeof(buf), "%s/elm-ql-%i/%s", rundir, getuid(), domain);
   if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
     {
        perror("elementary_quicklaunch: socket(AF_UNIX, SOCK_STREAM, 0)");
        exit(-1);
     }
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
   socket_unix.sun_path[(int)(sizeof(socket_unix.sun_path)/sizeof(socket_unix.sun_path[0])) - 1] = '\0';
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   if (connect(sock, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     {
        perror("elementary_quicklaunch: connect(sock, (struct sockaddr *)&socket_unix, socket_unix_len)");
        printf("elementary_quicklaunch: cannot connect to socket '%s'\n", buf);
        exit(1);
     }
   exe = argv[0];
   if (!(((exe[0] == '/')) ||
         ((exe[0] == '.') && (exe[1] == '/')) ||
         ((exe[0] == '.') && (exe[1] == '.') && (exe[2] == '/'))))
     {
        char *path = getenv("PATH");
        int exelen = strlen(argv[0]);
        if (path)
          {
             const char *p, *pp;

             p = path;
             pp = p;
             exe = NULL;
             for (;;)
               {
                  if ((*p == ':') || (!*p))
                    {
                       unsigned int len;

                       len = p - pp;
                       if (len < (sizeof(buf) - exelen - 3))
                         {
                            strncpy(buf, pp, len);
                            strcpy(buf + len, "/");
                            strcpy(buf + len + 1, argv[0]);
                            if (!access(buf, R_OK | X_OK))
                              {
                                 exe = buf;
                                 break;
                              }
                            if (!*p) break;
                            p++;
                            pp = p;
                         }
                    }
                  else
                    {
                       if (!*p) break;
                       p++;
                    }
               }
          }
     }
   if (exe)
     {
        if (!lstat(exe, &st))
          {
             if (S_ISLNK(st.st_mode))
               {
                  char buf2[PATH_MAX];

                  ssize_t len = readlink(exe, buf2, sizeof(buf2) - 1);
                  if (len >= 0)
                    {
                       char *p;
                       buf2[len] = 0;
                       p = strrchr(buf2, '/');
                       if (p) p++;
                       else p = buf2;
                       if (!strncasecmp(p, "elementary_run", 14))
                         we_are_elementary_run = 1;
                    }
               }
          }
     }
   if (we_are_elementary_run)
     {
        sargc = argc;
        sargv = argv;
     }
   else
     {
        sargc = argc - 1;
        sargv = &(argv[1]);
     }

   slen = 0;
   envnum = 0;

   // header:
   //  UL 'total bytes'
   //  UL 'argnum'
   //  UL 'envnum'
   slen += sizeof(unsigned long) * 3;

   for (i = 0; i < sargc; i++)
     {
        slen += sizeof(unsigned long);
        slen += strlen(sargv[i]) + 1;
     }

#ifdef HAVE_ENVIRON
   // count how much space is needed for environment
   for (i = 0; environ[i]; i++)
     {
        slen += sizeof(unsigned long);
        slen += strlen(environ[i]) + 1;
        envnum++;
     }
#endif

   // how much space is needed for cwd
   slen += sizeof(unsigned long);
   slen += strlen(cwd) + 1;

   // allocate buffer on stack
   sbuf = alloca(slen);

   // fill in header
   ((unsigned long *)(sbuf))[0] = slen - sizeof(unsigned long);
   ((unsigned long *)(sbuf))[1] = sargc;
   ((unsigned long *)(sbuf))[2] = envnum;
   // pos pointer after header
   pos = (unsigned char *)(&((((unsigned long *)(sbuf))[3 + sargc + envnum + 1])));
   n = 3;

   // fill in args
   for (i = 0; i < sargc; i++)
     {
        ((unsigned long *)(sbuf))[n] = (unsigned long)pos - (unsigned long)sbuf;
        strcpy((char *)pos, sargv[i]);
        pos += strlen(sargv[i]) + 1;
        n++;
     }

#ifdef HAVE_ENVIRON
   // fill in environ
   for (i = 0; environ[i]; i++)
     {
        ((unsigned long *)(sbuf))[n] = (unsigned long)pos - (unsigned long)sbuf;
        strcpy((char *)pos, environ[i]);
        pos += strlen(environ[i]) + 1;
        n++;
     }
#endif

   // fill in cwd
   ((unsigned long *)(sbuf))[n] = (unsigned long)pos - (unsigned long)sbuf;
   n++;
   strcpy((char *)pos, cwd);

   if (write(sock, sbuf, slen) < 0)
     printf("elementary_quicklaunch: cannot write to socket '%s'\n", buf);
   close(sock);

   free(cwd);
   return 0;
}
