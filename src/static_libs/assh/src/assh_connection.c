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

#include <assh/assh_connection.h>

#include <assh/assh_service.h>
#include <assh/assh_session.h>
#include <assh/assh_packet.h>
#include <assh/assh_transport.h>
#include <assh/assh_event.h>
#include <assh/assh_queue.h>
#include <assh/assh_map.h>
#include <assh/assh_alloc.h>

ASSH_EVENT_SIZE_SASSERT(connection);

enum assh_connection_state_e
{
  ASSH_CONNECTION_ST_START, 
  ASSH_CONNECTION_ST_IDLE,
  ASSH_CONNECTION_ST_EVENT_REQUEST,
  ASSH_CONNECTION_ST_EVENT_REQUEST_REPLY,
  ASSH_CONNECTION_ST_EVENT_CHANNEL_OPEN,
  ASSH_CONNECTION_ST_EVENT_CHANNEL_OPEN_REPLY,
  ASSH_CONNECTION_ST_EVENT_CHANNEL_DATA,
  ASSH_CONNECTION_ST_EVENT_CHANNEL_CLOSE,
  ASSH_CONNECTION_ST_EVENT_CHANNEL_EOF,
  ASSH_CONNECTION_ST_FIN,
};

struct assh_connection_context_s
{
  enum assh_connection_state_e state;

  struct assh_queue_s request_rqueue; //< global requests we have to acknowledged
  struct assh_queue_s request_lqueue; //< global requests waiting for a reply from the remote host

  struct assh_map_entry_s *channel_map; //< allocated channels

  struct assh_packet_s *pck;          //< packet kept during event processing
  struct assh_queue_s closing_queue;  //< closing channels with some pending requests left

  uint32_t ch_id_counter;
};

static void assh_request_queue_cleanup(struct assh_session_s *s,
				       struct assh_queue_s *q)
{
  while (q->count > 0)
    {
      struct assh_queue_entry_s *rqe = assh_queue_back(q);
      struct assh_request_s *rq = (void*)rqe;

      assh_packet_release(rq->reply_pck);

      assh_queue_remove(q, rqe);
      assh_free(s->ctx, rqe);
    }
}

static void assh_channel_cleanup(struct assh_channel_s *ch)
{
  struct assh_session_s *s = ch->session;

  assh_request_queue_cleanup(s, &ch->request_rqueue);
  assh_request_queue_cleanup(s, &ch->request_lqueue);
  assh_packet_release(ch->data_pck);

  assh_free(s->ctx, ch);  
}

static void assh_channel_queue_cleanup(struct assh_session_s *s,
				       struct assh_queue_s *q)
{
  while (q->count > 0)
    {
      struct assh_queue_entry_s *che = assh_queue_back(q);
      struct assh_channel_s *ch = (void*)che;

      assh_queue_remove(q, che);
      assh_channel_cleanup(ch);
    }
}

/************************************************* incoming request */

static void assh_request_dequeue(struct assh_session_s *s,
                                 struct assh_channel_s *ch)
{
  struct assh_connection_context_s *pv = s->srv_pv;
  struct assh_queue_s *q = ch == NULL ? &pv->request_rqueue : &ch->request_rqueue;

  /* send and release ready replies present on queue in the right order */
  while (q->count > 0)
    {
      struct assh_queue_entry_s *rqe = assh_queue_back(q);
      struct assh_request_s *rq = (void*)rqe;
      if (rq->status != ASSH_REQUEST_ST_REPLY_READY)
        return;
      assh_transport_push(s, rq->reply_pck);
      rq->reply_pck = NULL;
      assh_queue_remove(q, rqe);
      assh_free(s->ctx, rq);
    }
}

assh_error_t
assh_request_failed_reply(struct assh_request_s *rq)
{
  assh_error_t err;
  struct assh_session_s *s = rq->session;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(rq->status != ASSH_REQUEST_ST_REPLY_POSTPONED,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  /* prepare failed reply packet */
  struct assh_channel_s *ch = rq->ch;

  if (ch != NULL)
    {
      switch (ch->status)
	{
	case ASSH_CHANNEL_ST_OPEN_SENT:
	case ASSH_CHANNEL_ST_OPEN_RECEIVED:
	case ASSH_CHANNEL_ST_CLOSE_CALLED:
	case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
	  ASSH_ERR_GTO(ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

	case ASSH_CHANNEL_ST_OPEN:
	case ASSH_CHANNEL_ST_EOF_SENT:
	case ASSH_CHANNEL_ST_EOF_RECEIVED:
	  break;

        case ASSH_CHANNEL_ST_EOF_CLOSE:
	case ASSH_CHANNEL_ST_CLOSING:
          /* unable to send a reply for this event after exchanging
             close packets with the remote host. */
	  return ASSH_NO_DATA;
	}

      ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_FAILURE,
		     4, &rq->reply_pck) | ASSH_ERRSV_CONTINUE, err);
      ASSH_ASSERT(assh_packet_add_u32(rq->reply_pck, ch->remote_id));
    }
  else
    {
      ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_REQUEST_FAILURE,
                     0, &rq->reply_pck) | ASSH_ERRSV_CONTINUE, err);
    }

  rq->status = ASSH_REQUEST_ST_REPLY_READY;
  assh_request_dequeue(s, ch);

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

assh_error_t
assh_request_success_reply(struct assh_request_s *rq,
                           const uint8_t *rsp_data,
                           size_t rsp_data_size)
{
  assh_error_t err;
  struct assh_session_s *s = rq->session;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(rq->status != ASSH_REQUEST_ST_REPLY_POSTPONED,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  /* prepare success reply packet */
  struct assh_channel_s *ch = rq->ch;

  if (ch != NULL)
    {
      switch (ch->status)
	{
	case ASSH_CHANNEL_ST_OPEN_SENT:
	case ASSH_CHANNEL_ST_OPEN_RECEIVED:
	case ASSH_CHANNEL_ST_CLOSE_CALLED:
	case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
	  ASSH_ERR_GTO(ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

	case ASSH_CHANNEL_ST_OPEN:
	case ASSH_CHANNEL_ST_EOF_SENT:
	case ASSH_CHANNEL_ST_EOF_RECEIVED:
	  break;

        case ASSH_CHANNEL_ST_EOF_CLOSE:
	case ASSH_CHANNEL_ST_CLOSING:
          /* unable to send a reply for this event after exchanging
             close packets with the remote host. */
	  return ASSH_NO_DATA;
	}

      ASSH_CHK_RET(rsp_data_size > 0,
		   ASSH_ERR_OUTPUT_OVERFLOW | ASSH_ERRSV_CONTINUE);

      ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_SUCCESS,
                     4, &rq->reply_pck) | ASSH_ERRSV_CONTINUE, err);

      ASSH_ASSERT(assh_packet_add_u32(rq->reply_pck, ch->remote_id));
    }
  else
    {
      ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_REQUEST_SUCCESS,
                     rsp_data_size, &rq->reply_pck) | ASSH_ERRSV_CONTINUE, err);
      /* add request specific data to the reply */
      uint8_t *data;
      ASSH_ASSERT(assh_packet_add_array(rq->reply_pck, rsp_data_size, &data));
      memcpy(data, rsp_data, rsp_data_size);
    }

  rq->status = ASSH_REQUEST_ST_REPLY_READY;
  assh_request_dequeue(s, ch);

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

