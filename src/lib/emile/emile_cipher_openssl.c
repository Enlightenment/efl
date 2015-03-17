#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_OPENSSL
# include <openssl/sha.h>
# include <openssl/evp.h>
# include <openssl/hmac.h>
# include <openssl/rand.h>
#endif /* ifdef HAVE_GNUTLS */

#include <Eina.h>

#include "Emile.h"

#include "emile_private.h"

#define MAX_KEY_LEN   EVP_MAX_KEY_LENGTH
#define MAX_IV_LEN    EVP_MAX_IV_LENGTH

Eina_Bool
_emile_cipher_init(void)
{
#ifdef HAVE_OPENSSL
   ERR_load_crypto_strings();
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
#endif /* ifdef HAVE_OPENSSL */

   return EINA_TRUE;
}

static Eina_Bool
emile_pbkdf2_sha1(const char          *key,
                  int                  key_len,
                  const unsigned char *salt,
                  unsigned int         salt_len,
                  int                  iter,
                  unsigned char       *res,
                  int                  res_len)
{
   unsigned char digest[20];
   unsigned char tab[4];
   unsigned char *p = res;
   unsigned char *buf;
   unsigned long i;
   int digest_len = 20;
   int len = res_len;
   int tmp_len;
   int j, k;
   HMAC_CTX hctx;

   buf = alloca(salt_len + 4);
   if (!buf) return EINA_FALSE;

   for (i = 1; len; len -= tmp_len, p += tmp_len, i++)
     {
        if (len > digest_len)
          tmp_len = digest_len;
        else
          tmp_len = len;

        tab[0] = (unsigned char)(i & 0xff000000) >> 24;
        tab[1] = (unsigned char)(i & 0x00ff0000) >> 16;
        tab[2] = (unsigned char)(i & 0x0000ff00) >> 8;
        tab[3] = (unsigned char)(i & 0x000000ff) >> 0;

        HMAC_Init(&hctx, key, key_len, EVP_sha1());
        HMAC_Update(&hctx, salt, salt_len);
        HMAC_Update(&hctx, tab, 4);
        HMAC_Final(&hctx, digest, NULL);
        memcpy(p, digest, tmp_len);

        for (j = 1; j < iter; j++)
          {
             HMAC(EVP_sha1(), key, key_len, digest, 20, digest, NULL);
             for (k = 0; k < tmp_len; k++)
               p[k] ^= digest[k];
          }
        HMAC_cleanup(&hctx);
     }

   return EINA_TRUE;
}

EAPI Eina_Binbuf *
emile_binbuf_cipher(const Eina_Binbuf *data,
                    const char *key,
                    unsigned int length)
{
   /* Cipher declarations */
   Eina_Binbuf *result;
   unsigned char *pointer;
   unsigned char iv[MAX_IV_LEN];
   unsigned char ik[MAX_KEY_LEN];
   unsigned char key_material[MAX_IV_LEN + MAX_KEY_LEN];
   unsigned int salt;
   unsigned int tmp = 0;
   unsigned int crypted_length;
   int opened = 0;
   /* Openssl declarations*/
   EVP_CIPHER_CTX ctx;
   unsigned int *buffer = NULL;
   int tmp_len;

   if (!emile_cipher_init()) return NULL;

   /* Openssl salt generation */
   if (!RAND_bytes((unsigned char *)&salt, sizeof (unsigned int)))
     return NULL;

   result = eina_binbuf_new();
   if (!result) return NULL;

   emile_pbkdf2_sha1(key,
                     length,
                     (unsigned char *)&salt,
                     sizeof(unsigned int),
                     2048,
                     key_material,
                     MAX_KEY_LEN + MAX_IV_LEN);

   memcpy(iv, key_material, MAX_IV_LEN);
   memcpy(ik, key_material + MAX_IV_LEN, MAX_KEY_LEN);

   memset(key_material, 0, sizeof (key_material));

   crypted_length = ((((eina_binbuf_length_get(data) + sizeof (unsigned int)) >> 5) + 1) << 5)
     + sizeof (unsigned int);

   eina_binbuf_append_length(result, (unsigned char*) &salt, sizeof (salt));
   memset(&salt, 0, sizeof (salt));

   tmp = htonl(eina_binbuf_length_get(data));
   buffer = malloc(crypted_length - sizeof (int));
   if (!buffer) goto on_error;
   *buffer = tmp;

   eina_binbuf_append_length(result,
                             (unsigned char *) buffer,
                             crypted_length - sizeof (int));
   memcpy(buffer + 1,
          eina_binbuf_string_get(data),
          eina_binbuf_length_get(data));

   /* Openssl create the corresponding cipher
      AES with a 256 bit key, Cipher Block Chaining mode */
   EVP_CIPHER_CTX_init(&ctx);
   if (!EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, ik, iv))
     goto on_error;

   opened = 1;

   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   pointer = (unsigned char*) eina_binbuf_string_get(result);

   /* Openssl encrypt */
   if (!EVP_EncryptUpdate(&ctx, pointer + sizeof (int), &tmp_len,
                          (unsigned char *)buffer,
                          eina_binbuf_length_get(data) + sizeof(unsigned int)))
     goto on_error;

   /* Openssl close the cipher */
   if (!EVP_EncryptFinal_ex(&ctx, pointer + sizeof (int) + tmp_len,
                            &tmp_len))
     goto on_error;

   EVP_CIPHER_CTX_cleanup(&ctx);
   free(buffer);

   return result;

