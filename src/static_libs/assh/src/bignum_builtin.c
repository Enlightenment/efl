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
#include <assh/assh_context.h>
#include <assh/assh_packet.h>
#include <assh/assh_prng.h>
#include <assh/assh_alloc.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CONFIG_ASSH_BIGNUM_WORD
# define CONFIG_ASSH_BIGNUM_WORD 32
#endif

#if CONFIG_ASSH_BIGNUM_WORD == 8
/* 8 bits big number word is useful for testing bignum algorithms.
   Because there are fewer possible word values, it will test more
   corner cases in a short time. */
typedef uint8_t assh_bnword_t;
typedef int8_t assh_bnsword_t;
typedef uint16_t assh_bnlong_t;
typedef int16_t assh_bnslong_t;
#define ASSH_BN_FMT "%02x"
#define assh_bn_clz(x) ASSH_CLZ8((assh_bnword_t)x)
#define assh_bn_ctz(x) ASSH_CTZ8((assh_bnword_t)x)

#elif CONFIG_ASSH_BIGNUM_WORD == 16
typedef uint16_t assh_bnword_t;
typedef int16_t assh_bnsword_t;
typedef uint32_t assh_bnlong_t;
typedef int32_t assh_bnslong_t;
#define ASSH_BN_FMT "%04x"
#define assh_bn_clz(x) ASSH_CLZ16((assh_bnword_t)x)
#define assh_bn_ctz(x) ASSH_CTZ16((assh_bnword_t)x)

#elif CONFIG_ASSH_BIGNUM_WORD == 32
typedef uint32_t assh_bnword_t;
typedef int32_t assh_bnsword_t;
typedef uint64_t assh_bnlong_t;
typedef int64_t assh_bnslong_t;
#define ASSH_BN_FMT "%08x"
#define assh_bn_clz(x) ASSH_CLZ32((assh_bnword_t)x)
#define assh_bn_ctz(x) ASSH_CTZ32((assh_bnword_t)x)

#elif CONFIG_ASSH_BIGNUM_WORD == 64
typedef uint64_t assh_bnword_t;
typedef int64_t assh_bnsword_t;
typedef unsigned __int128 assh_bnlong_t;
typedef signed __int128 assh_bnslong_t;
#define ASSH_BN_FMT "%016llx"
#define assh_bn_clz(x) ASSH_CLZ64((assh_bnword_t)x)
#define assh_bn_ctz(x) ASSH_CTZ64((assh_bnword_t)x)
#define 

#endif

#define ASSH_BN_WORDMAX (assh_bnword_t)-1LL

/** Minimum number of words for karatsuba to switch to school mul. */
#define ASSH_BIGNUM_KARATSUBA_THRESHOLD 32

/** @This specifies the number of bits in a big number word. */
#define ASSH_BIGNUM_W (sizeof(assh_bnword_t) * 8)

struct assh_bignum_scratch_s
{
  size_t words;
  assh_bnword_t *n;
  assh_bool_t secur;
};

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_bignum_scratch_expand(struct assh_context_s *c,
                           struct assh_bignum_scratch_s *sc,
                           size_t words)
{
  assh_error_t err;

  if (sc->words < words)
    {
      ASSH_ERR_RET(assh_realloc(c, (void**)&sc->n,
        words * sizeof(assh_bnword_t), sc->secur
        ? ASSH_ALLOC_SECUR : ASSH_ALLOC_INTERNAL));
      sc->words = words;
    }

  return ASSH_OK;
}

struct assh_bignum_mt_s
{
  struct assh_bignum_s mod;
  assh_bnword_t n0;
};

ASSH_FIRST_FIELD_ASSERT(assh_bignum_mt_s, mod);

#ifdef CONFIG_ASSH_DEBUG
static void
assh_bignum_dump(assh_bnword_t *x, size_t l)
{
  size_t i;
  fprintf(stderr, "0x");  
  for (i = l; i-- > 0; )
    fprintf(stderr, ASSH_BN_FMT, x[i]);
  fprintf(stderr, "\n");  
}
#endif

static inline size_t
assh_bignum_words(size_t bits)
{
  return (((bits - 1) | (ASSH_BIGNUM_W - 1)) + 1) / ASSH_BIGNUM_W;
}

static void
assh_bignum_cswap(struct assh_bignum_s *a,
                  struct assh_bignum_s *b,
                  assh_bool_t c)
{
  size_t i, l = assh_bignum_words(a->bits);
  assh_bnword_t m = ~((assh_bnword_t)c - 1);
  assh_bnword_t *an = a->n;
  assh_bnword_t *bn = b->n;
  assert(a->bits == b->bits);

  for (i = 0; i < l; i++)
    {
      an[i] ^= bn[i] & m;
      bn[i] ^= an[i] & m;
      an[i] ^= bn[i] & m;
    }
}

/* This function copies a value between two big number objects. */
static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_copy(struct assh_bignum_s *dst,
                 const struct assh_bignum_s *src)
{
  assh_error_t err;
  size_t al = assh_bignum_words(dst->bits);
  size_t bl = assh_bignum_words(src->bits);
  size_t i, l = ASSH_MIN(al, bl);
  assh_bnword_t x = 0;
  assh_bnword_t *dn = dst->n, *sn = src->n;

  dst->secret = src->secret;

  for (i = 0; i < l; i++)
    x = dn[i] = sn[i];

  if (dst->bits % ASSH_BIGNUM_W)
    x >>= (dst->bits % ASSH_BIGNUM_W);
  else
    x = 0;

  for (; i < al; i++)
    dn[i] = 0;
  for (; i < bl; i++)
    x |= sn[i];

  ASSH_CHK_RET(dst->bits != src->bits && x != 0,
               ASSH_ERR_OUTPUT_OVERFLOW);

  return ASSH_OK;
}

/* This function stores the value of a big number into a buffer. The
   destination format can be: ASSH_BIGNUM_MPINT, ASSH_BIGNUM_STRING,
   ASSH_BIGNUM_LSB_RAW, ASSH_BIGNUM_MSB_RAW. The buffer must be large
   enough to hold the value. */
static void
assh_bignum_to_buffer(const struct assh_bignum_s *bn,
                      uint8_t * __restrict__ in,
                      enum assh_bignum_fmt_e format)
{
  size_t i, l = ASSH_ALIGN8(bn->bits) / 8;
  assh_bnword_t *n = bn->n;
  uint8_t *m = in;

  if (format == ASSH_BIGNUM_MPINT ||
      format == ASSH_BIGNUM_STRING)
    m += 4;

  uint8_t *p = m;

  if (format == ASSH_BIGNUM_LSB_RAW)
    for (i = 0; i < l; i++)
      *p++ = n[i / sizeof(assh_bnword_t)]
        >> ((i % sizeof(assh_bnword_t)) * 8);
  else
    for (i = l; i-- > 0; )
      {
        uint8_t b = n[i / sizeof(assh_bnword_t)]
          >> ((i % sizeof(assh_bnword_t)) * 8);
        if (p == m && format == ASSH_BIGNUM_MPINT)
          {
            if (!b)
              continue;
            if (b & 0x80)
              *p++ = 0;
          }
        *p++ = b;
      }