/* event done, may send a reply */
static ASSH_EVENT_DONE_FCN(assh_event_request_done)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
  ASSH_CHK_RET(pv->state != ASSH_CONNECTION_ST_EVENT_REQUEST,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  /* release request packet */
  assh_packet_release(pv->pck);
  pv->pck = NULL;
  pv->state = ASSH_CONNECTION_ST_IDLE;

  struct assh_request_s *rq = e->connection.request.rq;

  /* acknowledge request */
  switch (e->connection.request.reply)
    {
    case ASSH_CONNECTION_REPLY_SUCCESS:
      if (rq != NULL)
        ASSH_ERR_RET(assh_request_success_reply(rq,
                      e->connection.request.rsp_data.data,
                      e->connection.request.rsp_data.size)
		     | ASSH_ERRSV_DISCONNECT);
      break;
    case ASSH_CONNECTION_REPLY_FAILED:
      if (rq != NULL)
        ASSH_ERR_RET(assh_request_failed_reply(rq)
		     | ASSH_ERRSV_DISCONNECT);
      break;
    case ASSH_CONNECTION_REPLY_POSTPONED:
      ASSH_CHK_RET(rq == NULL, ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
      break;
    case ASSH_CONNECTION_REPLY_CLOSED:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  return ASSH_OK;
}

/* setup an event from incoming request */
static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_request(struct assh_session_s *s,
                            struct assh_packet_s *p,
                            struct assh_event_s *e,
                            assh_bool_t global)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  /* parse packet */
  uint8_t *type, *want_reply, *data;
  struct assh_channel_s *ch;

  /* parse packet */
  if (global)
    {
      ch = NULL;
      type = p->head.end;
    }
  else
    {
      /* lookup channel */
      uint32_t ch_id = -1;
      ASSH_ERR_RET(assh_packet_check_u32(p, &ch_id, p->head.end, &type)
		   | ASSH_ERRSV_DISCONNECT);
      ch = (void*)assh_map_lookup(&pv->channel_map, ch_id, NULL);
      ASSH_CHK_RET(ch == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

      switch (ch->status)
	{
	case ASSH_CHANNEL_ST_OPEN_SENT:
	case ASSH_CHANNEL_ST_OPEN_RECEIVED:
	  ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

	case ASSH_CHANNEL_ST_OPEN:
	case ASSH_CHANNEL_ST_EOF_SENT:
	case ASSH_CHANNEL_ST_EOF_RECEIVED:
	  break;

        case ASSH_CHANNEL_ST_EOF_CLOSE:
	case ASSH_CHANNEL_ST_CLOSE_CALLED:
	  /* ignore request; our channel close packet will clear all
             pending requests on the remote side. */
	  return ASSH_OK;

	case ASSH_CHANNEL_ST_CLOSING:
	case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
          /* This channel id has been removed from the channel map
             when the close packet was received. */
	  ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
	}
    }

  ASSH_ERR_RET(assh_packet_check_string(p, type, &want_reply)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_array(p, want_reply, 1, &data)
	       | ASSH_ERRSV_DISCONNECT);

  struct assh_request_s *rq = NULL;
  if (*want_reply)
    {
      /* allocate a new request and push on appropriate queue */
      ASSH_ERR_RET(assh_alloc(s->ctx, sizeof(*rq), ASSH_ALLOC_INTERNAL, (void**)&rq)
		   | ASSH_ERRSV_DISCONNECT);
      assh_queue_push_front(global ? &pv->request_rqueue
			           : &ch->request_rqueue, &rq->qentry);
      rq->status = ASSH_REQUEST_ST_REPLY_POSTPONED;
      rq->session = s;
      rq->ch = ch;
      rq->reply_pck = NULL;
    }

  /* setup event */
  e->id = ASSH_EVENT_REQUEST;
  e->f_done = assh_event_request_done;

  e->connection.request.ch = ch;
  e->connection.request.rq = rq;

  struct assh_buffer_s *type_ = &e->connection.request.type;
  type_->str = (char*)type + 4;
  type_->len = want_reply - type - 4;

  struct assh_buffer_s *rq_data = &e->connection.request.rq_data;
  rq_data->size = p->data + p->data_size - data;
  rq_data->data = rq_data->size > 0 ? data : NULL;

  struct assh_buffer_s *rsp_data = &e->connection.request.rsp_data;
  rsp_data->data = NULL;
  rsp_data->size = 0;

  e->connection.request.reply = ASSH_CONNECTION_REPLY_FAILED;

  /* keep packet for type and rq_data buffers */
  pv->pck = assh_packet_refinc(p);

  pv->state = ASSH_CONNECTION_ST_EVENT_REQUEST;

  return ASSH_OK;
}

/************************************************* outgoing request */

/* send a new request */
assh_error_t assh_request(struct assh_session_s *s,
                          struct assh_channel_s *ch,
                          const char *type, size_t type_len,
                          const uint8_t *data, size_t data_len,
                          struct assh_request_s **rq_)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  /* prepare request packet */
  struct assh_packet_s *pout;
  size_t size = 4 + type_len + 1 + 4 + data_len;

  if (ch == NULL)
    {
      ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_GLOBAL_REQUEST, size, &pout)
		   | ASSH_ERRSV_CONTINUE, err);
    }
  else
    switch (ch->status)
      {
      case ASSH_CHANNEL_ST_OPEN_SENT:
      case ASSH_CHANNEL_ST_OPEN_RECEIVED:
      case ASSH_CHANNEL_ST_CLOSE_CALLED:
      case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
	ASSH_ERR_GTO(ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

      case ASSH_CHANNEL_ST_EOF_CLOSE:
      case ASSH_CHANNEL_ST_CLOSING:
	return ASSH_NO_DATA;

      case ASSH_CHANNEL_ST_OPEN:
      case ASSH_CHANNEL_ST_EOF_SENT:
      case ASSH_CHANNEL_ST_EOF_RECEIVED:
	ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_REQUEST, 4 + size, &pout)
		     | ASSH_ERRSV_CONTINUE, err);
	ASSH_ASSERT(assh_packet_add_u32(pout, ch->remote_id));  
      }

  uint8_t *str;
  ASSH_ASSERT(assh_packet_add_string(pout, type_len, &str));
  memcpy(str, type, type_len);
  ASSH_ASSERT(assh_packet_add_array(pout, 1, &str));
  *str = (rq_ != NULL);
  ASSH_ASSERT(assh_packet_add_array(pout, data_len, &str));
  memcpy(str, data, data_len);

  struct assh_request_s *rq = NULL;

  /* push a new entry in the request queue */
  if (rq_ != NULL)
    {
      ASSH_ERR_GTO(assh_alloc(s->ctx, sizeof(*rq), ASSH_ALLOC_INTERNAL, (void**)&rq)
		   | ASSH_ERRSV_CONTINUE, err_pkt);
      assh_queue_push_front(ch == NULL ? &pv->request_lqueue
                                       : &ch->request_lqueue, &rq->qentry);
      rq->status = ASSH_REQUEST_ST_WAIT_REPLY;
      rq->session = s;
      rq->ch = ch;
      rq->reply_pck = NULL;
      *rq_ = rq;
    }

  assh_transport_push(s, pout);

  return ASSH_OK;

 err_pkt:
  assh_packet_release(pout);
 err:
  return assh_session_error(s, err);
}

