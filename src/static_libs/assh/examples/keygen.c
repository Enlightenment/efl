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

#include <assh/assh_context.h>

#include <assh/key_rsa.h>
#include <assh/key_dsa.h>
#include <assh/key_eddsa.h>

#ifdef CONFIG_ASSH_USE_GCRYPT
# include <gcrypt.h>
#endif

int main(int argc, char *argv[])
{
#ifdef CONFIG_ASSH_USE_GCRYPT
  if (!gcry_check_version(GCRYPT_VERSION))
    return -1;
#endif

  size_t bits = 0;
  if (argc > 1)
    bits = atoi(argv[1]);

  const struct assh_key_s *key;

  struct assh_context_s *context;

  if (assh_context_create(&context, ASSH_SERVER, NULL, NULL))
    abort();

  if (context == NULL)
    abort();

  if (assh_context_prng(context, NULL))
    abort();

  if (assh_key_create(context, &key, bits, &assh_key_dsa, ASSH_ALGO_ANY))
    abort();

  if (assh_key_validate(context, key))
    abort();

  size_t len;

  if (key->algo->f_output(context, key, NULL, &len, ASSH_KEY_FMT_PV_OPENSSH_V1_KEY))
    abort();

  uint8_t blob[len];

  if (key->algo->f_output(context, key, blob, &len, ASSH_KEY_FMT_PV_OPENSSH_V1_KEY))
    abort();

  FILE *f = fopen("dsa_key", "wb");
  fwrite(blob, len, 1, f);
  fclose(f);

  assh_context_release(context);

  return 0;
}

