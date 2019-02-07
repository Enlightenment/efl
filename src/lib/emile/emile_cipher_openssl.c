#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/dh.h>

#include <Eina.h>

#include "Emile.h"

#include "emile_private.h"

#define MAX_KEY_LEN   EVP_MAX_KEY_LENGTH
#define MAX_IV_LEN    EVP_MAX_IV_LENGTH

struct _Emile_SSL
{
   Emile_SSL *parent;
   SSL_CTX *ssl_ctx;
   SSL *ssl;

   const char *last_error;
   const char *verify_name;

   int ssl_err;
   Emile_SSL_State ssl_state;
   Emile_Want_Type ssl_want;

   Eina_Bool server : 1;
   Eina_Bool listen : 1;
   Eina_Bool connecting : 1;
   Eina_Bool handshaking : 1;
   Eina_Bool upgrade : 1;
   Eina_Bool crl_flag : 1;
   Eina_Bool verify : 1;
   Eina_Bool verify_basic : 1;
};

Eina_Bool
_emile_cipher_init(void)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
   ERR_load_crypto_strings();
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
#endif /* if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER) */

   return EINA_TRUE;
}

EAPI Eina_Bool
emile_binbuf_hmac_sha1(const char *key,
                       unsigned int key_len,
                       const Eina_Binbuf *data,
                       unsigned char digest[20])
{
   HMAC(EVP_sha1(),
        key, key_len,
        eina_binbuf_string_get(data), eina_binbuf_length_get(data),
        digest, NULL);
   return EINA_TRUE;
}

EAPI Eina_Bool
emile_binbuf_sha1(const Eina_Binbuf * data, unsigned char digest[20])
{
   const EVP_MD *md = EVP_sha1();
   Eina_Slice slice = eina_binbuf_slice_get(data);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
   EVP_MD_CTX *ctx = EVP_MD_CTX_new();

   EVP_DigestInit_ex(ctx, md, NULL);

   EVP_DigestUpdate(ctx, slice.mem, slice.len);
   EVP_DigestFinal_ex(ctx, digest, NULL);

   EVP_MD_CTX_free(ctx);
#else
   EVP_MD_CTX ctx;

   EVP_MD_CTX_init(&ctx);
   EVP_DigestInit_ex(&ctx, md, NULL);

   EVP_DigestUpdate(&ctx, slice.mem, slice.len);
   EVP_DigestFinal_ex(&ctx, digest, NULL);

   EVP_MD_CTX_cleanup(&ctx);
#endif
   return EINA_TRUE;
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
   /* Openssl declarations*/
   EVP_CIPHER_CTX *ctx = NULL;
   unsigned int *buffer = NULL;
   int tmp_len;

   if (algo != EMILE_AES256_CBC) return NULL;
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

   tmp = eina_htonl(eina_binbuf_length_get(data));
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
   ctx = EVP_CIPHER_CTX_new();
   if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, ik, iv))
     goto on_error;

   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   pointer = (unsigned char*) eina_binbuf_string_get(result);

   /* Openssl encrypt */
   if (!EVP_EncryptUpdate(ctx, pointer + sizeof (int), &tmp_len,
                          (unsigned char *)buffer,
                          eina_binbuf_length_get(data) + sizeof(unsigned int)))
     goto on_error;

   /* Openssl close the cipher */
   if (!EVP_EncryptFinal_ex(ctx, pointer + sizeof (int) + tmp_len,
                            &tmp_len))
     goto on_error;

   EVP_CIPHER_CTX_free(ctx);
   ctx = NULL;
   free(buffer);

   return result;

on_error:
   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   /* Openssl error */
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
   if (ctx)
     EVP_CIPHER_CTX_cleanup(ctx);
#endif /* if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER) */

   free(buffer);

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
   EVP_CIPHER_CTX *ctx = NULL;
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

   /* Openssl create the corresponding cipher */
   ctx = EVP_CIPHER_CTX_new();

   if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, ik, iv))
     goto on_error;

   memset(iv, 0, sizeof (iv));
   memset(ik, 0, sizeof (ik));

   /* Openssl decrypt */
   if (!EVP_DecryptUpdate(ctx,
                          (void*) eina_binbuf_string_get(result), &tmp,
                          (void*) (over + 1), tmp_len))
     goto on_error;

   /* Openssl close the cipher*/
   EVP_CIPHER_CTX_free(ctx);
   ctx = NULL;

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

   if (ctx)
     EVP_CIPHER_CTX_free(ctx);

   eina_binbuf_free(result);

   return NULL;
}

