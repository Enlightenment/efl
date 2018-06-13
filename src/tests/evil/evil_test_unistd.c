/* EVIL - EFL library for Windows port
 * Copyright (C) 2017 Vincent Torri
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN

#include <Evil.h>

#include "evil_suite.h"


#define FDREAD  0
#define FDWRITE 1

typedef struct
{
  int val;
  int fd_write;
} data;


static DWORD WINAPI
thread(void *param)
{
   data *d;
   void *buf[1];

   Sleep (2 * 1000);
   d = (data *)param;
   buf[0] = param;
   send(d->fd_write, (char *)buf, sizeof(buf), 0);

   return 0;
}

EFL_START_TEST(evil_unistd_pipe)
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

   ret = pipe(sockets);
   fail_if(ret < 0);

   FD_SET(sockets[FDREAD], &rfds);
   fcntl(sockets[FDREAD], F_SETFL, O_NONBLOCK);

   d = (data *)malloc(sizeof (data));
   fail_if(d == NULL);

   d->val = 14;
   d->fd_write = sockets[FDWRITE];

   h = CreateThread(NULL, 0, thread, d, 0, &thread_id);
   fail_if(h == NULL);

   ret = select(sockets[FDREAD] + 1, &rfds, NULL, NULL, &t);
   fail_if(ret <= 0);

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
      fail_if(!d2 || (d2->val != d->val));
     }

   CloseHandle(h);
   free(d);
}
EFL_END_TEST

void evil_test_unistd(TCase *tc)
{
   tcase_add_test(tc, evil_unistd_pipe);
}
