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
   @short SSH packets management
   @internal
*/

#ifndef ASSH_PACKET_H_
#define ASSH_PACKET_H_

#include "assh.h"
#include "assh_queue.h"

#include <string.h>

/** @internal SSH protocol packet header */
struct assh_packet_head_s
{
  uint8_t   pck_len[4];
  uint8_t   pad_len;
  uint8_t   msg;
  uint8_t   end[0];
};

/** @internal SSH packet object */
struct assh_packet_s
{
  union {
    /** Packet queue entry, valid when packet is allocated. */
    struct assh_queue_entry_s entry;
    /** Spare packet pool entry, valid when packet has been released. */
    struct assh_packet_s *pool_next;
  };

  /** Associated assh context */
  struct assh_context_s *ctx;

  /** Size of the allocated packet data buffer. */
  uint_fast32_t alloc_size;
  /** Amount of valid packet data. This value is increased when adding
      data to the packet and is used when the packet is finally sent out. */
  uint_fast32_t data_size;

  /** Number of references to this packet. */
  uint_fast16_t ref_count;

  union {
    uint8_t                   data[0];
    struct assh_packet_head_s head;
  };
};

ASSH_FIRST_FIELD_ASSERT(assh_packet_s, entry);

/** @internal @This specifies the standard values for ssh message ids. */
enum assh_ssh_msg_e
{
  SSH_MSG_INVALID                   =   0,

  /** @multiple SSH transport related messages */
  SSH_MSG_DISCONNECT                =   1,
  SSH_MSG_IGNORE                    =   2,
  SSH_MSG_UNIMPLEMENTED             =   3,
  SSH_MSG_DEBUG                     =   4,
  SSH_MSG_SERVICE_REQUEST           =   5,
  SSH_MSG_SERVICE_ACCEPT            =   6,
  SSH_MSG_KEXINIT                   =  20,
  SSH_MSG_NEWKEYS                   =  21,

  /** @multiple SSH key-exchange related messages */
  SSH_MSG_KEX_DH_REQUEST            =  30,
  SSH_MSG_KEX_DH_REPLY              =  31,
  SSH_MSG_KEX_ECDH_INIT             =  30,
  SSH_MSG_KEX_ECDH_REPLY            =  31,

  SSH_MSG_KEX_DH_GEX_REQUEST_OLD    =  30,
  SSH_MSG_KEX_DH_GEX_GROUP          =  31,
  SSH_MSG_KEX_DH_GEX_INIT           =  32,
  SSH_MSG_KEX_DH_GEX_REPLY          =  33,
  SSH_MSG_KEX_DH_GEX_REQUEST        =  34,

  SSH_MSG_KEXRSA_PUBKEY             =  30,
  SSH_MSG_KEXRSA_SECRET             =  31,
  SSH_MSG_KEXRSA_DONE               =  32,

  /** @multiple SSH user authentication related messages */
  SSH_MSG_USERAUTH_REQUEST          =  50,
  SSH_MSG_USERAUTH_FAILURE          =  51,
  SSH_MSG_USERAUTH_SUCCESS          =  52,
  SSH_MSG_USERAUTH_BANNER           =  53,
  SSH_MSG_USERAUTH_PK_OK            =  60,
  SSH_MSG_USERAUTH_PASSWD_CHANGEREQ =  60,

  /** @multiple SSH connection service related messages */
  SSH_MSG_GLOBAL_REQUEST            =  80,
  SSH_MSG_REQUEST_SUCCESS           =  81,
  SSH_MSG_REQUEST_FAILURE           =  82,
  SSH_MSG_CHANNEL_OPEN              =  90,
  SSH_MSG_CHANNEL_OPEN_CONFIRMATION =  91,
  SSH_MSG_CHANNEL_OPEN_FAILURE      =  92,
  SSH_MSG_CHANNEL_WINDOW_ADJUST     =  93,
  SSH_MSG_CHANNEL_DATA              =  94,
  SSH_MSG_CHANNEL_EXTENDED_DATA     =  95,
  SSH_MSG_CHANNEL_EOF               =  96,
  SSH_MSG_CHANNEL_CLOSE             =  97,
  SSH_MSG_CHANNEL_REQUEST           =  98,
  SSH_MSG_CHANNEL_SUCCESS           =  99,
  SSH_MSG_CHANNEL_FAILURE           = 100,
};

