#define EFL_NET_DIALER_WEBSOCKET_PROTECTED 1
#define EFL_NET_DIALER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"
#include "Emile.h"

/*
 * NOTE:
 *
 * Test this code against Autobahnsuite: http://autobahn.ws/testsuite/
 * See src/examples/ecore/efl_net_dialer_websocket_autobahntestee
 *
 * Known failed tests:
 *
 *  - Cases 6.4.x: fail fast.
 *
 *    STATUS: WONTFIX
 *
 *    These are non-strict and requires UTF-8 to be checked per frame,
 *    something we do not do as it's left to the user and the user
 *    only gets full frames. At the end user will fail, but taking
 *    more data then it should.
 *
 *
 *  - Case 6.7.1: message is UTF-8 null byte (1 byte = \x00).
 *
 *    STATUS: WONTFIX
 *
 *    We handle text messages as NULL terminated strings and when
 *    sending back, we do not include such null terminator in the
 *    payload. To do so we'd need to use a cumbersome API that
 *    specifies the size of the string.
 *
 */


/* just to check curl_version_info_data and warn on old versions */
#include "ecore_con_url_curl.h"

#define MY_CLASS EFL_NET_DIALER_WEBSOCKET_CLASS

typedef enum _Efl_Net_Dialer_Websocket_Opcode {
  EFL_NET_DIALER_WEBSOCKET_OPCODE_CONTINUATION = 0x0,
  EFL_NET_DIALER_WEBSOCKET_OPCODE_TEXT = 0x1,
  EFL_NET_DIALER_WEBSOCKET_OPCODE_BINARY = 0x2,
  EFL_NET_DIALER_WEBSOCKET_OPCODE_CLOSE = 0x8,
  EFL_NET_DIALER_WEBSOCKET_OPCODE_PING = 0x9,
  EFL_NET_DIALER_WEBSOCKET_OPCODE_PONG = 0xa,
} Efl_Net_Dialer_Websocket_Opcode;

static inline Eina_Bool
_efl_net_dialer_websocket_opcode_control_check(Efl_Net_Dialer_Websocket_Opcode opcode)
{
   switch (opcode)
     {
      case EFL_NET_DIALER_WEBSOCKET_OPCODE_CONTINUATION:
      case EFL_NET_DIALER_WEBSOCKET_OPCODE_TEXT:
      case EFL_NET_DIALER_WEBSOCKET_OPCODE_BINARY:
         return EINA_FALSE;
      default:
         return EINA_TRUE;
     }
}

static inline Eina_Bool
_efl_net_dialer_websocket_close_reason_check(Efl_Net_Dialer_Websocket_Close_Reason r)
{
   switch (r)
     {
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_NORMAL:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_GOING_AWAY:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PROTOCOL_ERROR:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_UNEXPECTED_DATA:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_INCONSISTENT_DATA:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_POLICY_VIOLATION:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_TOO_BIG:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_MISSING_EXTENSION:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_SERVER_ERROR:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_IANA_REGISTRY_START:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_IANA_REGISTRY_END:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PRIVATE_START:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PRIVATE_END:
         return EINA_TRUE;
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_NO_REASON:
      case EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_ABRUPTLY:
         return EINA_FALSE;
     }

   if ((r >= EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_IANA_REGISTRY_START) &&
       (r <= EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_IANA_REGISTRY_END))
     return EINA_TRUE;

   if ((r >= EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PRIVATE_START) &&
       (r <= EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PRIVATE_END))
     return EINA_TRUE;

   return EINA_FALSE;
}

/*
 * WebSocket is a framed protocol in the format:
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-------+-+-------------+-------------------------------+
 *   |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 *   |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 *   |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 *   | |1|2|3|       |K|             |                               |
 *   +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 *   |     Extended payload length continued, if payload len == 127  |
 *   + - - - - - - - - - - - - - - - +-------------------------------+
 *   |                               |Masking-key, if MASK set to 1  |
 *   +-------------------------------+-------------------------------+
 *   | Masking-key (continued)       |          Payload Data         |
 *   +-------------------------------- - - - - - - - - - - - - - - - +
 *   :                     Payload Data continued ...                :
 *   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 *   |                     Payload Data continued ...                |
 *   +---------------------------------------------------------------+
 *
 * See https://tools.ietf.org/html/rfc6455#section-5.2
 */
typedef struct _Efl_Net_Dialer_Websocket_Frame_Header {
    /* first byte: fin + opcode */
    uint8_t opcode : 4;
    uint8_t _reserved : 3;
    uint8_t fin : 1;

    /* second byte: mask + payload length */
    uint8_t payload_len : 7; /* if 126, uses extra 2 bytes (uint16_t)
                              * if 127, uses extra 8 bytes (uint64_t)
                              * if <=125 is self-contained
                              */
    uint8_t mask : 1; /* if 1, uses 4 extra bytes */
} Efl_Net_Dialer_Websocket_Frame_Header;

typedef struct _Efl_Net_Dialer_Websocket_Frame {
   EINA_INLIST;
   size_t len; /* total frame size to send */
   Efl_Net_Dialer_Websocket_Frame_Header header;
} Efl_Net_Dialer_Websocket_Frame;

typedef struct _Efl_Net_Dialer_Websocket_Pending_Read {
   EINA_INLIST;
   size_t len;
   uint8_t *bytes;
} Efl_Net_Dialer_Websocket_Pending_Read;

typedef struct _Efl_Net_Dialer_Websocket_Data {
   Eo *http;
   Eina_Future *close_timeout;
   Eina_Future *job;
   Eina_Stringshare *address_dial; /* must rewrite ws->http, wss->https */
   Eina_Stringshare *address_remote; /* must rewrite ws->http, wss->https */
   struct {
      Eina_List *requested;
      Eina_List *received;
   } protocols;
   char accept_key[29]; /* 28 + \0 */
   struct {
      struct {
         uint64_t total_len;
         uint64_t used_len;
         uint8_t *payload;
         Efl_Net_Dialer_Websocket_Opcode opcode;
         Eina_Bool fin;
      } current;

      struct {
         uint64_t total_len;
         uint64_t used_len;
         uint8_t *payload;
         Efl_Net_Dialer_Websocket_Opcode opcode;
      } fragmented;

      Efl_Net_Dialer_Websocket_Pending_Read *pending_read;
      size_t pending_read_offset;

      /* for current frame */
      uint8_t tmpbuf[sizeof(Efl_Net_Dialer_Websocket_Frame_Header) + sizeof(uint64_t)];
      uint8_t done; /* of tmpbuf, for header */
      uint8_t needed; /* of tmpbuf, for header */
   } recv;
   struct {
      Efl_Net_Dialer_Websocket_Frame *pending;
      size_t offset;
   } send;
   Efl_Net_Dialer_Websocket_Streaming_Mode streaming_mode;
   Eina_Bool connected;
   Eina_Bool close_requested;
   Eina_Bool can_read;
   Eina_Bool can_write;
} Efl_Net_Dialer_Websocket_Data;

static void _efl_net_dialer_websocket_job_schedule(Eo *o, Efl_Net_Dialer_Websocket_Data *pd);

static void
_efl_net_dialer_websocket_send_pending_remove(Efl_Net_Dialer_Websocket_Data *pd)
{
   Efl_Net_Dialer_Websocket_Frame *f = pd->send.pending;
   Eina_Inlist *lst = EINA_INLIST_GET(f);

   lst = eina_inlist_remove(lst, lst);
   pd->send.pending = EINA_INLIST_CONTAINER_GET(lst, Efl_Net_Dialer_Websocket_Frame);

   free(f);
   pd->send.offset = 0;
}

