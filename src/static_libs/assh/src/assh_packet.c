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

#include <assh/assh_packet.h>
#include <assh/assh_bignum.h>
#include <assh/assh_queue.h>
#include <assh/assh_session.h>
#include <assh/assh_alloc.h>

#include <assert.h>
#include <string.h>

/* This function returns the index of the bucket associated to a given
   packet size in the allocator pool. */
static inline struct assh_packet_pool_s *
assh_packet_pool(struct assh_context_s *c, uint32_t size)
{
  int i = sizeof(int) * 8 - ASSH_CLZ32(size) - ASSH_PCK_POOL_MIN;
  if (i < 0)
    i = 0;
  else if (i >= ASSH_PCK_POOL_SIZE)
    i = ASSH_PCK_POOL_SIZE - 1;
  return c->pool + i;
}

assh_error_t
assh_packet_alloc(struct assh_context_s *c,
                  uint8_t msg, size_t payload_size,
                  struct assh_packet_s **result)
{
  assh_error_t err; 

  ASSH_CHK_RET(payload_size > ASSH_MAX_PCK_PAYLOAD_SIZE, ASSH_ERR_OUTPUT_OVERFLOW);

  size_t size = /* pck_len */ 4 + /* pad_len */ 1 + /* msg */ 1 + payload_size +
          /* mac */ ASSH_MAX_MAC_SIZE + /* padding */ (4 + ASSH_MAX_BLOCK_SIZE - 1);

  ASSH_ERR_RET(assh_packet_alloc2(c, msg, size, result));
  return ASSH_OK;
}

assh_error_t
assh_packet_alloc2(struct assh_context_s *c,
                   uint8_t msg, size_t size,
                   struct assh_packet_s **result)
{
  struct assh_packet_s *p, **r;
  assh_error_t err;
  struct assh_packet_pool_s *pl = assh_packet_pool(c, size);

  /* get from pool */
  for (r = &pl->pck; (p = *r) != NULL; r = &(*r)->pool_next)
    {
      if (p->alloc_size >= size)
	{
	  *r = p->pool_next;
          pl->size -= p->alloc_size;
          pl->count--;
	  break;
	}
    }

  /* fallback to alloc */
  if (p == NULL)
    {
      ASSH_ERR_RET(assh_alloc(c, sizeof(*p) + size, ASSH_ALLOC_PACKET, (void*)&p));
      p->alloc_size = size;
    }

  /* init */
  p->ref_count = 1;
  p->ctx = c;
  p->data_size = /* pck_len */ 4 + /* pad_len */ 1 + /* msg */ 1;
  memset(p->data, 0, p->alloc_size);
  p->head.msg = msg;

  *result = p;
  return ASSH_OK;
}

void assh_packet_release(struct assh_packet_s *p)
{
  if (p == NULL || --p->ref_count > 0)
    return;

  assert(p->ref_count == 0);

  struct assh_context_s *c = p->ctx;
  struct assh_packet_pool_s *pl = assh_packet_pool(c, p->alloc_size);

  if (pl->size + p->alloc_size >= c->pck_pool_max_bsize ||
      c->pck_pool_size + p->alloc_size >= c->pck_pool_max_size)
    {
      assh_free(c, p);
    }
  else
    {
      p->pool_next = pl->pck;
      pl->pck = p;
      pl->count++;
      pl->size += p->alloc_size;
    }
}

ASSH_WARN_UNUSED_RESULT assh_error_t
assh_packet_dup(struct assh_packet_s *p, struct assh_packet_s **copy)
{
  assh_error_t err;

  ASSH_ERR_RET(assh_packet_alloc2(p->ctx, 0, p->alloc_size, copy));
  struct assh_packet_s *r = *copy;

  memcpy(r->data, p->data, p->data_size);
  r->data_size = p->data_size;

  return ASSH_OK;
}

assh_error_t assh_packet_add_mpint(struct assh_context_s *ctx,
                                   struct assh_packet_s *p,
                                   const struct assh_bignum_s *bn)
{
  assh_error_t err;
  size_t l = assh_bignum_size_of_num(ASSH_BIGNUM_MPINT, bn);

  uint8_t *s;
  ASSH_ERR_RET(assh_packet_add_array(p, l, &s));

  ASSH_ERR_RET(assh_bignum_convert(ctx,
    ASSH_BIGNUM_NATIVE, ASSH_BIGNUM_MPINT, bn, s));

  p->data_size -= l - assh_load_u32(s) - 4;
  return ASSH_OK;
}