  if (in < m)
    assh_store_u32(in, p - m);
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_from_buffer(struct assh_bignum_s *bn,
                        const uint8_t * __restrict__ data,
                        size_t data_len, enum assh_bignum_fmt_e format)
{
  assh_error_t err;
  size_t i, j, l = assh_bignum_words(bn->bits);
  size_t k = 0;
  assh_bnword_t x, *n = bn->n;

  if (l > 0)
    {
      for (i = 0; i < l; i++)
        {
          x = 0;
          for (j = 0; j < ASSH_BIGNUM_W && k < data_len; j += 8)
            {
              size_t m = k++;
              if (format != ASSH_BIGNUM_LSB_RAW)
                m = data_len - m - 1;
              x |= (assh_bnword_t)data[m] << j;
            }
          n[i] = x;
        }

      if (bn->bits % ASSH_BIGNUM_W)
        {
          assh_bnword_t mask = ASSH_BN_WORDMAX
            >> ((ASSH_BIGNUM_W - bn->bits) & (ASSH_BIGNUM_W - 1));

          if (format == ASSH_BIGNUM_MSB_RAW || format == ASSH_BIGNUM_LSB_RAW)
            n[i - 1] &= mask;
          else
            ASSH_CHK_RET(x & ~mask, ASSH_ERR_OUTPUT_OVERFLOW);
        }
    }

  ASSH_CHK_RET(k < data_len, ASSH_ERR_OUTPUT_OVERFLOW);
  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_from_uint(struct assh_bignum_s *bn,
                      uintptr_t x)
{
  size_t i, l = assh_bignum_words(bn->bits);
  assh_bnword_t *n = bn->n;
  assh_error_t err;

  for (i = 0; i < l; i++)
    {
      n[i] = x;
      x = (ASSH_BIGNUM_W < sizeof(x) * 8) ? x >> ASSH_BIGNUM_W : 0;
    }

  ASSH_CHK_RET(x != 0, ASSH_ERR_NUM_OVERFLOW);
  return ASSH_OK;
}

static assh_bool_t assh_bignum_eq_uint(const assh_bnword_t a,
                                       const assh_bnword_t *b, size_t bl)
{
  size_t i;
  assh_bnword_t r = b[0] ^ a;

  for (i = 1; i < bl; i++)
    r |= b[i];

  return !r;
}

static assh_bool_t assh_bignum_eq(const assh_bnword_t *a, size_t al,
                                  const assh_bnword_t *b, size_t bl)
{
  size_t i;
  assh_bnword_t r = 0;

  for (i = 0; i < al && i < bl; i++)
    r |= a[i] ^ b[i];
  for (; i < al; i++)
    r |= a[i];
  for (; i < bl; i++)
    r |= b[i];

  return !r;
}

static int_fast8_t assh_bignum_cmp(const struct assh_bignum_s *a,
                                   const struct assh_bignum_s *b)
{
  size_t al = assh_bignum_words(a->bits);
  size_t bl = assh_bignum_words(b->bits);
  size_t i, l = ASSH_MIN(al, bl);
  assh_bnword_t *an = a->n, *bn = b->n;
  int_fast8_t lt = 0, gt = 0, eq;

#warning FIXME check constant time
  for (i = 0; i < l; i++)
    {
      eq = (an[i] == bn[i]);
      lt = (an[i] < bn[i]) | (lt & eq);
      gt = (an[i] > bn[i]) | (gt & eq);
    }
  for (; i < bl; i++)
    {
      eq = (0 == bn[i]);
      lt = (0 != bn[i]) | (lt & eq);
      gt = (gt & eq);
    }
  for (; i < al; i++)
    {
      eq = (an[i] == 0);
      lt = (lt & eq);
      gt = (an[i] != 0) | (gt & eq);
    }

  return gt - lt;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_rand(struct assh_context_s *c,
                 struct assh_bignum_s *bn,
                 const struct assh_bignum_s *min,
                 const struct assh_bignum_s *max,
                 enum assh_prng_quality_e quality)
{
  assh_error_t err;
  assh_bnword_t *n = bn->n;
  size_t i, l = assh_bignum_words(bn->bits);

  bn->secret |= quality != ASSH_PRNG_QUALITY_WEAK;

  if (l == 0)
    return ASSH_OK;

  ASSH_ERR_RET(c->prng->f_get(c, (uint8_t*)n,
                 l * sizeof(assh_bnword_t), quality));

  while (1)
    {
      if (bn->bits % ASSH_BIGNUM_W)
        n[l - 1] >>= (ASSH_BIGNUM_W - bn->bits) & (ASSH_BIGNUM_W - 1);

      if ((min == NULL || assh_bignum_cmp(bn, min) > 0) &&
          (max == NULL || assh_bignum_cmp(bn, max) < 0))
        break;

      for (i = 1; i < l; i++)
        n[l - 1] = n[l];

      ASSH_ERR_RET(c->prng->f_get(c, (uint8_t*)(n + l - 1),
                     sizeof(assh_bnword_t), quality));
    }

  return ASSH_OK;
}

/*********************************************************************** shift */

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_rshift(struct assh_bignum_s *dst,
                   const struct assh_bignum_s *src,
                   uint_fast16_t n)
{
  assert(src->bits == dst->bits);
  assert(n < src->bits);
  dst->secret = src->secret;

  size_t i, l = assh_bignum_words(src->bits);

  if (dst == src && n == 0)
    return ASSH_OK;

  assh_bnword_t *dn = dst->n, *sn = src->n;

  assh_bnword_t o = sn[n / ASSH_BIGNUM_W];
  for (i = 0; i < l - n / ASSH_BIGNUM_W - 1; i++)
    {
      assh_bnword_t x = sn[i + n / ASSH_BIGNUM_W + 1];
      dn[i] = ((assh_bnlong_t)o >> (n % ASSH_BIGNUM_W))
        | ((assh_bnlong_t)x << (ASSH_BIGNUM_W - n % ASSH_BIGNUM_W));
      o = x;
    }
  for (; i < l; i++)
    {
      dn[i] = ((assh_bnlong_t)o >> (n % ASSH_BIGNUM_W));
      o = 0;
    }

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_lshift(struct assh_bignum_s *dst,
                   const struct assh_bignum_s *src,
                   uint_fast16_t n)
{
  assert(src->bits == dst->bits);
  assert(n < src->bits);
  dst->secret = src->secret;

  ssize_t i, l = assh_bignum_words(src->bits);

  if (dst == src && n == 0)
    return ASSH_OK;

  assh_bnword_t *dn = dst->n, *sn = src->n;

  assh_bnword_t o = sn[l - 1 - n / ASSH_BIGNUM_W];
  for (i = 0; i < l - n / ASSH_BIGNUM_W - 1; i++)
    {
      assh_bnword_t x = sn[l - 2 - i - n / ASSH_BIGNUM_W];
      dn[l - 1 - i] = ((assh_bnlong_t)o << (n % ASSH_BIGNUM_W))
        | ((assh_bnlong_t)x >> (ASSH_BIGNUM_W - n % ASSH_BIGNUM_W));
      o = x;
    }
  for (; i < l; i++)
    {
      dn[l - 1 - i] = ((assh_bnlong_t)o << (n % ASSH_BIGNUM_W));
      o = 0;
    }

  return ASSH_OK;
}

/*********************************************************************** add, sub */

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_addsub(struct assh_bignum_s *dst,
                   const struct assh_bignum_s *a,
                   const struct assh_bignum_s *b,
                   const struct assh_bignum_s *mod,
                   assh_bnword_t smask /* 0:add, -1:sub */)
{
  assh_error_t err;
  assh_bnword_t amask = 0;

  dst->secret = a->secret | b->secret;

  /* make A the largest bits length */
  if (a->bits < b->bits)
    {
      ASSH_SWAP(a, b);
      amask ^= smask;
    }

  size_t dl = assh_bignum_words(dst->bits);
  size_t al = assh_bignum_words(a->bits);
  size_t bl = assh_bignum_words(b->bits);
  const assh_bnword_t *an = a->n;
  const assh_bnword_t *bn = b->n;
  assh_bnword_t *dn = dst->n;

  assert(dl >= al);

  assh_bnlong_t t = (assh_bnlong_t)(smask & 1) << ASSH_BIGNUM_W;
  assh_bnword_t bmask = amask ^ smask;
  size_t i;

  /* add/sub numbers */
  for (i = 0; i < bl; i++)
    dn[i] = t = (assh_bnlong_t)(an[i] ^ amask) + (bn[i] ^ bmask) + (t >> ASSH_BIGNUM_W);
  for (; i < al; i++)
    dn[i] = t = (assh_bnlong_t)(an[i] ^ amask) + bmask + (t >> ASSH_BIGNUM_W);
  for (; i < dl; i++)
    dn[i] = t = smask + (t >> ASSH_BIGNUM_W);

  t ^= (assh_bnlong_t)(smask & 1) << ASSH_BIGNUM_W;

  /* handle overflow condition */

  if (mod)
    {
      assert(dst->bits == mod->bits);
      assh_bnword_t q = ((t >> ASSH_BIGNUM_W) ^ 1) - 1;
      assh_bnword_t *m;

      if (mod->montgomery)
        {
          struct assh_bignum_mt_s *mt = (void*)mod;
          assh_bnword_t *r1 = (assh_bnword_t*)mt->mod.n + 2 * dl;
#warning FIXME add either r%n or n-r%n depending on compare of the first word?
          /* add/sub r%n on overflow */
          m = r1;
          /* switch between add/sub */
          smask = ~smask;
        }
      else
        {
          m = mod->n;
          if (a->bits < dst->bits)
            return ASSH_OK;
        }

      /* masked reduce */
      t = (assh_bnlong_t)(q & 1 & ~smask) << ASSH_BIGNUM_W;
      for (i = 0; i < dl; i++)
        dn[i] = t = (assh_bnlong_t)dn[i] + (q & (m[i] ^ (assh_bnword_t)~smask))
          + (t >> ASSH_BIGNUM_W);
    }
  else
    {
      size_t l = dst->bits % ASSH_BIGNUM_W;
      if (!l)
        l = ASSH_BIGNUM_W;
      ASSH_CHK_RET(t >> l != 0, ASSH_ERR_NUM_OVERFLOW);
    }

  return ASSH_OK;
}

/*********************************************************************** div, modinv */

static inline int assh_bignum_div_cmp(const assh_bnword_t *a, unsigned int alen,
				      const assh_bnword_t *b, unsigned int blen)
{
  if (alen != blen)
    return blen - alen;

  int i;
  for (i = blen - 1; i >= 0; i--)
    {
      if (a[i] < b[i])
	return 1;
      if (a[i] > b[i])
	return -1;
    }

  return 0;
}

/** reduce size to strip leading nul words */
static inline assh_bool_t
assh_bignum_div_strip(unsigned int *len, const assh_bnword_t *x)
{
  while (*len > 0 && x[*len - 1] == 0)
    (*len)--;
  return (*len == 0);
}

/** find number of leading zero bits and get a word full of msb significant bits */
static inline void
assh_bignum_div_clz(unsigned int len, const assh_bnword_t *x,
                    unsigned int *z, unsigned int *l, assh_bnword_t *t)
{
  *z = assh_bn_clz(x[len - 1]);
  *l = ASSH_BIGNUM_W - *z + (len - 1) * ASSH_BIGNUM_W;

  *t = (x[len - 1] << *z);
  if (len > 1)
    *t |= ((assh_bnlong_t)x[len - 2] >> (ASSH_BIGNUM_W - *z));  
}

/** find suitable factor and left shift amount for subtraction of the divisor */
static inline assh_bnword_t
assh_bignum_div_factor(assh_bnword_t at, assh_bnword_t bt,
                       unsigned int d, unsigned int *sa, unsigned int *da)
{
  assh_bnword_t bi = bt + 1;
  if (bi != 0)
    bt = bi;

  assh_bnword_t q = ((assh_bnlong_t)(at - 1) << (ASSH_BIGNUM_W - 1)) / bt;

  if (d < (ASSH_BIGNUM_W - 1))
    {
      q >>= (ASSH_BIGNUM_W - 1) - d;
      *da = *sa = 0;
    }
  else
    {
      *da = (d - (ASSH_BIGNUM_W - 1)) / ASSH_BIGNUM_W;
      *sa = (d - (ASSH_BIGNUM_W - 1)) % ASSH_BIGNUM_W;
    }

  return q ? q : 1;
}

/** compute r = r - (b << (sa + da * W)) * q */
static inline void
assh_bignum_div_update_r(unsigned int b_len, const assh_bnword_t * __restrict__ b,
                         unsigned int r_len, assh_bnword_t * __restrict__ r,
                         assh_bnword_t q, unsigned int sa, unsigned int da)
{
  assh_bnlong_t t = (assh_bnlong_t)1 << ASSH_BIGNUM_W;
  assh_bnlong_t m = 0;
  assh_bnword_t bo = 0;
  unsigned int i;

  assert(b_len + da <= r_len);

  for (i = 0; i < b_len; i++)
    {
      assh_bnword_t bi = b[i];
      m = (assh_bnword_t)((bi << sa) | ((assh_bnlong_t)bo >> (ASSH_BIGNUM_W - sa)))
	* (assh_bnlong_t)q + (m >> ASSH_BIGNUM_W);
      r[i + da] = t = (assh_bnlong_t)r[i + da]
	+ (assh_bnword_t)~m + (t >> ASSH_BIGNUM_W);
      bo = bi;
    }
  for (; i < r_len - da; i++)
    {
      m = ((assh_bnlong_t)bo >> (ASSH_BIGNUM_W - sa)) * q + (m >> ASSH_BIGNUM_W);
      r[i + da] = t = (assh_bnlong_t)r[i + da]
	+ (assh_bnword_t)~m + (t >> ASSH_BIGNUM_W);
      bo = 0;
    }
}

/** compute d = d + q << (sa + da * W) */
static inline void
assh_bignum_div_update_q(unsigned int d_len, assh_bnword_t * __restrict__ d,
                         assh_bnword_t q, unsigned int sa, unsigned int da)
{
  assh_bnlong_t t, carry = (assh_bnlong_t)q << sa;
  unsigned int i;
  for (i = da; carry != 0 && i < d_len; i++)
    {
      d[i] = t = (assh_bnlong_t)d[i] + carry;
      carry = t >> ASSH_BIGNUM_W;
    }
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_div_euclidean(assh_bnword_t * __restrict__ r,
                          unsigned int r_len,
                          assh_bnword_t * __restrict__ d,
                          unsigned int d_len,
                          const assh_bnword_t * __restrict__ b,
                          unsigned int b_len)
{
  assh_error_t err;
  unsigned int az, al, bz, bl, da, sa;
  assh_bnword_t at, bt, q;

  /* div by zero */
  ASSH_CHK_RET(assh_bignum_div_strip(&b_len, b), ASSH_ERR_NUM_OVERFLOW);

  assh_bignum_div_clz(b_len, b, &bz, &bl, &bt);

  while (1)
    {
      /* skip leading zero words */
      if (assh_bignum_div_strip(&r_len, r))
	break;

      assh_bignum_div_clz(r_len, r, &az, &al, &at);

      /* test for termination by compairing the remainder and the divisor */
      if (assh_bignum_div_cmp(r, r_len, b, b_len) > 0)
	break;

      /* find factor */
      q = assh_bignum_div_factor(at, bt, al - bl, &sa, &da);

      /* update the remainder */
      assh_bignum_div_update_r(b_len, b, r_len, r, q, sa, da);

      /* update the quotient */
      if (d != NULL)
	assh_bignum_div_update_q(d_len, d, q, sa, da);
    }

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_div(struct assh_context_s *ctx,
                struct assh_bignum_scratch_s *sc,
                struct assh_bignum_s *r,
                struct assh_bignum_s *d,
                const struct assh_bignum_s *a,
                const struct assh_bignum_s *b)
{
  assh_error_t err;

  assert(r != d && a != d && b != d && b != r);
  assert(!a->secret && !b->secret);
  assert(a->bits >= b->bits);
  assert(d == NULL || d->bits >= a->bits);

  size_t al = assh_bignum_words(a->bits);
  size_t bl = assh_bignum_words(b->bits);
  assh_bnword_t *dn = NULL, *rn;
  size_t rl;

  if (r != NULL)
    {
      r->secret = 0;
      if (r->bits >= a->bits)
        {
          rl = assh_bignum_words(r->bits);
          rn = r->n;
          goto done;
        }
    }

  rl = al;
  ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, rl));
  rn = sc->n;
 done:

  if (a->n != rn)
    {
      memcpy(rn, a->n, al * sizeof(assh_bnword_t));
      memset((assh_bnword_t*)rn + al, 0, (rl - al) * sizeof(assh_bnword_t));
    }

  size_t dl = 0;
  if (d != NULL)
    {
      d->secret = 0;
      dl = assh_bignum_words(d->bits);
      dn = d->n;
      memset(dn, 0, dl * sizeof(assh_bnword_t));
    }

  ASSH_ERR_RET(assh_bignum_div_euclidean(rn, rl, dn, dl, b->n, bl));

  if (rn == sc->n && r != NULL)
    memcpy(r->n, rn, assh_bignum_words(r->bits) * sizeof(assh_bnword_t));

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_modinv(struct assh_context_s *ctx,
                   struct assh_bignum_scratch_s *sc,
                   struct assh_bignum_s *u,
                   const struct assh_bignum_s *a,
                   const struct assh_bignum_s *m)
{
  assh_error_t err;

  assert(u != a && u != m);
  assert(!a->secret && !m->secret);
  assert(a->bits <= m->bits);
  assert(u->bits >= a->bits);

  u->secret = 0;

  size_t ul = assh_bignum_words(u->bits);
  size_t al = assh_bignum_words(a->bits);
  size_t ml = assh_bignum_words(m->bits);

  ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, ml * 3));

  assh_bnword_t * __restrict__ un = u->n;
  const assh_bnword_t * __restrict__ an = a->n;
  const assh_bnword_t * __restrict__ mn = m->n;

  assh_bnword_t *r = sc->n;
  assh_bnword_t *p = sc->n + ml;
  assh_bnword_t *v = sc->n + ml * 2;

  memcpy(r, mn, ml * sizeof(assh_bnword_t));
  memcpy(p, an, al * sizeof(assh_bnword_t));
  memset(p + al, 0, (ml - al) * sizeof(assh_bnword_t));

  memset(v, 0, ml * sizeof(assh_bnword_t));
  memset(un + 1, 0, (ul - 1) * sizeof(assh_bnword_t));
  un[0] = 1;

  unsigned int rl = ml, pl = ml;
  assh_bnword_t *xr = r, *xp = p, *xu = un, *xv = v;

  ASSH_CHK_RET(assh_bignum_div_strip(&rl, xr) ||
	       assh_bignum_div_strip(&pl, xp) ||
	       rl < pl, ASSH_ERR_NUM_OVERFLOW);

  while (1)
    {
      unsigned int az, as, bz, bs, da, sa;
      assh_bnword_t at, bt, q;

      /* find factor */
      assh_bignum_div_clz(rl, xr, &az, &as, &at);
      assh_bignum_div_clz(pl, xp, &bz, &bs, &bt);
      ASSH_CHK_RET(as < bs, ASSH_ERR_NUM_OVERFLOW);

      q = assh_bignum_div_factor(at, bt, as - bs, &sa, &da);

      assh_bignum_div_update_r(pl, xp, rl, xr, q, sa, da);

      /* skip leading zero words */
      if (assh_bignum_div_strip(&rl, xr))
	break;

      assh_bignum_div_update_r(ml - da, xu, ml, xv, q, sa, da);

      if (assh_bignum_div_cmp(xr, rl, xp, pl) > 0)
	{
	  ASSH_SWAP(rl, pl);
	  ASSH_SWAP(xr, xp);
	  ASSH_SWAP(xu, xv);
	}
    }

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_gcd(struct assh_context_s *ctx,
                struct assh_bignum_scratch_s *sc,
                struct assh_bignum_s *g,
                const struct assh_bignum_s *a,
                const struct assh_bignum_s *b)
{
  assh_error_t err;

  assert(g->bits >= a->bits || g->bits >= b->bits);
  assert(!a->secret && !b->secret);
  g->secret = 0;

  size_t al = assh_bignum_words(a->bits);
  size_t bl = assh_bignum_words(b->bits);

  size_t l = al > bl ? al : bl;

  ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, l));

  assh_bnword_t * __restrict__ gn = g->n;
  const assh_bnword_t * __restrict__ an = a->n;
  const assh_bnword_t * __restrict__ bn = b->n;

  assh_bnword_t *xr = sc->n;
  assh_bnword_t *xp = gn;

  /* use largest buffer between scratch and result for the largest
     input number, the gcd value will be available in both buffers
     at the end */
  if (al < bl)
    ASSH_SWAP(xr, xp);

  memmove(xr, an, al * sizeof(assh_bnword_t));
  memmove(xp, bn, bl * sizeof(assh_bnword_t));

  unsigned int rl = al, pl = al;

  ASSH_CHK_RET(assh_bignum_div_strip(&rl, xr) ||
	       assh_bignum_div_strip(&pl, xp),
               ASSH_ERR_NUM_OVERFLOW);

  while (1)
    {
      unsigned int az, al, bz, bl, da, sa;
      assh_bnword_t at, bt, q;

      int c = assh_bignum_div_cmp(xr, rl, xp, pl);
      if (c == 0)
        break;
      if (c > 0)
	{
	  ASSH_SWAP(rl, pl);
	  ASSH_SWAP(xr, xp);
	}

      assh_bignum_div_clz(rl, xr, &az, &al, &at);
      assh_bignum_div_clz(pl, xp, &bz, &bl, &bt);
      ASSH_CHK_RET(al < bl, ASSH_ERR_NUM_OVERFLOW);

      q = assh_bignum_div_factor(at, bt, al - bl, &sa, &da);

      assh_bignum_div_update_r(pl, xp, rl, xr, q, sa, da);

      ASSH_CHK_RET(assh_bignum_div_strip(&rl, xr),
                   ASSH_ERR_NUM_OVERFLOW);
    }

  return ASSH_OK;
}

/*********************************************************************** mul */

static void
assh_bignum_school_mul(assh_bnword_t * __restrict__ r,
                       const assh_bnword_t *a, unsigned int alen,
                       const assh_bnword_t *b, unsigned int blen)
{
  memset(r, 0, alen * sizeof(assh_bnword_t));

  unsigned int j, i;
  assh_bnlong_t t;

  for (j = 0; j < blen; j++)
    {
      for (t = i = 0; i < alen; i++)
	r[i + j] = t = (assh_bnlong_t)a[i] * b[j] + r[i + j] + (t >> ASSH_BIGNUM_W);
      r[i + j] = (t >> ASSH_BIGNUM_W);
    }
}

#if !defined(__OPTIMIZE_SIZE__)
static void
assh_bignum_karatsuba(assh_bnword_t * __restrict__ r,
                      const assh_bnword_t *a, const assh_bnword_t *b,
                      assh_bnword_t *scratch, unsigned int l)
{
  if (l < ASSH_BIGNUM_KARATSUBA_THRESHOLD || (l & 1))
    return assh_bignum_school_mul(r, a, l, b, l);

  /*
    scratch buffer:
      layout: x[h], y_[h], z1[l+1]
      size: 2*l+1        per stack frame
            4*l+log2(l)  on initial call
  */

#define ASSH_KARA_SCRATCH(len) (len * 4)
          /* + log2(len) - ASSH_KARA_SCRATCH(ASSH_BIGNUM_KARATSUBA_THRESHOLD) */

  unsigned int i, h = l / 2;
  assh_bnlong_t tx = 0, ty = 0;
  assh_bnword_t cx, cy;

  assh_bnword_t *x = scratch;
  assh_bnword_t *y_ = scratch + h;
  assh_bnword_t *y = x;

  /* compute high/low parts sums */
  for (i = 0; i < h; i++)
    x[i] = tx = (assh_bnlong_t)a[i + h] + a[i] + (tx >> ASSH_BIGNUM_W);
  cy = cx = (assh_bnsword_t)(tx >> 1) >> (ASSH_BIGNUM_W - 1);

  if (a != b)
    {
      y = y_;
      for (i = 0; i < h; i++)
	y[i] = ty = (assh_bnlong_t)b[i + h] + b[i] + (ty >> ASSH_BIGNUM_W);
      cy = (assh_bnsword_t)(ty >> 1) >> (ASSH_BIGNUM_W - 1);
    }

  /* recusive calls */
  assh_bnword_t *z1 = scratch + l;

  scratch += 2 * l + 1;
  assh_bignum_karatsuba(r + l, a + h, b + h, scratch, h); /* z0 */
  assh_bignum_karatsuba(z1   , x    , y    , scratch, h); /* z1 */
  assh_bignum_karatsuba(r    , a    , b    , scratch, h); /* z2 */

  /* z1 = z1 - z0 - z2 */
  assh_bnlong_t t = (assh_bnlong_t)2 << ASSH_BIGNUM_W;
  for (i = 0; i < h; i++)
    z1[i] = t = (assh_bnlong_t)z1[i] + (assh_bnword_t)~r[i]
              + (assh_bnword_t)~r[i + l] + (t >> ASSH_BIGNUM_W);
  for (; i < l; i++)
    z1[i] = t = (assh_bnlong_t)z1[i] + (assh_bnword_t)~r[i]
              + (assh_bnword_t)~r[i + l] + (t >> ASSH_BIGNUM_W)
              + (x[i-h] & cy) + (y[i-h] & cx);
  z1[i] = (t >> ASSH_BIGNUM_W) - 2 - (cx & cy);

  /* add z1 to result */
  t = 0;
  for (i = h; i < l+h+1; i++)
    r[i] = t = (assh_bnlong_t)r[i] + z1[i - h] + (t >> ASSH_BIGNUM_W);
  for (; i < l*2; i++)
    r[i] = t = (assh_bnlong_t)r[i] + (t >> ASSH_BIGNUM_W);
}
#endif

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_mul(struct assh_context_s *ctx,
                struct assh_bignum_scratch_s *sc,
                struct assh_bignum_s *r,
                const struct assh_bignum_s *a,
                const struct assh_bignum_s *b)
{
  assh_error_t err;

  assert(r != a && r != b);

  r->secret = a->secret | b->secret;

  ASSH_CHK_RET(r->bits < a->bits + b->bits, ASSH_ERR_OUTPUT_OVERFLOW);

  size_t rl = assh_bignum_words(r->bits);
  size_t al = assh_bignum_words(a->bits);
  size_t bl = assh_bignum_words(b->bits);
  size_t l = al + bl;

#if !defined(__OPTIMIZE_SIZE__)
  if (al == bl && !(al & 1))
    {
      ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, ASSH_KARA_SCRATCH(al)));
      assh_bignum_karatsuba(r->n, a->n, b->n, sc->n, al);
    }
  else
#endif
    assh_bignum_school_mul(r->n, a->n, al, b->n, bl);

