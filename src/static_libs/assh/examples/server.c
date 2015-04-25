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

#include <assh/assh_session.h>
#include <assh/assh_context.h>
#include <assh/assh_service.h>
#include <assh/assh_userauth_server.h>
#include <assh/assh_connection.h>
#include <assh/assh_event.h>
#include <assh/assh_algo.h>
#include <assh/assh_packet.h>

#include <assh/helper_fd.h>
#include <assh/helper_key.h>

#include <assh/key_rsa.h>
#include <assh/key_dsa.h>
#include <assh/key_eddsa.h>

#ifdef CONFIG_ASSH_USE_GCRYPT
# include <gcrypt.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

int main()
{
#ifdef CONFIG_ASSH_USE_GCRYPT
  if (!gcry_check_version(GCRYPT_VERSION))
    return -1;
#endif

  int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  assert(sock >= 0);

  int		tmp = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));

  struct sockaddr_in	addr = 
    {
      .sin_port = htons(22222),
      .sin_family = AF_INET,
    };

  if (bind(sock, (struct sockaddr*)&addr, sizeof (struct sockaddr_in)) < 0)
    abort();

  if (listen(sock, 8) < 0)
    abort();

  /** init a server context */
  struct assh_context_s context;
  assh_context_init(&context, ASSH_SERVER);

  /** register authentication and connection services */
  if (assh_service_register_default(&context) != ASSH_OK)
    return -1;

  /** register algorithms */
  if (assh_algo_register_default(&context, 99, 10) != ASSH_OK)
    return -1;

  /** load host key */
#if 1
  if (assh_load_hostkey_filename(&context, &assh_key_dsa, ASSH_ALGO_SIGN, "dsa_host_key",
				 ASSH_KEY_FMT_PV_RFC2440_PEM_ASN1) != ASSH_OK)
    fprintf(stderr, "unable to load dsa key\n");

  if (assh_load_hostkey_filename(&context, &assh_key_dsa, ASSH_ALGO_SIGN, "dsa2048_host_key",
				 ASSH_KEY_FMT_PV_RFC2440_PEM_ASN1) != ASSH_OK)
    fprintf(stderr, "unable to load dsa 2048 key\n");
#endif

#if 1
  if (assh_load_hostkey_filename(&context, &assh_key_rsa, ASSH_ALGO_SIGN, "rsa_host_key",
				 ASSH_KEY_FMT_PV_RFC2440_PEM_ASN1) != ASSH_OK)
    fprintf(stderr, "unable to load rsa key\n");
#endif

#if 1
  if (assh_load_hostkey_filename(&context, &assh_key_ed25519, ASSH_ALGO_SIGN, "ed25519_host_key",
				 ASSH_KEY_FMT_PV_OPENSSH_V1_KEY) != ASSH_OK)
    fprintf(stderr, "unable to load ed25519 key\n");