/** @internal @This specifies standard ssh disconnect reasons. */
enum assh_ssh_disconnect_e
{
  SSH_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT    =  1,
  SSH_DISCONNECT_PROTOCOL_ERROR                 =  2,
  SSH_DISCONNECT_KEY_EXCHANGE_FAILED            =  3,
  SSH_DISCONNECT_RESERVED                       =  4,
  SSH_DISCONNECT_MAC_ERROR                      =  5,
  SSH_DISCONNECT_COMPRESSION_ERROR              =  6,
  SSH_DISCONNECT_SERVICE_NOT_AVAILABLE          =  7,
  SSH_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED =  8,
  SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE        =  9,
  SSH_DISCONNECT_CONNECTION_LOST                = 10,
  SSH_DISCONNECT_BY_APPLICATION                 = 11,
  SSH_DISCONNECT_TOO_MANY_CONNECTIONS           = 12,
  SSH_DISCONNECT_AUTH_CANCELLED_BY_USER         = 13,
  SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE = 14,
  SSH_DISCONNECT_ILLEGAL_USER_NAME              = 15,
};

/** @internal @This allocates a new packet. The @tt alloc_size
    parameter specifies total allocated size. No range checking is
    performed on the size parameter. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_alloc2(struct assh_context_s *c,
                  uint8_t msg, size_t alloc_size,
                  struct assh_packet_s **p);

/** @internal @This allocates a new packet. The @tt
    payload_size parameter specifies the amount of bytes needed
    between the message id byte and the mac bytes. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_alloc(struct assh_context_s *c,
                  uint8_t msg, size_t payload_size,
                  struct assh_packet_s **result);

/** @internal @This decreases the reference counter of the
    packet and release the packet if the new counter value is
    zero. */
void assh_packet_release(struct assh_packet_s *p);

/** @internal @This increase the reference counter of the packet. */
ASSH_INLINE struct assh_packet_s *
assh_packet_refinc(struct assh_packet_s *p)
{
  p->ref_count++;
  return p;
}

/** @internal @This creates a copy of a packet. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_dup(struct assh_packet_s *p,
                struct assh_packet_s **copy);

/** @internal @This allocates an array of bytes in a packet
    and returns a pointer to the array. If there is not enough space
    left in the packet, an error is returned. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_add_array(struct assh_packet_s *p, size_t len, uint8_t **result);

/** @internal @This allocates an unsigned 32 bits integer in a packet
    and sets its value. If there is not enough space left in the
    packet, an error is returned. */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_add_u32(struct assh_packet_s *p, uint32_t value)
{
  uint8_t *be;
  assh_error_t err = assh_packet_add_array(p, 4, &be);
  if (!err)
    assh_store_u32(be, value);
  return err;
}

/** @internal @This allocates a string in a packet and returns
    a pointer to the first char of the string. If there is not enough
    space left in the packet, and error is returned. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_add_string(struct assh_packet_s *p, size_t len, uint8_t **result);

/** @internal @This enlarges a string previously allocated in
    a packet and returns a pointer to the first additional char of the
    string. If there is not enough space left in the packet, an error
    is returned. The string must be the last allocated thing in the
    packet when this function is called. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_enlarge_string(struct assh_packet_s *p, uint8_t *str,
                           size_t len, uint8_t **result);

/** @internal @This reduces the size of a string previously
    allocated in a packet. The string must be the last allocated thing
    in the packet when this function is called. */
void
assh_packet_shrink_string(struct assh_packet_s *p, uint8_t *str,
                          size_t new_len);