static uint8_t *
_efl_net_dialer_websocket_frame_bytes_get(Efl_Net_Dialer_Websocket_Frame *f)
{
   return (uint8_t *)&f->header;
}

static uint8_t *
_efl_net_dialer_websocket_frame_mask_get(Efl_Net_Dialer_Websocket_Frame *f)
{
   uint8_t *bytes = _efl_net_dialer_websocket_frame_bytes_get(f)
     + sizeof(Efl_Net_Dialer_Websocket_Frame_Header);

   if (f->header.payload_len == 127)
     return bytes + sizeof(uint64_t);
   else if (f->header.payload_len == 126)
     return bytes + sizeof(uint16_t);
   else
     return bytes;
}

static uint8_t *
_efl_net_dialer_websocket_frame_payload_get(Efl_Net_Dialer_Websocket_Frame *f)
{
   return _efl_net_dialer_websocket_frame_mask_get(f) + 4;
}

static Efl_Net_Dialer_Websocket_Frame *
_efl_net_dialer_websocket_send_pending_add(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Efl_Net_Dialer_Websocket_Opcode opcode, size_t payload_len)
{
   uint8_t *bytes;
   size_t len = payload_len + 4 /* sizeof mask */;
   Efl_Net_Dialer_Websocket_Frame *f;
   Eina_Inlist *lst;
   int i;

   if (payload_len > UINT16_MAX) len += sizeof(uint64_t);
   else if (payload_len > 125) len += sizeof(uint16_t);

   f = malloc(sizeof(Efl_Net_Dialer_Websocket_Frame) + len);
   EINA_SAFETY_ON_NULL_RETURN_VAL(f, NULL);

   f->len = sizeof(Efl_Net_Dialer_Websocket_Frame_Header) + len;
   f->header.opcode = opcode;
   f->header.fin = 1;
   f->header.mask = 1;
   f->header.payload_len = ((payload_len > UINT16_MAX) ? 127 :
                            ((payload_len > 125) ? 126 : payload_len));

   bytes = _efl_net_dialer_websocket_frame_bytes_get(f)
     + sizeof(Efl_Net_Dialer_Websocket_Frame_Header);

   if (f->header.payload_len == 127)
     {
        uint64_t plen = payload_len;
#ifndef WORDS_BIGENDIAN
        plen = eina_swap64(plen);
#endif
        memcpy(bytes, &plen, sizeof(plen));
        bytes += sizeof(plen);
     }
   else if (f->header.payload_len == 126)
     {
        uint16_t plen = payload_len;
#ifndef WORDS_BIGENDIAN
        plen = eina_swap16(plen);
#endif
        memcpy(bytes, &plen, sizeof(plen));
        bytes += sizeof(plen);
     }

   for (i = 0; i < 4; i++, bytes++)
     *bytes = rand() & 0xff;

   lst = EINA_INLIST_GET(pd->send.pending);
   lst = eina_inlist_append(lst, EINA_INLIST_GET(f));
   pd->send.pending = EINA_INLIST_CONTAINER_GET(lst, Efl_Net_Dialer_Websocket_Frame);

   DBG("opcode=%#x, payload_len=%zd, f=%p len=%zd", opcode, payload_len, f, f->len);

   /* say we can't write to throttle writers until we actually write */
   efl_io_writer_can_write_set(o, EINA_FALSE);

   if (efl_io_writer_can_write_get(pd->http))
     _efl_net_dialer_websocket_job_schedule(o, pd);

   return f;
}

static void
_efl_net_dialer_websocket_frame_write(Efl_Net_Dialer_Websocket_Frame *f, size_t offset, const void *mem, size_t len)
{
   uint8_t *mask = _efl_net_dialer_websocket_frame_mask_get(f);
   uint8_t *payload = _efl_net_dialer_websocket_frame_payload_get(f);
   const uint8_t *input = mem;
   uint8_t *o;

   for (o = payload + offset; o < payload + offset + len; o++, input++)
     *o = *input ^ mask[(o - payload) & 0x3];
}

static void
_efl_net_dialer_websocket_send(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Efl_Net_Dialer_Websocket_Opcode opcode, const void *mem, size_t len)
{
   Efl_Net_Dialer_Websocket_Frame *f;

   f = _efl_net_dialer_websocket_send_pending_add(o, pd, opcode, len);
   EINA_SAFETY_ON_NULL_RETURN(f);

   _efl_net_dialer_websocket_frame_write(f, 0, mem, len);
}

static void
_efl_net_dialer_websocket_job_send(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   Eina_Slice slice = {
     .bytes = _efl_net_dialer_websocket_frame_bytes_get(pd->send.pending) + pd->send.offset,
     .len = pd->send.pending->len - pd->send.offset,
   };
   Eina_Error err = efl_io_writer_write(pd->http, &slice, NULL);

   pd->send.offset += slice.len;

   if (pd->send.offset == pd->send.pending->len)
     {
        _efl_net_dialer_websocket_send_pending_remove(pd);
        if (!pd->close_requested)
          efl_io_writer_can_write_set(o, EINA_TRUE);
     }

   if ((err) && (err != EAGAIN))
     {
        ERR("could not write to HTTP socket #%d '%s'", err, eina_error_msg_get(err));
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
     }
}

static void
_efl_net_dialer_websocket_recv_frame_steal_and_queue(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, uint8_t **p_payload, size_t len)
{
   Efl_Net_Dialer_Websocket_Pending_Read *pr;
   Eina_Inlist *lst;

   if (len == 0) return;

   pr = malloc(sizeof(Efl_Net_Dialer_Websocket_Pending_Read));
   EINA_SAFETY_ON_NULL_RETURN(pr);

   pr->len = len;
   pr->bytes = *p_payload;

   lst = EINA_INLIST_GET(pd->recv.pending_read);
   lst = eina_inlist_append(lst, EINA_INLIST_GET(pr));
   pd->recv.pending_read = EINA_INLIST_CONTAINER_GET(lst, Efl_Net_Dialer_Websocket_Pending_Read);

   *p_payload = NULL;

   efl_io_reader_can_read_set(o, EINA_TRUE);
}

#define _efl_net_dialer_websocket_close_protocol_error(o, message) \
  do \
    { \
       if (!efl_io_closer_closed_get(o)) \
         { \
            WRN("dialer=%p closing with PROTOCOL ERROR (%d)", o, EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PROTOCOL_ERROR); \
            efl_net_dialer_websocket_close_request(o, EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PROTOCOL_ERROR, message); \
         } \
    } \
  while (0)

static Eina_Bool
_efl_net_dialer_websocket_job_dispatch_frame_validate(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   if ((pd->close_requested) && (pd->recv.current.opcode != EFL_NET_DIALER_WEBSOCKET_OPCODE_CLOSE))
     {
        DBG("dialer=%p dropped frame opcode=%#x: close requested", o, pd->recv.current.opcode);
        return EINA_FALSE;
     }

   if ((!pd->recv.current.fin) &&
       _efl_net_dialer_websocket_opcode_control_check(pd->recv.current.opcode))
     WRN("dialer=%p server sent forbidden fragmented control frame opcode=%#x.",
         o, pd->recv.current.opcode);
   else if ((pd->recv.current.opcode == EFL_NET_DIALER_WEBSOCKET_OPCODE_CONTINUATION) &&
            (pd->recv.fragmented.opcode == 0))
     WRN("dialer=%p server sent continuation frame after non-fragmentable frame", o);
   else
     return EINA_TRUE;

   _efl_net_dialer_websocket_close_protocol_error(o, NULL);
   return EINA_FALSE;
}