  memset((assh_bnword_t*)r->n + l, 0, (rl - l) * sizeof(assh_bnword_t));

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_mul_mod(struct assh_context_s *ctx,
                    struct assh_bignum_scratch_s *sc,
                    struct assh_bignum_s *r,
                    const struct assh_bignum_s *a,
                    const struct assh_bignum_s *b,
                    const struct assh_bignum_s *m)
{
  assh_error_t err;

  assert(!a->secret && !b->secret && !m->secret);

  ASSH_CHK_RET(r->bits < m->bits, ASSH_ERR_OUTPUT_OVERFLOW);

  size_t al = assh_bignum_words(a->bits);
  size_t bl = assh_bignum_words(b->bits);
  size_t ml = assh_bignum_words(m->bits);
  size_t rl = assh_bignum_words(r->bits);

  size_t l = al + bl; /* result size */

  size_t scratch_len = l;
#if !defined(__OPTIMIZE_SIZE__)
  assh_bool_t use_kara = al == bl && !(al & 1);
  if (use_kara)
    scratch_len += ASSH_KARA_SCRATCH(al);
#endif

  ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, scratch_len));
  assh_bnword_t *x = sc->n;

#if !defined(__OPTIMIZE_SIZE__)
  if (use_kara)
    assh_bignum_karatsuba(x, a->n, b->n, sc->n + l, al);
  else