on_error:
   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   /* Openssl error */
   if (opened)
     EVP_CIPHER_CTX_cleanup(&ctx);

   free(buffer);

   /* General error */
   eina_binbuf_free(result);

   return NULL;
}


EAPI Eina_Binbuf *
emile_binbuf_decipher(const Eina_Binbuf *data,
                      const char *key,
                      unsigned int length)
{
   Eina_Binbuf *result = NULL;
   unsigned int *over;
   EVP_CIPHER_CTX ctx;
   unsigned char ik[MAX_KEY_LEN];
   unsigned char iv[MAX_IV_LEN];
   unsigned char key_material[MAX_KEY_LEN + MAX_IV_LEN];
   unsigned int salt;
   unsigned int size;
   int tmp_len;
   int tmp = 0;
   int opened = 0;

   if (!emile_cipher_init()) return NULL;

   over = (unsigned int*) eina_binbuf_string_get(data);
   size = eina_binbuf_length_get(data);

   /* At least the salt and an AES block */
   if (size < sizeof(unsigned int) + 16)
     return NULL;

   /* Get the salt */
   salt = *over;

   /* Generate the iv and the key with the salt */
   emile_pbkdf2_sha1(key, length, (unsigned char *)&salt,
                     sizeof(unsigned int), 2048, key_material,
                     MAX_KEY_LEN + MAX_IV_LEN);

   memcpy(iv, key_material, MAX_IV_LEN);
   memcpy(ik, key_material + MAX_IV_LEN, MAX_KEY_LEN);

   memset(key_material, 0, sizeof (key_material));
   memset(&salt, 0, sizeof (salt));

   /* Align to AES block size if size is not align */
   tmp_len = size - sizeof (unsigned int);
   if ((tmp_len & 0x1F) != 0) goto on_error;

   result = eina_binbuf_new();
   if (!result) goto on_error;

   eina_binbuf_append_length(result, (unsigned char*) (over + 1), tmp_len);

   /* Openssl create the corresponding cipher */
   EVP_CIPHER_CTX_init(&ctx);
   opened = 1;

   if (!EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, ik, iv))
     goto on_error;

   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   /* Openssl decrypt */
   if (!EVP_DecryptUpdate(&ctx,
                          (void*) eina_binbuf_string_get(result), &tmp,
                          (void*) (over + 1), tmp_len))
     goto on_error;

   /* Openssl close the cipher*/
   EVP_CIPHER_CTX_cleanup(&ctx);

   /* Get the decrypted data size */
   tmp = *(unsigned int*)(eina_binbuf_string_get(result));
   tmp = ntohl(tmp);
   if (tmp > tmp_len || tmp <= 0)
     goto on_error;

   /* Remove header and padding  */
   eina_binbuf_remove(result, 0, sizeof (unsigned int));
   eina_binbuf_remove(result, tmp, eina_binbuf_length_get(result));

   return result;

on_error:
   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   if (opened)
     EVP_CIPHER_CTX_cleanup(&ctx);

   eina_binbuf_free(result);

   return NULL;
}