/** @internal @This update the size of the packet using the
    size header of the string. The string must be the last allocated
    thing in the packet when this function is called. */
void assh_packet_string_resized(struct assh_packet_s *p, uint8_t *str);

/** @internal @This allocates a string in a packet and writes
    the given big number in mpint representation as string content. A
    storage size large enough to store the number may be passed to the
    function or 0 if it is not known. */
assh_error_t ASSH_WARN_UNUSED_RESULT
assh_packet_add_mpint(struct assh_context_s *ctx,
                      struct assh_packet_s *p,
                      const struct assh_bignum_s *bn);

/** @internal @This checks that an array is well inside a
    buffer. If no error is returned, the @tt next parameter is set to
    point to the first byte following the array in the buffer. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_check_array(const uint8_t *buffer, size_t buffer_len,
                 const uint8_t *array, size_t array_len, uint8_t **next);

/** @internal @This checks that a string is well inside a
    buffer. If no error is returned, the @tt next parameter is set to
    point to the first byte following the string in the buffer. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_check_string(const uint8_t *buffer, size_t buffer_len,
                  const uint8_t *str, uint8_t **next);

/** @internal @This checks that an asn1 DER value is well inside a
    buffer. If no error is returned, the @tt value parameter is set to
    point to the first byte of the value and the @tt next parameter is
    set to point to the first byte in the buffer following the
    value. Any of these two parameters may be @tt NULL. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_check_asn1(const uint8_t *buffer, size_t buffer_len, const uint8_t *str,
                uint8_t **value, uint8_t **next);

/** @internal @This checks that a string is well inside packet
    bounds. @see assh_check_string */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_check_string(const struct assh_packet_s *p, const uint8_t *str,
                         uint8_t **next)
{
  return assh_check_string(p->data, p->data_size, str, next);
}

/** @internal @This checks that an array is well inside packet
    bounds. @see assh_check_array */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_check_array(const struct assh_packet_s *p, const uint8_t *array,
                        size_t array_len, uint8_t **next)
{
  return assh_check_array(p->data, p->data_size, array, array_len, next);
}

/** @internal @This checks that a 32 bits integer is well
    inside packet bounds and converts the value from network byte
    order. @see assh_packet_check_array */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_check_u32(struct assh_packet_s *p, uint32_t *u32,
		      const uint8_t *data, uint8_t **next)
{
  assh_error_t err = assh_packet_check_array(p, data, 4, next);
  if (!err)
    *u32 = assh_load_u32(data);
  return err;
}

/** @internal @This compare two buffers of byte of the same length in
    constant time. */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT assh_bool_t
assh_memcmp(const uint8_t *nula, const uint8_t *nulb, size_t len)
{
  assh_bool_t r = 0;
  while (len--)
    r |= nula[len] ^ nulb[len];
  return r;  
}

/** @internal @This compares a ssh string with a size header to a @tt
    NUL terminated string. No bound checking is performed. */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT int
assh_ssh_string_compare(const uint8_t *ssh_str, const char *nul_str)
{
  size_t l = assh_load_u32(ssh_str);
  return strncmp((const char*)ssh_str + 4, nul_str, l) || nul_str[l] != '\0';
}

/** @internal @This copies a ssh string to a nul terminated
    string. An error is returned if the size of the buffer is not
    large enough to store the string along with its nul terminating
    byte. */
ASSH_WARN_UNUSED_RESULT assh_error_t
assh_ssh_string_copy(const uint8_t *ssh_str, char *nul_str, size_t max_len);

/** @internal @This compares the content of an @ref assh_buffer_s
    object with a nul terminated string. */
ASSH_INLINE ASSH_WARN_UNUSED_RESULT uint_fast8_t
assh_buffer_strcmp(const struct assh_buffer_s *buf, const char *nul_str)
{
  uint_fast16_t i;
  for (i = 0; i < buf->len; i++)
    if (!nul_str[i] || buf->str[i] != nul_str[i])
      return 1;
  return nul_str[i];
}

#endif