static void
_efl_net_dialer_websocket_job_dispatch_frame_text(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, uint8_t **p_payload, size_t len)
{
   char *text = (char *)*p_payload;

   /* len == 0 may contain an old payload to be reused with realloc() */
   if (len == 0) text = "";

   efl_event_callback_call(o, EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_TEXT, text);
   if (pd->streaming_mode == EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_TEXT)
     _efl_net_dialer_websocket_recv_frame_steal_and_queue(o, pd, p_payload, len);
}

static void
_efl_net_dialer_websocket_job_dispatch_frame_binary(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, uint8_t **p_payload, size_t len)
{
   Eina_Slice slice = {
     .bytes = *p_payload,
     .len = len,
   };
   efl_event_callback_call(o, EFL_NET_DIALER_WEBSOCKET_EVENT_MESSAGE_BINARY, &slice);
   if (pd->streaming_mode == EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_BINARY)
     _efl_net_dialer_websocket_recv_frame_steal_and_queue(o, pd, p_payload, len);
}

static void
_efl_net_dialer_websocket_job_dispatch_frame(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   DBG("frame opcode=%#x fin=%d, length=%" PRIu64,
       pd->recv.current.opcode, pd->recv.current.fin, pd->recv.current.used_len);

   if (!_efl_net_dialer_websocket_job_dispatch_frame_validate(o, pd))
     return;

   switch (pd->recv.current.opcode)
     {
      case EFL_NET_DIALER_WEBSOCKET_OPCODE_CONTINUATION:
         if (pd->recv.current.fin)
           {
              if (pd->recv.fragmented.opcode == EFL_NET_DIALER_WEBSOCKET_OPCODE_TEXT)
                _efl_net_dialer_websocket_job_dispatch_frame_text(o, pd, &pd->recv.current.payload, pd->recv.current.used_len);
              else if (pd->recv.fragmented.opcode == EFL_NET_DIALER_WEBSOCKET_OPCODE_BINARY)
                _efl_net_dialer_websocket_job_dispatch_frame_binary(o, pd, &pd->recv.current.payload, pd->recv.current.used_len);

              memset(&pd->recv.fragmented, 0, sizeof(pd->recv.fragmented));
           }
         else
           {
              pd->recv.fragmented.payload = pd->recv.current.payload;
              pd->recv.fragmented.used_len = pd->recv.current.used_len;
              pd->recv.fragmented.total_len = pd->recv.current.total_len;
              pd->recv.current.payload = NULL;
              pd->recv.current.used_len = 0;
              pd->recv.current.total_len = 0;
           }
         break;

      case EFL_NET_DIALER_WEBSOCKET_OPCODE_TEXT:
         if (pd->recv.current.fin)
           _efl_net_dialer_websocket_job_dispatch_frame_text(o, pd, &pd->recv.current.payload, pd->recv.current.used_len);
         else
           {
              pd->recv.fragmented.payload = pd->recv.current.payload;
              pd->recv.fragmented.used_len = pd->recv.current.used_len;
              pd->recv.fragmented.total_len = pd->recv.current.total_len;
              pd->recv.fragmented.opcode = pd->recv.current.opcode;

              pd->recv.current.payload = NULL;
              pd->recv.current.used_len = 0;
              pd->recv.current.total_len = 0;
              pd->recv.current.opcode = 0;
              pd->recv.current.fin = 0;
           }
         break;

      case EFL_NET_DIALER_WEBSOCKET_OPCODE_BINARY:
         if (pd->recv.current.fin)
           _efl_net_dialer_websocket_job_dispatch_frame_binary(o, pd, &pd->recv.current.payload, pd->recv.current.used_len);
         else
           {
              pd->recv.fragmented.payload = pd->recv.current.payload;
              pd->recv.fragmented.used_len = pd->recv.current.used_len;
              pd->recv.fragmented.total_len = pd->recv.current.total_len;
              pd->recv.fragmented.opcode = pd->recv.current.opcode;

              pd->recv.current.payload = NULL;
              pd->recv.current.used_len = 0;
              pd->recv.current.total_len = 0;
              pd->recv.current.opcode = 0;
              pd->recv.current.fin = 0;
           }
         break;

      case EFL_NET_DIALER_WEBSOCKET_OPCODE_CLOSE:
        {
           Efl_Net_Dialer_Websocket_Closed_Reason reason = {
             .reason = EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_NO_REASON,
             .message = "",
           };

           if (pd->recv.current.used_len >= sizeof(uint16_t))
             {
                uint16_t r;
                memcpy(&r, pd->recv.current.payload, sizeof(uint16_t));
#ifndef WORDS_BIGENDIAN
                r = eina_swap16(r);
#endif
                if (!_efl_net_dialer_websocket_close_reason_check(r))
                  {
                     WRN("dialer=%p invalid CLOSE reason: %hu", o, r);
                     _efl_net_dialer_websocket_close_protocol_error(o, "invalid close reason");
                     r = EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_PROTOCOL_ERROR;
                  }
                reason.reason = r;
                reason.message = (const char *)pd->recv.current.payload + sizeof(uint16_t);
             }
           else if ((pd->recv.current.used_len > 0) &&
                    (pd->recv.current.used_len < sizeof(uint16_t)))
             {
                WRN("dialer=%p invalid CLOSE payload length: %" PRIu64, o, pd->recv.current.used_len);
                _efl_net_dialer_websocket_close_protocol_error(o, "invalid close payload length");
             }

           efl_event_callback_call(o, EFL_NET_DIALER_WEBSOCKET_EVENT_CLOSED_REASON, &reason);
           if ((!pd->recv.pending_read) &&
               (pd->streaming_mode != EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_DISABLED))
             efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);

           if (!pd->close_requested)
             efl_io_closer_close(o);
           else
             efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
           if (pd->close_timeout)
             eina_future_cancel(pd->close_timeout);
           break;
        }

      case EFL_NET_DIALER_WEBSOCKET_OPCODE_PING:
         DBG("got PING from server '%.*s', echo as PONG.", (int)pd->recv.current.used_len, pd->recv.current.payload);
         _efl_net_dialer_websocket_send(o, pd, EFL_NET_DIALER_WEBSOCKET_OPCODE_PONG, pd->recv.current.payload, pd->recv.current.used_len);
         break;

      case EFL_NET_DIALER_WEBSOCKET_OPCODE_PONG:
        {
           char *text = (char *)pd->recv.current.payload;
           if (pd->recv.current.used_len == 0) text = "";
           efl_event_callback_call(o, EFL_NET_DIALER_WEBSOCKET_EVENT_PONG, text);
           break;
        }

      default:
         WRN("dialer=%p unexpected WebSocket opcode: %#x.", o, pd->recv.current.opcode);
         _efl_net_dialer_websocket_close_protocol_error(o, "unexpected opcode");
     }
}

