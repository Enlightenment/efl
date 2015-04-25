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

#include <assh/assh_transport.h>

#include <assh/assh_context.h>
#include <assh/assh_session.h>
#include <assh/assh_packet.h>
#include <assh/assh_queue.h>
#include <assh/assh_service.h>
#include <assh/assh_cipher.h>
#include <assh/assh_mac.h>
#include <assh/assh_kex.h>
#include <assh/assh_event.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>

ASSH_EVENT_SIZE_SASSERT(transport);

void assh_transport_push(struct assh_session_s *s,
			 struct assh_packet_s *p)
{
  struct assh_queue_s *q = &s->out_queue;

  /* service packets are postponed during kex */
  assh_bool_t kex_msg = p->head.msg <= 49 &&
    p->head.msg != SSH_MSG_SERVICE_REQUEST &&
    p->head.msg != SSH_MSG_SERVICE_ACCEPT;

  switch (s->tr_st)
    {
    case ASSH_TR_KEX_INIT:
    case ASSH_TR_KEX_WAIT:
    case ASSH_TR_KEX_RUNNING:
    case ASSH_TR_NEWKEY:
    case ASSH_TR_SERVICE_KEX:
      if (!kex_msg)
	q = &s->alt_queue;
      assh_queue_push_back(q, &p->entry);
      break;

    case ASSH_TR_SERVICE:
      assh_queue_push_back(q, &p->entry);
      break;

    case ASSH_TR_FIN:
    case ASSH_TR_CLOSED:
      assh_packet_release(p);
      break;
    }
}

