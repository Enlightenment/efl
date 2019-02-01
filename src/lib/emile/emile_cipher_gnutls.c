#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gnutls/abstract.h>
#include <gnutls/x509.h>
#include <gcrypt.h>

#include <Eina.h>

#include "Emile.h"

#include "emile_private.h"

#define MAX_KEY_LEN   32
#define MAX_IV_LEN    16

struct _Emile_SSL
{
   const char *last_error;
   const char *cert_file;
   const char *name;

   gnutls_certificate_credentials_t cert;
   gnutls_session_t session;

   union {
      struct {
         gnutls_datum_t session_ticket;
      } client;
      struct {
         gnutls_anon_client_credentials_t anoncred_c;
         gnutls_anon_server_credentials_t anoncred_s;
         gnutls_psk_client_credentials_t pskcred_c;
         gnutls_psk_server_credentials_t pskcred_s;
         char *cert_file;
         gnutls_dh_params_t dh_params;
      } server;
   } u;

   Emile_Cipher_Type t;
   Emile_SSL_State ssl_state;

   Eina_Bool server : 1;
   Eina_Bool verify : 1;
   Eina_Bool verify_basic : 1;
};

GCRY_THREAD_OPTION_PTHREAD_IMPL;

Eina_Bool
_emile_cipher_init(void)
{
   if (gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread))
     WRN("YOU ARE USING PTHREADS, "
         "BUT I CANNOT INITIALIZE THREADSAFE GCRYPT OPERATIONS!");

   /* Before the library can be used, it must initialize itself if needed. */
   if (gcry_control(GCRYCTL_ANY_INITIALIZATION_P) == 0)
     {
        gcry_check_version(NULL);
        /* Disable warning messages about problems with the secure memory subsystem.
           This command should be run right after gcry_check_version. */
        if (gcry_control(GCRYCTL_DISABLE_SECMEM_WARN))
          return EINA_FALSE;

        /* This command is used to allocate a pool of secure memory and thus
           enabling the use of secure memory. It also drops all extra privileges the
           process has (i.e. if it is run as setuid (root)). If the argument nbytes
           is 0, secure memory will be disabled. The minimum amount of secure memory
           allocated is currently 16384 bytes; you may thus use a value of 1 to
           request that default size. */
        if (gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0))
          WRN("BIG FAT WARNING: I AM UNABLE TO REQUEST SECMEM, "
              "Cryptographic operation are at risk !");
     }

   if (gnutls_global_init())
     return EINA_FALSE;

   return EINA_TRUE;
}

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

EAPI Eina_Bool
emile_binbuf_hmac_sha1(const char *key,
                       unsigned int key_len,
                       const Eina_Binbuf *data,
                       unsigned char digest[20])
{
   return emile_hmac_sha1(key, key_len,
                          eina_binbuf_string_get(data), eina_binbuf_length_get(data),
                          digest);
}