static void
_efl_net_dialer_websocket_job_receive(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   const uint8_t fh_len = sizeof(Efl_Net_Dialer_Websocket_Frame_Header);
   Eina_Error err;

   while ((pd->recv.done < pd->recv.needed) &&
          (efl_io_reader_can_read_get(pd->http)))
     {
        uint64_t frame_len;
        Eina_Rw_Slice rw_slice = {
          .bytes = pd->recv.tmpbuf + pd->recv.done,
          .len = pd->recv.needed - pd->recv.done,
        };

        err = efl_io_reader_read(pd->http, &rw_slice);
        EINA_SAFETY_ON_TRUE_GOTO(err != 0, error);
        pd->recv.done += rw_slice.len;

        if (pd->recv.done != pd->recv.needed)
          continue;

        if (pd->recv.needed == fh_len)
          {
             Efl_Net_Dialer_Websocket_Frame_Header fh;
             memcpy(&fh, pd->recv.tmpbuf, pd->recv.needed);

             pd->recv.current.opcode = fh.opcode;
             pd->recv.current.fin = fh.fin;

             if (fh._reserved)
               {
                  WRN("dialer=%p server sent reserved bits %#x, opcode=%#x, but no extension was negotiated.",
                      o, fh._reserved, fh.opcode);
                  _efl_net_dialer_websocket_close_protocol_error(o, "reserved bits not negotiated");
               }

             if (fh.mask)
               {
                  WRN("dialer=%p server masked frame opcode=%#x.", o, fh.opcode);
                  _efl_net_dialer_websocket_close_protocol_error(o, "server sent masked frame");
               }

             if (fh.payload_len == 127)
               {
                  if (_efl_net_dialer_websocket_opcode_control_check(fh.opcode))
                    {
                       WRN("dialer=%p server sent forbidden 64-bit control frame %#x.", o, fh.opcode);
                       _efl_net_dialer_websocket_close_protocol_error(o, "control frames are limited to 125 bytes");
                    }
                  pd->recv.needed += sizeof(uint64_t);
                  continue;
               }
             else if (fh.payload_len == 126)
               {
                  if (_efl_net_dialer_websocket_opcode_control_check(fh.opcode))
                    {
                       WRN("dialer=%p server sent forbidden 16-bit control frame %#x.", o, fh.opcode);
                       _efl_net_dialer_websocket_close_protocol_error(o, "control frames are limited to 125 bytes");
                    }
                  pd->recv.needed += sizeof(uint16_t);
                  continue;
               }
             else
               frame_len = fh.payload_len;
          }
        else if (pd->recv.needed == fh_len + sizeof(uint64_t))
          {
             uint64_t len;

             memcpy(&len, pd->recv.tmpbuf + fh_len, sizeof(len));
#ifndef WORDS_BIGENDIAN
             len = eina_swap64(len);
#endif
             frame_len = len;
          }
        else if (pd->recv.needed == fh_len + sizeof(uint16_t))
          {
             uint16_t len;

             memcpy(&len, pd->recv.tmpbuf + fh_len, sizeof(len));
#ifndef WORDS_BIGENDIAN
             len = eina_swap16(len);
#endif
             frame_len = len;
          }
        else
          {
             CRI("shouldn't reach done=%u, needed=%u", pd->recv.done, pd->recv.needed);
             frame_len = 0;
          }

        if (pd->recv.current.opcode == EFL_NET_DIALER_WEBSOCKET_OPCODE_CONTINUATION)
          {
             if (pd->recv.fragmented.opcode == 0)
               {
                  WRN("dialer=%p server sent continuation frame after non-fragmentable frame", o);
                  _efl_net_dialer_websocket_close_protocol_error(o, "nothing to continue");
               }
             if (pd->recv.current.payload)
               free(pd->recv.current.payload);
             pd->recv.current.payload = pd->recv.fragmented.payload;
             pd->recv.current.used_len = pd->recv.fragmented.used_len;
             pd->recv.current.total_len = pd->recv.fragmented.total_len;
             pd->recv.fragmented.payload = NULL;
             pd->recv.fragmented.used_len = 0;
             pd->recv.fragmented.total_len = 0;
          }
        else if ((!_efl_net_dialer_websocket_opcode_control_check(pd->recv.current.opcode)) &&
                 (pd->recv.fragmented.opcode != 0))
          {
             WRN("dialer=%p server sent non-control frame opcode=%#x while continuation was expected", o, pd->recv.current.opcode);
             _efl_net_dialer_websocket_close_protocol_error(o, "expected continuation or control frames");
          }

        if (frame_len > 0)
          {
             void *tmp = realloc(pd->recv.current.payload, pd->recv.current.total_len + frame_len + 1);
             err = errno;
             EINA_SAFETY_ON_NULL_GOTO(tmp, error);
             pd->recv.current.payload = tmp;
             pd->recv.current.total_len += frame_len;
          }
     }

   if ((!efl_io_reader_can_read_get(pd->http)) &&
       (pd->recv.done < pd->recv.needed))
     return; /* more header to do */

   while ((pd->recv.current.used_len < pd->recv.current.total_len) &&
          (efl_io_reader_can_read_get(pd->http)))
     {
        Eina_Rw_Slice rw_slice = {
          .bytes = pd->recv.current.payload + pd->recv.current.used_len,
          .len = pd->recv.current.total_len - pd->recv.current.used_len,
        };

        err = efl_io_reader_read(pd->http, &rw_slice);
        EINA_SAFETY_ON_TRUE_GOTO(err != 0, error);
        pd->recv.current.used_len += rw_slice.len;
     }
   if (pd->recv.current.total_len > 0)
     pd->recv.current.payload[pd->recv.current.used_len] = '\0';

   if ((!efl_io_reader_can_read_get(pd->http)) &&
       (pd->recv.current.used_len < pd->recv.current.total_len))
     return; /* more payload to do */

   _efl_net_dialer_websocket_job_dispatch_frame(o, pd);
   pd->recv.done = 0;
   pd->recv.needed = sizeof(Efl_Net_Dialer_Websocket_Frame_Header);
   /* no need to free payload here, let realloc() reuse it */
   pd->recv.current.used_len = 0;
   pd->recv.current.total_len = 0;
   return;

 error:
   ERR("could not receive data header=%u/%u, payload=%" PRIu64 "/%" PRIu64 ": %s",
       pd->recv.done, pd->recv.needed,
       pd->recv.current.used_len, pd->recv.current.total_len,
       eina_error_msg_get(err));
   efl_ref(o);
   efl_io_closer_close(pd->http);
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
   efl_unref(o);
}

static Eina_Value
_efl_net_dialer_websocket_job(Eo *o, const Eina_Value v)
{
   Efl_Net_Dialer_Websocket_Data *pd = efl_data_scope_get(o, MY_CLASS);

   efl_ref(o);

   if (efl_io_reader_can_read_get(pd->http))
     _efl_net_dialer_websocket_job_receive(o, pd);

   if (efl_io_writer_can_write_get(pd->http) && pd->send.pending)
     _efl_net_dialer_websocket_job_send(o, pd);

   if (efl_io_reader_can_read_get(pd->http))
     _efl_net_dialer_websocket_job_schedule(o, pd);

   efl_unref(o);
   return v;
}

static void
_efl_net_dialer_websocket_job_schedule(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   Eo *loop;

   if (pd->job) return;

   loop = efl_loop_get(o);
   if (!loop) return;

   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_job(loop),
                                  .success = _efl_net_dialer_websocket_job,
                                  .storage = &pd->job);
}

static void
_efl_net_dialer_websocket_http_can_read_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Dialer_Websocket_Data *pd = efl_data_scope_get(o, MY_CLASS);
   if (efl_io_reader_can_read_get(pd->http))
     _efl_net_dialer_websocket_job_schedule(o, pd);
}

static void
_efl_net_dialer_websocket_http_can_write_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Dialer_Websocket_Data *pd = efl_data_scope_get(o, MY_CLASS);
   if (efl_io_writer_can_write_get(pd->http))
     _efl_net_dialer_websocket_job_schedule(o, pd);
}

