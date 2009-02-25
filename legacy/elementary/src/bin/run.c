#include "elementary_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

int
main(int argc, char **argv)
{
   int sock, socket_unix_len, i;
   struct sockaddr_un socket_unix;
   char buf[PATH_MAX];
   struct stat st;
   char *exe;
   int we_are_elementary_run = 0;
   char *disp;
   char *cwd;
   
   int sargc, slen;
   unsigned char *sbuf = NULL, *pos;
   char **sargv = NULL;
   
   if (!getcwd(buf, sizeof(buf) - 1))
     {
        fprintf(stderr, "elementary_quicklaunch: currect working dir too big.\n");
        exit(-1);
     }
   cwd = strdup(buf);
   if (!(disp = getenv("DISPLAY")))
     {
        fprintf(stderr, "elementary_quicklaunch: DISPLAY env var not set\n");
        exit(-1);
     }
   snprintf(buf, sizeof(buf), "/tmp/elm-ql-%i/%s", getuid(), disp);
   if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
     {
        perror("elementary_quicklaunch: socket(AF_UNIX, SOCK_STREAM, 0)");
        exit(-1);
     }
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
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
             const char *p, *pp, *s;
             int exelen;
             
             p = path;
             pp = p;
             exe = NULL;
             for (;;)
               {
                  if ((*p == ':') || (*p == 0))
                    {
                       int len;
                       
                       len = p - pp;
                       if (len < (sizeof(buf) - exelen - 3))
                         {
                            strncpy(buf, pp, len);
                            strcpy(buf + len, "/");
                            strcpy(buf + len + 1, argv[0]);
                            if (access(buf, R_OK | X_OK) == 0)
                              {
                                 exe = buf;
                                 break;
                              }
                            if (*p == 0) break;
                            p++;
                            pp = p;
                         }
                    }
                  else
                    {
                       if (*p == 0) break;
                       p++;
                    }
               }
          }
     }
   if (exe)
     {
        if (lstat(exe, &st) == 0)
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
   slen = sizeof(unsigned long) + sizeof(unsigned long);
   for (i = 0; i < sargc; i++)
     {
        slen += sizeof(unsigned long);
        slen += strlen(sargv[i]) + 1;
     }
   slen += strlen(cwd) + 1;
   sbuf = alloca(slen);
   ((unsigned long *)(sbuf))[0] = slen - sizeof(unsigned long);
   ((unsigned long *)(sbuf))[1] = sargc;
   pos = (unsigned char *)(&((((unsigned long *)(sbuf))[2 + sargc])));
   for (i = 0; i < sargc; i++)
     {
        ((unsigned long *)(sbuf))[2 + i] = 
          (unsigned long)pos - ((unsigned long)sbuf + sizeof(unsigned long));
        strcpy(pos, sargv[i]);
        pos += strlen(sargv[i]) + 1;
     }
   strcpy(pos, cwd);
   write(sock, sbuf, slen);
   close(sock);
   return 0;
}