#endif

  if (assh_load_hostkey_filename(&context, &assh_key_eddsa_e382, ASSH_ALGO_SIGN, "e382_host_key",
				 ASSH_KEY_FMT_PV_OPENSSH_V1_KEY) != ASSH_OK)
    fprintf(stderr, "unable to load eddsa e382 key\n");

  if (assh_load_hostkey_filename(&context, &assh_key_eddsa_e521, ASSH_ALGO_SIGN, "e521_host_key",
				 ASSH_KEY_FMT_PV_OPENSSH_V1_KEY) != ASSH_OK)
    fprintf(stderr, "unable to load eddsa e521 key\n");

  signal(SIGPIPE, SIG_IGN);

  /** random data source */
  int rnd_fd = open("/dev/urandom", O_RDONLY);
  if (rnd_fd < 0)
    return -1;

  while (1)
    {
      struct sockaddr_in con_addr;
      socklen_t addr_size = sizeof(con_addr);

      int conn = accept(sock, (struct sockaddr*)&con_addr, &addr_size);

      /** init a session for the incoming connection */
      struct assh_session_s session;
      if (assh_session_init(&context, &session) != ASSH_OK)
	return -1;

      time_t t = time(0);
      fprintf(stderr, "============== %s\n", ctime(&t));

      /** rely on an event table to handle most events returned by the assh core */
      struct assh_event_hndl_table_s ev_table;
      assh_event_table_init(&ev_table);

      /** register helper event handlers to process io events using
	  file descriptors. */
      struct assh_fd_context_s fd_ctx;
      assh_fd_events_register(&ev_table, &fd_ctx, conn, rnd_fd);

#if 0
      /** register helper event handlers to process ssh-connection
	  (rfc4254) events in a way to serve shell */
      struct assh_unix_shell_server_s ush_ctx;
      assh_unix_shell_server_register(&ev_table, &ush_ctx);
#endif

      while (1)
	{
	  struct assh_event_s event;

	  /** get events from the core and use registered event
	      handlers to process most events. */
	  assh_error_t err = assh_event_table_run(&session, &ev_table, &event);
	  if (ASSH_ERR_ERROR(err) != ASSH_OK)
	    {
	      fprintf(stderr, "assh error %x in main loop (errno=%i)\n", err, errno);

	      if (ASSH_ERR_ERROR(err) == ASSH_ERR_CLOSED)
		{
		  close(conn);
		  break;
		}

	      continue;
	    }

	  /** we still have to process events not handled in the table */
	  switch (event.id)
	    {
	    case ASSH_EVENT_USERAUTH_SERVER_USERKEY: {
	      /* XXX check that event public key is in the list of
		 user authorized keys. */

#warning validate key ? keys should be validated once when added to the list

	      event.userauth_server.userkey.found = 1;
	      err = assh_event_done(&session, &event);
	      break;
	    }

	    case ASSH_EVENT_USERAUTH_SERVER_PASSWORD:
	      /* XXX check that event user/password pair matches. */
	      event.userauth_server.password.success = 1;
	      err = assh_event_done(&session, &event);
	      break;

	    case ASSH_EVENT_CHANNEL_OPEN: {
	      struct assh_event_channel_open_s *co_e = &event.connection.channel_open;

	      if (!assh_buffer_strcmp(&co_e->type, "session"))
		{
		  co_e->reply = ASSH_CONNECTION_REPLY_SUCCESS;
		}
	      err = assh_event_done(&session, &event);
	      break;
	    }

	    case ASSH_EVENT_REQUEST: {
	      struct assh_event_request_s *rq_e = &event.connection.request;

	      if (!assh_buffer_strcmp(&rq_e->type, "shell"))
		{
		  rq_e->reply = ASSH_CONNECTION_REPLY_SUCCESS;
		}
	      else if (!assh_buffer_strcmp(&rq_e->type, "pty-req"))
		{
		  rq_e->reply = ASSH_CONNECTION_REPLY_SUCCESS;
		}
	      err = assh_event_done(&session, &event);
	      break;
	    }

	    case ASSH_EVENT_CHANNEL_DATA: {
	      struct assh_event_channel_data_s *dt_e = &event.connection.channel_data;

	      uint8_t *data;
	      size_t size = dt_e->data.size;

	      /* allocate output data packet */
	      assh_error_t perr = assh_channel_data_alloc(dt_e->ch, &data, &size, size);

	      if (perr == ASSH_OK)  /* copy input data to output buffer */
		memcpy(data, dt_e->data.data, size);

	      /* acknowledge input data event before sending */
	      err = assh_event_done(&session, &event);

	      if (perr == ASSH_OK)  /* send output data */
		err = assh_channel_data_send(dt_e->ch, size);
	      break;
	    }

	    default:
	      printf("Don't know how to handle event %u\n", event.id);
	      err = assh_event_done(&session, &event);
	    }

	  if (err != ASSH_OK)
	    fprintf(stderr, "assh error %i in main loop (errno=%i)\n", err, errno);
	}

      assh_session_cleanup(&session);
      break;
    }

  assh_context_cleanup(&context);

  return 0;
}