EAPI Emile_SSL *
emile_cipher_server_listen(Emile_Cipher_Type t)
{
   Emile_SSL *r;
   DH *dh_params = NULL;
   int options;
   int dh = 0;

   if (!emile_cipher_init()) return NULL;

   r = calloc(1, sizeof (Emile_SSL));
   if (!r) return NULL;

   switch (t)
     {
      case EMILE_SSLv23:
         r->ssl_ctx = SSL_CTX_new(SSLv23_server_method());
         if (!r->ssl_ctx) goto on_error;
         options = SSL_CTX_get_options(r->ssl_ctx);
         SSL_CTX_set_options(r->ssl_ctx,
                             options | SSL_OP_NO_SSLv2 | SSL_OP_SINGLE_DH_USE);
         break;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
      case EMILE_TLSv1:
         r->ssl_ctx = SSL_CTX_new(TLSv1_server_method());
         break;
#endif
      default:
         free(r);
         return NULL;
     }

   if (!r->ssl_ctx) goto on_error;

   dh_params = DH_new();
   if (!dh_params) goto on_error;
   if (!DH_generate_parameters_ex(dh_params, 1024, DH_GENERATOR_5, NULL))
     goto on_error;
   if (!DH_check(dh_params, &dh))
     goto on_error;
   if ((dh & DH_CHECK_P_NOT_PRIME) || (dh & DH_CHECK_P_NOT_SAFE_PRIME))
     goto on_error;
   if (!DH_generate_key(dh_params))
     goto on_error;
   if (!SSL_CTX_set_tmp_dh(r->ssl_ctx, dh_params))
     goto on_error;

   DH_free(dh_params);
   INF("DH params successfully generated and applied!");

   if (!SSL_CTX_set_cipher_list(r->ssl_ctx,
                                "aRSA+HIGH:+kEDH:+kRSA:!kSRP:!kPSK:+3DES:!MD5"))
     goto on_error;

   return r;

 on_error:
   if (dh)
     {
        if (dh & DH_CHECK_P_NOT_PRIME)
          ERR("openssl error: dh_params could not generate a prime!");
        else
          ERR("openssl error: dh_params could not generate a safe prime!");
     }
   else
     {
        ERR("openssl error: %s.", ERR_reason_error_string(ERR_get_error()));
     }
   emile_cipher_free(r);
   return NULL;
}