#endif
    assh_bignum_school_mul(x, a->n, al, b->n, bl);

  ASSH_ERR_RET(assh_bignum_div_euclidean(x, l, NULL, 0, m->n, ml));

  r->secret = 0;
  memcpy(r->n, x, ml * sizeof(assh_bnword_t));
  memset(r->n + ml, 0, (rl - ml) * sizeof(assh_bnword_t));

  return ASSH_OK;
}

/********************************************************* montgomery */

static assh_bnword_t assh_bnword_mt_modinv(assh_bnword_t a)
{
  uint_fast8_t i, sh = assh_bn_clz(a);
  assh_bnword_t b = -(a << sh);
  assh_bnword_t q = -((assh_bnword_t)1 << sh);
  assh_bnword_t c, r = 1;

  while (a)
    {
      if (a < b)
        {
          ASSH_SWAP(a, b);
          ASSH_SWAP(r, q);
        }
      sh = assh_bn_clz(b) - assh_bn_clz(a);
      c = b << sh;
      i = (c > a);
      a -= (c >> i);
      r -= q << (sh - i);
    }

  return q;
}

static void
assh_bignum_mt_mul(const struct assh_bignum_mt_s *mt,
                   assh_bnword_t * __restrict__ a,
                   const assh_bnword_t * __restrict__ x,
                   const assh_bnword_t * __restrict__ y)
{
  size_t i, j;
  size_t ml = assh_bignum_words(mt->mod.bits);
  assh_bnword_t *m = mt->mod.n;
  assh_bnword_t q, k = 0;
  assh_bnlong_t p, t, r;

  for (i = 0; i < ml; i++)
    a[i] = 0;

  for (i = 0; i < ml; i++)
    {
      p = a[0] + (assh_bnlong_t)x[i] * y[0];
      q = p * mt->n0;
      r = (assh_bnlong_t)m[0] * q;
      /* Computes (p+r)/2 without overflow then drop the lower part.
         We have the carry in constant time without relying on an
         integer type larger than assh_bnlong_t. */
      t = ((p & r) + ((p ^ r) >> 1)) >> (ASSH_BIGNUM_W - 1);

      for (j = 1; j < ml; j++)
        {
          p = a[j] + (assh_bnlong_t)x[i] * y[j];
          r = (assh_bnlong_t)m[j] * q + t;
          t = ((p & r) + ((p ^ r) >> 1)) >> (ASSH_BIGNUM_W - 1);
          a[j-1] = p + r;
        }
      t += k;
      a[j-1] = t;
      k = t >> ASSH_BIGNUM_W;
    }

  /* Masked final subtraction */
  q = (k ^ 1) - 1;
  t = (assh_bnlong_t)(q & 1) << ASSH_BIGNUM_W;
  for (i = 0; i < ml; i++)
    a[i] = t = (assh_bnlong_t)a[i] + (q & ~m[i]) + (t >> ASSH_BIGNUM_W);
}

