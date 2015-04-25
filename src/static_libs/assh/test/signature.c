
#include <assh/assh_sign.h>
#include <assh/assh_context.h>
#include <assh/assh_prng.h>

#include <stdint.h>
#include "keys.h"
#include "prng_weak.h"

#ifdef CONFIG_ASSH_USE_GCRYPT
# include <gcrypt.h>
#endif

struct algo_s
{
  const struct assh_algo_sign_s *algo;
  const uint8_t *key;
  int gen_key;
  size_t kbits_min;
  size_t kbits_max;
  size_t key_len;
  size_t sign_len;
  const uint8_t *sign;
};

struct algo_s algos[] = {

  { &assh_sign_dsa,             dsa1024_key, 1, 1024, 4096, sizeof(dsa1024_key),
    55,  (const uint8_t *)
    "\x00\x00\x00\x07\x73\x73\x68\x2d\x64\x73\x73\x00\x00\x00\x28\x45"
    "\x81\x9e\x72\x7d\x7a\x34\xe1\xd6\x66\x6c\xc3\xe4\x65\xb8\x77\xa6"
    "\x37\x99\xc5\x66\x9f\x6e\x69\x07\x62\x21\x5c\x8c\x50\xe6\xe9\x18"
    "\x1d\x78\xba\x84\xe1\x27\x00" },

  { &assh_sign_rsa_sha1_md5,    rsa1024_key, 1, 1024, 1024, sizeof(rsa1024_key),
    143, (const uint8_t *)
    "\x00\x00\x00\x07\x73\x73\x68\x2d\x72\x73\x61\x00\x00\x00\x80\x4d"
    "\x18\xf5\xdf\xea\x0c\x3e\x15\xa0\x75\x50\x8b\x4f\x56\xa3\x3d\x6e"
    "\x93\xab\x03\x7e\x67\x33\xa6\xb0\xde\xc1\x5f\xf9\x3d\xb6\x5f\xc5"
    "\x46\x86\x4f\x02\x37\x86\x43\x45\xa9\x12\x12\xd7\x76\x0f\x3f\x49"
    "\xb8\xcf\xaf\x2e\x5e\x23\x00\xe3\xc4\x15\x73\x74\x3b\x51\x60\x41"
    "\xaf\xb8\xde\x32\x93\x11\x1f\xdf\xd3\xc6\x64\xec\x9f\x30\x5d\xd5"
    "\x1f\x36\x6d\xbc\xfb\xaa\x68\xe8\x9e\x2a\x23\x43\xc3\xa8\x9f\xf8"
    "\x76\xaa\xed\x93\x0c\xdb\x22\xac\x9c\xa7\x7a\x12\x53\xfe\x00\xef"
    "\x31\x8a\x44\x94\x4a\x53\x49\x8d\x2f\xd1\x38\x48\x30\x20\xae"
    "\x00\x00\x00\x07\x73\x73\x68\x2d" },

  { &assh_sign_rsa_sha256_2048, rsa2048_key, 0, 1024, 2048, sizeof(rsa2048_key),
    271, (const uint8_t *)
    "\x00\x00\x00\x07\x73\x73\x68\x2d\x72\x73\x61\x00\x00\x01\x00\x13"
    "\x54\xee\x4b\x3d\xf3\x7a\x81\xa0\x0f\x65\x8e\xc3\x91\x67\x46\x57"
    "\xe9\x5e\x1a\xe8\xee\xbe\x6f\xa4\xe7\xc3\xb5\xbc\xa6\xdd\xd6\x23"
    "\x79\xe5\x11\x52\x5e\x96\x4f\xe3\x46\x93\x39\xb0\xe7\x6e\x48\x2f"
    "\x6b\x8c\x8a\xc9\xca\xde\xe2\x1f\xe9\x91\x5e\x91\xd5\x82\x21\xe5"
    "\xe6\x8d\x56\x66\x7f\x5d\xb1\xa2\x45\x26\x9a\xed\xe9\xe3\xaf\x7b"
    "\x02\xe5\xd8\xd2\x25\x35\x57\x97\x70\xa1\x6e\xb6\x72\xff\x30\xbd"
    "\x86\xf3\xf1\x88\x06\xf1\x70\x4a\x85\x18\x84\xb2\xb8\x1f\x20\xc9"
    "\xf7\x26\x1f\xb4\x81\x19\xe0\xd2\xc1\xd8\x2a\x8f\xd1\x62\xe7\x01"
    "\x0e\xc0\x35\x92\x0b\x94\xc2\x9d\xdf\x50\x3b\x0c\x11\xa1\x1d\xcb"
    "\x80\x52\xe8\x09\x49\x39\xd6\x73\xcc\x3d\x77\xc5\xde\x24\x06\xbe"
    "\x8e\xf9\x09\x2b\x15\x60\x73\xc9\x47\xb5\x79\xaf\x2b\x39\x83\xf9"
    "\x23\x1b\xd2\xa9\xf6\x36\x41\xb3\x97\xcc\x48\xf0\x62\x4d\x73\xb1"
    "\x59\x12\xba\x9d\x20\x45\xe1\x0a\xa1\x83\x0a\x2b\x33\x7f\x90\x95"
    "\xa3\x56\xc7\xa9\xbd\x07\xa2\x8f\x81\x5b\x39\x72\x75\x08\x45\x06"
    "\xfb\xd5\x06\x1c\xd3\xb1\x8f\xd8\x05\xcc\x5d\xcb\xe9\x9b\x32\x75"
    "\xed\x72\xb4\xb4\xb1\xb9\xe1\xa3\x99\xb2\x09\xef\xed\x1a\x91" },