static void
_emile_cipher_print_verify_error(int error)
{
   switch (error)
     {
#define ERROR_OPENSSL(X) \
case (X):        \
  ERR("%s", #X); \
  break
#ifdef X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT);
#endif
#ifdef X509_V_ERR_UNABLE_TO_GET_CRL
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_GET_CRL);
#endif
#ifdef X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE);
#endif
#ifdef X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE);
#endif
#ifdef X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY);
#endif
#ifdef X509_V_ERR_CERT_SIGNATURE_FAILURE
        ERROR_OPENSSL(X509_V_ERR_CERT_SIGNATURE_FAILURE);
#endif
#ifdef X509_V_ERR_CRL_SIGNATURE_FAILURE
        ERROR_OPENSSL(X509_V_ERR_CRL_SIGNATURE_FAILURE);
#endif
#ifdef X509_V_ERR_CERT_NOT_YET_VALID
        ERROR_OPENSSL(X509_V_ERR_CERT_NOT_YET_VALID);
#endif
#ifdef X509_V_ERR_CERT_HAS_EXPIRED
        ERROR_OPENSSL(X509_V_ERR_CERT_HAS_EXPIRED);
#endif
#ifdef X509_V_ERR_CRL_NOT_YET_VALID
        ERROR_OPENSSL(X509_V_ERR_CRL_NOT_YET_VALID);
#endif
#ifdef X509_V_ERR_CRL_HAS_EXPIRED
        ERROR_OPENSSL(X509_V_ERR_CRL_HAS_EXPIRED);
#endif
#ifdef X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD
        ERROR_OPENSSL(X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD);
#endif
#ifdef X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD
        ERROR_OPENSSL(X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD);
#endif
#ifdef X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD
        ERROR_OPENSSL(X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD);
#endif
#ifdef X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD
        ERROR_OPENSSL(X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD);
#endif
#ifdef X509_V_ERR_OUT_OF_MEM
        ERROR_OPENSSL(X509_V_ERR_OUT_OF_MEM);
#endif
#ifdef X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
        ERROR_OPENSSL(X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT);
#endif
#ifdef X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN
        ERROR_OPENSSL(X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN);
#endif
#ifdef X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY);
#endif
#ifdef X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE);
#endif
#ifdef X509_V_ERR_CERT_CHAIN_TOO_LONG
        ERROR_OPENSSL(X509_V_ERR_CERT_CHAIN_TOO_LONG);
#endif
#ifdef X509_V_ERR_CERT_REVOKED
        ERROR_OPENSSL(X509_V_ERR_CERT_REVOKED);
#endif
#ifdef X509_V_ERR_INVALID_CA
        ERROR_OPENSSL(X509_V_ERR_INVALID_CA);
#endif
#ifdef X509_V_ERR_PATH_LENGTH_EXCEEDED
        ERROR_OPENSSL(X509_V_ERR_PATH_LENGTH_EXCEEDED);
#endif
#ifdef X509_V_ERR_INVALID_PURPOSE
        ERROR_OPENSSL(X509_V_ERR_INVALID_PURPOSE);
#endif
#ifdef X509_V_ERR_CERT_UNTRUSTED
        ERROR_OPENSSL(X509_V_ERR_CERT_UNTRUSTED);
#endif
#ifdef X509_V_ERR_CERT_REJECTED
        ERROR_OPENSSL(X509_V_ERR_CERT_REJECTED);
#endif
        /* These are 'informational' when looking for issuer cert */
#ifdef X509_V_ERR_SUBJECT_ISSUER_MISMATCH
        ERROR_OPENSSL(X509_V_ERR_SUBJECT_ISSUER_MISMATCH);
#endif
#ifdef X509_V_ERR_AKID_SKID_MISMATCH
        ERROR_OPENSSL(X509_V_ERR_AKID_SKID_MISMATCH);
#endif
#ifdef X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH
        ERROR_OPENSSL(X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH);
#endif
#ifdef X509_V_ERR_KEYUSAGE_NO_CERTSIGN
        ERROR_OPENSSL(X509_V_ERR_KEYUSAGE_NO_CERTSIGN);
#endif

#ifdef X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER
        ERROR_OPENSSL(X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER);
#endif
#ifdef X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION
        ERROR_OPENSSL(X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION);
#endif
#ifdef X509_V_ERR_KEYUSAGE_NO_CRL_SIGN
        ERROR_OPENSSL(X509_V_ERR_KEYUSAGE_NO_CRL_SIGN);
#endif
#ifdef X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION
        ERROR_OPENSSL(X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION);
#endif
#ifdef X509_V_ERR_INVALID_NON_CA
        ERROR_OPENSSL(X509_V_ERR_INVALID_NON_CA);
#endif
#ifdef X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED
        ERROR_OPENSSL(X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED);
#endif
#ifdef X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE
        ERROR_OPENSSL(X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE);
#endif
#ifdef X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED
        ERROR_OPENSSL(X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED);
#endif

#ifdef X509_V_ERR_INVALID_EXTENSION
        ERROR_OPENSSL(X509_V_ERR_INVALID_EXTENSION);
#endif
#ifdef X509_V_ERR_INVALID_POLICY_EXTENSION
        ERROR_OPENSSL(X509_V_ERR_INVALID_POLICY_EXTENSION);
#endif
#ifdef X509_V_ERR_NO_EXPLICIT_POLICY
        ERROR_OPENSSL(X509_V_ERR_NO_EXPLICIT_POLICY);
#endif
#ifdef X509_V_ERR_DIFFERENT_CRL_SCOPE
        ERROR_OPENSSL(X509_V_ERR_DIFFERENT_CRL_SCOPE);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_EXTENSION_FEATURE
        ERROR_OPENSSL(X509_V_ERR_UNSUPPORTED_EXTENSION_FEATURE);
#endif

#ifdef X509_V_ERR_UNNESTED_RESOURCE
        ERROR_OPENSSL(X509_V_ERR_UNNESTED_RESOURCE);
#endif

#ifdef X509_V_ERR_PERMITTED_VIOLATION
        ERROR_OPENSSL(X509_V_ERR_PERMITTED_VIOLATION);
#endif
#ifdef X509_V_ERR_EXCLUDED_VIOLATION
        ERROR_OPENSSL(X509_V_ERR_EXCLUDED_VIOLATION);
#endif
#ifdef X509_V_ERR_SUBTREE_MINMAX
        ERROR_OPENSSL(X509_V_ERR_SUBTREE_MINMAX);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_CONSTRAINT_TYPE
        ERROR_OPENSSL(X509_V_ERR_UNSUPPORTED_CONSTRAINT_TYPE);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_CONSTRAINT_SYNTAX
        ERROR_OPENSSL(X509_V_ERR_UNSUPPORTED_CONSTRAINT_SYNTAX);
#endif
#ifdef X509_V_ERR_UNSUPPORTED_NAME_SYNTAX
        ERROR_OPENSSL(X509_V_ERR_UNSUPPORTED_NAME_SYNTAX);
#endif
#ifdef X509_V_ERR_CRL_PATH_VALIDATION_ERROR
        ERROR_OPENSSL(X509_V_ERR_CRL_PATH_VALIDATION_ERROR);
#endif

        /* The application is not happy */
#ifdef X509_V_ERR_APPLICATION_VERIFICATION
        ERROR_OPENSSL(X509_V_ERR_APPLICATION_VERIFICATION);
#endif
     }
#undef ERROR_OPENSSL
}