assh_error_t
assh_check_asn1(const uint8_t *buffer, size_t buffer_len, const uint8_t *str,
                uint8_t **value, uint8_t **next)
{
  assh_error_t err;

  const uint8_t *e = buffer + buffer_len;
  ASSH_CHK_RET(str < buffer || str > e - 2, ASSH_ERR_INPUT_OVERFLOW);

  str++; /* discard type identifer */
  unsigned int l = *str++;
  if (l & 0x80)  /* long length form ? */
    {
      unsigned int ll = l & 0x7f;
      ASSH_CHK_RET(e - str < ll, ASSH_ERR_INPUT_OVERFLOW);
      for (l = 0; ll > 0; ll--)
        l = (l << 8) | *str++;
    }
  ASSH_CHK_RET(e - str < l, ASSH_ERR_INPUT_OVERFLOW);
  if (value != NULL)
    *value = (uint8_t*)str;
  if (next != NULL)
    *next = (uint8_t*)str + l;
  return ASSH_OK;
}

assh_error_t
assh_check_string(const uint8_t *buffer, size_t buffer_len,
                  const uint8_t *str, uint8_t **next)
{
  assh_error_t err;

  const uint8_t *e = buffer + buffer_len;
  ASSH_CHK_RET(str < buffer || str > e - 4, ASSH_ERR_INPUT_OVERFLOW);
  uint32_t s = assh_load_u32(str);
  ASSH_CHK_RET(e - 4 - str < s, ASSH_ERR_INPUT_OVERFLOW);
  if (next != NULL)
    *next = (uint8_t*)str + 4 + s;
  return ASSH_OK;
}

assh_error_t
assh_check_array(const uint8_t *buffer, size_t buffer_len,
                 const uint8_t *array, size_t array_len, uint8_t **next)
{
  assh_error_t err;

  const uint8_t *e = buffer + buffer_len;
  ASSH_CHK_RET(array < buffer || array > e, ASSH_ERR_INPUT_OVERFLOW);
  ASSH_CHK_RET(e - array < array_len, ASSH_ERR_INPUT_OVERFLOW);
  if (next != NULL)
    *next = (uint8_t*)array + array_len;
  return ASSH_OK;
}

assh_error_t
assh_packet_add_array(struct assh_packet_s *p, size_t len, uint8_t **result)
{
  assh_error_t err;

  ASSH_CHK_RET(p->data_size + len > p->alloc_size, ASSH_ERR_OUTPUT_OVERFLOW);
  uint8_t *d = p->data + p->data_size;
  p->data_size += len;
  *result = d;
  return ASSH_OK;
}

assh_error_t
assh_packet_add_string(struct assh_packet_s *p, size_t len, uint8_t **result)
{
  assh_error_t err;

  uint8_t *d;
  ASSH_ERR_RET(assh_packet_add_array(p, len + 4, &d));
  assh_store_u32(d, len);
  if (result != NULL)
    *result = d + 4;
  return ASSH_OK;
}

assh_error_t
assh_packet_enlarge_string(struct assh_packet_s *p, uint8_t *str,
                           size_t len, uint8_t **result)
{
  assh_error_t err;

  size_t olen = assh_load_u32(str - 4);
  assert(str + olen == p->data + p->data_size);
  ASSH_ERR_RET(assh_packet_add_array(p, len, result));
  assh_store_u32(str - 4, olen + len);
  return ASSH_OK;
}

void
assh_packet_shrink_string(struct assh_packet_s *p, uint8_t *str,
                          size_t new_len)
{
  size_t olen = assh_load_u32(str - 4);
  assert(str + olen == p->data + p->data_size);
  assert(olen >= new_len);
  assh_store_u32(str - 4, new_len);
  p->data_size -= olen - new_len;
}

void
assh_packet_string_resized(struct assh_packet_s *p, uint8_t *str)
{
  size_t len = assh_load_u32(str - 4);
  p->data_size = str - p->data + len;
}

assh_error_t
assh_ssh_string_copy(const uint8_t *ssh_str, char *nul_str, size_t max_len)
{
  assh_error_t err;

  size_t len = assh_load_u32(ssh_str);
  assert(max_len > 0);
  ASSH_CHK_RET(len > max_len - 1, ASSH_ERR_OUTPUT_OVERFLOW);
  memcpy(nul_str, ssh_str + 4, len);
  nul_str[len] = '\0';
  return ASSH_OK;
}