/* cleanup request reply event */
static ASSH_EVENT_DONE_FCN(assh_event_request_reply_done)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
  ASSH_CHK_RET(pv->state != ASSH_CONNECTION_ST_EVENT_REQUEST_REPLY,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  /* release packet */
  assh_packet_release(pv->pck);
  pv->pck = NULL;
  pv->state = ASSH_CONNECTION_ST_IDLE;

  /* pop and release request */
  struct assh_channel_s *ch = e->connection.request_reply.ch;
  struct assh_queue_s *q = ch == NULL
    ? &pv->request_lqueue : &ch->request_lqueue;

  struct assh_queue_entry_s *rqe = assh_queue_back(q);
  struct assh_request_s *rq = (void*)rqe;
  assert(e->connection.request_reply.rq == rq);

  assh_queue_remove(q, rqe);
  assh_free(s->ctx, rq);

  return ASSH_OK;
}

/* pop the next unreplied requests and report a reply failed event */
static assh_bool_t assh_request_reply_flush(struct assh_session_s *s,
					    struct assh_channel_s *ch,
					    struct assh_event_s *e)
{
  struct assh_connection_context_s *pv = s->srv_pv;
  struct assh_queue_s *q = ch == NULL
    ? &pv->request_lqueue : &ch->request_lqueue;

  if (q->count == 0)
    return 0;

  struct assh_request_s *rq = (void*)assh_queue_back(q);

  e->id = ASSH_EVENT_REQUEST_REPLY;
  e->f_done = assh_event_request_reply_done;

  e->connection.request_reply.ch = ch;
  e->connection.request_reply.rq = rq;
  e->connection.request_reply.reply = ASSH_CONNECTION_REPLY_CLOSED;

  struct assh_buffer_s *rsp_data = &e->connection.request_reply.rsp_data;
  rsp_data->size = 0;
  rsp_data->data = NULL;

  pv->state = ASSH_CONNECTION_ST_EVENT_REQUEST_REPLY;

  return 1;
}

/* setup an event from incoming request reply */
static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_request_reply(struct assh_session_s *s,
                                  struct assh_packet_s *p,
                                  struct assh_event_s *e,
                                  assh_bool_t global,
                                  assh_bool_t success)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  /* lookup channel */
  struct assh_channel_s *ch = NULL;
  uint8_t *data = p->head.end;
  struct assh_queue_s *q = &pv->request_lqueue;

  if (!global)
    {
      uint32_t ch_id = -1;
      ASSH_ERR_RET(assh_packet_check_u32(p, &ch_id, p->head.end, &data)
		   | ASSH_ERRSV_DISCONNECT);
      ch = (void*)assh_map_lookup(&pv->channel_map, ch_id, NULL);
      ASSH_CHK_RET(ch == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
      q = &ch->request_lqueue;

      switch (ch->status)
        {
        case ASSH_CHANNEL_ST_OPEN_SENT:
        case ASSH_CHANNEL_ST_OPEN_RECEIVED:
          ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

        case ASSH_CHANNEL_ST_OPEN:
        case ASSH_CHANNEL_ST_EOF_SENT:
        case ASSH_CHANNEL_ST_EOF_RECEIVED:
          break;

        case ASSH_CHANNEL_ST_EOF_CLOSE:
        case ASSH_CHANNEL_ST_CLOSE_CALLED:
          /* ignore the actual reply; request fail events will be
             reported when flushing the queue of sent requests. */
          return ASSH_OK;

        case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
        case ASSH_CHANNEL_ST_CLOSING:
          /* This channel id has been removed from the channel map
             when the close packet was received. */
          ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
        }
    }

  /* get next request in queue */
  ASSH_CHK_RET(q->count == 0,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  struct assh_request_s *rq = (void*)assh_queue_back(q);
  ASSH_CHK_RET(rq->status != ASSH_REQUEST_ST_WAIT_REPLY,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  /* setup event */
  e->id = ASSH_EVENT_REQUEST_REPLY;
  e->f_done = assh_event_request_reply_done;

  e->connection.request_reply.ch = ch;
  e->connection.request_reply.rq = rq;
  e->connection.request_reply.reply = success
    ? ASSH_CONNECTION_REPLY_SUCCESS : ASSH_CONNECTION_REPLY_FAILED;

  struct assh_buffer_s *rsp_data = &e->connection.request_reply.rsp_data;
  rsp_data->size = global && success ? p->data + p->data_size - data : 0;
  rsp_data->data = rsp_data->size > 0 ? data : NULL;

  /* keep packet for response data */
  if (rsp_data->size > 0)
    pv->pck = assh_packet_refinc(p);

  pv->state = ASSH_CONNECTION_ST_EVENT_REQUEST_REPLY;

  return ASSH_OK;
}

/************************************************* incoming channel open */

assh_error_t
assh_channel_open_failed_reply(struct assh_channel_s *ch,
                               enum assh_channel_open_reason_e reason)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(ch->status != ASSH_CHANNEL_ST_OPEN_RECEIVED,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  struct assh_packet_s *pout;

  /* send failed reply packet */
  ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_OPEN_FAILURE, 4 * 4, &pout)
	       | ASSH_ERRSV_CONTINUE, err);
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->remote_id));
  ASSH_ASSERT(assh_packet_add_u32(pout, reason));
  ASSH_ASSERT(assh_packet_add_string(pout, 0, NULL));
  ASSH_ASSERT(assh_packet_add_string(pout, 0, NULL));
  assh_transport_push(s, pout);

  /* release channel object */
  ASSH_ASSERT(!assh_map_remove_id(&pv->channel_map, ch->mentry.id));
  assh_channel_cleanup(ch);

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

