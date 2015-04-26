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
#include <assh/assh_algo.h>
#include <assh/assh_kex.h>
#include <assh/assh_cipher.h>
#include <assh/assh_mac.h>
#include <assh/assh_sign.h>
#include <assh/assh_compress.h>

#include <string.h>
#include <stdarg.h>

static int assh_algo_order(const struct assh_algo_s *a,
			   const struct assh_algo_s *b,
			   unsigned int safety)
{
  if (a->class_ != b->class_)
    return a->class_ - b->class_;
  return ((b->speed * (99 - safety) + b->safety * safety) -
	  (a->speed * (99 - safety) + a->safety * safety));
}

static void assh_algo_udpate(struct assh_context_s *c,
			     unsigned int safety,
			     unsigned int min_safety)
{
  /* sort algorithms by class and safety/speed factor */
  int_fast16_t i, j, k;
  for (i = 0; i < c->algos_count; i++)
    {
      const struct assh_algo_s *a = c->algos[i];
      for (j = i - 1; j >= 0; j--)
	{
	  const struct assh_algo_s *b = c->algos[j];
	  if (assh_algo_order(a, b, safety) > 0)
	    break;
	  c->algos[j + 1] = b;
	}
      c->algos[j + 1] = a;
    }

  /* remove duplicated names in the same class */
  for (i = 0; i < c->algos_count; i++)
    {
      for (k = j = i + 1; j < c->algos_count; j++)
	{
	  const struct assh_algo_s *a = c->algos[i];
	  const struct assh_algo_s *b = c->algos[j];

	  int d = a->class_ != b->class_;
	  if (k < j)
	    c->algos[k] = b;
	  else if (d)
	    goto next;
	  if (d || strcmp(a->name, b->name))
	    k++;
	  else if (a->priority < b->priority)
	    ASSH_SWAP(c->algos[k], c->algos[i]);
	}
      c->algos_count = k;
    next:;
    }

  /* estimate size of the kex init packet */
  size_t kex_init_size = /* random cookie */ 16;
  enum assh_algo_class_e last = ASSH_ALGO_ANY;
  for (i = 0; i < c->algos_count; i++)
    {
      const struct assh_algo_s *a = c->algos[i];
      size_t l = 0;

      if (a->class_ == last)
	l++;	/* strlen(",") */
      else
	l += 4;	/* string header */
      l += strlen(a->name);
      switch (a->class_)
	{
	case ASSH_ALGO_KEX:
	case ASSH_ALGO_SIGN:
	  kex_init_size += l;
	  break;
	case ASSH_ALGO_CIPHER:
	case ASSH_ALGO_MAC:
	case ASSH_ALGO_COMPRESS:
	  kex_init_size += l * 2;
	default:
	  break;
        }
      last = a->class_;
    }
  kex_init_size += /* empty languages */ 4 * 2 + /* fkpf */ 1 + /* reserved */ 4;
  c->kex_init_size = kex_init_size;
}

assh_error_t assh_algo_register(struct assh_context_s *c, unsigned int safety,
				unsigned int min_safety, const struct assh_algo_s *table[])
{
  assh_error_t err = ASSH_OK;
  size_t i, count = c->algos_count;

  ASSH_CHK_RET(safety > 99, ASSH_ERR_BAD_ARG);

  for (i = 0; table[i] != NULL; i++)
    {
      const struct assh_algo_s *algo = table[i];
      if (algo->safety < min_safety)
	continue;
      ASSH_CHK_RET(count == ASSH_MAX_ALGORITHMS, ASSH_ERR_MEM);
      c->algos[count++] = algo;
    }

  c->algos_count = count;
  assh_algo_udpate(c, safety, min_safety);

  return ASSH_OK;
}

const struct assh_algo_s *
assh_algo_registered(struct assh_context_s *c, uint_fast16_t i)
{
  if (i >= c->algos_count)
    return NULL;
  return c->algos[i];
}

assh_error_t assh_algo_register_va(struct assh_context_s *c, unsigned int safety,
				   unsigned int min_safety, ...)
{
  assh_error_t err = ASSH_OK;
  va_list ap;
  size_t count = c->algos_count;

  ASSH_CHK_RET(safety > 99, ASSH_ERR_BAD_ARG);

  va_start(ap, min_safety);

  /* append algorithms to the array */
  while (1)
    {
      struct assh_algo_s *algo = va_arg(ap, void*);
      if (algo == NULL)
        break;
      if (algo->safety < min_safety)
	continue;
      ASSH_CHK_GTO(count == ASSH_MAX_ALGORITHMS, ASSH_ERR_MEM, err_);
      c->algos[count++] = algo;
    }

  c->algos_count = count;
  assh_algo_udpate(c, safety, min_safety);

 err_:
  va_end(ap);
  return err;
}

void assh_algo_unregister(struct assh_context_s *c)
{
  c->algos_count = 0;
}

