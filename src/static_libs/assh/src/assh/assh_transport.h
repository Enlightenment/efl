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

/**
   @file
   @short SSH transport layer protocol (rfc4253)
*/

#ifndef ASSH_TRANSPORT_H_
#define ASSH_TRANSPORT_H_

#ifdef ASSH_EVENT_H_
# warning The assh/assh_event.h header should be included after assh_transport.h
#endif

#include "assh.h"

/** @internal @This specifies the transport status of an ssh session. */
enum assh_transport_state_e
{
  /** send a @ref SSH_MSG_KEXINIT packet then go to @ref ASSH_TR_KEX_WAIT */
  ASSH_TR_KEX_INIT,
  /** We wait for a @ref SSH_MSG_KEXINIT packet. */
  ASSH_TR_KEX_WAIT,
  /** Both @ref SSH_MSG_KEXINIT packet were sent, the key exchange is taking place. */
  ASSH_TR_KEX_RUNNING,
  /** The key exchange is over and a @ref SSH_MSG_NEWKEYS packet is expected. */
  ASSH_TR_NEWKEY,
  /** No key exchange is running, service packets are allowed. */
  ASSH_TR_SERVICE,
  /** Key re-exchange packet sent but not received, service packets are allowed. */
  ASSH_TR_SERVICE_KEX,
  /** Do not exchange packets with the remote side anymore. Report last events. */
  ASSH_TR_FIN,
  /** Session closed, no more event will be reported. */
  ASSH_TR_CLOSED,
};

/** @internal @This specifies state of the input stream parser */
enum assh_stream_in_state_e
{
  ASSH_TR_IN_IDENT,
  ASSH_TR_IN_IDENT_DONE,
  ASSH_TR_IN_HEAD,
  ASSH_TR_IN_HEAD_DONE,
  ASSH_TR_IN_PAYLOAD,
  ASSH_TR_IN_PAYLOAD_DONE,
};

/** @internal @This specifies state of the output stream generator */
enum assh_stream_out_state_e
{
  ASSH_TR_OUT_IDENT,
  ASSH_TR_OUT_IDENT_PAUSE,
  ASSH_TR_OUT_IDENT_DONE,
  ASSH_TR_OUT_PACKETS,
  ASSH_TR_OUT_PACKETS_ENCIPHERED,
  ASSH_TR_OUT_PACKETS_PAUSE,
  ASSH_TR_OUT_PACKETS_DONE,
};

/** The @ref ASSH_EVENT_READ event is reported in order to gather
    incoming ssh stream data from the remote host.

    The @ref buf field have to be filled with incoming data
    stream. The @ref assh_event_done function must be called once the
    data have been copied to the buffer and the @ref transferred field
    have been set to the amount of available data.

    If not enough data is available, it's ok to provide less than
    requested or even no data. The buffer will be provided again the
    next time this event is reported.
*/
struct assh_event_transport_read_s
{
  ASSH_EV_CONST struct assh_buffer_s buf;         //< input
  size_t                             transferred; //< output
};

/** The @ref ASSH_EVENT_WRITE event is reported when some ssh stream
    data is available for sending to the remote host. The @ref buf
    field provides a buffer which contain the output data. The @ref
    transferred field must be set to the amount of data sent. The @ref
    assh_event_done function can then be called once the output data
    have been sent so that the packet buffer is released.

    It's ok to set the @ref transferred field to a value less than the
    buffer size. If no data at all can be sent, the default value of
    the field can be left untouched. The buffer will remain valid and
    will be provided again the next time this event is returned.
*/
struct assh_event_transport_write_s
{
  ASSH_EV_CONST struct assh_buffer_s buf;         //< input
  size_t                             transferred; //< output
};

/** @internal */
union assh_event_transport_u
{
  struct assh_event_transport_read_s  read;
  struct assh_event_transport_write_s write;
};

/** @internal This function puts a packet in the output queue. The
    packet will be released once it has been enciphered and sent. */
void assh_transport_push(struct assh_session_s *s,
			 struct assh_packet_s *p);

/** @internal This function executes the transport output FSM code
    which enciphers packets and builds the output stream. It may
    report the @ref ASSH_EVENT_READ event. It is called from the @ref
    assh_event_get function. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_transport_write(struct assh_session_s *s,
                     struct assh_event_s *e);

/** @internal This function executes the transport input FSM code
    which extracts packets from the stream and decipher them. It may
    report the @ref ASSH_EVENT_WRITE event. It is called from the @ref
    assh_event_get function. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_transport_read(struct assh_session_s *s,
                    struct assh_event_s *e);

/** @internal This function dispatches incoming packets depending on
    packet message id and transport layer state. It is called from the
    @ref assh_event_get function. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_transport_dispatch(struct assh_session_s *s,
			struct assh_event_s *e);

#endif

