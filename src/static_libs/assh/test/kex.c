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

#include <assh/assh_session.h>
#include <assh/assh_context.h>
#include <assh/assh_kex.h>
#include <assh/assh_cipher.h>
#include <assh/assh_sign.h>
#include <assh/assh_mac.h>
#include <assh/assh_prng.h>
#include <assh/assh_compress.h>
#include <assh/assh_transport.h>
#include <assh/assh_connection.h>
#include <assh/assh_service.h>
#include <assh/assh_event.h>

#include <assh/key_rsa.h>
#include <assh/key_dsa.h>
#include <assh/key_eddsa.h>

#include "leaks_check.h"
#include "fifo.h"
#include "keys.h"

#ifdef CONFIG_ASSH_USE_GCRYPT
# include <gcrypt.h>
#endif

struct fifo_s fifo[2];
struct assh_context_s context[2];
struct assh_session_s session[2];

struct algo_with_key_s
{
  const void *algo;
  const struct assh_key_ops_s *key_algo;
  const uint8_t *key_blob;
  size_t key_size;
};

static const struct algo_with_key_s kex_list[] = 
  {
    { &assh_kex_none,              NULL, NULL, 0 },
    { &assh_kex_curve25519_sha256, NULL, NULL, 0 },
    { &assh_kex_m383_sha384,	   NULL, NULL, 0 },
    { &assh_kex_m511_sha512,	   NULL, NULL, 0 },
    { &assh_kex_dh_group1_sha1,	   NULL, NULL, 0 },
    { &assh_kex_dh_group14_sha1,   NULL, NULL, 0 },
    { &assh_kex_dh_gex_sha1,	   NULL, NULL, 0 },
    { &assh_kex_dh_gex_sha256_12,  NULL, NULL, 0 },
    { &assh_kex_dh_gex_sha256_8,   NULL, NULL, 0 },
    //    { &assh_kex_dh_gex_sha256_4,   NULL, NULL, 0 },
    { &assh_kex_rsa1024_sha1,	   &assh_key_rsa, rsa1024_key, sizeof(rsa1024_key) },
    { &assh_kex_rsa2048_sha256,	   &assh_key_rsa, rsa2048_key, sizeof(rsa2048_key) },
    { NULL },
  };

static const struct algo_with_key_s sign_list[] =
  {
    { &assh_sign_none,              &assh_key_none, (const uint8_t*)"\0", 0 },
    { &assh_sign_dsa,               &assh_key_dsa, dsa1024_key, sizeof(dsa1024_key) - 1 },
    //    { &assh_sign_dsa2048_sha224,    &assh_key_dsa, dsa2048_key, sizeof(dsa2048_key) },
    { &assh_sign_dsa2048_sha256,    &assh_key_dsa, dsa2048_key, sizeof(dsa2048_key) - 1 },
    { &assh_sign_dsa3072_sha256,    &assh_key_dsa, dsa3072_key, sizeof(dsa3072_key) - 1 },
    { &assh_sign_rsa_sha1_md5,      &assh_key_rsa, rsa1024_key, sizeof(rsa1024_key) - 1 },
    { &assh_sign_rsa_sha1,          &assh_key_rsa, rsa1024_key, sizeof(rsa1024_key) - 1 },
    { &assh_sign_rsa_sha1_2048,     &assh_key_rsa, rsa2048_key, sizeof(rsa2048_key) - 1 },
    { &assh_sign_rsa_sha256_2048,   &assh_key_rsa, rsa2048_key, sizeof(rsa2048_key) - 1 },
    { &assh_sign_rsa_sha256_3072,   &assh_key_rsa, rsa3072_key, sizeof(rsa3072_key) - 1 },
    { &assh_sign_ed25519,           &assh_key_ed25519, ed25519_key, sizeof(ed25519_key) - 1 },
    { NULL }
  };

static const struct assh_algo_cipher_s *cipher_list[] =
  {
    &assh_cipher_none,
    &assh_cipher_arc4,
    &assh_cipher_aes128_cbc,
    &assh_cipher_aes256_ctr,
    NULL
  };

static const struct assh_algo_mac_s *mac_list[] =
  {
    &assh_hmac_none,
    &assh_hmac_md5,
    NULL
  };

static const struct assh_algo_compress_s *comp_list[] =
  {
    &assh_compress_none,
    NULL
  };