const struct assh_algo_s *assh_algo_table[] = {
  /* kex */
  &assh_kex_curve25519_sha256.algo,
  &assh_kex_m383_sha384.algo,
  &assh_kex_m511_sha512.algo,
  &assh_kex_dh_group1_sha1.algo,
  &assh_kex_dh_group14_sha1.algo,
  &assh_kex_dh_gex_sha1.algo,
  &assh_kex_dh_gex_sha256_12.algo,
  &assh_kex_dh_gex_sha256_8.algo,
  &assh_kex_dh_gex_sha256_4.algo,
  &assh_kex_rsa1024_sha1.algo,
  &assh_kex_rsa2048_sha256.algo,
  /* sign */
  &assh_sign_dsa.algo,
  &assh_sign_dsa2048_sha224.algo,
  &assh_sign_dsa2048_sha256.algo,
  &assh_sign_dsa3072_sha256.algo,
  &assh_sign_rsa_sha1_md5.algo,
  &assh_sign_rsa_sha1.algo,
  &assh_sign_rsa_sha1_2048.algo,
  &assh_sign_rsa_sha256_2048.algo,
  &assh_sign_rsa_sha256_3072.algo,
  &assh_sign_ed25519.algo,
  &assh_sign_eddsa_e382.algo,
  &assh_sign_eddsa_e521.algo,
  /* ciphers */
#ifdef CONFIG_ASSH_USE_GCRYPT_CIPHERS
  &assh_cipher_tdes_cbc.algo,
  &assh_cipher_tdes_ctr.algo,
  &assh_cipher_cast128_cbc.algo,
  &assh_cipher_cast128_ctr.algo,
  &assh_cipher_blowfish_cbc.algo,
  &assh_cipher_blowfish_ctr.algo,
  &assh_cipher_twofish128_cbc.algo,
  &assh_cipher_twofish256_cbc.algo,
  &assh_cipher_twofish128_ctr.algo,
  &assh_cipher_twofish256_ctr.algo,
  &assh_cipher_serpent128_cbc.algo,
  &assh_cipher_serpent192_cbc.algo,
  &assh_cipher_serpent256_cbc.algo,
  &assh_cipher_serpent128_ctr.algo,
  &assh_cipher_serpent192_ctr.algo,
  &assh_cipher_serpent256_ctr.algo,
#endif
  &assh_cipher_arc4.algo,
  &assh_cipher_arc4_128.algo,
  &assh_cipher_arc4_256.algo,
  &assh_cipher_aes128_cbc.algo,
  &assh_cipher_aes192_cbc.algo,
  &assh_cipher_aes256_cbc.algo,
  &assh_cipher_aes128_ctr.algo,
  &assh_cipher_aes192_ctr.algo,
  &assh_cipher_aes256_ctr.algo,
  /* mac */
  &assh_hmac_md5.algo,
  &assh_hmac_md5_96.algo,
  &assh_hmac_sha1.algo,
  &assh_hmac_sha1_96.algo,
  &assh_hmac_sha256.algo,
  &assh_hmac_sha512.algo,
#ifdef CONFIG_ASSH_USE_GCRYPT_HASH
  &assh_hmac_ripemd160.algo,
#endif
  /* compress */
  &assh_compress_none.algo,
  NULL
};

assh_error_t assh_algo_by_name(struct assh_context_s *c,
			       enum assh_algo_class_e class_, const char *name,
			       size_t name_len, const struct assh_algo_s **algo)
{
  uint_fast16_t i;
  const struct assh_algo_s *a;

  for (i = 0; i < c->algos_count; i++)
    {
      a = c->algos[i];

      if (a->class_ == class_ &&
          !strncmp(name, a->name, name_len) && 
          a->name[name_len] == '\0')
	break;
    }

  if (i == c->algos_count)
    return ASSH_NOT_FOUND;

  *algo = a;
  return ASSH_OK;
}

assh_error_t assh_algo_by_key(struct assh_context_s *c,
			      const struct assh_key_s *key, uint_fast16_t *pos,
			      const struct assh_algo_s **algo)
{
  uint_fast16_t i = pos == NULL ? 0 : *pos;
  const struct assh_algo_s *a;

  for (; i < c->algos_count; i++)
    {
      a = c->algos[i];

      if (a->class_ == key->role &&
          a->f_suitable_key != NULL &&
	  a->f_suitable_key(c, a, key))
	break;
    }

  if (i >= c->algos_count)
    return ASSH_NOT_FOUND;

  if (pos != NULL)
    *pos = i;
  *algo = a;
  return ASSH_OK;
}

assh_bool_t
assh_algo_suitable_key(struct assh_context_s *c,
                       const struct assh_algo_s *algo,
                       const struct assh_key_s *key)
{
  if (algo->f_suitable_key == NULL)
    return 0;
  if (key != NULL &&
      key->role != algo->class_)
    return 0;
  return algo->f_suitable_key(c, algo, key);
}