  { &assh_sign_ed25519,         ed25519_key, 1, 255, 255, sizeof(ed25519_key),
    4 + 11 + 4 + 2 * 32, (const uint8_t *)
    "\x00\x00\x00\x0b\x73\x73\x68\x2d\x65\x64\x32\x35\x35\x31\x39\x00"
    "\x00\x00\x40\xde\xd8\x65\x27\x76\xb7\x37\x8e\x1d\xed\x4a\x87\xef"
    "\x6e\x4e\xf8\x5d\x2b\xa7\xd9\x02\xfd\x22\x29\x27\xd4\x3c\xf7\x5c"
    "\x6f\x5a\x28\xb8\xc6\x86\xde\x39\xd6\x73\xfc\xe9\x34\x28\xaa\xa5"
    "\x41\xe9\x03\x46\x63\x9f\xe6\x25\xef\xc5\xcd\x8b\x92\xd8\xa5\x67"
    "\x0a\xe6\x05" },

  { &assh_sign_eddsa_e382,      eddsa_e382_key, 1, 382, 382, sizeof(eddsa_e382_key),
    4 + 31 + 4 + 2 * 48, (const uint8_t *)
    "\x00\x00\x00\x1f\x65\x64\x64\x73\x61\x2d\x65\x33\x38\x32\x2d\x73"
    "\x68\x61\x6b\x65\x32\x35\x36\x40\x6c\x69\x62\x61\x73\x73\x68\x2e"
    "\x6f\x72\x67\x00\x00\x00\x60\x01\x50\xfa\xd7\xb4\xa5\x35\x43\xd9"
    "\x0f\x34\xdf\xea\xb8\x8f\xce\x3b\xa9\x3b\x25\xbf\x03\xa7\xa0\x53"
    "\x47\xbc\x5c\x7a\xa0\xa2\xc1\x8e\x1d\x27\x01\xcc\xbe\xab\xdd\x62"
    "\xfd\xcc\x9b\xd5\xf9\x62\x97\xa4\x5f\xc8\xc9\x53\x73\x9f\xd1\xc2"
    "\x8b\xd9\x27\x75\x06\xbb\xbe\x83\x88\xfa\xe8\xe4\xb1\xd6\x71\x9c"
    "\xf8\x16\x45\x7a\x41\xc6\x92\x9f\x8c\xe5\xdb\x1d\xae\x6b\x39\x53"
    "\xce\x33\xe4\x13\xb0\x34\x06" },

  { &assh_sign_eddsa_e521,      eddsa_e521_key, 1, 521, 521, sizeof(eddsa_e521_key),
    4 + 31 + 4 + 2 * 66, (const uint8_t *)
    "\x00\x00\x00\x1f\x65\x64\x64\x73\x61\x2d\x65\x35\x32\x31\x2d\x73"
    "\x68\x61\x6b\x65\x32\x35\x36\x40\x6c\x69\x62\x61\x73\x73\x68\x2e"
    "\x6f\x72\x67\x00\x00\x00\x84\xe3\x59\x17\x34\xfd\xae\x8c\x83\xa7"
    "\x78\x91\x20\xa0\xf9\xd6\x08\x48\xe1\x07\xe9\xfb\x47\x4a\x9c\x72"
    "\x23\xd6\x27\xe3\x76\x92\xd1\xee\xa8\x74\xb0\x38\x55\xc4\xcb\xa5"
    "\xec\xc4\x32\x94\x11\xd1\x76\x23\xfc\x42\xbf\xa5\x35\x5c\x05\x44"
    "\xa6\x3c\x63\x04\x1e\x57\x4e\x1b\x01\xc1\xd6\x08\x3c\x50\x7d\x27"
    "\x0b\x86\x5b\xbd\x4a\x58\x50\x91\x4f\x15\xf1\xde\xe4\xf2\x52\xac"
    "\x06\x3a\x1f\x46\xee\x89\x43\x1a\xd5\x21\x06\x28\xbd\xe9\xa8\xce"
    "\x2d\x2e\x01\x03\x50\x27\x8a\x02\x92\x68\x6b\xbe\xbf\xfa\x57\x68"
    "\x04\xd3\x3a\x8e\x6f\x71\x60\x27\x8d\x4e\x00" },