assh_error_t
assh_channel_open_success_reply2(struct assh_channel_s *ch,
                                 uint32_t pkt_size, uint32_t win_size,
                                 const uint8_t *rsp_data,
                                 size_t rsp_data_len)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(ch->status != ASSH_CHANNEL_ST_OPEN_RECEIVED,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  ASSH_CHK_GTO(pkt_size < 1, ASSH_ERR_BAD_ARG | ASSH_ERRSV_CONTINUE, err);

  struct assh_packet_s *pout;

  /* send confirmation reply packet */
  uint8_t *data;
  ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_OPEN_CONFIRMATION,
                 4 * 4 + rsp_data_len, &pout) | ASSH_ERRSV_CONTINUE, err);

  ch->lpkt_size = ASSH_MIN(pkt_size, ASSH_MAX_PCK_PAYLOAD_SIZE
                           - /* extended data message header */ 3 * 4);
  ch->lwin_size = ch->lwin_left = ASSH_MAX(win_size, ch->lpkt_size * 4);
  ch->status = ASSH_CHANNEL_ST_OPEN;

  ASSH_ASSERT(assh_packet_add_u32(pout, ch->remote_id));
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->mentry.id));
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->lwin_left));
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->lpkt_size));
  ASSH_ASSERT(assh_packet_add_array(pout, rsp_data_len, &data));
  memcpy(data, rsp_data, rsp_data_len);
  assh_transport_push(s, pout);

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

/* event done, reply to open */
static ASSH_EVENT_DONE_FCN(assh_event_channel_open_done)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
  ASSH_CHK_RET(pv->state != ASSH_CONNECTION_ST_EVENT_CHANNEL_OPEN,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  /* release channel open packet */
  assh_packet_release(pv->pck);
  pv->pck = NULL;
  pv->state = ASSH_CONNECTION_ST_IDLE;

  struct assh_event_channel_open_s *eo = &e->connection.channel_open;

  switch (eo->reply)
    {
    case ASSH_CONNECTION_REPLY_SUCCESS:
      ASSH_ERR_RET(assh_channel_open_success_reply2(eo->ch, eo->win_size,
                     eo->pkt_size, eo->rsp_data.data, eo->rsp_data.size)
		   | ASSH_ERRSV_DISCONNECT);
      break;
    case ASSH_CONNECTION_REPLY_FAILED:
      ASSH_ERR_RET(assh_channel_open_failed_reply(eo->ch, eo->reason)
		   | ASSH_ERRSV_DISCONNECT);
      break;
    case ASSH_CONNECTION_REPLY_POSTPONED:
      /* keep values for assh_channel_open_success_reply */
      eo->ch->lpkt_size = eo->pkt_size;
      eo->ch->lwin_size = eo->win_size;
      break;
    case ASSH_CONNECTION_REPLY_CLOSED:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  return ASSH_OK;      
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_channel_open(struct assh_session_s *s,
                                 struct assh_packet_s *p,
                                 struct assh_event_s *e)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  /* parse packet */
  uint8_t *type = p->head.end, *data;
  uint32_t rid = 0, win_size = 0, pkt_size = 0;
  ASSH_ERR_RET(assh_packet_check_string(p, type, &data)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_u32(p, &rid, data, &data)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_u32(p, &win_size, data, &data)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_u32(p, &pkt_size, data, &data)
	       | ASSH_ERRSV_DISCONNECT);

  ASSH_CHK_RET(pkt_size < 1, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  /* create channel object */
  struct assh_channel_s *ch;
  ASSH_ERR_RET(assh_alloc(s->ctx, sizeof(*ch), ASSH_ALLOC_INTERNAL, (void**)&ch)
	       | ASSH_ERRSV_DISCONNECT);

  ch->mentry.id = pv->ch_id_counter++;
  ch->remote_id = rid;
  ch->rpkt_size = pkt_size;
  ch->rwin_left = win_size;
  ch->status = ASSH_CHANNEL_ST_OPEN_RECEIVED;
  ch->session = s;
  ch->data_pck = NULL;
  assh_queue_init(&ch->request_rqueue);
  assh_queue_init(&ch->request_lqueue);

  assh_map_insert(&pv->channel_map, &ch->mentry);

  /* setup event */
  e->id = ASSH_EVENT_CHANNEL_OPEN;
  e->f_done = assh_event_channel_open_done;

  e->connection.channel_open.ch = ch;

  struct assh_buffer_s *type_ = &e->connection.channel_open.type;
  type_->str = (char*)type + 4;
  type_->len = assh_load_u32(type);

  e->connection.channel_open.win_size = win_size;
  e->connection.channel_open.pkt_size = pkt_size;

  struct assh_buffer_s *rq_data = &e->connection.channel_open.rq_data;
  rq_data->size = p->data + p->data_size - data;
  rq_data->data = rq_data->size > 0 ? data : NULL;

  e->connection.channel_open.reply = ASSH_CONNECTION_REPLY_FAILED;
  e->connection.channel_open.reason = SSH_OPEN_UNKNOWN_CHANNEL_TYPE;

  struct assh_buffer_s *rsp_data = &e->connection.channel_open.rsp_data;
  rsp_data->data = NULL;
  rsp_data->size = 0;

  /* keep packet for type and rq_data */
  pv->pck = assh_packet_refinc(p);

  pv->state = ASSH_CONNECTION_ST_EVENT_CHANNEL_OPEN;

  return ASSH_OK;
}

/************************************************* outgoing channel open */

assh_error_t
assh_channel_open2(struct assh_session_s *s,
                   const char *type, size_t type_len,
                   const uint8_t *data, size_t data_len,
                   uint32_t pkt_size, uint32_t win_size,
		   struct assh_channel_s **ch_)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  ASSH_CHK_GTO(pkt_size < 1, ASSH_ERR_BAD_ARG | ASSH_ERRSV_CONTINUE, err);

  /* alloc open msg packet */
  struct assh_packet_s *pout;
  size_t size = 4 + type_len + 3 * 4 + 4 + data_len;

  ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_OPEN, size, &pout)
	       | ASSH_ERRSV_CONTINUE, err);

  /* create new channel object */
  struct assh_channel_s *ch;
  ASSH_ERR_GTO(assh_alloc(s->ctx, sizeof(*ch), ASSH_ALLOC_INTERNAL, (void**)&ch)
	       | ASSH_ERRSV_CONTINUE, err_pkt);

  ch->lpkt_size = ASSH_MIN(pkt_size, ASSH_MAX_PCK_PAYLOAD_SIZE
                           - /* extended data message header */ 3 * 4);
  ch->lwin_size = ch->lwin_left = ASSH_MAX(win_size, ch->lpkt_size * 4);
  ch->mentry.id = pv->ch_id_counter++;
  ch->status = ASSH_CHANNEL_ST_OPEN_SENT;
  ch->session = s;
  ch->data_pck = NULL;
  assh_queue_init(&ch->request_rqueue);
  assh_queue_init(&ch->request_lqueue);

  *ch_ = ch;
  assh_map_insert(&pv->channel_map, &ch->mentry);

  /* send open msg */
  uint8_t *str;
  ASSH_ASSERT(assh_packet_add_string(pout, type_len, &str));
  memcpy(str, type, type_len);
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->mentry.id));
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->lwin_left));
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->lpkt_size));
  ASSH_ASSERT(assh_packet_add_array(pout, data_len, &str));
  memcpy(str, data, data_len);

  assh_transport_push(s, pout);

  return ASSH_OK;

 err_pkt:
  assh_packet_release(pout);
 err:
  return assh_session_error(s, err);
}

