#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_GNUTLS
# include <gnutls/abstract.h>
# include <gnutls/x509.h>
# include <gcrypt.h>
#endif /* ifdef HAVE_CIPHER */

#include <Eina.h>

#include "Emile.h"

#include "emile_private.h"

#define MAX_KEY_LEN   32
#define MAX_IV_LEN    16

# ifdef HAVE_GNUTLS
static inline Eina_Bool
emile_hmac_sha1(const void    *key,
                size_t         key_len,
                const void    *data,
                size_t         data_len,
                unsigned char *res)
{
   size_t hlen = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
   gcry_md_hd_t mdh;
   unsigned char *hash;
   gpg_error_t err;

   err = gcry_md_open(&mdh, GCRY_MD_SHA1, GCRY_MD_FLAG_HMAC);
   if (err != GPG_ERR_NO_ERROR)
     return EINA_FALSE;

   err = gcry_md_setkey(mdh, key, key_len);
   if (err != GPG_ERR_NO_ERROR)
     {
        gcry_md_close(mdh);
        return EINA_FALSE;
     }

   gcry_md_write(mdh, data, data_len);

   hash = gcry_md_read(mdh, GCRY_MD_SHA1);
   if (!hash)
     {
        gcry_md_close(mdh);
        return EINA_FALSE;
     }

   memcpy(res, hash, hlen);

   gcry_md_close(mdh);

   return EINA_TRUE;
}
# endif /* ifdef HAVE_GNUTLS */


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

        memcpy(buf, salt, salt_len);
        memcpy(buf + salt_len, tab, 4);
        if (!emile_hmac_sha1(key, key_len, buf, salt_len + 4, digest))
          return EINA_FALSE;
        memcpy(p, digest, tmp_len);

        for (j = 1; j < iter; j++)
          {
             if (!emile_hmac_sha1(key, key_len, digest, 20, digest))
               return EINA_FALSE;
             for (k = 0; k < tmp_len; k++)
               p[k] ^= digest[k];
          }
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
   /* Gcrypt declarations */
   gcry_error_t err = 0;
   gcry_cipher_hd_t cipher;

   if (!emile_cipher_init()) return NULL;

   /* Gcrypt salt generation */
   gcry_create_nonce((unsigned char *)&salt, sizeof(salt));

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
   eina_binbuf_append_length(result, (unsigned char*) &tmp, sizeof (tmp));
   eina_binbuf_append_buffer(result, data);

   while (eina_binbuf_length_get(result) < crypted_length)
     {
        int r;

        r = rand();
        eina_binbuf_append_length(result, (unsigned char*) &r, sizeof (r));
     }
   eina_binbuf_remove(result, crypted_length, eina_binbuf_length_get(result));

   /* Gcrypt create the corresponding cipher
      AES with a 256 bit key, Cipher Block Chaining mode */
   err = gcry_cipher_open(&cipher, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, 0);
   if (err) goto on_error;

   opened = 1;
   err = gcry_cipher_setiv(cipher, iv, MAX_IV_LEN);
   if (err) goto on_error;

   err = gcry_cipher_setkey(cipher, ik, MAX_KEY_LEN);
   if (err) goto on_error;

   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   pointer = (unsigned char*) eina_binbuf_string_get(result);

   /* Gcrypt encrypt */
   err = gcry_cipher_encrypt(cipher, pointer + sizeof (int),
                             eina_binbuf_length_get(result) - sizeof (int),
                             NULL, 0);
   if (err) goto on_error;

   /* Gcrypt close the cipher */
   gcry_cipher_close(cipher);

   return result;

on_error:
   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   /* Gcrypt error */
   if (opened)
     gcry_cipher_close(cipher);

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
   gcry_error_t err = 0;
   gcry_cipher_hd_t cipher;
   unsigned char ik[MAX_KEY_LEN];
   unsigned char iv[MAX_IV_LEN];
   unsigned char key_material[MAX_KEY_LEN + MAX_IV_LEN];
   unsigned int salt;
   unsigned int size;
   int tmp_len;
   int tmp = 0;

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

   /* Gcrypt create the corresponding cipher */
   err = gcry_cipher_open(&cipher, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, 0);
   if (err) goto on_error;

   err = gcry_cipher_setiv(cipher, iv, MAX_IV_LEN);
   if (err) goto on_error;

   err = gcry_cipher_setkey(cipher, ik, MAX_KEY_LEN);
   if (err) goto on_error;

   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   /* Gcrypt decrypt */
   err = gcry_cipher_decrypt(cipher,
                             (void*) eina_binbuf_string_get(result), tmp_len,
                             (void*) (over + 1), tmp_len);
   if (err) goto on_error;

   /* Gcrypt close the cipher */
   gcry_cipher_close(cipher);

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

   eina_binbuf_free(result);

   return NULL;
}
