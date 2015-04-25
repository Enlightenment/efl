/*

  libassh - asynchronous ssh2 client/server library.

  Copyright (C) 2013 Alexandre Becoulet <alexandre.becoulet@free.fr>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA

*/


#ifndef ASSH_HELPER_FD_H_
#define ASSH_HELPER_FD_H_

#include "assh_event.h"

struct assh_fd_context_s
{
  int ssh_fd;
  int rand_fd;

  struct assh_event_hndl_s h_read;
  struct assh_event_hndl_s h_write;
  struct assh_event_hndl_s h_prng_feed;
};

void assh_fd_events_register(struct assh_event_hndl_table_s *t,
			     struct assh_fd_context_s *ctx,
			     int ssh_fd, int rand_fd);

ASSH_EVENT_HANDLER_FCN(assh_fd_event_read);
ASSH_EVENT_HANDLER_FCN(assh_fd_event_write);
ASSH_EVENT_HANDLER_FCN(assh_fd_event_prng_feed);

#endif