static ASSH_EVENT_DONE_FCN(assh_event_channel_open_reply_done)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
  ASSH_CHK_RET(pv->state != ASSH_CONNECTION_ST_EVENT_CHANNEL_OPEN_REPLY,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  /* release packet */
  assh_packet_release(pv->pck);
  pv->pck = NULL;
  pv->state = ASSH_CONNECTION_ST_IDLE;

  struct assh_channel_s *ch = e->connection.channel_open_reply.ch;

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN:
      break;

    case ASSH_CHANNEL_ST_OPEN_SENT:
      /* we are left in this state if the open has not been accepted,
         release channel object */
      ASSH_ASSERT(!assh_map_remove_id(&pv->channel_map, ch->mentry.id));
      assh_channel_cleanup(ch);
      break;

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  return ASSH_OK;
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_channel_open_reply(struct assh_session_s *s,
                                       struct assh_packet_s *p,
                                       struct assh_event_s *e,
                                       assh_bool_t success)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  uint32_t ch_id = -1;
  uint8_t *data;
  ASSH_ERR_RET(assh_packet_check_u32(p, &ch_id, p->head.end, &data)
	       | ASSH_ERRSV_DISCONNECT);

  struct assh_channel_s *ch = (void*)assh_map_lookup(&pv->channel_map, ch_id, NULL);
  ASSH_CHK_RET(ch == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
  ASSH_CHK_RET(ch->status != ASSH_CHANNEL_ST_OPEN_SENT,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  e->id = ASSH_EVENT_CHANNEL_OPEN_REPLY;
  e->f_done = assh_event_channel_open_reply_done;

  e->connection.channel_open_reply.ch = ch;
  struct assh_buffer_s *rsp_data = &e->connection.channel_open_reply.rsp_data;

  if (success)
    {
      ASSH_ERR_RET(assh_packet_check_u32(p, &ch->remote_id, data, &data)
		   | ASSH_ERRSV_DISCONNECT);
      ASSH_ERR_RET(assh_packet_check_u32(p, &ch->rwin_left, data, &data)
		   | ASSH_ERRSV_DISCONNECT);
      ASSH_ERR_RET(assh_packet_check_u32(p, &ch->rpkt_size, data, &data)
		   | ASSH_ERRSV_DISCONNECT);

      ASSH_CHK_RET(ch->rpkt_size < 1, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

      e->connection.channel_open_reply.reply = ASSH_CONNECTION_REPLY_FAILED;

      rsp_data->size = p->data + p->data_size - data;
      rsp_data->data = rsp_data->size > 0 ? data : NULL;

      ch->status = ASSH_CHANNEL_ST_OPEN;
    }
  else
    {
      uint32_t reason = 0;
      ASSH_ERR_RET(assh_packet_check_u32(p, &reason, data, &data)
		   | ASSH_ERRSV_DISCONNECT);

      e->connection.channel_open_reply.reply = ASSH_CONNECTION_REPLY_SUCCESS;
      e->connection.channel_open_reply.reason = (enum assh_channel_open_reason_e)reason;

      rsp_data->data = NULL;
      rsp_data->size = 0;
    }

  /* keep packet for response data */
  if (rsp_data->size > 0)
    pv->pck = assh_packet_refinc(p);

  pv->state = ASSH_CONNECTION_ST_EVENT_CHANNEL_OPEN_REPLY;

  return ASSH_OK;
}

/************************************************* incoming channel data */

static ASSH_EVENT_DONE_FCN(assh_event_channel_data_done)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
  ASSH_CHK_RET(pv->state != ASSH_CONNECTION_ST_EVENT_CHANNEL_DATA,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  /* release request packet */
  assh_packet_release(pv->pck);
  pv->pck = NULL;
  pv->state = ASSH_CONNECTION_ST_IDLE;

  return ASSH_OK;
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_channel_data(struct assh_session_s *s,
                                 struct assh_packet_s *p,
				 struct assh_event_s *e,
                                 assh_bool_t ext)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  uint32_t ch_id = -1;
  uint8_t *data;
  ASSH_ERR_RET(assh_packet_check_u32(p, &ch_id, p->head.end, &data)
	       | ASSH_ERRSV_DISCONNECT);

  struct assh_channel_s *ch = (void*)assh_map_lookup(&pv->channel_map, ch_id, NULL);
  ASSH_CHK_RET(ch == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
    case ASSH_CHANNEL_ST_EOF_CLOSE:
      ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

    case ASSH_CHANNEL_ST_CLOSE_CALLED:
    case ASSH_CHANNEL_ST_OPEN:
    case ASSH_CHANNEL_ST_EOF_SENT:
      break;

    case ASSH_CHANNEL_ST_CLOSING:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  uint32_t ext_type = 0;
  if (ext)
    ASSH_ERR_RET(assh_packet_check_u32(p, &ext_type, data, &data)
		 | ASSH_ERRSV_DISCONNECT);

  uint32_t size = 0;
  ASSH_ERR_RET(assh_packet_check_u32(p, &size, data, &data) | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_array(p, data, size, NULL) | ASSH_ERRSV_DISCONNECT);

  ASSH_CHK_RET(size > ch->lpkt_size,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

#if 1
  ASSH_CHK_RET(size > ch->lwin_left,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
#else
  if (size > ch->lwin_left)
    size = ch->lwin_left;     /* ignore extra data, rfc4254 section 5.2 */
#endif

  /* update window and send adjustment */
  ch->lwin_left -= size;

#if 0
  ASSH_DEBUG("lwin_left=%u lwin_size=%u lpkt_size=%u\n",
	     ch->lwin_left, ch->lwin_size, ch->lpkt_size);
#endif

  if ((ch->lwin_left < ch->lwin_size / 2) &&
      ch->status != ASSH_CHANNEL_ST_CLOSE_CALLED)
    {
      uint32_t inc = ch->lwin_size - ch->lwin_left;

      struct assh_packet_s *pout;
      ASSH_ERR_RET(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_WINDOW_ADJUST, 2 * 4, &pout)
		   | ASSH_ERRSV_DISCONNECT);
      ASSH_ASSERT(assh_packet_add_u32(pout, ch->remote_id));
      ASSH_ASSERT(assh_packet_add_u32(pout, inc));
      assh_transport_push(s, pout);

      ch->lwin_left += inc;
    }

  /* setup event */
  e->id = ASSH_EVENT_CHANNEL_DATA;
  e->f_done = assh_event_channel_data_done;

  e->connection.channel_data.ch = ch;
  e->connection.channel_data.ext = ext;
  e->connection.channel_data.ext_type = ext_type;

  e->connection.channel_data.data.size = size;
  e->connection.channel_data.data.data = data;

  /* keep packet for data buffer */
  pv->pck = assh_packet_refinc(p);

  pv->state = ASSH_CONNECTION_ST_EVENT_CHANNEL_DATA;

  return ASSH_OK;
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_channel_window_adjust(struct assh_session_s *s,
                                          struct assh_packet_s *p,
					  struct assh_event_s *e)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  uint32_t ch_id = -1, inc = 0;
  uint8_t *data;
  ASSH_ERR_RET(assh_packet_check_u32(p, &ch_id, p->head.end, &data)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_u32(p, &inc, data, NULL)
	       | ASSH_ERRSV_DISCONNECT);

  struct assh_channel_s *ch = (void*)assh_map_lookup(&pv->channel_map, ch_id, NULL);
  ASSH_CHK_RET(ch == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
      ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

    case ASSH_CHANNEL_ST_EOF_SENT:
    case ASSH_CHANNEL_ST_EOF_CLOSE:
      return ASSH_OK;

    case ASSH_CHANNEL_ST_CLOSE_CALLED:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
    case ASSH_CHANNEL_ST_OPEN:
      break;

    case ASSH_CHANNEL_ST_CLOSING:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  if (inc == 0)
    return ASSH_OK;

  uint32_t left = ch->rwin_left + inc;

  ASSH_CHK_RET(left < ch->rwin_left, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  /* setup event */
  e->id = ASSH_EVENT_CHANNEL_WINDOW;
  e->f_done = NULL;

  e->connection.channel_window.ch = ch;
  e->connection.channel_window.old_size = ch->rwin_left;
  e->connection.channel_window.new_size = left;

  ch->rwin_left = left;

  return ASSH_OK;
}

/************************************************* outgoing channel data */

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_channel_data_alloc_chk(struct assh_channel_s *ch,
                            size_t *size, size_t min_size)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
    case ASSH_CHANNEL_ST_EOF_SENT:
    case ASSH_CHANNEL_ST_EOF_CLOSE:
    case ASSH_CHANNEL_ST_CLOSE_CALLED:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

    case ASSH_CHANNEL_ST_OPEN:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
      break;

    case ASSH_CHANNEL_ST_CLOSING:
      *size = 0;
      return ASSH_NO_DATA;
    }

  /* adjust size */
  size_t sz = *size;
  if (sz > ch->rpkt_size)
    sz = ch->rpkt_size;
  if (sz > ch->rwin_left)
    sz = ch->rwin_left;
  *size = sz;

  if (sz < min_size || min_size == 0)
    return ASSH_NO_DATA;

  /* release old unused packet */
  assh_packet_release(ch->data_pck);
  ch->data_pck = NULL;

  return ASSH_OK;
}

assh_error_t
assh_channel_data_alloc(struct assh_channel_s *ch,
                        uint8_t **data, size_t *size,
                        size_t min_size)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;

  ASSH_ERR_GTO(assh_channel_data_alloc_chk(ch, size, min_size), err);

  struct assh_packet_s *pout;

  ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_DATA,
		 2 * 4 + *size, &pout) | ASSH_ERRSV_CONTINUE, err);
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->remote_id));

  *data = pout->data + pout->data_size
    + /* room for data size */ 4;

  ch->data_pck = pout;

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

assh_error_t
assh_channel_data_alloc_ext(struct assh_channel_s *ch,
                            uint32_t ext_type,
                            uint8_t **data, size_t *size,
                            size_t min_size)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;

  ASSH_ERR_GTO(assh_channel_data_alloc_chk(ch, size, min_size), err);

  struct assh_packet_s *pout;

  ASSH_ERR_GTO(assh_packet_alloc(s->ctx, SSH_MSG_CHANNEL_EXTENDED_DATA,
		 3 * 4 + *size, &pout) | ASSH_ERRSV_CONTINUE, err);
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->remote_id));
  ASSH_ASSERT(assh_packet_add_u32(pout, ext_type));

  *data = pout->data + pout->data_size
    + /* room for data size */ 4;

  ch->data_pck = pout;

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