static void
_emile_cipher_session_print(SSL *ssl)
{
   Eina_Strbuf *str;
   SSL_SESSION *s;
   STACK_OF(X509) *sk;
   BIO *b;
   BUF_MEM *bptr;
   char log[4096];

   if (!eina_log_domain_level_check(_emile_log_dom_global, EINA_LOG_LEVEL_DBG))
     return ;

   str = eina_strbuf_new();
   if (!str) return ;

   log[0] = '\0';
   b = BIO_new(BIO_s_mem());
   sk = SSL_get_peer_cert_chain(ssl);
   if (sk)
     {
        int i;

        DBG("CERTIFICATES:");
        for (i = 0; i < sk_X509_num(sk); i++)
          {
             char *p;

             p = X509_NAME_oneline(X509_get_subject_name(sk_X509_value(sk, i)),
                                   log, sizeof (log));
             DBG("%2d s:%s", i, p);
             p = X509_NAME_oneline(X509_get_issuer_name(sk_X509_value(sk, i)),
                                   log, sizeof(log));
             DBG("   i:%s", p);

             PEM_write_bio_X509(b, sk_X509_value(sk, i));
             BIO_get_mem_ptr(b, &bptr);
             eina_strbuf_append_length(str, bptr->data, bptr->length);
             DBG("%s", eina_strbuf_string_get(str));
             eina_strbuf_reset(str);
             BIO_free(b);
          }
     }

   s = SSL_get_session(ssl);
   SSL_SESSION_print(b, s);
   BIO_get_mem_ptr(b, &bptr);
   eina_strbuf_append_length(str, bptr->data, bptr->length);
   DBG("%s", eina_strbuf_string_get(str));
   eina_strbuf_free(str);
   BIO_free(b);
}