static void
_efl_net_dialer_websocket_http_closed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Dialer_Websocket_Data *pd = efl_data_scope_get(o, MY_CLASS);
   efl_ref(o);
   if (!pd->close_requested)
     {
        Efl_Net_Dialer_Websocket_Closed_Reason reason = {
          .reason = EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_ABRUPTLY,
          .message = "",
        };
        pd->close_requested = EINA_TRUE;
        efl_event_callback_call(o, EFL_NET_DIALER_WEBSOCKET_EVENT_CLOSED_REASON, &reason);
     }
   if ((!pd->recv.pending_read) &&
       (pd->streaming_mode != EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_DISABLED))
     efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
   efl_unref(o);
}

static void
_efl_net_dialer_websocket_http_error(void *data, const Efl_Event *event)
{
   Eo *o = data;
   Efl_Net_Dialer_Websocket_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Error *perr = event->info;
   if ((pd->close_requested) && (*perr == EFL_NET_HTTP_ERROR_RECV_ERROR))
     return;
   efl_ref(o);
   if (!efl_io_closer_closed_get(o)) efl_io_closer_close(o);
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, perr);
   efl_unref(o);
}

static void
_efl_net_dialer_websocket_http_headers_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Dialer_Websocket_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Bool accepted = EINA_FALSE;
   Eina_Bool upgraded = EINA_FALSE;
   Eina_Bool connection_websocket = EINA_FALSE;
   Efl_Net_Http_Status status;
   const Efl_Net_Http_Header *header;
   Eina_Stringshare *str;
   Eina_Iterator *itr;

   status = efl_net_dialer_http_response_status_get(pd->http);
   if (status != EFL_NET_HTTP_STATUS_SWITCHING_PROTOCOLS)
     {
        Eina_Error err = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        if ((status >= 300) && (status < 400))
          return;
        WRN("failed WebSocket handshake: HTTP status=%d, expected=%d",
            status, EFL_NET_HTTP_STATUS_SWITCHING_PROTOCOLS);
        efl_ref(o);
        efl_io_closer_close(pd->http);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
        efl_unref(o);
        return;
     }

   EINA_LIST_FREE(pd->protocols.received, str)
     eina_stringshare_del(str);

   itr = efl_net_dialer_http_response_headers_get(pd->http);
   EINA_ITERATOR_FOREACH(itr, header)
     {
        if (!header->key) continue;
        if (strcasecmp(header->key, "Connection") == 0)
          upgraded = strcasecmp(header->value, "upgrade") == 0;
        else if (strcasecmp(header->key, "Upgrade") == 0)
          connection_websocket = strcasecmp(header->value, "websocket") == 0;
        else if (strcasecmp(header->key, "Sec-WebSocket-Accept") == 0)
          accepted = strcmp(header->value, pd->accept_key) == 0; /* case matters */
        else if (strcasecmp(header->key, "Sec-WebSocket-Protocol") == 0)
          {
             char **strv = eina_str_split(header->value, ",", 0);
             if (strv)
               {
                  int i;
                  for (i = 0; strv[i] != NULL; i++)
                    pd->protocols.received = eina_list_append(pd->protocols.received, eina_stringshare_add(strv[i]));
                  free(strv[0]);
                  free(strv);
               }
          }
     }
   eina_iterator_free(itr);
   efl_net_dialer_http_response_headers_clear(pd->http);

   if ((!upgraded) || (!connection_websocket) || (!accepted))
     {
        Eina_Error err = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        WRN("failed WebSocket handshake: upgraded=%d, connection_websocket=%d, accepted=%d",
            upgraded, connection_websocket, accepted);
        efl_ref(o);
        efl_io_closer_close(pd->http);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
        efl_unref(o);
        return;
     }

   str = efl_net_socket_address_remote_get(pd->http);
   if (str)
     {
        char *tmp = NULL;
        /* if ws:// or wss:// were used, rewrite-back */
        if (strncasecmp(pd->address_dial, "ws://", strlen("ws://")) == 0)
          {
             tmp = malloc(strlen(str) + strlen("ws://") - strlen("http://") + 1);
             if (tmp)
               {
                  memcpy(tmp, "ws://", strlen("ws://"));
                  memcpy(tmp + strlen("ws://"),
                         str + strlen("http://"),
                         strlen(str) - strlen("http://") + 1);
                  str = tmp;
               }
          }
        else if (strncasecmp(pd->address_dial, "wss://", strlen("wss://")) == 0)
          {
             tmp = malloc(strlen(str) + strlen("wss://") - strlen("https://") + 1);
             if (tmp)
               {
                  memcpy(tmp, "wss://", strlen("wss://"));
                  memcpy(tmp + strlen("wss://"),
                         str + strlen("https://"),
                         strlen(str) - strlen("https://") + 1);
                  str = tmp;
               }
          }

        efl_net_socket_address_remote_set(o, str);
        free(tmp);
     }

   efl_net_dialer_connected_set(o, EINA_TRUE);
   efl_io_writer_can_write_set(o, EINA_TRUE);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_net_dialer_websocket_http_cbs,
                           {EFL_IO_READER_EVENT_CAN_READ_CHANGED, _efl_net_dialer_websocket_http_can_read_changed},
                           {EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, _efl_net_dialer_websocket_http_can_write_changed},
                           {EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_dialer_websocket_http_closed},
                           {EFL_NET_DIALER_EVENT_ERROR, _efl_net_dialer_websocket_http_error},
                           {EFL_NET_DIALER_HTTP_EVENT_HEADERS_DONE, _efl_net_dialer_websocket_http_headers_done});

EOLIAN static Efl_Object *
_efl_net_dialer_websocket_efl_object_constructor(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   pd->recv.needed = sizeof(Efl_Net_Dialer_Websocket_Frame_Header);

   o = efl_constructor(efl_super(o, MY_CLASS));
   EINA_SAFETY_ON_NULL_RETURN_VAL(o, NULL);

   pd->http = efl_add(EFL_NET_DIALER_HTTP_CLASS, o,
                      efl_net_dialer_http_version_set(efl_added, EFL_NET_HTTP_VERSION_V1_1),
                      efl_net_dialer_http_method_set(efl_added, "GET"),
                      efl_net_dialer_http_primary_mode_set(efl_added, EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD),
                      efl_event_callback_array_add(efl_added, _efl_net_dialer_websocket_http_cbs(), o));
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->http, NULL);

   return o;
}

EOLIAN static void
_efl_net_dialer_websocket_efl_object_destructor(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   Eina_Stringshare *str;

   efl_event_callback_array_del(pd->http, _efl_net_dialer_websocket_http_cbs(), o);

   efl_del(pd->http);
   pd->http = NULL;

   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_dial, NULL);
   eina_stringshare_replace(&pd->address_remote, NULL);
   EINA_LIST_FREE(pd->protocols.requested, str)
     eina_stringshare_del(str);
   EINA_LIST_FREE(pd->protocols.received, str)
     eina_stringshare_del(str);

   free(pd->recv.current.payload);
   pd->recv.current.payload = NULL;
   free(pd->recv.fragmented.payload);
   pd->recv.fragmented.payload = NULL;

   while (pd->send.pending)
     _efl_net_dialer_websocket_send_pending_remove(pd);
}

