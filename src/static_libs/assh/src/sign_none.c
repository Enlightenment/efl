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

#include <assh/assh_sign.h>
#include <assh/assh_context.h>
#include <assh/assh_alloc.h>

#include <assert.h>

static ASSH_KEY_CLEANUP_FCN(assh_key_none_cleanup)
{
  assh_free(c, key);
}

static ASSH_KEY_OUTPUT_FCN(assh_key_none_output)
{
  assert(key->algo == &assh_key_none);

  *blob_len = 0;
  return ASSH_OK;
}

static ASSH_KEY_CMP_FCN(assh_key_none_cmp)
{
  assert(key->algo == &assh_key_none);
  return 1;
}

static ASSH_KEY_VALIDATE_FCN(assh_key_none_validate)
{
  assert(key->algo == &assh_key_none);
  return ASSH_OK;
}

static ASSH_KEY_LOAD_FCN(assh_key_none_load)
{
  assh_error_t err;

  ASSH_ERR_RET(assh_alloc(c, sizeof(**key), ASSH_ALLOC_SECUR, (void**)key));
  struct assh_key_s *k = *key;

  k->algo = &assh_key_none;

  return ASSH_OK;
}

static ASSH_KEY_CREATE_FCN(assh_key_none_create)
{
  assh_error_t err;

  ASSH_ERR_RET(assh_alloc(c, sizeof(**key), ASSH_ALLOC_SECUR, (void**)key));
  struct assh_key_s *k = *key;

  k->algo = &assh_key_none;

  return ASSH_OK;
}

const struct assh_key_ops_s assh_key_none =
{
  .type = "none",
  .f_output = assh_key_none_output,
  .f_validate = assh_key_none_validate,
  .f_cmp = assh_key_none_cmp,
  .f_load = assh_key_none_load,
  .f_create = assh_key_none_create,
  .f_cleanup = assh_key_none_cleanup,
};

static ASSH_SIGN_GENERATE_FCN(assh_sign_none_generate)
{
  *sign_len = 0;

  return ASSH_OK;
}

static ASSH_SIGN_CHECK_FCN(assh_sign_none_check)
{
  return ASSH_OK;
}

static ASSH_ALGO_SUITABLE_KEY_FCN(assh_sign_none_suitable_key)
{
  if (key == NULL)
    return c->type == ASSH_SERVER;
  return key->algo == &assh_key_none;
}

const struct assh_algo_sign_s assh_sign_none =
{
  .algo = {
    .name = "none@libassh.org", .class_ = ASSH_ALGO_SIGN,
    .safety = 0, .speed = 99,
    .f_suitable_key = assh_sign_none_suitable_key,
    .key = &assh_key_none,
  },
  .f_generate = assh_sign_none_generate,
  .f_check = assh_sign_none_check,
};