static void
_emile_cipher_client_handshake(Emile_SSL *client)
{
   X509 *cert;
   int ret = -1;

   if (!client) return ;

   switch (client->ssl_state)
     {
      case EMILE_SSL_STATE_INIT:
         client->ssl_state = EMILE_SSL_STATE_HANDSHAKING;
         client->handshaking = EINA_TRUE;
         EINA_FALLTHROUGH;

      case EMILE_SSL_STATE_HANDSHAKING:
         if (!client->ssl) goto on_error;

         ret = SSL_do_handshake(client->ssl);
         client->ssl_err = SSL_get_error(client->ssl, ret);

         if ((client->ssl_err == SSL_ERROR_SYSCALL) ||
             (client->ssl_err == SSL_ERROR_SSL))
           goto on_error;

         if (ret != 1)
           {
              if (client->ssl_err == SSL_ERROR_WANT_READ)
                client->ssl_want = EMILE_WANT_READ;
              else if (client->ssl_err == SSL_ERROR_WANT_WRITE)
                client->ssl_want = EMILE_WANT_WRITE;

              return ;
           }

         client->handshaking = EINA_FALSE;
         client->ssl_state = EMILE_SSL_STATE_DONE;
         EINA_FALLTHROUGH;
      case EMILE_SSL_STATE_DONE:
         break;
      case EMILE_SSL_STATE_ERROR:
         goto on_error;
     }

   _emile_cipher_session_print(client->ssl);
   if (!client->parent->verify &&
       !client->parent->verify_basic)
     return ;

   SSL_set_verify(client->ssl, SSL_VERIFY_PEER, NULL);
   /* use CRL/CA lists to verify */
   cert = SSL_get_peer_certificate(client->ssl);
   if (cert)
     {
        const char *verify_name;
        char *cert_name;
        char *s;
        int clen;
        int err;
        int name = 0;

        if (client->parent->verify)
          {
             err = SSL_get_verify_result(client->ssl);
             _emile_cipher_print_verify_error(err);
             if (err) goto on_error;
          }

        clen = X509_NAME_get_text_by_NID(X509_get_subject_name(cert),
                                         NID_subject_alt_name,
                                         NULL, 0);
        if (clen > 0)
          {
             name = NID_subject_alt_name;
          }
        else
          {
             clen = X509_NAME_get_text_by_NID(X509_get_subject_name(cert),
                                              NID_commonName,
                                              NULL, 0);
             if (clen <= 0) goto on_error;
             name = NID_commonName;
          }

        cert_name = alloca(++clen);
        X509_NAME_get_text_by_NID(X509_get_subject_name(cert),
                                  name, cert_name, clen);
        verify_name = client->parent->verify_name;

        INF("Cert name: '%s' vs verify name: '%s'.", cert_name, verify_name);

        if (!verify_name) goto on_error;
        if (strcasecmp(cert_name, verify_name)) goto on_error;
        if (verify_name[0] != '*') goto on_error;

        /* verify that their is only one wildcard in the client cert name */
        if (strchr(cert_name + 1, '*')) goto on_error;
        /* verify that we have a domain of at least *.X.TLD and not *.TLD */
        if (!strchr(cert_name + 2, '.')) goto on_error;
        s = strchr(verify_name, '.');
        if (!s) goto on_error;
        /* same as above for the stored name */
        if (!strchr(s + 1, '.')) goto on_error;
        if (strcasecmp(s, verify_name + 1)) goto on_error;

        DBG("Successfully verified certificate.");
     }

   return ;

 on_error:
   DBG("Failed to finish handshake.");
   client->ssl_state = EMILE_SSL_STATE_ERROR;
   return ;
}

EAPI Emile_SSL *
emile_cipher_client_connect(Emile_SSL *server, int fd)
{
   Emile_SSL *r;

   if (!server) return NULL;

   r = calloc(1, sizeof (Emile_SSL));
   if (!r) return NULL;

   r->parent = server;
   r->ssl = SSL_new(r->parent->ssl_ctx);
   if (!r->ssl) goto on_error;

   if (!SSL_set_fd(r->ssl, fd))
     goto on_error;

   SSL_set_accept_state(r->ssl);

   _emile_cipher_client_handshake(r);

   if (r->ssl_state == EMILE_SSL_STATE_ERROR) goto on_error;

   return r;

 on_error:
   emile_cipher_free(r);
   return NULL;
}

EAPI Emile_SSL *
emile_cipher_server_connect(Emile_Cipher_Type t)
{
   Emile_SSL *r;
   const char *msg;
   int options;

   if (!emile_cipher_init()) return NULL;

   r = calloc(1, sizeof (Emile_SSL));
   if (!r) return NULL;

   switch (t)
     {
      case EMILE_SSLv23:
         r->ssl_ctx = SSL_CTX_new(SSLv23_client_method());
         if (!r->ssl_ctx) goto on_error;
         options = SSL_CTX_get_options(r->ssl_ctx);
         SSL_CTX_set_options(r->ssl_ctx,
                             options | SSL_OP_NO_SSLv2 | SSL_OP_SINGLE_DH_USE);
         break;
      case EMILE_TLSv1:
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
         r->ssl_ctx = SSL_CTX_new(TLSv1_client_method());
         break;
#endif
      default:
         free(r);
         return NULL;
     }

   if (!SSL_CTX_set_cipher_list(r->ssl_ctx,
                                "aRSA+HIGH:+kEDH:+kRSA:!kSRP:!kPSK:+3DES:!MD5"))
     goto on_error;

   return r;

 on_error:
   msg = ERR_reason_error_string(ERR_get_error());
   ERR("OpenSSL error: '%s'.", msg);
   emile_cipher_free(r);
   return NULL;
}