static void
_efl_net_dialer_websocket_protocols_add(Efl_Net_Dialer_Websocket_Data *pd)
{
   const Eina_List *lst;
   Eina_Stringshare *str;
   char *protocols;
   size_t len = 0;
   size_t offset = 0;

   EINA_LIST_FOREACH(pd->protocols.requested, lst, str)
     {
        if (len) len += strlen(", ");
        len += eina_stringshare_strlen(str);
     }

   if (len == 0) return;
   protocols = malloc(len + 1);
   EINA_SAFETY_ON_NULL_RETURN(protocols);
   EINA_LIST_FOREACH(pd->protocols.requested, lst, str)
     {
        if (offset)
          {
             memcpy(protocols + offset, ", ", strlen(", "));
             offset += strlen(", ");
          }
        memcpy(protocols + offset, str, eina_stringshare_strlen(str));
        offset += eina_stringshare_strlen(str);
     }
   protocols[offset] = '\0';

   efl_net_dialer_http_request_header_add(pd->http, "Sec-WebSocket-Protocol", protocols);
   free(protocols);
}

static void
_efl_net_dialer_websocket_key_add(Efl_Net_Dialer_Websocket_Data *pd)
{
   Eina_Binbuf *binbuf_key;
   Eina_Strbuf *strbuf_key_base64;
   uint8_t key[16];
   const Eina_Slice guid_slice = EINA_SLICE_STR_LITERAL("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
   Eina_Slice ro_slice;
   Eina_Rw_Slice rw_slice;
   uint8_t sha1hash[20];
   Eina_Bool ret;
   size_t i;

    for (i = 0; i < sizeof(key); i++)
      key[i] = rand() & 0xff;

    binbuf_key = eina_binbuf_manage_new(key, sizeof(key), EINA_TRUE);
    EINA_SAFETY_ON_NULL_RETURN(binbuf_key);

    strbuf_key_base64 = emile_base64_encode(binbuf_key);
    eina_binbuf_free(binbuf_key);

    EINA_SAFETY_ON_NULL_RETURN(strbuf_key_base64);

    efl_net_dialer_http_request_header_add(pd->http, "Sec-WebSocket-Key", eina_strbuf_string_get(strbuf_key_base64));

    /* accept_key = base64(sha1(base64(random(16)) + guid)) */
    /* 1) base64(random(16)) + guid */
    eina_strbuf_append_slice(strbuf_key_base64, guid_slice);

    /* emile_binbuf_sha1() operates on binbuf, convert */
    ro_slice = eina_strbuf_slice_get(strbuf_key_base64);
    binbuf_key = eina_binbuf_manage_new(ro_slice.mem, ro_slice.len, EINA_TRUE);
    EINA_SAFETY_ON_NULL_GOTO(binbuf_key, free_strbuf);
    /* 2) sha1(base64(random(16)) + guid) */
    ret = emile_binbuf_sha1(binbuf_key, sha1hash);
    eina_binbuf_free(binbuf_key);
    eina_strbuf_free(strbuf_key_base64);

    EINA_SAFETY_ON_FALSE_RETURN(ret);

    /* 3) base64(sha1(base64(random(16)) + guid)) */
    binbuf_key = eina_binbuf_manage_new(sha1hash, sizeof(sha1hash), EINA_TRUE);
    EINA_SAFETY_ON_NULL_RETURN(binbuf_key);

    strbuf_key_base64 = emile_base64_encode(binbuf_key);
    eina_binbuf_free(binbuf_key);
    EINA_SAFETY_ON_NULL_RETURN(strbuf_key_base64);

    ro_slice = eina_strbuf_slice_get(strbuf_key_base64);

    rw_slice.mem = pd->accept_key;
    rw_slice.len = sizeof(pd->accept_key) - 1;
    rw_slice = eina_rw_slice_copy(rw_slice, ro_slice);
    rw_slice.bytes[rw_slice.len] = '\0';
    eina_strbuf_free(strbuf_key_base64);

    EINA_SAFETY_ON_TRUE_RETURN(rw_slice.len < sizeof(pd->accept_key) - 1);
    return;

 free_strbuf:
    eina_strbuf_free(strbuf_key_base64);
}

static void
_efl_net_dialer_websocket_request_headers_websocket_add(Efl_Net_Dialer_Websocket_Data *pd)
{
   efl_net_dialer_http_request_header_add(pd->http, "Upgrade", "websocket");
   efl_net_dialer_http_request_header_add(pd->http, "Connection", "Upgrade");
   efl_net_dialer_http_request_header_add(pd->http, "Expect", "101");
   efl_net_dialer_http_request_header_add(pd->http, "Transfer-Encoding", "");
   efl_net_dialer_http_request_header_add(pd->http, "Sec-WebSocket-Version", "13");

   _efl_net_dialer_websocket_protocols_add(pd);
   _efl_net_dialer_websocket_key_add(pd);
}

EOLIAN static Eina_Error
_efl_net_dialer_websocket_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, const char *address)
{
   const char *http_url = address;
   char *tmp = NULL;
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

   if (_c_init())
     {
        curl_version_info_data *cver = _c->curl_version_info(CURLVERSION_FOURTH);
        static Eina_Bool did_once = EINA_FALSE;
        /*
         * CURL 7.50.2 fixed bug
         * https://github.com/curl/curl/pull/899 that prevented
         * WebSocket with initial small frames (ie: 2 bytes) to
         * be processed, this was clear with autobahn test suite
         * Case 1.1.1
         *
         * commit 7bda07b0466a192e082f32d363d1b1ce1881d483
         * Author: Michael Kaufmann <mail@michael-kaufmann.ch>
         * Date:   Tue Jun 28 10:57:30 2016 +0200
         *
         *     HTTP: stop parsing headers when switching to unknown protocols
         *
         *     - unknown protocols probably won't send more headers (e.g. WebSocket)
         *     - improved comments and moved them to the correct case statements
         *
         *     Closes #899
         */
        if ((!did_once) && (cver) && (cver->version_num < 0x073202))
          {
             WRN("Your version of CURL='%s' is too old. >=7.50.2 is required for WebSockets to work properly", cver->version);
             did_once = EINA_TRUE;
          }
     }

   efl_net_dialer_address_dial_set(o, address);

   /* rewrite ws:// -> http://, wss:// -> https:// */
   if (strncasecmp(address, "ws://", strlen("ws://")) == 0) {
      tmp = malloc(strlen(address) + strlen("http://") - strlen("ws://") + 1);
      EINA_SAFETY_ON_NULL_RETURN_VAL(tmp, ENOMEM);
      memcpy(tmp, "http://", strlen("http://"));
      memcpy(tmp + strlen("http://"),
             address + strlen("ws://"),
             strlen(address) - strlen("ws://") + 1);
      http_url = tmp;
   } else if (strncasecmp(address, "wss://", strlen("wss://")) == 0) {
      tmp = malloc(strlen(address) + strlen("https://") - strlen("wss://") + 1);
      EINA_SAFETY_ON_NULL_RETURN_VAL(tmp, ENOMEM);
      memcpy(tmp, "https://", strlen("https://"));
      memcpy(tmp + strlen("https://"),
             address + strlen("wss://"),
             strlen(address) - strlen("wss://") + 1);
      http_url = tmp;
   }

   _efl_net_dialer_websocket_request_headers_websocket_add(pd);
   err = efl_net_dialer_dial(pd->http, http_url);
   free(tmp);
   return err;
}

EOLIAN static void
_efl_net_dialer_websocket_efl_net_dialer_address_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_dial, address);
}

EOLIAN static const char *
_efl_net_dialer_websocket_efl_net_dialer_address_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return pd->address_dial;
}

