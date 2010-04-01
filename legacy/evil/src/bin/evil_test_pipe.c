#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN

#include <Evil.h>

#include "evil_suite.h"
#include "evil_test_pipe.h"


#define FDREAD  0
#define FDWRITE 1

typedef struct
{
  int val;
  int fd_write;
} data;


static DWORD WINAPI
thread (void *param)
{
   data *d;
   void *buf[1];

   Sleep (2 * 1000);
   d = (data *)param;
   buf[0] = d;
   send(d->fd_write, (char *)buf, sizeof(buf), 0);

   return 0;
}

static int
test_pipe_test(void)
{
   int            sockets[2];
   struct timeval t;
   fd_set         rfds;
   int            ret;
   data          *d;
   DWORD          thread_id;
   HANDLE         h;

   FD_ZERO(&rfds);

   t.tv_sec = 5;
   t.tv_usec = 0;

   if (pipe(sockets) < 0)
     return 0;

   FD_SET(sockets[FDREAD], &rfds);
   fcntl(sockets[FDREAD], F_SETFL, O_NONBLOCK);

   d = (data *)malloc(sizeof (data));
   if (!d)
     return 0;

   d->val = 14;
   d->fd_write = sockets[FDWRITE];

   h = CreateThread(NULL, 0, thread, d, 0, &thread_id);
   if (!h)

   ret = select(sockets[FDREAD] + 1, &rfds, NULL, NULL, &t);

   if (ret < 0)
     goto free_d;

   if (ret == 0)
     goto close_h;

   if (ret > 0)
     {
        void *buf[1];
        data *d2 = NULL;
        int   len;

        while ((len = recv(sockets[FDREAD], (char *)buf, sizeof(buf), 0)) > 0)
          {
             if (len == sizeof(buf))
               {
                  d2 = (data *)buf[0];
                  break;
               }
          }
        if (d2 && (d2->val == d->val))
          ret = 1;
        else
          ret = 0;
     }

   CloseHandle(h);
   free(d);

   return 1;

 close_h:
   CloseHandle(h);
 free_d:
   free(d);
   return 0;
}

static int
test_pipe_run(suite *s)
{
   int res;

   res = test_pipe_test();

   return res;
}

int
test_pipe(suite *s)
{

   return test_pipe_run(s);
}