EAPI Eina_Bool
emile_cipher_free(Emile_SSL *emile)
{
   if (!emile) return EINA_FALSE;

   eina_stringshare_del(emile->last_error);
   emile->last_error = NULL;

   eina_stringshare_del(emile->verify_name);
   emile->verify_name = NULL;

   if (emile->ssl)
     {
        if (!SSL_shutdown(emile->ssl))
          SSL_shutdown(emile->ssl);

        SSL_free(emile->ssl);
     }
   emile->ssl = NULL;

   if (emile->ssl_ctx)
     SSL_CTX_free(emile->ssl_ctx);
   emile->ssl_ctx = NULL;

   free(emile);
   return EINA_TRUE;
}

EAPI Eina_Bool
emile_cipher_cafile_add(Emile_SSL *emile, const char *file)
{
   struct stat st;
   unsigned long err;

   if (stat(file, &st)) return EINA_FALSE;
   if (S_ISDIR(st.st_mode))
     {
        if (!SSL_CTX_load_verify_locations(emile->ssl_ctx, NULL, file))
          goto on_error;
     }
   else
     {
        if (!SSL_CTX_load_verify_locations(emile->ssl_ctx, file, NULL))
          goto on_error;
     }

   return EINA_TRUE;

 on_error:
   err = ERR_peek_last_error();
   if (!err) return EINA_FALSE;

   DBG("OpenSSL error: '%s'.", ERR_reason_error_string(err));
   eina_stringshare_replace(&emile->last_error, ERR_reason_error_string(err));
   return EINA_FALSE;
}

EAPI Eina_Bool
emile_cipher_cert_add(Emile_SSL *emile, const char *file)
{
   Eina_File *f;
   void *m;
   X509 *cert = NULL;
   BIO *bio = NULL;
   int err;

   f = eina_file_open(file, EINA_FALSE);
   if (!f) return EINA_FALSE;

   m = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!m) goto on_error;

   bio = BIO_new_mem_buf(m, eina_file_size_get(f));
   if (!bio) goto on_error;

   cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
   if (!cert) goto on_error;

   if (SSL_CTX_use_certificate(emile->ssl_ctx, cert) < 1)
     goto on_error;

   eina_file_map_free(f, m);
   eina_file_close(f);
   BIO_free(bio);

   return EINA_TRUE;

 on_error:
   err = ERR_peek_last_error();

   if (m) eina_file_map_free(f, m);
   if (f) eina_file_close(f);
   if (bio) BIO_free(bio);

   if (!err) return EINA_FALSE;

   DBG("OpenSSL error: '%s'.", ERR_reason_error_string(err));
   eina_stringshare_replace(&emile->last_error, ERR_reason_error_string(err));
   return EINA_FALSE;
}

EAPI Eina_Bool
emile_cipher_privkey_add(Emile_SSL *emile, const char *file)
{
   Eina_File *f;
   void *m;
   EVP_PKEY *privkey = NULL;
   BIO *bio = NULL;
   int err;

   f = eina_file_open(file, EINA_FALSE);
   if (!f) return EINA_FALSE;

   m = eina_file_map_all(f, EINA_FILE_WILLNEED);
   if (!m) goto on_error;

   bio = BIO_new_mem_buf(m, eina_file_size_get(f));
   if (!bio) goto on_error;

   privkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
   if (!privkey) goto on_error;

   eina_file_map_free(f, m);
   m = NULL;

   eina_file_close(f);
   f = NULL;

   if (SSL_CTX_use_PrivateKey(emile->ssl_ctx, privkey) < 1)
     goto on_error;

   if (SSL_CTX_check_private_key(emile->ssl_ctx) < 1)
     goto on_error;

   BIO_free(bio);

   return EINA_TRUE;

 on_error:
   err = ERR_peek_last_error();

   if (m) eina_file_map_free(f, m);
   if (f) eina_file_close(f);
   if (bio) BIO_free(bio);

   if (!err) return EINA_FALSE;

   DBG("OpenSSL error: '%s'.", ERR_reason_error_string(err));
   eina_stringshare_replace(&emile->last_error, ERR_reason_error_string(err));
   return EINA_FALSE;
}