  { NULL },
};

#define TEST_SIZE 128

#warning test key create too

struct assh_context_s context;

assh_error_t test_const()
{
  assh_error_t err;
  int i, j;

  for (i = 0; algos[i].algo; i++)
    {
      const struct assh_algo_sign_s *a = algos[i].algo;
      const struct assh_key_s *key;

      fprintf(stderr, "\n%s const sign/verify: ", a->algo.name);

      uint8_t key_blob[algos[i].key_len];
      memcpy(key_blob, algos[i].key, sizeof(key_blob));

      fprintf(stderr, "L");
      ASSH_ERR_RET(assh_key_load(&context, &key, a->algo.key, ASSH_ALGO_SIGN,
		 key_blob[0], key_blob + 1, sizeof(key_blob) - 1));

      size_t sign_len;

      uint8_t data[11 + 27 + 33];
      const uint8_t * ptr[3] = { data, data + 11, data + 11 + 27 };
      size_t sz[3] = { 11, 27, 33 };
      for (j = 0; j < sizeof(data); j++)
	data[j] = j;

      fprintf(stderr, "g");
      ASSH_ERR_RET(assh_sign_generate(&context, a, key, 3, ptr, sz, NULL, &sign_len));

      if (sign_len != algos[i].sign_len)
	{
	  fprintf(stderr, "expected len %zu\n", algos[i].sign_len);
	  fprintf(stderr, "wrong %zu\n", sign_len);
	  abort();
	}

      uint8_t sign[sign_len];
      ASSH_ERR_RET(assh_sign_generate(&context, a, key, 3, ptr, sz, sign, &sign_len));

      if (memcmp(algos[i].sign, sign, sign_len))
	{
	  assh_hexdump("expected", algos[i].sign, sign_len);
	  assh_hexdump("wrong", sign, sign_len);
	  abort();
	}

      fprintf(stderr, "v");
      if (assh_sign_check(&context, a, key, 3, ptr, sz, sign, sign_len))
	abort();

      data[0]++;

      fprintf(stderr, "V");
      if (!assh_sign_check(&context, a, key, 3, ptr, sz, sign, sign_len))
	abort();
    }

  return ASSH_OK;
}