static void
assh_bignum_mt_reduce(const struct assh_bignum_mt_s *mt,
                      assh_bnword_t * __restrict__ a,
                      const assh_bnword_t * __restrict__ x)
{
  size_t i, j;
  size_t ml = assh_bignum_words(mt->mod.bits);
  assh_bnword_t *m = mt->mod.n;
  assh_bnword_t q;
  assh_bnlong_t p, t, r;

  for (i = 0; i < ml; i++)
    a[i] = 0;

  for (i = 0; i < ml; i++)
    {
      p = a[0] + (assh_bnlong_t)x[i];
      q = p * mt->n0;
      r = (assh_bnlong_t)m[0] * q;
      t = p + r;

      for (j = 1; j < ml; j++)
        {
          p = a[j];
          r = (assh_bnlong_t)m[j] * q + (t >> ASSH_BIGNUM_W);
          t = p + r;
          a[j-1] = t;
        }
      a[j-1] = (t >> ASSH_BIGNUM_W);
    }

#warning FIXME a might be == mod
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_mt_init(struct assh_context_s *c,
                    struct assh_bignum_mt_s *mt,
                    const struct assh_bignum_s *mod)
{
  assh_error_t err;

  assert(!mod->secret);

  /* check modulus is odd */
  ASSH_CHK_RET(!(*(assh_bnword_t*)mod->n & 1), ASSH_ERR_NUM_OVERFLOW);

  /* compute r**2 % n */
  size_t ml = assh_bignum_words(mod->bits);

  assh_bnword_t *m = mt->mod.n;

  if (m == NULL || mt->mod.bits < mod->bits)
    {
      if (m != NULL)
        assh_free(c, m);
      ASSH_ERR_RET(assh_alloc(c, (ml * 3 + 1) * sizeof(assh_bnword_t),
                              ASSH_ALLOC_INTERNAL, (void**)&m));
      mt->mod.n = m;
    }

  /* copy the modulus */
  memcpy(m, mod->n, ml * sizeof(assh_bnword_t));

  mt->mod.bits = mod->bits;
  mt->mod.secret = 0;
  mt->mod.montgomery = 1;
  mt->n0 = assh_bnword_mt_modinv(-m[0]);

  assh_bnword_t *r2 = m + ml;

  size_t i;
  for (i = 0; i < ml * 2; i++)
    r2[i] = 0;
  r2[i] = 1;

  ASSH_ERR_GTO(assh_bignum_div_euclidean(r2, ml * 2 + 1, NULL, 0, m, ml), err_);

  assh_bnword_t *r1 = m + ml * 2;

  for (i = 0; i < ml; i++)
    r1[i] = 0;
  r1[i] = 1;

  ASSH_ERR_GTO(assh_bignum_div_euclidean(r1, ml + 1, NULL, 0, m, ml), err_);

  //  assh_hexdump("one", one, ml * sizeof(assh_bnword_t));

  return ASSH_OK;

 err_:
  mt->mod.n = NULL;
  assh_free(c, m);
  return err;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_mt_convert(struct assh_context_s *ctx,
                       struct assh_bignum_scratch_s *sc,
                       assh_bool_t fwd,
                       const struct assh_bignum_mt_s *mt,
                       struct assh_bignum_s *r,
                       const struct assh_bignum_s *a)
{
  assh_error_t err;

  assert(mt->mod.bits == a->bits && mt->mod.bits == r->bits);
  size_t ml = assh_bignum_words(mt->mod.bits);
  assh_bnword_t *t = r->n;

  r->secret = a->secret;

  if (r == a)
    {
      ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, ml));
      t = sc->n;
    }

  if (fwd)
    {
      assh_bnword_t *r2 = (assh_bnword_t*)mt->mod.n + ml;
      assh_bignum_mt_mul(mt, t, r2, a->n);
    }
  else
    assh_bignum_mt_reduce(mt, t, a->n);

  if (r == a)
    memcpy(r->n, t, ml * sizeof(assh_bnword_t));

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_mul_mod_mt(struct assh_context_s *ctx,
                       struct assh_bignum_scratch_s *sc,
                       struct assh_bignum_s *r,
                       const struct assh_bignum_s *a,
                       const struct assh_bignum_s *b,
                       const struct assh_bignum_mt_s *mt)
{
  assh_error_t err = ASSH_OK;

  assert(mt->mod.bits == a->bits &&
         mt->mod.bits == b->bits &&
         mt->mod.bits == r->bits);

  r->secret = a->secret | b->secret;

  size_t rl = assh_bignum_words(r->bits);
  assh_bnword_t *rn = r->n;

  if (r == a || r == b)
    {
      ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, rl));
      rn = sc->n;
    }

  assh_bignum_mt_mul(mt, rn, a->n, b->n);

  if (rn == sc->n)
    memcpy(r->n, rn, rl * sizeof(assh_bnword_t));

 err:
  return err;
}

static void
assh_bignum_cmov(assh_bnword_t *a, const assh_bnword_t *b,
                 size_t l, assh_bool_t c)
{
  assh_bnword_t m = ~((assh_bnword_t)c - 1);
  size_t i;

  for (i = 0; i < l; i++)
    a[i] = (b[i] & m) | (a[i] & ~m);
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_expmod_mt(struct assh_context_s *ctx,
                      struct assh_bignum_scratch_s *sc,
                      struct assh_bignum_s *r,
                      const struct assh_bignum_s *a,
                      const struct assh_bignum_s *b,
                      const struct assh_bignum_mt_s *mt)
{
  assh_error_t err;

  assert(r != b);
  assert(mt->mod.bits == a->bits &&
         mt->mod.bits == r->bits);

  r->secret = a->secret | b->secret;

  size_t ml = assh_bignum_words(mt->mod.bits);

  ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, ml * 2));

  assh_bnword_t *sq = sc->n;
  assh_bnword_t *tmp = sq + ml;
  assh_bnword_t *bn = b->n;
  assh_bnword_t *rn = r->n;
  uint_fast16_t i = 0;

  memcpy(sq, a->n, ml * sizeof(assh_bnword_t));

  assh_bnword_t *r1 = (assh_bnword_t*)mt->mod.n + 2 * ml;
  memcpy(rn, r1, ml * sizeof(assh_bnword_t));

  while (1)
    {
      /* constant time when exponent is secret */
      assh_bool_t c = (bn[i / ASSH_BIGNUM_W] >> (i % ASSH_BIGNUM_W)) & 1;
      volatile assh_bool_t d = b->secret | c;

      if (d)
        {
          assh_bignum_mt_mul(mt, tmp, rn, sq);
          assh_bignum_cmov(rn, tmp, ml, c);
        }

      if (++i == b->bits)
        break;

      assh_bignum_mt_mul(mt, tmp, sq, sq);
      memcpy(sq, tmp, ml * sizeof(assh_bnword_t));
    }

  return ASSH_OK;
}

/* compute inverse using the Fermat little theorem */
static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_modinv_mt(struct assh_context_s *ctx,
                      struct assh_bignum_scratch_s *sc,
                      struct assh_bignum_s *r,
                      const struct assh_bignum_s *a,
                      const struct assh_bignum_mt_s *mt)
{
  assh_error_t err;

  assert(mt->mod.bits == a->bits &&
         mt->mod.bits == r->bits);

  r->secret = a->secret;
  /* prime modulus as been checked as non-secret in mt_init */

  size_t ml = assh_bignum_words(mt->mod.bits);

  ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, ml * 2));

  assh_bnword_t *sq = sc->n;
  assh_bnword_t *tmp = sq + ml;
  uint_fast16_t i = 0;
  assh_bnword_t *rn = r->n;

  /* prime modulus - 2 */
  assh_bnword_t p, *pn = mt->mod.n;
  assh_bnslong_t t = (assh_bnslong_t)-2 << ASSH_BIGNUM_W;

  memcpy(sq, a->n, ml * sizeof(assh_bnword_t));

  assh_bnword_t *r1 = (assh_bnword_t*)mt->mod.n + 2 * ml;
  memcpy(rn, r1, ml * sizeof(assh_bnword_t));

  while (1)
    {
      if (i % ASSH_BIGNUM_W == 0)
        p = t = (assh_bnslong_t)pn[i / ASSH_BIGNUM_W] + (t >> ASSH_BIGNUM_W);

      if ((p >> (i % ASSH_BIGNUM_W)) & 1)
        {
          assh_bignum_mt_mul(mt, tmp, rn, sq);
          memcpy(rn, tmp, ml * sizeof(assh_bnword_t));
        }

      if (++i == mt->mod.bits)
        break;

      assh_bignum_mt_mul(mt, tmp, sq, sq);
      memcpy(sq, tmp, ml * sizeof(assh_bnword_t));
    }

  return ASSH_OK;
}

