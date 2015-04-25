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
#include <assh/assh_packet.h>
#include <assh/assh_kex.h>
#include <assh/assh_prng.h>
#include <assh/assh_queue.h>
#include <assh/assh_service.h>
#include <assh/assh_transport.h>

assh_error_t assh_session_init(struct assh_context_s *c,
			       struct assh_session_s *s)
{
  assh_error_t err;

  ASSH_ERR_RET(assh_context_prng(c, NULL));
  s->ctx = c;

  assh_transport_state(s, ASSH_TR_KEX_INIT);

  s->ident_len = 0;
  s->session_id_len = 0;

  s->kex_init_local = NULL;
  s->kex_init_remote = NULL;
  s->kex_pv = NULL;
  s->kex_bytes = 0;
  s->kex_max_bytes = ASSH_REKEX_THRESHOLD;

#ifdef CONFIG_ASSH_CLIENT
  s->srv_rq = NULL;
  s->srv_index = 0;
#endif
  s->srv = NULL;

  s->stream_out_st = ASSH_TR_OUT_IDENT;
  assh_queue_init(&s->out_queue);
  assh_queue_init(&s->alt_queue);
  s->stream_out_size = 0;
  s->cur_keys_out = NULL;
  s->new_keys_out = NULL;
  s->out_seq = 0;

  s->stream_in_st = ASSH_TR_IN_IDENT;
  s->stream_in_pck = NULL;
  s->stream_in_size = 0;
  s->in_pck = NULL;
  s->cur_keys_in = NULL;
  s->new_keys_in = NULL;
  s->in_seq = 0;

  c->session_count++;

  return ASSH_OK;
}

static void assh_pck_queue_cleanup(struct assh_queue_s *q)
{
  while (q->count > 0)
    {
      struct assh_queue_entry_s *e = assh_queue_front(q);
      assh_queue_remove(q, e);

      struct assh_packet_s *p = (struct assh_packet_s*)e;
      assh_packet_release(p);
    }
}

void assh_session_cleanup(struct assh_session_s *s)
{
  if (s->kex_pv != NULL)
    s->kex->f_cleanup(s);
  assert(s->kex_pv == NULL);

  if (s->srv != NULL)
    s->srv->f_cleanup(s);

  assh_packet_release(s->kex_init_local);
  assh_packet_release(s->kex_init_remote);

  assh_pck_queue_cleanup(&s->out_queue);
  assh_pck_queue_cleanup(&s->alt_queue);

  assh_kex_keys_cleanup(s, s->cur_keys_in);
  assh_kex_keys_cleanup(s, s->cur_keys_out);
  assh_kex_keys_cleanup(s, s->new_keys_in);
  assh_kex_keys_cleanup(s, s->new_keys_out);

  assh_packet_release(s->in_pck);
  assh_packet_release(s->stream_in_pck);

  s->ctx->session_count--;
}

assh_error_t assh_session_error(struct assh_session_s *s, assh_error_t inerr)
{
  if ((inerr & ASSH_ERRSV_FATAL) || s->tr_st == ASSH_TR_CLOSED)
    {
      assh_transport_state(s, ASSH_TR_CLOSED);
      return inerr | ASSH_ERRSV_FATAL;
    }

  if ((inerr & ASSH_ERRSV_FIN) || s->tr_st == ASSH_TR_FIN)
    {
      assh_transport_state(s, ASSH_TR_FIN);
      return inerr | ASSH_ERRSV_FIN;
    }

  uint32_t reason = SSH_DISCONNECT_RESERVED;
  const char *desc = NULL;

  switch (inerr & 0xfff)
    {
    case ASSH_ERR_BAD_DATA:
    case ASSH_ERR_PROTOCOL:
#ifdef CONFIG_ASSH_VERBOSE_ERROR
      reason = SSH_DISCONNECT_PROTOCOL_ERROR;
      break;
#endif

    case ASSH_ERR_INPUT_OVERFLOW:
    case ASSH_ERR_OUTPUT_OVERFLOW:
    case ASSH_ERR_IO:
    case ASSH_ERR_BAD_VERSION:
    case ASSH_ERR_DISCONNECTED:
    case ASSH_ERR_CLOSED:
      assh_transport_state(s, ASSH_TR_FIN);
      return inerr | ASSH_ERRSV_FIN;      

    case ASSH_ERR_STATE:
      assh_transport_state(s, ASSH_TR_CLOSED);
      return inerr | ASSH_ERRSV_FATAL;      

    case ASSH_ERR_MEM:
      reason = SSH_DISCONNECT_RESERVED;
#ifdef CONFIG_ASSH_VERBOSE_ERROR
      desc = "memory resource shortage";
#endif
      break;
    case ASSH_ERR_NUM_OVERFLOW:
      reason = SSH_DISCONNECT_RESERVED;
#ifdef CONFIG_ASSH_VERBOSE_ERROR
      desc = "numerical overflow";
#endif
      break;
    case ASSH_ERR_MAC:
#ifdef CONFIG_ASSH_VERBOSE_ERROR
      desc = "mac error";
#endif
      reason = SSH_DISCONNECT_MAC_ERROR;
      break;
    case ASSH_ERR_CRYPTO:
#ifdef CONFIG_ASSH_VERBOSE_ERROR
      desc = "crypto error";
#endif
      reason = SSH_DISCONNECT_RESERVED;
      break;
    case ASSH_ERR_NOTSUP:
      desc = "not supported";
      reason = SSH_DISCONNECT_RESERVED;
      break;
    case ASSH_ERR_KEX_FAILED:
      reason = SSH_DISCONNECT_KEY_EXCHANGE_FAILED;
      break;
    case ASSH_ERR_MISSING_KEY:
      desc = "missing key";
      reason = SSH_DISCONNECT_RESERVED;
      break;
    case ASSH_ERR_MISSING_ALGO:
      desc = "algorithm not available";
      reason = SSH_DISCONNECT_RESERVED;
      break;
    case ASSH_ERR_HOSTKEY_SIGNATURE:
      reason = SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE;
      break;
    case ASSH_ERR_SERVICE_NA:
      reason = SSH_DISCONNECT_SERVICE_NOT_AVAILABLE;
      break;
    case ASSH_ERR_NO_AUTH:
      reason = SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE;
      break;
    case ASSH_ERR_NO_MORE_SERVICE:
      reason = SSH_DISCONNECT_BY_APPLICATION;
      break;
    case ASSH_ERR_WEAK_ALGORITHM:
      desc = "weak key or algorithm parameters";
      reason = SSH_DISCONNECT_RESERVED;
      break;
    }

  if (!(inerr & ASSH_ERRSV_DISCONNECT))
    return inerr;

  ASSH_DEBUG("disconnect packet reason: %u (%s)\n", reason, desc);

  struct assh_packet_s *pout;
  size_t sz = 0;
  if (desc != NULL)
    sz = 4 + strlen(desc);

  if (assh_packet_alloc(s->ctx, SSH_MSG_DISCONNECT, 3 * 4 + sz, &pout) == ASSH_OK)
    {
      ASSH_ASSERT(assh_packet_add_u32(pout, reason)); /* reason code */

      uint8_t *str;
      ASSH_ASSERT(assh_packet_add_string(pout, sz, &str)); /* description */
      if (desc != NULL)
	memcpy(str, desc, sz - 4);

      ASSH_ASSERT(assh_packet_add_string(pout, 0, NULL)); /* language */

      assh_transport_push(s, pout);
    }

  assh_transport_state(s, ASSH_TR_FIN);
  return inerr | ASSH_ERRSV_FIN;
}

