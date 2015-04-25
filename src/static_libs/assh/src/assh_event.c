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

#define ASSH_EV_CONST /* write access to event const fields */

#include <assh/assh_context.h>
#include <assh/assh_transport.h>
#include <assh/assh_packet.h>
#include <assh/assh_session.h>
#include <assh/assh_kex.h>
#include <assh/assh_prng.h>
#include <assh/assh_service.h>
#include <assh/assh_event.h>

#include <assert.h>

static ASSH_EVENT_DONE_FCN(assh_event_random_done)
{
  return s->ctx->prng->f_feed(s->ctx, e->prng.feed.buf,
			      e->prng.feed.size);
  return ASSH_OK;
}

assh_error_t assh_event_get(struct assh_session_s *s,
			    struct assh_event_s *event)
{
  assh_error_t err;

  ASSH_CHK_RET(s->tr_st == ASSH_TR_CLOSED,
	       ASSH_ERR_CLOSED | ASSH_ERRSV_FIN);

  /* need to get some entropy for the prng */
  if (s->ctx->prng_entropy < 0)
    {
      event->id = ASSH_EVENT_PRNG_FEED;
      event->f_done = &assh_event_random_done;
      event->prng.feed.size = ASSH_MIN(-s->ctx->prng_entropy,
				       sizeof (event->prng.feed.buf));
      goto done;
    }

  event->id = ASSH_EVENT_INVALID;

  /* process the next incoming deciphered packet */
  ASSH_ERR_GTO(assh_transport_dispatch(s, event), err);

  if (event->id != ASSH_EVENT_INVALID)
    goto done;

  /* key re-exchange should have occured at this point */
  ASSH_CHK_RET(s->kex_bytes > ASSH_REKEX_THRESHOLD + ASSH_MAX_PCK_LEN * 16,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  /* initiate key re-exchange */
  if (s->tr_st == ASSH_TR_SERVICE && s->kex_bytes > s->kex_max_bytes)
    {
      ASSH_ERR_RET(assh_kex_send_init(s) | ASSH_ERRSV_DISCONNECT);
      assh_transport_state(s, ASSH_TR_SERVICE_KEX);
    }

  /* run the state machine which converts output packets to enciphered
     ssh stream */
  ASSH_ERR_GTO(assh_transport_write(s, event), err);

  if (event->id != ASSH_EVENT_INVALID)
    goto done;

  if (s->tr_st == ASSH_TR_FIN)
    {
      /* all events have been reported, end of session. */
      assh_transport_state(s, ASSH_TR_CLOSED);
      ASSH_ERR_RET(ASSH_ERR_CLOSED | ASSH_ERRSV_FIN);
    }

  /* run the state machine which extracts deciphered packets from the
     input ssh stream. */
  ASSH_ERR_GTO(assh_transport_read(s, event), err);

 done:
#ifdef CONFIG_ASSH_DEBUG_EVENT
  if (event->id > 2)
   ASSH_DEBUG("event id=%u\n", event->id);
#endif
  return ASSH_OK;

 err:
  return assh_session_error(s, err);
}

assh_error_t
assh_event_done(struct assh_session_s *s,
                struct assh_event_s *e)
{
  assh_error_t err;

  ASSH_CHK_RET(s->tr_st == ASSH_TR_CLOSED,
	       ASSH_ERR_CLOSED | ASSH_ERRSV_FIN);

#ifdef CONFIG_ASSH_DEBUG_EVENT
  if (e->id > 2)
    ASSH_DEBUG("event done id=%u\n", e->id);
#endif

  if (e->f_done == NULL)
    return ASSH_OK;

  ASSH_ERR_GTO(e->f_done(s, e), err);

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

void assh_event_table_init(struct assh_event_hndl_table_s *t)
{
  unsigned int i;
  for (i = 0; i < ASSH_EVENT_COUNT; i++)
    t->table[i] = NULL;
}

void assh_event_table_register(struct assh_event_hndl_table_s *t,
			       enum assh_event_id_e id,
			       struct assh_event_hndl_s *h,
			       assh_event_hndl_func_t *f, void *ctx)
{
  struct assh_event_hndl_s **t_ = t->table + id;

  h->next = *t_;
  *t_ = h;
  h->f_handler = f;
  h->ctx = ctx;
}

assh_error_t
assh_event_table_run(struct assh_session_s *s,
		     struct assh_event_hndl_table_s *t, 
		     struct assh_event_s *e)
{
  assh_error_t err;
  struct assh_event_hndl_s *h;

 continue_:
  ASSH_ERR_RET(assh_event_get(s, e));

 cont_:
  for (h = t->table[e->id]; h != NULL; h = h->next)
    {
      struct assh_event_s n;
      n.id = ASSH_EVENT_INVALID;
      ASSH_ERR_GTO(h->f_handler(s, e, &n, h->ctx), err);

      switch (err)
	{
	case ASSH_OK:
	  ASSH_ERR_RET(assh_event_done(s, e));
	  if (n.id != ASSH_EVENT_INVALID)
	    {
	      memcpy(e, &n, sizeof(n));
	      goto cont_;
	    }
	  goto continue_;

	case ASSH_NO_DATA:
	  continue;

	default:
	  ASSH_ERR_RET(err);
	}
    }

  return ASSH_OK;

 err:
  return assh_session_error(s, err);
}