assh_error_t test_loop()
{
  assh_error_t err;
  int i;

  for (i = 0; algos[i].algo; i++)
    {
      const struct assh_algo_sign_s *a = algos[i].algo;
      const struct assh_key_s *key;

      fprintf(stderr, "\n%s sign/verify: ", a->algo.name);

      uint8_t key_blob[algos[i].key_len];
      memcpy(key_blob, algos[i].key, sizeof(key_blob));

      if (!algos[i].gen_key)
	{
	  fprintf(stderr, "L");
	  ASSH_ERR_RET(assh_key_load(&context, &key, a->algo.key, ASSH_ALGO_SIGN,
	                 key_blob[0], key_blob + 1, sizeof(key_blob) - 1));
	  ASSH_ERR_RET(assh_key_validate(&context, key));
	}

      int size;
      for (size = TEST_SIZE; size != 0; )
	{
	  if (algos[i].gen_key)
	    {
	      size_t kbits = algos[i].kbits_min + rand()
                           % (algos[i].kbits_max - algos[i].kbits_min + 1);
              fprintf(stderr, "N");
	      ASSH_ERR_RET(assh_key_create(&context, &key, kbits,
	                    a->algo.key, ASSH_ALGO_SIGN));
              fprintf(stderr, "C");
	      ASSH_ERR_RET(assh_key_validate(&context, key));
            }

	  size--;
	  uint8_t data[size];
	  ASSH_ERR_RET(context.prng->f_get(&context, data, size,
                                           ASSH_PRNG_QUALITY_WEAK));

	  const uint8_t * ptr[8];
	  size_t sz[8];
	  int c = 0;
	  int s = 0;
	  while (s < size)
	    {
	      int r = rand() % 128 + 128;
	      if (s + r > TEST_SIZE)
		r = TEST_SIZE - s;
	      ptr[c] = data + s;
	      sz[c] = r;
	      s += r;
              c++;
	    }

	  size_t sign_len;

          fprintf(stderr, "g");

	  ASSH_ERR_RET(assh_sign_generate(&context, a, key, c, ptr, sz, NULL, &sign_len));
	  assert(sign_len > 0);

	  uint8_t sign[sign_len];
	  ASSH_ERR_RET(assh_sign_generate(&context, a, key, c, ptr, sz, sign, &sign_len));

          fprintf(stderr, "v");

	  err = assh_sign_check(&context, a, key, c, ptr, sz, sign, sign_len);
	  assert(err == ASSH_OK);

          unsigned int r1 = rand() % sign_len;
          unsigned char r2 = rand();
          if (!r2)
            r2++;
#ifdef CONFIG_ASSH_DEBUG_SIGN
          fprintf(stderr, "Mangling signature byte %u, previous=0x%02x, new=0x%02x\n",
                    r1, sign[r1], sign[r1] ^ r2);
#endif
	  sign[r1] ^= r2;

          fprintf(stderr, "V");

	  err = assh_sign_check(&context, a, key, c, ptr, sz, sign, sign_len);
	  assert(err != ASSH_OK);

	  sign[r1] ^= r2;

	  err = assh_sign_check(&context, a, key, c, ptr, sz, sign, sign_len);
	  assert(err == ASSH_OK);

	  if (size)
	    {
	      r1 = rand() % size;

#ifdef CONFIG_ASSH_DEBUG_SIGN
	      fprintf(stderr, "Mangling data byte %u, previous=0x%02x, new=0x%02x\n",
	                r1, data[r1], data[r1] ^ r2);
#endif
	      data[r1] ^= r2;

	      err = assh_sign_check(&context, a, key, c, ptr, sz, sign, sign_len);
	      assert(err != ASSH_OK);
	    }

	  if (algos[i].gen_key)
	    assh_key_drop(&context, &key);
	}

      if (!algos[i].gen_key)
	assh_key_drop(&context, &key);
    }

  for (i = 0; algos[i].algo; i++)
    {
      const struct assh_algo_sign_s *a = algos[i].algo;
      const struct assh_key_s *key;

      fprintf(stderr, "\n%s key load/validate: ", a->algo.name);

      uint8_t key_blob[algos[i].key_len];

      /* test key loading and validation */
      int j;
      for (j = 0; j < TEST_SIZE; j++)
	{
          memcpy(key_blob, algos[i].key, sizeof(key_blob));
	  int bad = j > 0;

	  if (bad)		/* mangle key blob on odd iterations */
	    {
	      unsigned int r1 = rand() % sizeof(key_blob);
	      unsigned char r2 = rand();
	      if (!r2)
		r2++;
#ifdef CONFIG_ASSH_DEBUG_SIGN
	      fprintf(stderr, "Mangling key byte %u, previous=0x%02x, new=0x%02x\n",
                        r1, key_blob[r1], key_blob[r1] ^ r2);
#endif
	      key_blob[r1] ^= r2;
	      fprintf(stderr, "B");
	    }
	  else
	    {
	      fprintf(stderr, "G");
	    }

	  fprintf(stderr, "l");
	  err = assh_key_load(&context, &key, a->algo.key, ASSH_ALGO_SIGN,
			      key_blob[0], key_blob + 1, sizeof(key_blob) - 1);

	  if (!bad)
	    assert(err == ASSH_OK);
	  else if (err != ASSH_OK)
	    continue;

	  fprintf(stderr, "C");

	  err = assh_key_validate(&context, key);

	  assert(bad || (err == ASSH_OK));
	  //	  assert(!bad || (err != ASSH_OK));

          assh_key_drop(&context, &key);
        }
    }

  return ASSH_OK;
}

int main(int argc, char **argv)
{
  assh_error_t err;
  int i;

#ifdef CONFIG_ASSH_USE_GCRYPT
  if (!gcry_check_version(GCRYPT_VERSION))
    return -1;
#endif

  assh_context_init(&context, ASSH_SERVER);
  ASSH_ERR_RET(assh_context_prng(&context, &assh_prng_weak));

  for (i = 0; algos[i].algo; i++)
    ASSH_ERR_RET(assh_algo_register_va(&context, 0, 0, algos[i].algo, NULL));

  int t = 0;
  srand(t);
  fprintf(stderr, "\nSeed: %u\n", t);

  if (test_const())
    return 1;

  if (test_loop())
    return 1;

  fprintf(stderr, "\nDone.\n");
  return 0;
}