EOLIAN static void
_efl_net_dialer_websocket_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Eina_Bool connected)
{
   if (pd->connected == connected) return;
   pd->connected = connected;
   if (connected)
     efl_event_callback_call(o, EFL_NET_DIALER_EVENT_CONNECTED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_net_dialer_connected_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return pd->connected;
}

EOLIAN static void
_efl_net_dialer_websocket_efl_net_dialer_proxy_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *proxy_url)
{
   efl_net_dialer_proxy_set(pd->http, proxy_url);
}

EOLIAN static const char *
_efl_net_dialer_websocket_efl_net_dialer_proxy_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_net_dialer_proxy_get(pd->http);
}

EOLIAN static void
_efl_net_dialer_websocket_efl_net_dialer_timeout_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, double seconds)
{
   efl_net_dialer_timeout_dial_set(pd->http, seconds);
}

EOLIAN static double
_efl_net_dialer_websocket_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_net_dialer_timeout_dial_get(pd->http);
}

EOLIAN static const char *
_efl_net_dialer_websocket_efl_net_socket_address_local_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_net_socket_address_local_get(pd->http);
}

EOLIAN static void
_efl_net_dialer_websocket_efl_net_socket_address_remote_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *address)
{
   if (eina_stringshare_replace(&pd->address_remote, address))
     efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
}

EOLIAN static const char *
_efl_net_dialer_websocket_efl_net_socket_address_remote_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return pd->address_remote;
}

static void
_efl_net_dialer_websocket_recv_pending_read_remove(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   Efl_Net_Dialer_Websocket_Pending_Read *pr = pd->recv.pending_read;
   Eina_Inlist *lst = EINA_INLIST_GET(pr);

   lst = eina_inlist_remove(lst, lst);
   pd->recv.pending_read = EINA_INLIST_CONTAINER_GET(lst, Efl_Net_Dialer_Websocket_Pending_Read);

   free(pr->bytes);
   free(pr);

   pd->recv.pending_read_offset = 0;
   efl_io_reader_can_read_set(o, !!pd->recv.pending_read);
}

EOLIAN static Eina_Error
_efl_net_dialer_websocket_efl_io_reader_read(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Eina_Rw_Slice *rw_slice)
{
   size_t todo;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);

   if (!pd->recv.pending_read) return EAGAIN;

   todo = rw_slice->len;
   while ((pd->recv.pending_read) && (todo > 0))
     {
        Eina_Slice src = {
          .bytes = pd->recv.pending_read->bytes + pd->recv.pending_read_offset,
          .len = pd->recv.pending_read->len - pd->recv.pending_read_offset,
        };
        Eina_Rw_Slice dst = {
          .bytes = rw_slice->bytes + rw_slice->len - todo,
          .len = todo,
        };
        dst = eina_rw_slice_copy(dst, src);
        todo -= dst.len;
        pd->recv.pending_read_offset += dst.len;
        if (pd->recv.pending_read_offset == pd->recv.pending_read->len)
          _efl_net_dialer_websocket_recv_pending_read_remove(o, pd);
     }

   rw_slice->len -= todo;

   if ((pd->close_requested) && (!pd->recv.pending_read))
     efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);

   return 0;
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_io_reader_can_read_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return pd->can_read;
}

EOLIAN static void
_efl_net_dialer_websocket_efl_io_reader_can_read_set(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Eina_Bool can_read)
{
   if (pd->streaming_mode == EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_DISABLED) return;
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_io_reader_eos_get(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   return !pd->can_read && efl_io_closer_closed_get(o);
}

EOLIAN static Eina_Error
_efl_net_dialer_websocket_efl_io_writer_write(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   Efl_Net_Dialer_Websocket_Opcode opcode;

   EINA_SAFETY_ON_NULL_RETURN_VAL(slice, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->streaming_mode == EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_DISABLED, EINVAL);

   if (pd->streaming_mode == EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_TEXT)
     opcode = EFL_NET_DIALER_WEBSOCKET_OPCODE_TEXT;
   else
     opcode = EFL_NET_DIALER_WEBSOCKET_OPCODE_BINARY;

   if (remaining)
     {
        remaining->mem = NULL;
        remaining->len = 0;
     }

   _efl_net_dialer_websocket_send(o, pd, opcode, slice->mem, slice->len);
   return 0;
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_io_writer_can_write_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_net_dialer_websocket_efl_io_writer_can_write_set(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Eina_Bool can_write)
{
   if (pd->streaming_mode == EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_DISABLED) return;
   if (can_write && efl_io_closer_closed_get(o))
     can_write = EINA_FALSE;
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

EOLIAN static Eina_Error
_efl_net_dialer_websocket_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Websocket_Data *pd)
{
   if (pd->close_requested) return 0;
   efl_net_dialer_websocket_close_request(o, EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_NORMAL, NULL);
   return 0;
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_io_closer_closed_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return pd->close_requested || efl_io_closer_closed_get(pd->http);
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_io_closer_close_on_exec_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, Eina_Bool close_on_exec)
{
   return efl_io_closer_close_on_exec_set(pd->http, close_on_exec);
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_io_closer_close_on_exec_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_io_closer_close_on_exec_get(pd->http);
}

EOLIAN static void
_efl_net_dialer_websocket_efl_io_closer_close_on_destructor_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, Eina_Bool close_on_destructor)
{
   efl_io_closer_close_on_destructor_set(pd->http, close_on_destructor);
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_efl_io_closer_close_on_destructor_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_io_closer_close_on_destructor_get(pd->http);
}

EOLIAN static void
_efl_net_dialer_websocket_streaming_mode_set(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Efl_Net_Dialer_Websocket_Streaming_Mode streaming_mode)
{
   if (streaming_mode == EFL_NET_DIALER_WEBSOCKET_STREAMING_MODE_DISABLED)
     {
        while (pd->recv.pending_read)
          _efl_net_dialer_websocket_recv_pending_read_remove(o, pd);
     }

   pd->streaming_mode = streaming_mode;
}

EOLIAN static Efl_Net_Dialer_Websocket_Streaming_Mode
_efl_net_dialer_websocket_streaming_mode_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return pd->streaming_mode;
}

EOLIAN static void
_efl_net_dialer_websocket_user_agent_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *user_agent)
{
   efl_net_dialer_http_user_agent_set(pd->http, user_agent);
}

EOLIAN static const char *
_efl_net_dialer_websocket_user_agent_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_net_dialer_http_user_agent_get(pd->http);
}

EOLIAN static void
_efl_net_dialer_websocket_authentication_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *username, const char *password, Efl_Net_Http_Authentication_Method method, Eina_Bool restricted)
{
   efl_net_dialer_http_authentication_set(pd->http, username, password, method, restricted);
}

EOLIAN static void
_efl_net_dialer_websocket_authentication_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char **username, const char **password, Efl_Net_Http_Authentication_Method *method, Eina_Bool *restricted)
{
   efl_net_dialer_http_authentication_get(pd->http, username, password, method, restricted);
}

EOLIAN static void
_efl_net_dialer_websocket_allow_redirects_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, Eina_Bool allow_redirects)
{
   efl_net_dialer_http_allow_redirects_set(pd->http, allow_redirects);
}

EOLIAN static Eina_Bool
_efl_net_dialer_websocket_allow_redirects_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_net_dialer_http_allow_redirects_get(pd->http);
}

EOLIAN static void
_efl_net_dialer_websocket_cookie_jar_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *path)
{
   efl_net_dialer_http_cookie_jar_set(pd->http, path);
}

EOLIAN static const char *
_efl_net_dialer_websocket_cookie_jar_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return efl_net_dialer_http_cookie_jar_get(pd->http);
}

