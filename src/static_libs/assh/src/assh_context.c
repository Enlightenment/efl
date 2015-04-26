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

#warning write a perl script to check error RET/GTO order

#include <assh/assh_context.h>
#include <assh/assh_packet.h>
#include <assh/assh_algo.h>
#include <assh/assh_key.h>
#include <assh/assh_kex.h>
#include <assh/assh_prng.h>
#include <assh/assh_alloc.h>
#include <assh/assh_bignum.h>

#include <stdlib.h>

#ifdef CONFIG_ASSH_USE_GCRYPT
# include <gcrypt.h>
#endif

static ASSH_ALLOCATOR(assh_default_allocator)
{
  assh_error_t err;

#ifdef CONFIG_ASSH_USE_GCRYPT_ALLOCATOR
  if (size == 0)
    {
      gcry_free(*ptr);
      return ASSH_OK;
    }
  else if (*ptr == NULL)
    {
      switch (type)
	{
	case ASSH_ALLOC_INTERNAL:
	case ASSH_ALLOC_PACKET:
	  *ptr = gcry_malloc(size);
	  break;
	case ASSH_ALLOC_SECUR:
	case ASSH_ALLOC_SCRATCH:
	  *ptr = gcry_malloc_secure(size);
	  break;
	}
      ASSH_CHK_RET(*ptr == NULL, ASSH_ERR_MEM);
      return ASSH_OK;
    }
  else
    {
      *ptr = gcry_realloc(*ptr, size);
      ASSH_CHK_RET(*ptr == NULL, ASSH_ERR_MEM);
      return ASSH_OK;
    }
#else
# warning The default allocator relies on the standard non-secur realloc function
  *ptr = realloc(*ptr, size);
  ASSH_CHK_RET(size != 0 && *ptr == NULL, ASSH_ERR_MEM);
  return ASSH_OK;
#endif
}

void assh_context_init(struct assh_context_s *c,
                       enum assh_context_type_e type,
		       assh_allocator_t *alloc,
		       void *alloc_pv)
{
  c->session_count = 0;

  assert(
#ifdef CONFIG_ASSH_CLIENT
	 type == ASSH_CLIENT ||
#endif
#ifdef CONFIG_ASSH_SERVER
	 type == ASSH_SERVER ||
#endif
	 0);

  c->type = type;

  if (alloc == NULL)
    alloc = assh_default_allocator;
  c->f_alloc = alloc;
  c->alloc_pv = alloc_pv;

  c->prng = NULL;
  c->keys = NULL;
  c->kex_init_size = 0;

  c->algos_count = 0;

  int i;
  for (i = 0; i < ASSH_PCK_POOL_SIZE; i++)
    {
      c->pool[i].pck = NULL;
      c->pool[i].count = 0;
      c->pool[i].size = 0;
    }

  c->pck_pool_max_size = 1 << 20;
  c->pck_pool_max_bsize = c->pck_pool_max_size / ASSH_PCK_POOL_SIZE;
  c->pck_pool_size = 0;

  c->srvs_count = 0;

#ifdef CONFIG_ASSH_USE_GCRYPT_BIGNUM
  c->bignum = &assh_bignum_gcrypt;
#else
  c->bignum = &assh_bignum_builtin;
#endif
}

ASSH_WARN_UNUSED_RESULT assh_error_t
assh_context_create(struct assh_context_s **ctx,
		    enum assh_context_type_e type,
		    assh_allocator_t *alloc, void *alloc_pv)
{
  assh_error_t err;

  if (alloc == NULL)
    {
      alloc = assh_default_allocator;
      alloc_pv = NULL;
    }

  *ctx = NULL;
  ASSH_ERR_RET(alloc(alloc_pv, (void**)ctx, sizeof(struct assh_context_s), ASSH_ALLOC_INTERNAL));

  assh_context_init(*ctx, type, alloc, alloc_pv);

  return ASSH_OK;
}

void assh_context_release(struct assh_context_s *ctx)
{
  assh_context_cleanup(ctx);
  assh_free(ctx, ctx);
}

static void assh_pck_pool_cleanup(struct assh_context_s *c)
{
  int i;
  for (i = 0; i < ASSH_PCK_POOL_SIZE; i++)
    {
      struct assh_packet_s *n, *p;
      struct assh_packet_pool_s *pl = c->pool + i;

      for (p = pl->pck; p != NULL; p = n)
        {
          n = p->pool_next;
          pl->size -= p->alloc_size;
          pl->count--;
          assh_free(c, p);
        }

      assert(pl->count == 0);
      assert(pl->size == 0);
      pl->pck = NULL;
    }
}

void assh_context_cleanup(struct assh_context_s *c)
{
  assert(c->session_count == 0);

  assh_pck_pool_cleanup(c);

  assh_key_flush(c, &c->keys);

  if (c->prng != NULL)
    c->prng->f_cleanup(c);
}

assh_error_t assh_context_prng(struct assh_context_s *c,
			       const struct assh_prng_s *prng)
{
  assh_error_t err;

  if (prng == NULL)
    {
      if (c->prng != NULL)
	return ASSH_OK;
#ifdef CONFIG_ASSH_USE_GCRYPT_PRNG
      prng = &assh_prng_gcrypt;
#else
      prng = &assh_prng_xswap;
#endif
    }

  if (c->prng != NULL)
    c->prng->f_cleanup(c);

  c->prng = prng;
  ASSH_ERR_RET(prng->f_init(c));

  return ASSH_OK;
}

#ifdef CONFIG_ASSH_DEBUG

#include <stdio.h>

void assh_hexdump(const char *name, const void *data, unsigned int len)
{
  int i, j;
  const uint8_t *data_ = data;
  const int width = 32;

  fprintf(stderr, "--- %s (%u bytes) ---\n", name, len);
  for (i = 0; i < len; i += width)
    {
      for (j = 0; j < width && i + j < len; j++)
        fprintf(stderr, "%02x ", data_[i + j]);
      for (; j < width; j++)
        fputs("   ", stderr);
      for (j = 0; j < width && i + j < len; j++)
        fprintf(stderr, "%c", (unsigned)data_[i + j] - 32 < 96 ? data_[i + j] : '.');
      fputc('\n', stderr);
    }
  fputc('\n', stderr);
}

#endif