static ASSH_EVENT_DONE_FCN(assh_event_read_done)
{
  assh_error_t err;
  struct assh_kex_keys_s *k = s->cur_keys_in;
  unsigned int bsize = k == NULL ? 16 : ASSH_MAX(k->cipher->block_size, 16);

  size_t rd_size = e->transport.read.transferred;
  assert(rd_size <= e->transport.read.buf.size);
  s->stream_in_size += rd_size;

  switch (s->stream_in_st)
    {
    /* process ident text lines */
    case ASSH_TR_IN_IDENT_DONE: {
      unsigned int i;

      /* look for End of Line */
      for (i = s->stream_in_size - rd_size; i < s->stream_in_size; i++)
	if (s->ident_str[i] == '\n')
	  {
	    s->stream_in_size -= i + 1;

	    /* test line prefix */
	    if (i >= 7 && !strncmp((char*)s->ident_str, "SSH-", 4))
	      {
		ASSH_CHK_RET(strncmp((char*)s->ident_str + 4, "2.0", 3),
			     ASSH_ERR_BAD_VERSION | ASSH_ERRSV_FIN);

		/* copy remaining unused bytes to packet header buffer */
		memcpy(s->stream_in_pck_head, s->ident_str + i + 1, s->stream_in_size);

		/* ajust and keep ident string length */
		if (s->ident_str[i - 1] == '\r')
		  i--;
		s->ident_len = i;

		s->stream_in_st = ASSH_TR_IN_HEAD;
		return ASSH_OK;
	      }

	    /* discard this line */
	    memmove(s->ident_str, s->ident_str + i + 1, s->stream_in_size);
	    i = 0;
	  }

      ASSH_CHK_RET(s->stream_in_size >= sizeof(s->ident_str),
		   ASSH_ERR_INPUT_OVERFLOW | ASSH_ERRSV_FIN);

      s->stream_in_st = ASSH_TR_IN_IDENT;
      return ASSH_OK;
    }

    /* decipher packet head, compute packet length and allocate packet */
    case ASSH_TR_IN_HEAD_DONE: {

      if (s->stream_in_size < bsize)
	{
	  /* not enough header data yet to decipher the 1st block */
	  s->stream_in_st = ASSH_TR_IN_HEAD;
	  return ASSH_OK;
	}

      /* decipher */
      if (k != NULL)
	ASSH_ERR_RET(k->cipher->f_process(k->cipher_ctx,
			  s->stream_in_pck_head, bsize) | ASSH_ERRSV_DISCONNECT);

      /* compute various length values */
      size_t len = assh_load_u32(s->stream_in_pck_head);
      uint8_t pad_len = s->stream_in_pck_head[4];
      unsigned int align = k == NULL ? 8 : ASSH_MAX(k->cipher->block_size, 8);

      ASSH_CHK_RET(len > ASSH_MAX_PCK_LEN - 4 || len < 12,
		   ASSH_ERR_INPUT_OVERFLOW | ASSH_ERRSV_DISCONNECT);

      len += 4;
      ASSH_CHK_RET(len % align || pad_len < 4,
		   ASSH_ERR_INPUT_OVERFLOW | ASSH_ERRSV_DISCONNECT);

      if (k != NULL)
	len += k->mac->mac_size;

      /* allocate actual packet and copy header */
      struct assh_packet_s *p;
      ASSH_ERR_RET(assh_packet_alloc2(s->ctx, 0, len - 6, &p) | ASSH_ERRSV_DISCONNECT);

      memcpy(p->data, s->stream_in_pck_head, s->stream_in_size);
      p->data_size = len;
      s->stream_in_pck = p;

      if (len > s->stream_in_size)
	{
	  s->stream_in_st = ASSH_TR_IN_PAYLOAD;
	  return ASSH_OK;
	}
    }

    /* decipher remaining packet data, check MAC and accept packet */
    case ASSH_TR_IN_PAYLOAD_DONE: {
      struct assh_packet_s *p = s->stream_in_pck;

      if (s->stream_in_size < p->data_size)
	{
	  /* not enough data for the whole packet yet */
	  s->stream_in_st = ASSH_TR_IN_PAYLOAD;
	  return ASSH_OK;
	}

      size_t mac_len = 0;

      if (k != NULL)
	{
	  mac_len = k->mac->mac_size;

	  /* decipher */
	  ASSH_ERR_RET(k->cipher->f_process(
              k->cipher_ctx, p->data + bsize,
	      p->data_size - bsize - mac_len) | ASSH_ERRSV_DISCONNECT);
	}

#ifdef CONFIG_ASSH_DEBUG_PROTOCOL
      ASSH_DEBUG("incoming packet: session=%p tr_st=%i, size=%zu, msg=%u\n",
		 s, s->tr_st, p->data_size, p->head.msg);
      assh_hexdump("in packet", p->data, p->data_size);
#endif

      if (k != NULL)
	{
	  /* compute and compare MAC */
	  ASSH_ERR_RET(k->mac->f_check(k->mac_ctx, s->in_seq, p->data,
					p->data_size - mac_len,
					p->data + p->data_size - mac_len)
		       | ASSH_ERRSV_DISCONNECT);

#warning FIXME decompress
	}

      s->kex_bytes += p->data_size;

      /* reduce packet size to actual payload */
      p->data_size -= mac_len + p->head.pad_len;

      /* push completed incoming packet for dispatch */
      assert(s->in_pck == NULL);
      s->in_pck = p;

      /* reinit input state */
      s->in_seq++;
      s->stream_in_pck = NULL;
      s->stream_in_st = ASSH_TR_IN_HEAD;
      s->stream_in_size = 0;
      return ASSH_OK;
    }

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  return ASSH_OK;
}

assh_error_t assh_transport_read(struct assh_session_s *s,
				 struct assh_event_s *e)
{
  assh_error_t err;
  struct assh_kex_keys_s *k = s->cur_keys_in;
  uint8_t **data = &e->transport.read.buf.data;
  size_t *size = &e->transport.read.buf.size;

  switch (s->stream_in_st)
    {
    /* read stream into ident buffer */
    case ASSH_TR_IN_IDENT:
      *data = s->ident_str + s->stream_in_size;
      *size = ASSH_MIN(16, sizeof(s->ident_str) - s->stream_in_size);
      s->stream_in_st = ASSH_TR_IN_IDENT_DONE;
      break;

    /* read stream into packet head buffer */
    case ASSH_TR_IN_HEAD: {
      unsigned int bsize = k == NULL ? 16 : ASSH_MAX(k->cipher->block_size, 16);
      *data = s->stream_in_pck_head + s->stream_in_size;
      *size = bsize - s->stream_in_size;
      s->stream_in_st = ASSH_TR_IN_HEAD_DONE;
      break;
    }

    /* read stream into actual packet buffer */
    case ASSH_TR_IN_PAYLOAD: {
      struct assh_packet_s *p = s->stream_in_pck;
      *data = p->data + s->stream_in_size;
      *size = p->data_size - s->stream_in_size;
      s->stream_in_st = ASSH_TR_IN_PAYLOAD_DONE;
      assert(s->in_pck == NULL);
      break;
    }

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  e->id = ASSH_EVENT_READ;
  e->f_done = &assh_event_read_done;
  e->transport.read.transferred = 0;
  return ASSH_OK;
}

static ASSH_EVENT_DONE_FCN(assh_event_write_done)
{
  assh_error_t err;

  size_t wr_size = e->transport.write.transferred;
  assert(wr_size <= e->transport.write.buf.size);
  s->stream_out_size += wr_size;

  switch (s->stream_out_st)
    {
    /* check if sending of ident string has completed */
    case ASSH_TR_OUT_IDENT_DONE:
      s->stream_out_st = s->stream_out_size >= sizeof(ASSH_IDENT) - 1
	? ASSH_TR_OUT_PACKETS : ASSH_TR_OUT_IDENT_PAUSE;
      return ASSH_OK;

    /* check if sending of packet has completed */
    case ASSH_TR_OUT_PACKETS_DONE: {
      assert(s->out_queue.count > 0);

      struct assh_queue_entry_s *e = assh_queue_front(&s->out_queue);
      struct assh_packet_s *p = (void*)e;

      if (s->stream_out_size < p->data_size)
	{
	  /* packet partially sent, need to report one more write
	     event later. Yield to the input state machine for now. */
	  s->stream_out_st = ASSH_TR_OUT_PACKETS_PAUSE;
	  return ASSH_OK;
	}

      /* pop and release packet */
      assh_queue_remove(&s->out_queue, e);
      assh_packet_release(p);

      s->stream_out_st = ASSH_TR_OUT_PACKETS;
      return ASSH_OK;
    }

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  return ASSH_OK;
}

assh_error_t assh_transport_write(struct assh_session_s *s,
				  struct assh_event_s *e)
{
  assh_error_t err;
  uint8_t **data = &e->transport.write.buf.data;
  size_t *size = &e->transport.write.buf.size;

  switch (s->stream_out_st)
    {
    /* the write stream buffer is the constant ident string */
    case ASSH_TR_OUT_IDENT: {
      *data = (uint8_t*)ASSH_IDENT + s->stream_out_size;
      *size = sizeof(ASSH_IDENT) - 1 - s->stream_out_size;
      s->stream_out_st = ASSH_TR_OUT_IDENT_DONE;
      break;
    }

    /* the last ident buffer write was incomplete, yield to input */
    case ASSH_TR_OUT_IDENT_PAUSE:
      s->stream_out_st = ASSH_TR_OUT_IDENT;
      return ASSH_OK;

    /* the next output packet must be enciphered before write */
    case ASSH_TR_OUT_PACKETS: {

      /* nothing to output, yield to input */
      if (s->out_queue.count == 0)
	return ASSH_OK;

      struct assh_packet_s *p = (void*)assh_queue_front(&s->out_queue);

      /* compute various length and payload pointer values */
      struct assh_kex_keys_s *k = s->cur_keys_out;
      unsigned int align = 8;
      size_t mac_len = 0;

      if (k != NULL)
	{
	  align = ASSH_MAX(k->cipher->block_size, 8);
	  mac_len = k->mac->mac_size;
	}

      size_t pad_len = align - p->data_size % align;
      if (pad_len < 4)
	pad_len += align;

      assert(pad_len >= 4 && pad_len < 255);

      p->data_size += pad_len + mac_len;
      assert(p->data_size <= p->alloc_size);

      assh_store_u32(p->data, p->data_size - 4 - mac_len);
      p->head.pad_len = pad_len;
      uint8_t *mac_ptr = p->data + p->data_size - mac_len;
      uint8_t *pad = mac_ptr - pad_len;

      if (pad_len > 0)
	memset(pad, 42, pad_len);

#warning FIXME compress

#ifdef CONFIG_ASSH_DEBUG_PROTOCOL
      ASSH_DEBUG("outgoing packet: session=%p tr_st=%i, size=%zu, msg=%u\n",
		 s, s->tr_st, p->data_size, p->head.msg);
      assh_hexdump("out packet", p->data, p->data_size);
#endif

      assh_bool_t newkey = p->head.msg == SSH_MSG_NEWKEYS;

      /* compute MAC and encrypt packet */
      if (k != NULL)
	{
	  ASSH_ERR_RET(k->mac->f_compute(k->mac_ctx, s->out_seq, p->data,
			 p->data_size - mac_len, mac_ptr) | ASSH_ERRSV_FIN);

	  ASSH_ERR_RET(k->cipher->f_process(k->cipher_ctx, p->data,
			    p->data_size - mac_len) | ASSH_ERRSV_FIN);
	}

      if (newkey)
	{
	  /* release the old output cipher/mac context and install the new one */
	  assh_kex_keys_cleanup(s, s->cur_keys_out);
	  s->cur_keys_out = s->new_keys_out;
	  s->new_keys_out = NULL;
	}

      s->kex_bytes += p->data_size;
      s->out_seq++;

      /* reinit output state */
      s->stream_out_size = 0;
      *data = p->data;
      *size = p->data_size;
      s->stream_out_st = ASSH_TR_OUT_PACKETS_DONE;
      break;
    }

    /* the write stream buffer is an already enciphered output packet */
    case ASSH_TR_OUT_PACKETS_ENCIPHERED: {

      assert(s->out_queue.count != 0);
      struct assh_packet_s *p = (void*)assh_queue_front(&s->out_queue);

      *data = p->data + s->stream_out_size;
      *size = p->data_size - s->stream_out_size;
      s->stream_out_st = ASSH_TR_OUT_PACKETS_DONE;
      break;
    }

    /* the last packet buffer write was incomplete, yield to input */
    case ASSH_TR_OUT_PACKETS_PAUSE:
      s->stream_out_st = ASSH_TR_OUT_PACKETS_ENCIPHERED;
      return ASSH_OK;

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  /* a buffer is available for output, return a write event */
  e->id = ASSH_EVENT_WRITE;
  e->f_done = &assh_event_write_done;
  e->transport.write.transferred = 0;
  return ASSH_OK;
}

assh_error_t assh_transport_dispatch(struct assh_session_s *s,
				     struct assh_event_s *e)
{
  assh_error_t err = ASSH_OK;
  enum assh_ssh_msg_e msg = SSH_MSG_INVALID;
  struct assh_packet_s *p = s->tr_st < ASSH_TR_FIN ? s->in_pck : NULL;

  if (p != NULL)
    {
      msg = p->head.msg;

      /* handle common packets */
      switch (msg)
	{
	case SSH_MSG_INVALID:
	  ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_FIN);

	case SSH_MSG_DISCONNECT:
	  ASSH_ERR_RET(ASSH_ERR_DISCONNECTED | ASSH_ERRSV_FIN);

	case SSH_MSG_DEBUG:
	case SSH_MSG_IGNORE:
	  goto done;
	default:
	  break;
	}
    }

  /* transport protocol state machine */
  switch (s->tr_st)
    {
    /* send first kex init packet during session init */
    case ASSH_TR_KEX_INIT:
      ASSH_ERR_RET(assh_kex_send_init(s) | ASSH_ERRSV_DISCONNECT);
      assh_transport_state(s, ASSH_TR_KEX_WAIT);

    /* wait for initial kex init packet during session init */
    case ASSH_TR_KEX_WAIT:
      if (msg == SSH_MSG_INVALID)
	goto done;
      ASSH_CHK_RET(msg != SSH_MSG_KEXINIT, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
    kex_init:
      ASSH_ERR_RET(assh_kex_got_init(s, p) | ASSH_ERRSV_DISCONNECT);

      /* switch to key exchange running state */
      assh_transport_state(s, ASSH_TR_KEX_RUNNING);
      p = NULL;
      msg = 0;

    /* key exchange algorithm is running (session init or rekeying) */
    case ASSH_TR_KEX_RUNNING:
        /* allowed msgs are 0 (no packet),
	   1-4, 7-19, 20-29, 30-49 */
      ASSH_CHK_RET(msg > 49 || msg == SSH_MSG_SERVICE_REQUEST || msg == SSH_MSG_SERVICE_ACCEPT,
		   ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

      if (!s->kex_bad_guess || msg == SSH_MSG_INVALID)
	ASSH_ERR_RET(s->kex->f_process(s, p, e) | ASSH_ERRSV_DISCONNECT);
      else
	s->kex_bad_guess = 0;
      goto done;

    /* kex exchange is over, NEWKEYS packet expected */
    case ASSH_TR_NEWKEY:
      if (msg == SSH_MSG_INVALID)
	goto done;
      ASSH_CHK_RET(msg != SSH_MSG_NEWKEYS, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

      /* release the old input cipher/mac context and install the new one */
      assert(s->new_keys_in != NULL);
      assh_kex_keys_cleanup(s, s->cur_keys_in);
      s->cur_keys_in = s->new_keys_in;
      s->new_keys_in = NULL;

      /* move postponed service packets to output queue */
      assh_queue_concat(&s->out_queue, &s->alt_queue);

      /* switch to service running state */
      p = NULL;
      msg = 0;

      assh_transport_state(s, ASSH_TR_SERVICE);

    /* service is running or have to be started */
    service:
    case ASSH_TR_SERVICE:
      switch (msg)
	{
        /* received a rekeying request, reply and switch to ASSH_TR_KEX_RUNNING */
	case SSH_MSG_KEXINIT:
	  ASSH_CHK_RET(s->new_keys_out != NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_FIN);
	  ASSH_ERR_RET(assh_kex_send_init(s) | ASSH_ERRSV_DISCONNECT);
	  goto kex_init;

	/* handle a service request packet */
	case SSH_MSG_SERVICE_REQUEST:
#ifdef CONFIG_ASSH_SERVER
	  if (s->ctx->type == ASSH_SERVER)
	    ASSH_ERR_RET(assh_service_got_request(s, p) | ASSH_ERRSV_DISCONNECT);
	  else
#endif
	    ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
	  p = NULL;
	  break;

	/* handle a service accept packet */
        case SSH_MSG_SERVICE_ACCEPT:
#ifdef CONFIG_ASSH_CLIENT
	  if (s->ctx->type == ASSH_CLIENT)
	    ASSH_ERR_RET(assh_service_got_accept(s, p) | ASSH_ERRSV_DISCONNECT);
	  else
#endif
	    ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
	  p = NULL;
	  break;

	/* dispatch packet to running service */
        default:
	  ASSH_CHK_RET(s->srv == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

	/* no packet */
        case SSH_MSG_INVALID:
	  break;
	}
      break;

    /* service is running, key re-exchange initiated */
    case ASSH_TR_SERVICE_KEX:
      if (msg == SSH_MSG_KEXINIT)
	goto kex_init;
      goto service;

    case ASSH_TR_FIN:
      if (s->srv == NULL)
	return ASSH_OK;
      break;

    case ASSH_TR_CLOSED:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  if (s->srv == NULL)
    {
#ifdef CONFIG_ASSH_CLIENT
      /* client send a service request if no service is currently running */
      if (s->ctx->type == ASSH_CLIENT && s->srv_rq == NULL)
	ASSH_ERR_RET(assh_service_send_request(s) | ASSH_ERRSV_DISCONNECT);
#endif
      goto done;
    }

  do {
    /* call service processing function, with or without a packet */
    err = s->srv->f_process(s, p, e);
  } while (err == ASSH_NO_DATA && e->id == ASSH_EVENT_INVALID);

  if (err == ASSH_NO_DATA)
    return ASSH_OK;

 done:
  assh_packet_release(s->in_pck);
  s->in_pck = NULL;
  ASSH_ERR_RET(err | ASSH_ERRSV_DISCONNECT);
  return ASSH_OK;
}