EAPI Eina_Bool
emile_cipher_crl_add(Emile_SSL *emile, const char *file)
{
   X509_LOOKUP *lu;
   X509_STORE *st;
   int err;

   st = SSL_CTX_get_cert_store(emile->ssl_ctx);
   if (!st) goto on_error;

   lu = X509_STORE_add_lookup(st, X509_LOOKUP_file());
   if (!lu) goto on_error;

   if (X509_load_crl_file(lu, file, X509_FILETYPE_PEM) < 1)
     goto on_error;

   if (!emile->crl_flag)
     {
        X509_STORE_set_flags(st,
                             X509_V_FLAG_CRL_CHECK |
                             X509_V_FLAG_CRL_CHECK_ALL);
        emile->crl_flag = EINA_TRUE;
     }

   return EINA_TRUE;

 on_error:
   err = ERR_peek_last_error();
   if (!err) return EINA_FALSE;

   DBG("OpenSSL error: '%s'.", ERR_reason_error_string(err));
   eina_stringshare_replace(&emile->last_error, ERR_reason_error_string(err));
   return EINA_FALSE;
}

EAPI int
emile_cipher_read(Emile_SSL *emile, Eina_Binbuf *buffer)
{
   int err;
   int num;

   if (!emile->ssl) return -1;
   if (eina_binbuf_length_get(buffer) <= 0) return 0;

   if (emile->ssl_state == EMILE_SSL_STATE_HANDSHAKING)
     _emile_cipher_client_handshake(emile);
   if (emile->ssl_state == EMILE_SSL_STATE_ERROR)
     return -1;
   else if (emile->ssl_state == EMILE_SSL_STATE_HANDSHAKING)
     return 0;

   num = SSL_read(emile->ssl,
                  (void*) eina_binbuf_string_get(buffer),
                  eina_binbuf_length_get(buffer));
   emile->ssl_err = SSL_get_error(emile->ssl, num);

   switch (emile->ssl_err)
     {
      case SSL_ERROR_WANT_READ: emile->ssl_want = EMILE_WANT_READ; break;
      case SSL_ERROR_WANT_WRITE: emile->ssl_want = EMILE_WANT_WRITE; break;
      case SSL_ERROR_ZERO_RETURN:
      case SSL_ERROR_SYSCALL:
      case SSL_ERROR_SSL:
         err = ERR_peek_last_error();
         if (!err) return -1;

         DBG("OpenSSL error: '%s'.", ERR_reason_error_string(err));
         eina_stringshare_replace(&emile->last_error, ERR_reason_error_string(err));
         return -1;

      default:
         emile->ssl_want = EMILE_WANT_NOTHING;
         break;
     }

   return num < 0 ? 0 : num;
}

EAPI int
emile_cipher_write(Emile_SSL *emile, const Eina_Binbuf *buffer)
{
   int num;
   int err;

   if (!emile->ssl) return -1;
   if (!buffer || eina_binbuf_length_get(buffer) <= 0) return 0;

   if (emile->ssl_state == EMILE_SSL_STATE_HANDSHAKING)
     _emile_cipher_client_handshake(emile);
   if (emile->ssl_state == EMILE_SSL_STATE_ERROR)
     return -1;
   else if (emile->ssl_state == EMILE_SSL_STATE_HANDSHAKING)
     return 0;

   num = SSL_write(emile->ssl,
                   (void*) eina_binbuf_string_get(buffer),
                   eina_binbuf_length_get(buffer));
   emile->ssl_err = SSL_get_error(emile->ssl, num);

   switch (emile->ssl_err)
     {
      case SSL_ERROR_WANT_READ: emile->ssl_want = EMILE_WANT_READ; break;
      case SSL_ERROR_WANT_WRITE: emile->ssl_want = EMILE_WANT_WRITE; break;
      case SSL_ERROR_ZERO_RETURN:
      case SSL_ERROR_SYSCALL:
      case SSL_ERROR_SSL:
         err = ERR_peek_last_error();
         if (!err) return -1;

         DBG("OpenSSL error: '%s'.", ERR_reason_error_string(err));
         eina_stringshare_replace(&emile->last_error, ERR_reason_error_string(err));
         return -1;

      default:
         emile->ssl_want = EMILE_WANT_NOTHING;
         break;
     }

   return num < 0 ? 0 : num;
}

EAPI const char *
emile_cipher_error_get(const Emile_SSL *emile)
{
   return emile->last_error;
}

EAPI Eina_Bool
emile_cipher_verify_name_set(Emile_SSL *emile, const char *name)
{
   return eina_stringshare_replace(&emile->verify_name, name);
}

EAPI const char *
emile_cipher_verify_name_get(const Emile_SSL *emile)
{
   return emile->verify_name;
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