static inline Eina_Bool
emile_sha1(const void    *data,
           size_t         data_len,
           unsigned char *res)
{
   size_t hlen = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
   gcry_md_hd_t mdh;
   unsigned char *hash;
   gpg_error_t err;

   err = gcry_md_open(&mdh, GCRY_MD_SHA1, 0);
   if (err != GPG_ERR_NO_ERROR)
     return EINA_FALSE;

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

EAPI Eina_Bool
emile_binbuf_sha1(const Eina_Binbuf * data, unsigned char digest[20])
{
   Eina_Slice slice = eina_binbuf_slice_get(data);
   return emile_sha1(slice.mem, slice.len, digest);
}

EAPI Eina_Binbuf *
emile_binbuf_cipher(Emile_Cipher_Algorithm algo,
                    const Eina_Binbuf *data,
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

   if (algo != EMILE_AES256_CBC) return NULL;
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

   tmp = eina_htonl(eina_binbuf_length_get(data));
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
emile_binbuf_decipher(Emile_Cipher_Algorithm algo,
                      const Eina_Binbuf *data,
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

   if (algo != EMILE_AES256_CBC) return NULL;
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
   tmp = eina_ntohl(tmp);
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

// FIXME: handshaking and fun


EAPI Emile_SSL *
emile_cipher_server_listen(Emile_Cipher_Type t)
{
   Emile_SSL *r;
   int ret;

   if (t != EMILE_SSLv23 &&
       t != EMILE_TLSv1)
     return NULL;

   r = calloc(1, sizeof (Emile_SSL));
   if (!r) return NULL;

   ret = gnutls_certificate_allocate_credentials(&r->cert);
   if (ret) goto on_error;

   r->t = t;
   r->server = EINA_TRUE;

   return r;

 on_error:
   ERR("GNUTLS error: %s - %s.",
       gnutls_strerror_name(ret),
       gnutls_strerror(ret));
   emile_cipher_free(r);
   return NULL;
}

EAPI Emile_SSL *
emile_cipher_client_connect(Emile_SSL *server EINA_UNUSED, int fd EINA_UNUSED)
{
   return NULL;
}

EAPI Emile_SSL *
emile_cipher_server_connect(Emile_Cipher_Type t)
{
   const char *priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT";
   Emile_SSL *r;
   int ret;

   switch (t)
     {
      case EMILE_SSLv23:
         break;
      case EMILE_TLSv1:
         priority = "NORMAL:%VERIFY_ALLOW_X509_V1_CA_CRT:!VERS-SSL3.0";
         break;
      default:
         return NULL;
     }

   r = calloc(1, sizeof (Emile_SSL));
   if (!r) return NULL;

   r->server = EINA_FALSE;

   ret = gnutls_certificate_allocate_credentials(&r->cert);
   if (ret) goto on_error;

   ret = gnutls_init(&r->session, GNUTLS_CLIENT);
   if (ret) goto on_error;

   ret = gnutls_session_ticket_enable_client(r->session);
   if (ret) goto on_error;

   // FIXME: Delay that until later access

   ret = gnutls_server_name_set(r->session, GNUTLS_NAME_DNS,
                                r->name, strlen(r->name));
   if (ret) goto on_error;

   ret = gnutls_priority_set_direct(r->session, priority, NULL);
   if (ret) goto on_error;

   gnutls_handshake_set_private_extensions(r->session, 1);
   ret = gnutls_credentials_set(r->session, GNUTLS_CRD_CERTIFICATE, r->cert);

   return r;

 on_error:
   // FIXEM: cleanly destroy session
   free(r);
   return NULL;
}

EAPI Eina_Bool
emile_cipher_free(Emile_SSL *emile EINA_UNUSED)
{
   return EINA_TRUE;
}

EAPI Eina_Bool
emile_cipher_cafile_add(Emile_SSL *emile, const char *file)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;
   struct stat st;
   int count = 0;
   int ret;

   if (stat(file, &st)) return EINA_FALSE;
   if (S_ISDIR(st.st_mode))
     {
        it = eina_file_direct_ls(file);
        EINA_ITERATOR_FOREACH(it, info)
          {
             if (!(info->type == EINA_FILE_UNKNOWN ||
                   info->type == EINA_FILE_REG ||
                   info->type == EINA_FILE_LNK))
               continue ;

             ret = gnutls_certificate_set_x509_trust_file(emile->cert,
                                                          file,
                                                          GNUTLS_X509_FMT_PEM);
             if (ret > 0) count += ret;
          }
        eina_iterator_free(it);
     }
   else
     {
        ret = gnutls_certificate_set_x509_trust_file(emile->cert,
                                                     file,
                                                     GNUTLS_X509_FMT_PEM);
        if (ret > 0) count += ret;
     }

   if (!count) ERR("Could not load CA file from '%s'.", file);
   return !count ? EINA_FALSE : EINA_TRUE;
}

EAPI Eina_Bool
emile_cipher_cert_add(Emile_SSL *emile, const char *file)
{
   return eina_stringshare_replace(&emile->cert_file, file);
}

EAPI Eina_Bool
emile_cipher_privkey_add(Emile_SSL *emile, const char *file)
{
   int ret;

   ret = gnutls_certificate_set_x509_key_file(emile->cert,
                                              emile->cert_file,
                                              file,
                                              GNUTLS_X509_FMT_PEM);
   if (ret)
     ERR("Could not load certificate/key file ('%s'/'%s').",
         emile->cert_file, file);
   return ret ? EINA_FALSE : EINA_TRUE;
}

EAPI Eina_Bool
emile_cipher_crl_add(Emile_SSL *emile, const char *file)
{
   int ret;

   ret = gnutls_certificate_set_x509_crl_file(emile->cert, file,
                                              GNUTLS_X509_FMT_PEM);
   if (ret)
     ERR("Could not load CRL file from '%s'.", file);
   return ret ? EINA_FALSE : EINA_TRUE;
}

EAPI int
emile_cipher_read(Emile_SSL *emile, Eina_Binbuf *buffer)
{
   int num;

   if (!buffer || eina_binbuf_length_get(buffer) <= 0) return 0;
   if (emile->ssl_state == EMILE_SSL_STATE_HANDSHAKING)
     {
        DBG("Ongoing GNUTLS handshaking.");
        //_emile_cipher_handshaking(emile);
        if (emile->ssl_state == EMILE_SSL_STATE_ERROR)
          return -1;
        return 0;
     }

   num = gnutls_record_recv(emile->session,
                            (void*) eina_binbuf_string_get(buffer),
                            eina_binbuf_length_get(buffer));
   return num;
}

EAPI int
emile_cipher_write(Emile_SSL *emile EINA_UNUSED, const Eina_Binbuf *buffer EINA_UNUSED)
{
   return 0;
}

EAPI const char *
emile_cipher_error_get(const Emile_SSL *emile)
{
   return emile->last_error;
}

EAPI Eina_Bool
emile_cipher_verify_name_set(Emile_SSL *emile, const char *name)
{
   return eina_stringshare_replace(&emile->name, name);
}

EAPI const char *
emile_cipher_verify_name_get(const Emile_SSL *emile)
{
   return emile->name;
}

EAPI void
emile_cipher_verify_set(Emile_SSL *emile, Eina_Bool verify)
{
   emile->verify = verify;
}

EAPI void
emile_cipher_verify_basic_set(Emile_SSL *emile, Eina_Bool verify_basic)
{
   emile->verify_basic = verify_basic;
}

EAPI Eina_Bool
emile_cipher_verify_get(const Emile_SSL *emile)
{
   return emile->verify;
}

EAPI Eina_Bool
emile_cipher_verify_basic_get(const Emile_SSL *emile)
{
   return emile->verify_basic;
}