EOLIAN static void
_efl_net_dialer_websocket_ping(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, const char *reason)
{
   size_t len;
   EINA_SAFETY_ON_TRUE_RETURN(pd->close_requested);
   if (!reason) reason = "";

   len = strlen(reason);
   if (len > 125)
     {
        WRN("reason is over 125 bytes! chopped '%s'", reason);
        len = 125;
     }
   _efl_net_dialer_websocket_send(o, pd, EFL_NET_DIALER_WEBSOCKET_OPCODE_PING,
                                  reason, len);
}

EOLIAN static void
_efl_net_dialer_websocket_text_send(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, const char *text)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->close_requested);
   if (!text) text = "";
   _efl_net_dialer_websocket_send(o, pd, EFL_NET_DIALER_WEBSOCKET_OPCODE_TEXT,
                                  text, strlen(text));
}

EOLIAN static void
_efl_net_dialer_websocket_binary_send(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, const Eina_Slice blob)
{
   EINA_SAFETY_ON_TRUE_RETURN(pd->close_requested);
   _efl_net_dialer_websocket_send(o, pd, EFL_NET_DIALER_WEBSOCKET_OPCODE_BINARY,
                                  blob.mem, blob.len);
}

static Eina_Value
_efl_net_dialer_websocket_close_request_timeout(Eo *o, const Eina_Value v)
{
   DBG("server did not close the TCP socket, timeout");
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
   return v;
}

EOLIAN static void
_efl_net_dialer_websocket_close_request(Eo *o, Efl_Net_Dialer_Websocket_Data *pd, Efl_Net_Dialer_Websocket_Close_Reason reason, const char *message)
{
   Efl_Net_Dialer_Websocket_Frame *f;
   uint16_t r = reason;
   size_t len;
   EINA_SAFETY_ON_TRUE_RETURN(pd->close_requested);

   if (pd->close_timeout)
     eina_future_cancel(pd->close_timeout);

   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_timeout(efl_loop_get(o), 2.0),
                                  .success = _efl_net_dialer_websocket_close_request_timeout,
                                  .storage = &pd->close_timeout);

   efl_io_writer_can_write_set(o, EINA_FALSE);

   if ((!reason) || (reason == EFL_NET_DIALER_WEBSOCKET_CLOSE_REASON_NO_REASON))
     {
        f = _efl_net_dialer_websocket_send_pending_add(o, pd, EFL_NET_DIALER_WEBSOCKET_OPCODE_CLOSE, 0);
        EINA_SAFETY_ON_NULL_RETURN(f);
        pd->close_requested = EINA_TRUE;
        return;
     }

   if (!message) message = "";

#ifndef WORDS_BIGENDIAN
   r = eina_swap16(r);
#endif

   len = strlen(message);
   if (len > 123) /* 2 for code */
     {
        WRN("message is over 123 bytes! chopped '%s'", message);
        len = 123;
     }

   f = _efl_net_dialer_websocket_send_pending_add(o, pd, EFL_NET_DIALER_WEBSOCKET_OPCODE_CLOSE, sizeof(r) + len);
   EINA_SAFETY_ON_NULL_RETURN(f);

   _efl_net_dialer_websocket_frame_write(f, 0, &r, sizeof(r));
   _efl_net_dialer_websocket_frame_write(f, sizeof(r), message, len);
   pd->close_requested = EINA_TRUE;
}

typedef struct _Efl_Net_Dialer_Websocket_Blacklist_Header {
   const char *header;
   size_t len;
} Efl_Net_Dialer_Websocket_Blacklist_Header;

static const Efl_Net_Dialer_Websocket_Blacklist_Header _efl_net_dialer_websocket_blacklisted_headers[] = {
#define _M(x) {x, sizeof(x) - 1}
  _M("Upgrade"),
  _M("Connection"),
  _M("Expect"),
  _M("Transfer-Encoding"),
  _M("Sec-WebSocket-Key"),
  _M("Sec-WebSocket-Protocol"),
  _M("Sec-WebSocket-Version"),
#undef _M
  {NULL, 0}
};

static Eina_Bool
_efl_net_dialer_websocket_blacklisted_header_check(const char *key)
{
   const Efl_Net_Dialer_Websocket_Blacklist_Header *bh;
   size_t len;

   len = strlen(key);
   for (bh = _efl_net_dialer_websocket_blacklisted_headers;
        bh->header != NULL;
        bh++)
     {
        if ((bh->len == len) && (memcmp(bh->header, key, len) == 0))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}


EOLIAN static void
_efl_net_dialer_websocket_request_header_add(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *key, const char *value)
{
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(value);

   if (_efl_net_dialer_websocket_blacklisted_header_check(key))
     {
        ERR("HTTP header conflicts with WebSocket: %s: %s", key, value);
        return;
     }

   efl_net_dialer_http_request_header_add(pd->http, key, value);
}

EOLIAN static void
_efl_net_dialer_websocket_request_headers_clear(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{

   efl_net_dialer_http_request_headers_clear(pd->http);
   _efl_net_dialer_websocket_request_headers_websocket_add(pd);
}

typedef struct _Eina_Iterator_Filtered_Header
{
   Eina_Iterator iterator;
   Eina_Iterator *full;
} Eina_Iterator_Filtered_Header;

static Eina_Bool
eina_iterator_filtered_header_next(Eina_Iterator_Filtered_Header *it, void **data)
{
   Efl_Net_Http_Header *header;
   EINA_ITERATOR_FOREACH(it->full, header)
     {
        if (!header->key) continue;
        if (!_efl_net_dialer_websocket_blacklisted_header_check(header->key))
          {
             *data = header;
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static const struct filtered *
eina_iterator_filtered_header_get_container(Eina_Iterator_Filtered_Header *it)
{
   return eina_iterator_container_get(it->full);
}

static void
eina_iterator_filtered_header_free(Eina_Iterator_Filtered_Header *it)
{
   eina_iterator_free(it->full);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_net_dialer_websocket_request_headers_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   Eina_Iterator_Filtered_Header *it;

   it = calloc(1, sizeof(Eina_Iterator_Filtered_Header));
   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   it->full = efl_net_dialer_http_request_headers_get(pd->http);
   EINA_SAFETY_ON_NULL_GOTO(it->full, error);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(eina_iterator_filtered_header_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(eina_iterator_filtered_header_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_iterator_filtered_header_free);

   return &it->iterator;

 error:
   free(it);
   return NULL;
}

EOLIAN static void
_efl_net_dialer_websocket_request_protocol_add(Eo *o  EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd, const char *protocol)
{
   EINA_SAFETY_ON_NULL_RETURN(protocol);
   protocol = eina_stringshare_add(protocol);
   pd->protocols.requested = eina_list_append(pd->protocols.requested, protocol);
}

EOLIAN static Eina_Iterator *
_efl_net_dialer_websocket_request_protocols_get(Eo *o  EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return eina_list_iterator_new(pd->protocols.requested);
}

EOLIAN static void
_efl_net_dialer_websocket_request_protocols_clear(Eo *o  EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   Eina_Stringshare *str;
   EINA_LIST_FREE(pd->protocols.requested, str)
     eina_stringshare_del(str);
}

EOLIAN static Eina_Iterator *
_efl_net_dialer_websocket_response_protocols_get(Eo *o  EINA_UNUSED, Efl_Net_Dialer_Websocket_Data *pd)
{
   return eina_list_iterator_new(pd->protocols.received);
}

#include "efl_net_dialer_websocket.eo.c"
