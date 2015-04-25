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

#include <assh/assh_cipher.h>

struct assh_cipher_arc4_context_s
{
  uint8_t a, b;
  uint8_t s[256];
};

static void assh_arc4_set_key(struct assh_cipher_arc4_context_s *ctx,
			     const uint8_t *key, size_t klen)
{
  ctx->a = ctx->b = 0;
  unsigned int i;
  for (i = 0; i < 256; i++)
    ctx->s[i] = i;

  unsigned int a, b;
  for (a = b = 0; a < 256; a++)
    {
      uint8_t tmp = ctx->s[a];
      b = (uint8_t)(b + tmp + key[a % klen]);
      ctx->s[a] = ctx->s[b];
      ctx->s[b] = tmp;
    }
}

static void assh_arc4_run(struct assh_cipher_arc4_context_s *ctx,
			  uint8_t *data, size_t len)
{
  unsigned int a = ctx->a, b = ctx->b;

  while (len--)
    {
      a = (uint8_t)(a + 1);
      b = (uint8_t)(b + ctx->s[a]);
      uint8_t tmp = ctx->s[b];
      ctx->s[b] = ctx->s[a];
      ctx->s[a] = tmp;
      if (data != NULL)
	*data++ ^= ctx->s[(uint8_t)(ctx->s[a] + ctx->s[b])];
    }

  ctx->a = a;
  ctx->b = b;
}

static ASSH_CIPHER_INIT_FCN(assh_arc4_init)
{
  struct assh_cipher_arc4_context_s *ctx = ctx_;  
  assh_arc4_set_key(ctx, key, 16);
  return ASSH_OK;
}

static ASSH_CIPHER_INIT_FCN(assh_arc4_128_init)
{
  struct assh_cipher_arc4_context_s *ctx = ctx_;
  assh_arc4_set_key(ctx, key, 16);
  assh_arc4_run(ctx, NULL, 1536);
  return ASSH_OK;
}

static ASSH_CIPHER_INIT_FCN(assh_arc4_256_init)
{
  struct assh_cipher_arc4_context_s *ctx = ctx_;
  assh_arc4_set_key(ctx, key, 32);
  assh_arc4_run(ctx, NULL, 1536);
  return ASSH_OK;
}

static ASSH_CIPHER_PROCESS_FCN(assh_arc4_process)
{
  struct assh_cipher_arc4_context_s *ctx = ctx_;
  assh_arc4_run(ctx, data, len);
  return ASSH_OK;
}

static ASSH_CIPHER_CLEANUP_FCN(assh_arc4_cleanup)
{
}

const struct assh_algo_cipher_s assh_cipher_arc4 =
{
  .algo = { .name = "arcfour", .class_ = ASSH_ALGO_CIPHER, .safety = 5, .speed = 80 },
  .ctx_size = sizeof(struct assh_cipher_arc4_context_s),
  .block_size = 8,
  .key_size = 16,
  .is_stream = 1,
  .f_init = assh_arc4_init,
  .f_process = assh_arc4_process,
  .f_cleanup = assh_arc4_cleanup,
};

const struct assh_algo_cipher_s assh_cipher_arc4_128 =
{
  .algo = { .name = "arcfour128", .class_ = ASSH_ALGO_CIPHER, .safety = 10, .speed = 80 },
  .ctx_size = sizeof(struct assh_cipher_arc4_context_s),
  .block_size = 8,
  .key_size = 16,
  .is_stream = 1,
  .f_init = assh_arc4_128_init,
  .f_process = assh_arc4_process,
  .f_cleanup = assh_arc4_cleanup,
};

const struct assh_algo_cipher_s assh_cipher_arc4_256 =
{
  .algo = { .name = "arcfour256", .class_ = ASSH_ALGO_CIPHER, .safety = 15, .speed = 80 },
  .ctx_size = sizeof(struct assh_cipher_arc4_context_s),
  .block_size = 8,
  .key_size = 32,
  .is_stream = 1,
  .f_init = assh_arc4_256_init,
  .f_process = assh_arc4_process,
  .f_cleanup = assh_arc4_cleanup,
};

