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

#include <assh/assh_bignum.h>
#include <assh/assh_packet.h>

#include <stdlib.h>

size_t assh_bignum_size_of_bits(enum assh_bignum_fmt_e fmt, size_t bits)
{
  assh_error_t err;
  size_t l, n = ASSH_ALIGN8(bits) / 8; /* bytes size */

  switch (fmt)
    {
    case ASSH_BIGNUM_NATIVE:
    case ASSH_BIGNUM_STEMP:
    case ASSH_BIGNUM_TEMP:
      l = sizeof(struct assh_bignum_s);
      break;
    case ASSH_BIGNUM_MPINT:
      l = /* size header */ 4 + /* 00 sign */ 1 + /* data */ n;
      break;
    case ASSH_BIGNUM_STRING:
      l = /* size header */ 4 + /* data */ n;
      break;
    case ASSH_BIGNUM_MSB_RAW:
    case ASSH_BIGNUM_LSB_RAW:
      l = n;
      break;
    case ASSH_BIGNUM_ASN1:
      l = n < (1 << 7)  ? n + 2 :
          n < (1 << 8)  ? n + 3 :
          n < (1 << 16) ? n + 4 :
          n < (1 << 24) ? n + 5 : n + 6;
      break;
    case ASSH_BIGNUM_HEX:
      l = n * 2;
      break;
    case ASSH_BIGNUM_INT:
      ASSH_CHK_RET(bits >= sizeof(intptr_t) * 8, ASSH_ERR_INPUT_OVERFLOW);
      l = sizeof(intptr_t);
      break;
    default:
      abort();
    }

  return l;
}

static void assh_asn1_size(const uint8_t *asn1, size_t *head_len, size_t *val_len)
{
  size_t n = 2, l = asn1[1];
  asn1 += 2;
  if (l & 0x80)  /* long length form ? */
    {
      size_t ll = l & 0x7f;
      for (l = 0; ll > 0; ll--, asn1++, n++)
        l = (l << 8) | *asn1;
    }
  if (head_len != NULL)
    *head_len = n;
  if (val_len != NULL)
    *val_len = l;
}

/* clz on hex digit */
static inline uint8_t assh_hex_clz(char c)
{
  /* perfect hash on hex chars in "7f08192a3b4c5d6e" order */
  uint32_t x = ((0x20DB6DC0 * (uint32_t)(c | 32)) >> 27);
  /* shift based lookup table */
  return (0x11122331 >> (x & 30)) & 3;
}

ASSH_WARN_UNUSED_RESULT assh_error_t
assh_bignum_size_of_data(enum assh_bignum_fmt_e fmt,
                         const void *data, size_t *size,
                         size_t *val_size, size_t *bits)
{
  assh_error_t err;
  size_t l, n, b;

  switch (fmt)
    {
    case ASSH_BIGNUM_NATIVE:
    case ASSH_BIGNUM_STEMP:
    case ASSH_BIGNUM_TEMP: {
      const struct assh_bignum_s *bn = data;
      l = sizeof(struct assh_bignum_s);
      b = bn->bits;
      n = ASSH_ALIGN8(b) / 8;
      break;
    }

    case ASSH_BIGNUM_MPINT: {
      const uint8_t *mpint = data;
      n = assh_load_u32(mpint);
      l = n + 4;
      if (n > 0)
        {
          ASSH_CHK_RET(mpint[4] & 0x80, ASSH_ERR_NUM_OVERFLOW);
          if (mpint[4] == 0)
            {
              mpint++, n--;
              ASSH_CHK_RET(n == 0, ASSH_ERR_BAD_DATA);
              ASSH_CHK_RET((mpint[4] & 0x80) == 0, ASSH_ERR_BAD_DATA);
            }
        }
      ASSH_CHK_RET(n > 0 && mpint[4] == 0, ASSH_ERR_BAD_DATA);
      b = n * 8;
      if (n)
        b -= ASSH_CLZ8(mpint[4]);
      break;
    }

    case ASSH_BIGNUM_STRING: {
      const uint8_t *str = data;
      n = assh_load_u32(str);
      l = n + 4;
      while (n > 0 && str[0] == 0)
        str++, n--;
      b = n * 8;
      if (n)
        b -= ASSH_CLZ8(str[0]);
      break;
    }

    case ASSH_BIGNUM_ASN1: {
      const uint8_t *asn1 = data;
      ASSH_CHK_RET(*asn1 != 0x02, ASSH_ERR_BAD_DATA);
      assh_asn1_size(asn1, &l, &n);
#warning reject negative numbers as in mpint?
      asn1 += l;
      l += n;
      while (n > 0 && asn1[0] == 0)
        asn1++, n--;
      b = n * 8;
      if (n)
        b -= ASSH_CLZ8(asn1[0]);
      break;
    }

    case ASSH_BIGNUM_LSB_RAW: {
      assert(size != NULL);
      const uint8_t *raw = data;
      n = l = *size;
      while (n > 0 && raw[n - 1] == 0)
        n--;
      b = n * 8;
      if (b)
        b -= ASSH_CLZ8(raw[n - 1]);
      break;
    }

    case ASSH_BIGNUM_MSB_RAW: {
      assert(size != NULL);
      const uint8_t *raw = data;
      n = l = *size;
      while (n > 0 && raw[0] == 0)
        raw++, n--;
      b = n * 8;
      if (b)
        b -= ASSH_CLZ8(raw[0]);
      break;
    }

    case ASSH_BIGNUM_HEX: {
      const char *hex = data;
      l = strlen(hex);
      n = l;
      while (n > 0 && hex[0] == '0')
        hex++, n--;
      b = n * 4;
      if (n)
        b -= assh_hex_clz(hex[0]);
      break;
    }

    case ASSH_BIGNUM_INT: {
      n = l = sizeof(intptr_t);
      b = l * 8;
      break;
    }

    case ASSH_BIGNUM_SIZE:
      b = (intptr_t)data;
      n = l = ASSH_ALIGN8(b) / 8;
      break;
    default:
      ASSH_ERR_RET(ASSH_ERR_BAD_ARG);
    }

  if (size != NULL)
    *size = l;

  if (val_size != NULL)
    *val_size = n;

  if (bits != NULL)
    *bits = b;

  return ASSH_OK;
}