#define TEST_FAIL(...)				\
  do {						\
    fprintf(stderr, "FAIL " __VA_ARGS__);	\
    return 1;					\
  } while (0)

int test(const struct assh_algo_kex_s *kex,
 	 const struct assh_algo_sign_s *sign,
 	 const struct assh_algo_cipher_s *cipher,
 	 const struct assh_algo_mac_s *mac,
 	 const struct assh_algo_compress_s *comp,
	 const struct algo_with_key_s *kex_key,
	 const struct algo_with_key_s *sign_key)
{
  assh_context_init(&context[0], ASSH_SERVER);
  assh_context_init(&context[1], ASSH_CLIENT);
  uint_fast8_t i;

  fprintf(stderr, "==== %s, %s, %s, %s, %s ====\n",
	  kex->algo.name, sign->algo.name, cipher->algo.name,
	  mac->algo.name, comp->algo.name);

  for (i = 0; i < 2; i++)
    {
      struct assh_context_s *c = &context[i];

      fifo_init(&fifo[i]);

      assh_context_allocator(c, assh_leaks_allocator, NULL);

      if (assh_service_register_va(c, &assh_service_connection, NULL))
	return -1;

      if (assh_algo_register_va(c, 0, 0, kex, sign, cipher, mac, comp, NULL) != ASSH_OK)
	return -1;

      if (i == 0 && sign_key->key_algo != NULL)
	if (assh_key_load(c, &c->keys,
			  sign_key->key_algo, ASSH_ALGO_SIGN, sign_key->key_blob[0],
			  sign_key->key_blob + 1, sign_key->key_size))
	  return -1;

      if (i == 0 && kex_key->key_algo != NULL)
	if (assh_key_load(c, &c->keys,
			  kex_key->key_algo, ASSH_ALGO_KEX, kex_key->key_blob[0],
			  kex_key->key_blob + 1, kex_key->key_size))
	  return -1;

      if (assh_session_init(c, &session[i]) != ASSH_OK)
	return -1;
    }

  while (1) 
    {
      for (i = 0; i < 2; i++)
	{
	  struct assh_event_s event;
	  assh_error_t err;

	  err = assh_event_get(&session[i], &event);
	  if (err != ASSH_OK)
	    return 1;

	  switch (event.id)
	    {
	    case ASSH_EVENT_KEX_HOSTKEY_LOOKUP:
	      event.kex.hostkey_lookup.accept = 1;
	      break;

	    case ASSH_EVENT_READ: {
	      struct assh_event_transport_read_s *te = &event.transport.read;
	      te->transferred = fifo_read(&fifo[i], te->buf.data,
					  te->buf.size % (rand() % FIFO_BUF_SIZE + 1));
	      break;
	    }

	    case ASSH_EVENT_WRITE: {
	      struct assh_event_transport_write_s *te = &event.transport.write;
	      te->transferred = fifo_write(&fifo[i ^ 1], te->buf.data,
					   te->buf.size % (rand() % FIFO_BUF_SIZE + 1));
	      break;	    
	    }

	    case ASSH_EVENT_CONNECTION_START:
	      assh_session_cleanup(&session[0]);
	      assh_context_cleanup(&context[0]);
	      assh_session_cleanup(&session[1]);
	      assh_context_cleanup(&context[1]);

	      if (alloc_size != 0)
		TEST_FAIL("memory leak detected, %zu bytes allocated\n", alloc_size);

	      return 0;

	    default:
	      ASSH_DEBUG("event %u\n", event.id);
	    }

	  err = assh_event_done(&session[i], &event);
	  if (err != ASSH_OK)
	    return 1;
	}
    }
}

int main()
{
  const struct algo_with_key_s *kex = kex_list;
  const struct algo_with_key_s *sign = sign_list;
  const struct assh_algo_cipher_s **cipher = cipher_list;
  const struct assh_algo_mac_s **mac = mac_list;
  const struct assh_algo_compress_s **comp = comp_list;

  while (*mac)
    {
      while (*cipher)
	{
	  while (sign->algo)
	    {
	      while (kex->algo)
		{
		  if (test(kex->algo, sign->algo, *cipher, *mac, *comp,
			   kex, sign))
		    return -1;
		  kex++;
		}
	      kex = kex_list;
	      sign++;
	    }
	  sign = sign_list;
	  cipher++;
	}
      cipher = cipher_list;
      mac++;
    }

  return 0;
}

