# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN

#include <stdio.h>

#include "Evil.h"

#define FDREAD  0
#define FDWRITE 1

typedef struct
{
  int val;
  int fd_write;
} data;


DWORD WINAPI
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

int
main (int argc, char *argv[])
{
   int sockets[2];
   WSADATA  version_data;
   int ret;
   fd_set         rfds;
   struct timeval t;
   data *d;
   DWORD thread_id;
   HANDLE h;

   FD_ZERO(&rfds);

   t.tv_sec = 5;
   t.tv_usec = 0;

   WSAStartup(MAKEWORD(2, 2), &version_data);

   if (pipe(sockets) < 0)
     {
        printf ("error\n");
        return -1;
     }
   FD_SET(sockets[FDREAD], &rfds);
   d = (data *)malloc(sizeof (data));
   d->val = 14;
   d->fd_write = sockets[FDWRITE];
   printf (" pointeur sent........: %p\n", d);

   h = CreateThread (NULL, 0, thread, d, 0, &thread_id);

   ret = select(sockets[FDREAD] + 1, &rfds, NULL, NULL, &t);

   if (ret < 0) return -1;

   if (ret == 0) {
      printf ("temps expire\n");
   }

   if (ret > 0)
     {
        data *d;
        int len;
        int j = 0;
        void *buf[1];

        while ((len = recv(sockets[FDREAD], (char *)buf, sizeof(buf), 0)) > 0)
          {
             if (len == sizeof(buf))
               {
                  d = buf[0];
                  printf (" pointeur received....: %p\n", d);
                  j = d->val;
                  printf (" value (should be 14) : %d\n", j);
               }
          }
     }

   CloseHandle (h);

   return 0;
}