assh_error_t
assh_channel_data_send(struct assh_channel_s *ch, size_t size)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
    case ASSH_CHANNEL_ST_EOF_SENT:
    case ASSH_CHANNEL_ST_EOF_CLOSE:
    case ASSH_CHANNEL_ST_CLOSE_CALLED:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      ASSH_ERR_GTO(ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

    case ASSH_CHANNEL_ST_OPEN:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
      break;

    case ASSH_CHANNEL_ST_CLOSING:
      return ASSH_NO_DATA;
    }

  struct assh_packet_s *pout = ch->data_pck;

  ASSH_CHK_GTO(pout == NULL, ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(size > pout->alloc_size - pout->data_size - 4,
	       ASSH_ERR_OUTPUT_OVERFLOW | ASSH_ERRSV_CONTINUE, err);

  ASSH_ASSERT(assh_packet_add_u32(pout, size));

  assert(ch->rwin_left >= size);

  pout->data_size += size;
  ch->rwin_left -= size;

  assh_transport_push(ch->session, ch->data_pck);
  ch->data_pck = NULL;

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

/************************************************* incoming channel close/eof */

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_send_channel_close(struct assh_session_s *s,
                                   struct assh_channel_s *ch,
                                   uint8_t msg)
{
  assh_error_t err;

  struct assh_packet_s *pout;
  ASSH_ERR_RET(assh_packet_alloc(s->ctx, msg, 4, &pout)
	       | ASSH_ERRSV_CONTINUE);
  ASSH_ASSERT(assh_packet_add_u32(pout, ch->remote_id));

  assh_transport_push(s, pout);

  return ASSH_OK;
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_channel_close(struct assh_session_s *s,
                                  struct assh_packet_s *p,
                                  struct assh_event_s *e)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  uint32_t ch_id = -1;
  uint8_t *data;
  ASSH_ERR_RET(assh_packet_check_u32(p, &ch_id, p->head.end, &data)
	       | ASSH_ERRSV_DISCONNECT);

  struct assh_map_entry_s **chp;
  struct assh_channel_s *ch = (void*)assh_map_lookup(&pv->channel_map, ch_id, &chp);
  ASSH_CHK_RET(ch == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
      ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

    case ASSH_CHANNEL_ST_OPEN:
    case ASSH_CHANNEL_ST_EOF_SENT:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
      ASSH_ERR_RET(assh_connection_send_channel_close(s, ch, SSH_MSG_CHANNEL_CLOSE)
		   | ASSH_ERRSV_DISCONNECT);
      ch->status = ASSH_CHANNEL_ST_CLOSING;
      break;

    case ASSH_CHANNEL_ST_CLOSE_CALLED:
      ch->status = ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING;
      break;

    case ASSH_CHANNEL_ST_EOF_CLOSE:
      ch->status = ASSH_CHANNEL_ST_CLOSING;
      break;

    case ASSH_CHANNEL_ST_CLOSING:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      /* This channel id has been removed from the channel map
         when the close packet was received. */
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  /* move channel from id lookup map to closing queue */
  assh_map_remove(chp, (void*)ch);
  assh_queue_push_front(&pv->closing_queue, &ch->qentry);

  return ASSH_OK;
}

static ASSH_EVENT_DONE_FCN(assh_event_channel_close_done)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
  ASSH_CHK_RET(pv->state != ASSH_CONNECTION_ST_EVENT_CHANNEL_CLOSE,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  pv->state = ASSH_CONNECTION_ST_IDLE;

  struct assh_channel_s *ch = e->connection.channel_close.ch;

  assh_queue_remove(&pv->closing_queue, (void*)ch);
  assh_channel_cleanup(ch);

  return ASSH_OK;
}

static ASSH_EVENT_DONE_FCN(assh_event_channel_eof_done)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_RET(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
  ASSH_CHK_RET(pv->state != ASSH_CONNECTION_ST_EVENT_CHANNEL_EOF,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  pv->state = ASSH_CONNECTION_ST_IDLE;

  return ASSH_OK;
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_connection_got_channel_eof(struct assh_session_s *s,
                                struct assh_packet_s *p,
                                struct assh_event_s *e)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  uint32_t ch_id = -1;
  uint8_t *data;
  ASSH_ERR_RET(assh_packet_check_u32(p, &ch_id, p->head.end, &data)
	       | ASSH_ERRSV_DISCONNECT);

  struct assh_map_entry_s **chp;
  struct assh_channel_s *ch = (void*)assh_map_lookup(&pv->channel_map, ch_id, &chp);
  ASSH_CHK_RET(ch == NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
    case ASSH_CHANNEL_ST_EOF_CLOSE:
      ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

    case ASSH_CHANNEL_ST_OPEN:
      ch->status = ASSH_CHANNEL_ST_EOF_RECEIVED;
      break;

    case ASSH_CHANNEL_ST_EOF_SENT:
      ASSH_ERR_RET(assh_connection_send_channel_close(s, ch, SSH_MSG_CHANNEL_CLOSE)
		   | ASSH_ERRSV_DISCONNECT);
      ch->status = ASSH_CHANNEL_ST_EOF_CLOSE;
      break;

    case ASSH_CHANNEL_ST_CLOSE_CALLED:      
      return ASSH_OK;

    case ASSH_CHANNEL_ST_CLOSING:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      /* This channel id has been removed from the channel map
         when the close packet was received. */
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  e->id = ASSH_EVENT_CHANNEL_EOF;
  e->f_done = assh_event_channel_eof_done;
  e->connection.channel_eof.ch = ch;

  pv->state = ASSH_CONNECTION_ST_EVENT_CHANNEL_EOF;

  return ASSH_OK;
}

/************************************************* outgoing channel close/eof */

assh_error_t
assh_channel_eof(struct assh_channel_s *ch)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

    case ASSH_CHANNEL_ST_OPEN:
      ASSH_ERR_RET(assh_connection_send_channel_close(s, ch, SSH_MSG_CHANNEL_EOF)
		   | ASSH_ERRSV_CONTINUE);
      ch->status = ASSH_CHANNEL_ST_EOF_SENT;
      break;

    case ASSH_CHANNEL_ST_EOF_RECEIVED:
      ASSH_ERR_RET(assh_connection_send_channel_close(s, ch, SSH_MSG_CHANNEL_CLOSE)
		   | ASSH_ERRSV_CONTINUE);
      ch->status = ASSH_CHANNEL_ST_EOF_CLOSE;
      break;

    case ASSH_CHANNEL_ST_CLOSING:
      break;

    case ASSH_CHANNEL_ST_EOF_SENT:
    case ASSH_CHANNEL_ST_EOF_CLOSE:
    case ASSH_CHANNEL_ST_CLOSE_CALLED:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      ASSH_ERR_GTO(ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
    }

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

assh_error_t
assh_channel_close(struct assh_channel_s *ch)
{
  assh_error_t err;
  struct assh_session_s *s = ch->session;
  struct assh_connection_context_s *pv = s->srv_pv;

  ASSH_CHK_GTO(s->srv != &assh_service_connection,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
  ASSH_CHK_GTO(pv->state != ASSH_CONNECTION_ST_IDLE,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

  switch (ch->status)
    {
    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
      ASSH_ERR_GTO(ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);

    case ASSH_CHANNEL_ST_OPEN:
    case ASSH_CHANNEL_ST_EOF_SENT:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
      /** send a close packet, the actual closing will occur when
          the close reply packet will be received. */
      ASSH_ERR_GTO(assh_connection_send_channel_close(s, ch, SSH_MSG_CHANNEL_CLOSE)
		   | ASSH_ERRSV_CONTINUE, err);

    case ASSH_CHANNEL_ST_EOF_CLOSE:
      ch->status = ASSH_CHANNEL_ST_CLOSE_CALLED;
      break;

    case ASSH_CHANNEL_ST_CLOSING:
      ch->status = ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING;
      break;

    case ASSH_CHANNEL_ST_CLOSE_CALLED:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      ASSH_ERR_GTO(ASSH_ERR_STATE | ASSH_ERRSV_FATAL, err);
    }

  return ASSH_OK;
 err:
  return assh_session_error(s, err);
}

/************************************************* connection service */

/* service initialization */
static ASSH_SERVICE_INIT_FCN(assh_connection_init)
{
  struct assh_connection_context_s *pv;
  assh_error_t err;

  ASSH_ERR_RET(assh_alloc(s->ctx, sizeof(*pv),
                 ASSH_ALLOC_INTERNAL, (void**)&pv) | ASSH_ERRSV_CONTINUE);

  assh_queue_init(&pv->request_rqueue);
  assh_queue_init(&pv->request_lqueue);
  assh_queue_init(&pv->closing_queue);

  pv->channel_map = NULL;
  pv->pck = NULL;
  pv->ch_id_counter = 0;

  s->srv = &assh_service_connection;
  s->srv_pv = pv;

  pv->state = ASSH_CONNECTION_ST_START;

  return ASSH_OK;
}

static void assh_channel_cleanup_i(struct assh_map_entry_s *ch_, void *unused)
{
  assh_channel_cleanup((struct assh_channel_s*)ch_);
}

static ASSH_SERVICE_CLEANUP_FCN(assh_connection_cleanup)
{
  struct assh_connection_context_s *pv = s->srv_pv;

  assh_packet_release(pv->pck);

  assh_request_queue_cleanup(s, &pv->request_rqueue);
  assh_request_queue_cleanup(s, &pv->request_lqueue);

  assh_map_iter(pv->channel_map, NULL, &assh_channel_cleanup_i);
  assh_channel_queue_cleanup(s, &pv->closing_queue);

  assh_free(s->ctx, pv);

  s->srv_pv = NULL;
  s->srv = NULL;
}

static void assh_channel_force_close_i(struct assh_map_entry_s *ch_, void *pv_)
{
  struct assh_channel_s *ch = (struct assh_channel_s*)ch_;
  struct assh_connection_context_s *pv = pv_;

  switch (ch->status)
    {

    case ASSH_CHANNEL_ST_OPEN_SENT:
    case ASSH_CHANNEL_ST_OPEN_RECEIVED:
    case ASSH_CHANNEL_ST_OPEN:
    case ASSH_CHANNEL_ST_EOF_SENT:
    case ASSH_CHANNEL_ST_EOF_RECEIVED:
    case ASSH_CHANNEL_ST_EOF_CLOSE:
      ch->status = ASSH_CHANNEL_ST_CLOSING;
      break;

    case ASSH_CHANNEL_ST_CLOSE_CALLED:
      ch->status = ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING;      
      break;

    case ASSH_CHANNEL_ST_CLOSING:
    case ASSH_CHANNEL_ST_CLOSE_CALLED_CLOSING:
      /* can not be in channel map */
      assert(0);
    }

  assh_queue_push_front(&pv->closing_queue, &ch->qentry);
}

static ASSH_SERVICE_PROCESS_FCN(assh_connection_process)
{
  assh_error_t err;
  struct assh_connection_context_s *pv = s->srv_pv;

  switch (pv->state)
    {
    case ASSH_CONNECTION_ST_START:
      assert(p == NULL);
      e->id = ASSH_EVENT_CONNECTION_START;
      e->f_done = NULL;
      pv->state = ASSH_CONNECTION_ST_IDLE;
      return ASSH_OK;

    case ASSH_CONNECTION_ST_IDLE:
      break;

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  /* move all channels to the closing queue */
  if (s->tr_st == ASSH_TR_FIN && pv->channel_map != NULL)
    {
      assh_map_iter(pv->channel_map, pv, &assh_channel_force_close_i);
      pv->channel_map = NULL;
    }

  /* report channel closing related events */
  if (pv->closing_queue.count > 0)
    {
      struct assh_channel_s *ch = (void*)assh_queue_back(&pv->closing_queue);

      /* pop a pending channel request and report an event */
      if (assh_request_reply_flush(s, ch, e))
        return ASSH_NO_DATA;

      /* report channel close event */
      e->id = ASSH_EVENT_CHANNEL_CLOSE;
      e->f_done = assh_event_channel_close_done;
      e->connection.channel_close.ch = ch;

      pv->state = ASSH_CONNECTION_ST_EVENT_CHANNEL_CLOSE;

      return ASSH_NO_DATA;
    }

  /* flush global requests */
  if (s->tr_st == ASSH_TR_FIN)
    {
      /* pop a pending global request and report an event */
      if (assh_request_reply_flush(s, NULL, e))
        return ASSH_NO_DATA;

      /* no more event to report, last return of this session */
      return ASSH_OK;
    }

  /* handle incoming packet, if any */
  if (p == NULL)
    return ASSH_OK;

  switch (p->head.msg)
    {
    case SSH_MSG_GLOBAL_REQUEST:
      ASSH_ERR_RET(assh_connection_got_request(s, p, e, 1));
      break;
    case SSH_MSG_REQUEST_SUCCESS:
      ASSH_ERR_RET(assh_connection_got_request_reply(s, p, e, 1, 1));
      break;
    case SSH_MSG_REQUEST_FAILURE:
      ASSH_ERR_RET(assh_connection_got_request_reply(s, p, e, 1, 0));
      break;
    case SSH_MSG_CHANNEL_OPEN:
      ASSH_ERR_RET(assh_connection_got_channel_open(s, p, e));
      break;
    case SSH_MSG_CHANNEL_OPEN_CONFIRMATION:
      ASSH_ERR_RET(assh_connection_got_channel_open_reply(s, p, e, 1));
      break;
    case SSH_MSG_CHANNEL_OPEN_FAILURE:
      ASSH_ERR_RET(assh_connection_got_channel_open_reply(s, p, e, 0));
      break;
    case SSH_MSG_CHANNEL_WINDOW_ADJUST:
      ASSH_ERR_RET(assh_connection_got_channel_window_adjust(s, p, e));
      break;
    case SSH_MSG_CHANNEL_DATA:
      ASSH_ERR_RET(assh_connection_got_channel_data(s, p, e, 0));
      break;
    case SSH_MSG_CHANNEL_EXTENDED_DATA:
      ASSH_ERR_RET(assh_connection_got_channel_data(s, p, e, 1));
      break;
    case SSH_MSG_CHANNEL_EOF:
      ASSH_ERR_RET(assh_connection_got_channel_eof(s, p, e));
      break;
    case SSH_MSG_CHANNEL_CLOSE:
      ASSH_ERR_RET(assh_connection_got_channel_close(s, p, e));
      break;
    case SSH_MSG_CHANNEL_REQUEST:
      ASSH_ERR_RET(assh_connection_got_request(s, p, e, 0));
      break;
    case SSH_MSG_CHANNEL_SUCCESS:
      ASSH_ERR_RET(assh_connection_got_request_reply(s, p, e, 0, 1));
      break;
    case SSH_MSG_CHANNEL_FAILURE:
      ASSH_ERR_RET(assh_connection_got_request_reply(s, p, e, 0, 0));
      break;
    default:
      ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
    }

  return ASSH_OK;
}

const struct assh_service_s assh_service_connection =
{
  .name = "ssh-connection",
  .side = ASSH_CLIENT_SERVER,
  .f_init = assh_connection_init,
  .f_cleanup = assh_connection_cleanup,
  .f_process = assh_connection_process,  
};