#include "bignum_builtin_primes.h"

struct assh_bignum_sieve_s
{
  uint16_t offsets[ASSH_SIEVE_PRIMES];
};

/* compute offsets of prime number sieve */
static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_sieve_init(struct assh_context_s *ctx,
                       struct assh_bignum_sieve_s * __restrict__ s,
                       const struct assh_bignum_s *bn)
{
  assh_error_t err;
  size_t l = assh_bignum_words(bn->bits);
  assh_bnword_t *n = bn->n;  
  size_t i, k;

  /* compute n modulus some prime numbers */
  for (i = 0; i < ASSH_SIEVE_PRIMES; i++)
    {
      uint32_t o = n[0] % assh_primes[i];
      uint16_t m = ASSH_BN_WORDMAX % assh_primes[i] + 1;
      uint16_t p = assh_primes[i];
      uint16_t p2m = m;

      for (k = 1; k < l; k++)
        {
          /* prevent overflow */
          if ((k & 63) == 0)
            o %= p;

          /* update the sieve with the next big number word */
          o += (uint32_t)m * (n[k] % p);
          m = ((uint32_t)m * p2m) % p;
        }

      s->offsets[i] = o % p;
    }

  /* adjust sieve values */
  for (i = 0; i < ASSH_SIEVE_PRIMES; i++)
    {
      uint16_t o = s->offsets[i];
      uint16_t p = assh_primes[i];
      if (o)
        o = p - o;
      /* keep s->offsets[n] + bignum odd */
      if ((o ^ n[0] ^ 1) & 1)
        o += p;
      s->offsets[i] = o;
    }

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_miller_rabin(struct assh_context_s *ctx,
                         struct assh_bignum_scratch_s *sc,
                         const struct assh_bignum_s *bn, size_t rounds)
{
  assh_error_t err;
  size_t l = assh_bignum_words(bn->bits);
  assh_bnword_t *n = bn->n;  
  size_t i;

  assert(bn->bits > 0 && (n[0] & 1));

  ASSH_ERR_RET(assh_bignum_scratch_expand(ctx, sc, l * 4));

  assh_bnword_t *an = sc->n;
  assh_bnword_t *cn = sc->n + l;
  assh_bnword_t *zn = sc->n + l * 2;
  assh_bnword_t *tn = sc->n + l * 3;

  /* compute c = n - 1 */
  assh_bnlong_t t = 0;
  for (i = 0; i < l; i++)
    cn[i] = t = (assh_bnlong_t)n[i] + ASSH_BN_WORDMAX + (t >> ASSH_BIGNUM_W);

  /* b = clz(c) */
  size_t b = 0;
  while (1)
    {
      ASSH_CHK_RET(b >= l, ASSH_ERR_NUM_OVERFLOW);
      if (cn[b])
        break;
      b++;
    }
  b = b * ASSH_BIGNUM_W + assh_bn_ctz(cn[b]);
  assert(b > 0);

  /* Initialize a temporary montgomery context. We do not need to
     compute r2 for conversion to montgomery representation because
     the random value a is assumed to be in montgomery representation.
     We do not need to compute r1 because the LSB of the exponent
     is always set so we can skip the first multiply by one operation. */
  struct assh_bignum_mt_s mt;
  mt.mod.bits = l * ASSH_BIGNUM_W;
  mt.n0 = assh_bnword_mt_modinv(-n[0]);
  mt.mod.n = n;

  while (rounds--)
    {
      /* generate a in range [2, n - 2] */
#warning FIXME generate better random a
      for (i = 0; i + 1 < l; i++)
        an[i] = rand();
      an[i] = n[i] - 1;

      /* compute z = a**(c >> b) % n */
      memcpy(zn, an, l * sizeof(assh_bnword_t)); /* LSB(c >> b) is always set */

      for (i = b + 1; i < bn->bits; i++)
        {
          assh_bool_t c = (cn[i / ASSH_BIGNUM_W]
                           >> (i % ASSH_BIGNUM_W)) & 1;
          volatile assh_bool_t d = bn->secret | c;

          assh_bignum_mt_mul(&mt, tn, an, an);
          memcpy(an, tn, l * sizeof(assh_bnword_t));

          if (d)
            {
              /* constant time exp */
              assh_bignum_mt_mul(&mt, tn, zn, an);
              assh_bignum_cmov(zn, tn, l, c);
            }
        }

      /* probable prime if z == 1 */
      assh_bignum_mt_reduce(&mt, tn, zn);
      if (assh_bignum_eq_uint(1, tn, l))
        continue;

      i = b - 1;
      while (1)
        {
          /* probable prime if z == p - 1 */
          assh_bignum_mt_reduce(&mt, tn, zn);
          if (assh_bignum_eq(tn, l, cn, l))
            break;

          if (!i--)
            return ASSH_NOT_FOUND;

          /* z = z**2 % p */
          assh_bignum_mt_mul(&mt, tn, zn, zn);
          memcpy(zn, tn, l * sizeof(assh_bnword_t));

          /* composite if z == 1 */
          assh_bignum_mt_reduce(&mt, tn, zn);
          if (assh_bignum_eq_uint(1, tn, l))
            return ASSH_NOT_FOUND;
        }
    }

  return ASSH_OK;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_check_prime(struct assh_context_s *ctx,
                        struct assh_bignum_scratch_s *sc,
                        const struct assh_bignum_s *bn)
{
  assh_error_t err;
  size_t l = assh_bignum_words(bn->bits);
  assh_bnword_t *n = bn->n;
  size_t i;

  if (l == 0 || !(n[0] & 1))
    return ASSH_NOT_FOUND;

  struct assh_bignum_sieve_s *sieve;
  ASSH_ERR_RET(assh_alloc(ctx, sizeof(*sieve),
                 bn->secret ? ASSH_ALLOC_SECUR : ASSH_ALLOC_INTERNAL,
                 (void**)&sieve));

  ASSH_ERR_GTO(assh_bignum_sieve_init(ctx, sieve, bn), err_);

  assh_bool_t composite = 0;
  for (i = 0; i < ASSH_SIEVE_PRIMES; i++)
    composite |= !sieve->offsets[i];

  if (composite)
    return ASSH_NOT_FOUND;

  /* number of round estimate for probability of n with unknown origin
     being composite equals to 2**-128 */
  ASSH_ERR_GTO(assh_bignum_miller_rabin(ctx, sc, bn, 64), err_);

 err_:
  assh_free(ctx, sieve);
  return err;
}

static assh_error_t ASSH_WARN_UNUSED_RESULT
assh_bignum_next_prime(struct assh_context_s *ctx,
                       struct assh_bignum_scratch_s *sc,
                       struct assh_bignum_s *bn)
{
  assh_error_t err;
  const size_t l = assh_bignum_words(bn->bits);
  assh_bnword_t *n = bn->n;  
  uint32_t sieve_bits[8];
  uint32_t k, offset = 0;
  size_t i, j;

  /* simple formula for upper bound of max prime gap */
  const uint32_t max_prime_gap = (bn->bits * bn->bits) / 2;

  struct assh_bignum_sieve_s * __restrict__ sieve;
  ASSH_ERR_RET(assh_alloc(ctx, sizeof(*sieve),
                 bn->secret ? ASSH_ALLOC_SECUR : ASSH_ALLOC_INTERNAL,
                 (void**)&sieve));

  ASSH_ERR_GTO(assh_bignum_sieve_init(ctx, sieve, bn), err_);

  for (k = 0; k < max_prime_gap; k += 512)
    {
      /* update sieve bitmap */
      memset(sieve_bits, 0, sizeof(sieve_bits));
      for (i = 0; i < ASSH_SIEVE_PRIMES; i++)
        {
          uint32_t s = sieve->offsets[i];
          while (s - k < 512)
            {
              uint32_t x = (s - k) >> 1;
              sieve_bits[x / 32] |= 1 << (x % 32);
              s += assh_primes[i] * 2;
            }
          sieve->offsets[i] = s;
        }

      /* test remaining candidate primes */
      for (i = 0; i < 8; i++)
        {
          uint32_t x;
          for (x = ~sieve_bits[i]; x; x &= (x - 1))
            {
              uint32_t o = k + (i * 32 + ASSH_CTZ32(x)) * 2 + (sieve->offsets[0] & 1);
              ASSH_DEBUG("candidate prime at offset %u, mask is %x\n", o, x);

              /* advance big number to candidate prime */
              assh_bnword_t c = o - offset;
              ASSH_CHK_GTO(c != o - offset, ASSH_ERR_NUM_OVERFLOW, err_);
              offset = o;
              assh_bnlong_t t = (assh_bnlong_t)c << ASSH_BIGNUM_W;
              for (j = 0; j < l; j++)
                n[j] = t = (assh_bnlong_t)n[j] + (t >> ASSH_BIGNUM_W);
              ASSH_CHK_GTO((t >> ASSH_BIGNUM_W) != 0, ASSH_ERR_NUM_OVERFLOW, err_);

              /* test prime candidate. number of rounds estimate for a
                 randomly generated number taken from: "Average case
                 error estimates for the strong probable prime test,
                 Mathematics of Computation 61" */
              ASSH_ERR_GTO(assh_bignum_miller_rabin(ctx, sc, bn, 7), err_);
              if (err == ASSH_OK)
                goto err_;
            }
        }
    }

  /* give up */
  ASSH_ERR_GTO(ASSH_ERR_NUM_OVERFLOW, err_);

 err_:
  assh_free(ctx, sieve);
  return err;
}

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_bignum_realloc(struct assh_context_s *c,
                    struct assh_bignum_s *bn)
{
  if (bn->n != NULL)
    return ASSH_OK;
#warning should not always alloc in secur memory, should update allocation
  return assh_realloc(c, &bn->n, assh_bignum_words(bn->bits) *
                      sizeof(assh_bnword_t), ASSH_ALLOC_SECUR);
}

static ASSH_BIGNUM_CONVERT_FCN(assh_bignum_builtin_convert)
{
  assh_error_t err;

  const struct assh_bignum_s *srcn = src;
  struct assh_bignum_s *dstn = dst;

  if (srcfmt == ASSH_BIGNUM_NATIVE ||
      srcfmt == ASSH_BIGNUM_STEMP ||
      srcfmt == ASSH_BIGNUM_TEMP)
    {
      switch (dstfmt)
        {
        case ASSH_BIGNUM_NATIVE:
        case ASSH_BIGNUM_TEMP:
        case ASSH_BIGNUM_STEMP:
          ASSH_ERR_RET(assh_bignum_realloc(c, dstn));
          ASSH_ERR_RET(assh_bignum_copy(dstn, srcn));
          break;
        case ASSH_BIGNUM_MPINT:
        case ASSH_BIGNUM_STRING:
        case ASSH_BIGNUM_MSB_RAW:
        case ASSH_BIGNUM_LSB_RAW:
          assh_bignum_to_buffer(srcn, dst, dstfmt);
          break;

        default:
          ASSH_ERR_RET(ASSH_ERR_NOTSUP);
        }
    }
  else
    {
      size_t l, n, b;

      assert(dstfmt == ASSH_BIGNUM_NATIVE ||
             dstfmt == ASSH_BIGNUM_STEMP ||
             dstfmt == ASSH_BIGNUM_TEMP);

      if (srcfmt == ASSH_BIGNUM_MSB_RAW ||
          srcfmt == ASSH_BIGNUM_LSB_RAW)
        {
          b = dstn->bits;
          n = l = ASSH_ALIGN8(b) / 8;
        }
      else
        {
          ASSH_ERR_RET(assh_bignum_size_of_data(srcfmt, src, &l, &n, &b));
          ASSH_CHK_RET(dstn->bits < b, ASSH_ERR_NUM_OVERFLOW);
        }

      switch (srcfmt)
        {
        case ASSH_BIGNUM_STRING:
        case ASSH_BIGNUM_MPINT:
        case ASSH_BIGNUM_ASN1:
          ASSH_ERR_RET(assh_bignum_realloc(c, dstn));
          ASSH_ERR_RET(assh_bignum_from_buffer(dstn, src + l - n, n, srcfmt));
          break;

        case ASSH_BIGNUM_MSB_RAW:
        case ASSH_BIGNUM_LSB_RAW:
          ASSH_ERR_RET(assh_bignum_realloc(c, dstn));
          ASSH_ASSERT(assh_bignum_from_buffer(dstn, src, n, srcfmt));
          break;

        case ASSH_BIGNUM_INT:
          ASSH_CHK_RET(dstn->bits < sizeof(uintptr_t) * 8, ASSH_ERR_NUM_OVERFLOW);
          ASSH_ERR_RET(assh_bignum_realloc(c, dstn));
          ASSH_ERR_RET(assh_bignum_from_uint(dstn, (uintptr_t)src));
          break;

        case ASSH_BIGNUM_SIZE:
          dstn->bits = b;
          break;

        default:
          ASSH_ERR_RET(ASSH_ERR_NOTSUP);
        }
    }

  return ASSH_OK;
}

static ASSH_BIGNUM_BYTECODE_FCN(assh_bignum_builtin_bytecode)
{
  uint_fast8_t flen, tlen, mlen;
  assh_error_t err;
  uint_fast8_t i, j, k, pc = 0;
  struct assh_bignum_scratch_s sc
    = { .words = 0, .n = NULL, .secur = 0 };

  /* find number of arguments and temporaries */
  for (mlen = tlen = flen = 0; format[flen]; flen++)
    {
      switch (format[flen])
        {
        case ASSH_BIGNUM_TEMP:
        case ASSH_BIGNUM_STEMP:
          tlen++;
          break;
        case ASSH_BIGNUM_MT:
          mlen++;
          break;
        }
    }

  void *args[flen];
  struct assh_bignum_s tmp[tlen];
  struct assh_bignum_mt_s mt[mlen];

  memset(tmp, 0, sizeof(tmp));

#warning DEBUG remove
  struct assh_bignum_mt_s *mtg;

  for (i = j = k = 0; i < flen; i++)
    switch (format[i])
      {
      case ASSH_BIGNUM_STEMP:
        tmp[j].secret = 1;
        sc.secur = 1;
      case ASSH_BIGNUM_TEMP:
        args[i] = &tmp[j];
        j++;
        break;
      case ASSH_BIGNUM_MT:
        mtg = mt + k;
        mt[k].mod.n = NULL;
        args[i] = &mt[k];
        k++;
        break;
      case ASSH_BIGNUM_SIZE:
        args[i] = (void*)va_arg(ap, size_t);
        break;
      case ASSH_BIGNUM_NATIVE: {
        struct assh_bignum_s *bn = va_arg(ap, void *);
        args[i] = bn;
        sc.secur |= bn->secret;
        break;
      }
      default:
        args[i] = va_arg(ap, void *);
      }

  while (1)
    {
      uint32_t opc = ops[pc];
      enum assh_bignum_opcode_e op = opc >> 26;
      uint_fast8_t oa = (opc >> 20) & 0x3f;
      uint_fast8_t ob = (opc >> 14) & 0x3f;
      uint_fast8_t oc = (opc >> 6) & 0xff;
      uint_fast8_t od = opc & 0x3f;
      uint_fast32_t value = (opc >> 6) & 0xfffff;

#if defined(CONFIG_ASSH_DEBUG) && 1
      const char *opnames[] = ASSH_BIGNUM_OP_NAMES;
      ASSH_DEBUG("exec=%p, pc=%u, op=%s, a=%u, b=%u, c=%u, d=%u, value=%u\n",
                 ops, pc, opnames[op], oa, ob, oc, od, value);
#if 0
      {
        size_t i;
        for (i = 0; i < flen; i++)
          if (format[i] == ASSH_BIGNUM_NATIVE ||
              format[i] == ASSH_BIGNUM_TEMP ||
              format[i] == ASSH_BIGNUM_STEMP)
            {
              struct assh_bignum_s *src = args[i];
              ASSH_DEBUG("%u: ", i);
              if (src->n)
                {
                  size_t l = assh_bignum_words(src->bits);
                  if (src->montgomery) {
                    assh_bnword_t t[l];
                    assh_bignum_mt_reduce(mtg, t, src->n);
                    assh_bignum_dump(t, l);
                  }
                  else
                    assh_bignum_dump(src->n, l);
                }
              else
                fprintf(stderr, "\n");
            }
      }
#endif
#endif

      pc++;

      switch (op)
        {
        case ASSH_BIGNUM_OP_END:
          goto end;

        case ASSH_BIGNUM_OP_MOVE:
          ASSH_ERR_GTO(assh_bignum_builtin_convert(c,
                    format[od], format[oc], args[od], args[oc]), err_sc);
          break;

        case ASSH_BIGNUM_OP_SIZER:
        case ASSH_BIGNUM_OP_SIZE: {
          size_t b, i;
          ASSH_ERR_GTO(assh_bignum_size_of_data(format[ob], args[ob],
                                                NULL, NULL, &b), err_sc);
          if (op == ASSH_BIGNUM_OP_SIZE)
            {
              struct assh_bignum_s *dst = args[oa];
              dst->bits = ((od >= 32) ? (b << (od - 32))
                           : (b >> (32 - od))) + (intptr_t)(int8_t)oc;
            }
          else
            for (i = oa; i <= oc; i++) 
              {
                struct assh_bignum_s *dst = args[i];
                dst->bits = b;
              }
          break;
        }

        case ASSH_BIGNUM_OP_SUB:
        case ASSH_BIGNUM_OP_ADD: {
          struct assh_bignum_s *dst = args[oa];
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          assh_bnword_t mask = (assh_bnword_t)(op == ASSH_BIGNUM_OP_ADD) - 1;
          if (od != ASSH_BOP_NOREG)
            ASSH_ERR_GTO(assh_bignum_addsub(dst, args[ob], args[oc], args[od], mask), err_sc);
          else
            ASSH_ERR_GTO(assh_bignum_addsub(dst, args[ob], args[oc], NULL, mask), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_MUL: {
          struct assh_bignum_s *dst = args[oa];
          struct assh_bignum_s *src1 = args[ob];
          struct assh_bignum_s *src2 = args[oc];
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          if (od == ASSH_BOP_NOREG)
            ASSH_ERR_GTO(assh_bignum_mul(c, &sc, dst, src1, src2), err_sc);
          else if (format[od] == ASSH_BIGNUM_MT)
            ASSH_ERR_GTO(assh_bignum_mul_mod_mt(c, &sc, dst, src1, src2, args[od]), err_sc);
          else
            ASSH_ERR_GTO(assh_bignum_mul_mod(c, &sc, dst, src1, src2, args[od]), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_EXPM: {
          struct assh_bignum_s *dst = args[oa];
          struct assh_bignum_s *src1 = args[ob];
          struct assh_bignum_s *src2 = args[oc];
          assert(format[od] == ASSH_BIGNUM_MT);
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          ASSH_ERR_GTO(assh_bignum_expmod_mt(c, &sc, dst, src1, src2, args[od]), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_MTINIT: {
          struct assh_bignum_s *mod = args[od];
          assert(format[oc] == ASSH_BIGNUM_MT);
          ASSH_ERR_GTO(assh_bignum_mt_init(c, args[oc], mod), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_MTFROM:
        case ASSH_BIGNUM_OP_MTTO: {
          uint_fast8_t i;
          for (i = 0; i < oa; i++)
            {
              struct assh_bignum_s *dst = args[ob + i];
              struct assh_bignum_s *src = args[oc + i];
              ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
              ASSH_ERR_GTO(assh_bignum_mt_convert(c, &sc, op == ASSH_BIGNUM_OP_MTTO,
                                                  args[od], dst, src), err_sc);
            }
          break;
        }

        case ASSH_BIGNUM_OP_DIV: {
          struct assh_bignum_s *dsta = NULL, *dstb = NULL;
          struct assh_bignum_s *src1 = args[oc], *src2 = args[od];
          if (oa != ASSH_BOP_NOREG)
            {
              dsta = args[oa];
              ASSH_ERR_GTO(assh_bignum_realloc(c, dsta), err_sc);
            }
          if (ob != ASSH_BOP_NOREG)
            {
              dstb = args[ob];
              ASSH_ERR_GTO(assh_bignum_realloc(c, dstb), err_sc);
            }
          ASSH_ERR_GTO(assh_bignum_div(c, &sc, dstb, dsta, src1, src2), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_INV: {
          struct assh_bignum_s *dst = args[ob];
          struct assh_bignum_s *src1 = args[oc];
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          if (format[od] == ASSH_BIGNUM_MT)
            ASSH_ERR_GTO(assh_bignum_modinv_mt(c, &sc, dst, src1, args[od]), err_sc);
          else
            ASSH_ERR_GTO(assh_bignum_modinv(c, &sc, dst, src1, args[od]), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_GCD: {
          struct assh_bignum_s *dst = args[ob];
          struct assh_bignum_s *src1 = args[oc], *src2 = args[od];
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          ASSH_ERR_GTO(assh_bignum_gcd(c, &sc, dst, src1, src2), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_SHR:
        case ASSH_BIGNUM_OP_SHL: {
          struct assh_bignum_s *dst = args[oa];
          struct assh_bignum_s *src = args[ob];
          size_t b = 0;
          ASSH_CHK_GTO(dst->bits != src->bits, ASSH_ERR_OUTPUT_OVERFLOW, err_sc);
          if (od != ASSH_BOP_NOREG)
            {
#warning FIXME constant time ?
              ASSH_ERR_GTO(assh_bignum_size_of_data(format[od], args[od],
                                                    NULL, NULL, &b), err_sc);
            }
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          switch (op)
            {
            case ASSH_BIGNUM_OP_SHR:
              ASSH_ERR_GTO(assh_bignum_rshift(dst, src, b + oc - 128), err_sc);
              break;
            case ASSH_BIGNUM_OP_SHL:
              ASSH_ERR_GTO(assh_bignum_lshift(dst, src, b + oc - 128), err_sc);
              break;
            default:
              abort();
            }
          break;
        }

        case ASSH_BIGNUM_OP_RAND: {
          struct assh_bignum_s *dst = args[oa];
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          ASSH_ERR_GTO(assh_bignum_rand(c, dst,
                         ob == ASSH_BOP_NOREG ? NULL : args[ob],
                         oc == ASSH_BOP_NOREG ? NULL : args[oc],
                         od), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_CMP: {
          int r = 0;
          struct assh_bignum_s *src1 = args[oa];
          struct assh_bignum_s *src2 = args[ob];
          if (ob == ASSH_BOP_NOREG)
            r = src1->n != NULL;
          else if (ob != oa)
            r = assh_bignum_cmp(src1, src2);
          switch (od)
            {
            case 0:             /* cmpeq */
              r = r != 0;
              break;
            case 1:             /* cmpne */
              r = r == 0;
              break;
            case 2:             /* cmplt */
              r = r >= 0;
              break;
            case 3:             /* cmplteq */
              r = r > 0;
              break;
            }
          if (r)
            ASSH_CHK_GTO(oc == 128, ASSH_ERR_NUM_COMPARE_FAILED, err_sc);
          else
            pc += oc - 128;
          break;
        }

        case ASSH_BIGNUM_OP_LADJMP: {
          if (assh_bignum_lad(args[od]))
            pc += oc - 128;
          break;
        }

        case ASSH_BIGNUM_OP_LADSWAP: {
          assh_bignum_cswap(args[ob], args[oc], assh_bignum_lad(args[od]));
          break;
        }

        case ASSH_BIGNUM_OP_LADLOOP: {
          struct assh_bignum_lad_s *lad = args[od];
          uint16_t bit = --lad->count;
          if (bit)
            {
              if (lad->msbyte_1st && (bit & 7) == 0)
                lad->data++;
              pc -= oc;
            }
          break;
        }

        case ASSH_BIGNUM_OP_TESTS:
        case ASSH_BIGNUM_OP_TESTC: {
          struct assh_bignum_s *src1 = args[oa];
          size_t b = ob;
          if (od != ASSH_BOP_NOREG)
            {
              ASSH_ERR_GTO(assh_bignum_size_of_data(format[od], args[od],
                                                    NULL, NULL, &b), err_sc);
              b -= ob;
            }
          assert(b < src1->bits);
          assh_bnword_t *n = src1->n;
          if ((n[b / ASSH_BIGNUM_W] >> (b % ASSH_BIGNUM_W)
               ^ (op != ASSH_BIGNUM_OP_TESTC)) & 1)
            ASSH_CHK_GTO(oc == 128, ASSH_ERR_NUM_COMPARE_FAILED, err_sc);
          else
            pc += oc - 128;
          break;
        }

        case ASSH_BIGNUM_OP_UINT: {
          struct assh_bignum_s *dst = args[od];
          ASSH_ERR_GTO(assh_bignum_realloc(c, dst), err_sc);
          ASSH_ERR_GTO(assh_bignum_from_uint(dst, value), err_sc);
          ASSH_CHK_GTO(dst->n == NULL, ASSH_ERR_MEM, err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_PRIME: {
          struct assh_bignum_s *dst = args[ob];
          ASSH_ERR_GTO(assh_bignum_next_prime(c, &sc, dst), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_PRIVACY: {
          struct assh_bignum_s *src = args[od];
          src->secret = oc;
          break;
        }

        case ASSH_BIGNUM_OP_PRINT: {
#ifdef CONFIG_ASSH_DEBUG
          struct assh_bignum_s *src = args[od];
          char id[5];
          size_t i;
          id[4] = 0;
          assh_store_u32le((uint8_t*)id, oc);
          fprintf(stderr, "[pc=%u, id=%s, type=%c] ", pc, id, format[od]);
          switch (format[od])
            {
            case ASSH_BIGNUM_NATIVE:
            case ASSH_BIGNUM_STEMP:
            case ASSH_BIGNUM_TEMP:
              fprintf(stderr, "[bits=%zu] ", src->bits);
              if (src->secret)
                fprintf(stderr, "secret ");
              if (src->n != NULL)
                assh_bignum_dump(src->n, assh_bignum_words(src->bits));
              else
                fprintf(stderr, "NULL\n");
              break;
            case ASSH_BIGNUM_SIZE:
              fprintf(stderr, "%u\n", (unsigned)(uintptr_t)args[od]);
              break;
            }
#endif
          break;
        }

        }
    }

 end:
  err = ASSH_OK;
 err_sc:;

  if (sc.n != NULL)
    assh_free(c, sc.n);

  /* release numbers */
  for (i = 0; i < tlen; i++)
    if (tmp[i].n != NULL)
      assh_free(c, tmp[i].n);

  for (i = 0; i < mlen; i++)
    if (mt[i].mod.n != NULL)
      assh_free(c, mt[i].mod.n);

  return err;
}

static ASSH_BIGNUM_RELEASE_FCN(assh_bignum_builtin_release)
{
  assh_free(ctx, bn->n);
}

const struct assh_bignum_algo_s assh_bignum_builtin =
{
  .name = "builtin",
  .f_bytecode = assh_bignum_builtin_bytecode,
  .f_convert = assh_bignum_builtin_convert,
  .f_release = assh_bignum_builtin_release,
};

