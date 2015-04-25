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

#include <assh/helper_key.h>
#include <assh/assh_context.h>
#include <assh/assh_packet.h>
#include <assh/assh_alloc.h>

#include <string.h>
#include <stdio.h>

struct assh_base64_ctx_s
{
  uint8_t *out, *out_start, *out_end;
  int in, pad;
  uint32_t x;
};

static void assh_base64_init(struct assh_base64_ctx_s *ctx, uint8_t *out,
			     size_t out_len)
{
  ctx->out_start = ctx->out = out;
  ctx->out_end = out + out_len;
  ctx->pad = ctx->in = 0;
  ctx->x = 0;
}

static assh_error_t assh_base64_update(struct assh_base64_ctx_s *ctx,
				       const uint8_t *b64, size_t b64_len)
{
  static const int8_t codes[128] =
    {
      -4, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,  /* blanks */
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,
      62, -1, -1, -1, 63,                              /* '+' and '/' */
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61,          /* '0' to '9'  */
      -1, -1, -1, -3, -1, -1, -1,                      /* '=' */
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,    /* A to Z */
      14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
      -1, -1, -1, -1, -1, -1,
      26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, /* a to z */
      39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
      -1, -1, -1, -1, -1
    };

  assh_error_t err;

  while (b64_len--)
    {
      int8_t x = *b64++, c = codes[(x | (x >> 7)) & 0x7f];
      switch (c)
        {
        case -1:
          ASSH_ERR_RET(ASSH_ERR_BAD_DATA);
        case -3:     /* padding char = */
          ASSH_CHK_RET(ctx->pad++ >= 2, ASSH_ERR_BAD_DATA);
        case -2:
          continue;  /* ignore blank chars */
        case -4:
	  return ASSH_OK;  /* NUL termination */
        default:
	  ASSH_CHK_RET(ctx->pad > 0, ASSH_ERR_BAD_DATA);
          ctx->x = (ctx->x << 6) | c;
	  if ((++ctx->in & 3) != 0)
	    continue;
	  ASSH_CHK_RET(ctx->out + 2 >= ctx->out_end, ASSH_ERR_OUTPUT_OVERFLOW);
	  *ctx->out++ = ctx->x >> 16;
	  *ctx->out++ = ctx->x >> 8;
	  *ctx->out++ = ctx->x;
	  ctx->x = 0;
        }
    }
  return ASSH_OK;
}

static assh_error_t assh_base64_final(struct assh_base64_ctx_s *ctx)
{
  assh_error_t err;

  ASSH_CHK_RET((ctx->in + ctx->pad) & 3, ASSH_ERR_BAD_DATA);

  ASSH_CHK_RET(ctx->out + ((2 - ctx->pad) % 2) >= ctx->out_end,
	       ASSH_ERR_OUTPUT_OVERFLOW);
  switch (ctx->pad)
    {
    case 2:
      *ctx->out++ = ctx->x >> 4;
      break;
    case 1:
      *ctx->out++ = ctx->x >> 10;
      *ctx->out++ = ctx->x >> 2;
    case 0:;
    }
  return ASSH_OK;
}

static inline size_t assh_base64_size(struct assh_base64_ctx_s *ctx)
{
  return ctx->out - ctx->out_start;
}

static assh_error_t assh_load_rfc4716(FILE *file, uint8_t *kdata, size_t *klen)
{
  struct assh_base64_ctx_s ctx;
  assh_error_t err;
  char in[80], *l;
  int state = 0;
  assh_base64_init(&ctx, kdata, *klen);

  while ((l = fgets(in, sizeof(in), file)))
    {
      size_t len = strlen(l);

      while (len && l[len - 1] <= ' ')
	l[--len] = '\0';
      if (!len)
	continue;

      switch (state)
	{
	case 0:
	  if (l[0] != '-' || !strstr(l, "BEGIN "))
	    continue;
	  state = 1;
	  continue;
	case 1:
	  state = 3;
	  if (!strchr(l, ':'))
	    break;
	case 2:
	  state = 1;
	  if (l[len - 1] == '\\')
	    state = 2;
	  continue;
	case 3:
	  if (l[0] != '-')
	    break;
	  ASSH_CHK_RET(!strstr(l, "END "), ASSH_ERR_BAD_DATA);
	  state = 0;
	  ASSH_ERR_RET(assh_base64_final(&ctx));
	  *klen = assh_base64_size(&ctx);
	  return ASSH_OK;
	}
      ASSH_ERR_RET(assh_base64_update(&ctx, (const uint8_t*)l, len));
    }

  ASSH_ERR_RET(ASSH_ERR_BAD_DATA);

  return ASSH_OK;
}

assh_error_t assh_load_key_file(struct assh_context_s *c,
				const struct assh_key_s **head,
				const struct assh_key_ops_s *algo,
				enum assh_algo_class_e role,
				FILE *file, enum assh_key_format_e format)
{
  assh_error_t err;
  size_t blob_len = 4096;

  ASSH_SCRATCH_ALLOC(c, uint8_t, blob, blob_len,
                     ASSH_ERRSV_CONTINUE, err_);

  switch (format)
    {
    case ASSH_KEY_FMT_PUB_RFC4716:
      assh_load_rfc4716(file, blob, &blob_len);
      format = ASSH_KEY_FMT_PUB_RFC4253_6_6;
      break;

    case ASSH_KEY_FMT_PV_RFC2440_PEM_ASN1:
      assh_load_rfc4716(file, blob, &blob_len);
      format = ASSH_KEY_FMT_PV_PEM_ASN1;
      break;

    case ASSH_KEY_FMT_OPENSSH_V1:
      assh_load_rfc4716(file, blob, &blob_len);
      format = ASSH_KEY_FMT_OPENSSH_V1_BLOB;
      break;

    default:
      blob_len = fread(blob, 1, blob_len, file);
      break;
    }

  ASSH_ERR_GTO(assh_key_load(c, head, algo, role, format, blob, blob_len), err_sc);

  err = ASSH_OK;

 err_sc:
  ASSH_SCRATCH_FREE(c, blob);
 err_:
  return err;
}

assh_error_t assh_load_key_filename(struct assh_context_s *c,
				    const struct assh_key_s **head,
				    const struct assh_key_ops_s *algo,
				    enum assh_algo_class_e role,
				    const char *filename,
				    enum assh_key_format_e format)
{
  assh_error_t err;

  FILE *file = fopen(filename, "r");
  ASSH_CHK_RET(file == NULL, ASSH_ERR_IO);

  ASSH_ERR_GTO(assh_load_key_file(c, head, algo, role, file, format), err_);

 err_:
  fclose(file);
  return err;
}

assh_error_t assh_load_hostkey_file(struct assh_context_s *c,
				    const struct assh_key_ops_s *algo,
				    enum assh_algo_class_e role,
				    FILE *file,
				    enum assh_key_format_e format)
{
#ifdef CONFIG_ASSH_SERVER
  if (c->type == ASSH_SERVER)
    return assh_load_key_file(c, &c->keys, algo, role, file, format);
#endif
  return ASSH_ERR_NOTSUP;
}

assh_error_t assh_load_hostkey_filename(struct assh_context_s *c,
					const struct assh_key_ops_s *algo,
					enum assh_algo_class_e role,
					const char *filename,
					enum assh_key_format_e format)
{
#ifdef CONFIG_ASSH_SERVER
  if (c->type == ASSH_SERVER)
    return assh_load_key_filename(c, &c->keys, algo, role, filename, format);
#endif
  return ASSH_ERR_NOTSUP;
}

