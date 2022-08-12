#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_SIGNATURE
# ifdef _WIN32
#  include <evil_private.h> /* mmap */
# else
#  include <sys/mman.h>
# endif
# include <openssl/rsa.h>
# include <openssl/objects.h>
# include <openssl/err.h>
# include <openssl/ssl.h>
# include <openssl/dh.h>
# include <openssl/dsa.h>
# include <openssl/evp.h>
# include <openssl/x509.h>
# include <openssl/pem.h>
#endif /* ifdef HAVE_SIGNATURE */

#ifdef HAVE_OPENSSL
# include <openssl/sha.h>
#endif /* ifdef HAVE_OPENSSL */

#ifdef HAVE_CIPHER
# include <openssl/evp.h>
# include <openssl/hmac.h>
# include <openssl/rand.h>
#endif /* ifdef HAVE_CIPHER */

#include <Emile.h>

#include "Eet.h"
#include "Eet_private.h"

#define MAX_KEY_LEN   EVP_MAX_KEY_LENGTH
#define MAX_IV_LEN    EVP_MAX_IV_LENGTH

struct _Eet_Key
{
   int                   references;
#ifdef HAVE_SIGNATURE
   X509                 *certificate;
   EVP_PKEY             *private_key;
#endif /* ifdef HAVE_SIGNATURE */
};

EAPI Eet_Key *
eet_identity_open(const char               *certificate_file,
                  const char               *private_key_file,
                  Eet_Key_Password_Callback cb)
{
#ifdef HAVE_SIGNATURE
   /* Signature declarations */
   Eet_Key *key = NULL;
   /* Openssl private declarations */
   EVP_PKEY *pkey = NULL;
   X509 *cert = NULL;

   if (!emile_cipher_init()) return NULL;

   /* Load the X509 certificate in memory. */
   {
      BIO* cert_bio = BIO_new_file(certificate_file, "rb");
      cert = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);
      BIO_free(cert_bio);
      if (!cert)
        goto on_error;
   }

   /* Check the presence of the public key. Just in case. */
   pkey = X509_get_pubkey(cert);
   if (!pkey)
     goto on_error;

   /* Load the private key in memory. */
   {
      BIO* private_key_bio = BIO_new_file(private_key_file, "rb");
      pkey = PEM_read_bio_PrivateKey(private_key_bio, NULL, cb, NULL);
      BIO_free(private_key_bio);
      if (!pkey)
        goto on_error;
   }

   /* Load the certificate and the private key in Eet_Key structure */
   key = malloc(sizeof(Eet_Key));
   if (!key)
     goto on_error;

   key->references = 1;
   key->certificate = cert;
   key->private_key = pkey;

   return key;

on_error:
   if (cert)
     X509_free(cert);

   if (pkey)
     EVP_PKEY_free(pkey);

#else
   (void) certificate_file;
   (void) private_key_file;
   (void) cb;
#endif /* ifdef HAVE_SIGNATURE */
   return NULL;
}

EAPI void
eet_identity_close(Eet_Key *key)
{
   if (!emile_cipher_init()) return ;

#ifdef HAVE_SIGNATURE
   if (!key || (key->references > 0))
     return;

   X509_free(key->certificate);
   EVP_PKEY_free(key->private_key);
   free(key);
# else
   (void)key;
#endif /* ifdef HAVE_SIGNATURE */
}

EAPI void
eet_identity_print(Eet_Key *key,
                   FILE    *out)
{
#ifdef HAVE_SIGNATURE
   RSA *rsa;
   DSA *dsa;
   DH *dh;

   if (!key)
     return;

   if (!emile_cipher_init()) return ;

   rsa = EVP_PKEY_get1_RSA(key->private_key);
   if (rsa)
     {
        fprintf(out, "Private key (RSA):\n");
        RSA_print_fp(out, rsa, 0);
     }

   dsa = EVP_PKEY_get1_DSA(key->private_key);
   if (dsa)
     {
        fprintf(out, "Private key (DSA):\n");
        DSA_print_fp(out, dsa, 0);
     }

   dh = EVP_PKEY_get1_DH(key->private_key);
   if (dh)
     {
        fprintf(out, "Private key (DH):\n");
        DHparams_print_fp(out, dh);
     }

   fprintf(out, "Public certificate:\n");
   X509_print_fp(out, key->certificate);
#else /* ifdef HAVE_SIGNATURE */
   key = NULL;
   out = NULL;
   ERR("You need to compile signature support in EET.");
#endif /* ifdef HAVE_SIGNATURE */
}

void
eet_identity_ref(Eet_Key *key)
{
   if (!key)
     return;

   key->references++;
}

void
eet_identity_unref(Eet_Key *key)
{
   if (!key)
     return;

   key->references--;
   eet_identity_close(key);
}

void *
eet_identity_compute_sha1(const void  *data_base,
                          unsigned int data_length,
                          int         *sha1_length)
{
   void *result;

#ifdef HAVE_SIGNATURE
   result = malloc(SHA_DIGEST_LENGTH);
   if (!result)
     return NULL;

   SHA1(data_base, data_length, result);
   if (sha1_length)
     *sha1_length = SHA_DIGEST_LENGTH;

#else /* ifdef HAVE_SIGNATURE */
   data_base = NULL;
   data_length = 0;
   sha1_length = NULL;
   result = NULL;
#endif /* ifdef HAVE_SIGNATURE */

   return result;
}

Eet_Error
eet_identity_sign(FILE    *fp,
                  Eet_Key *key)
{
#ifdef HAVE_SIGNATURE
   Eet_Error err = EET_ERROR_NONE;
   struct stat st_buf;
   void *data;
   int fd;
   int head[3];
   unsigned char *sign = NULL;
   unsigned char *cert = NULL;
# if (LIBRESSL_VERSION_NUMBER >= 0x3050000fL) || ((OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER))
   EVP_MD_CTX *md_ctx;
# else
   EVP_MD_CTX md_ctx;
# endif
   unsigned int sign_len = 0;
   int cert_len = 0;

   /* A few check and flush pending write. */
   if (!fp || !key || !key->certificate || !key->private_key)
     return EET_ERROR_BAD_OBJECT;

   if (!emile_cipher_init()) return EET_ERROR_NOT_IMPLEMENTED;

   /* Get the file size. */
   fd = fileno(fp);
   if (fd < 0)
     return EET_ERROR_BAD_OBJECT;

   if (fstat(fd, &st_buf) < 0)
     return EET_ERROR_MMAP_FAILED;

   /* let's make mmap safe and just get 0 pages for IO erro */
   eina_mmap_safety_enabled_set(EINA_TRUE);

   /* Map the file in memory. */
   data = mmap(NULL, st_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
   if (data == MAP_FAILED)
     return EET_ERROR_MMAP_FAILED;

   sign_len = EVP_PKEY_size(key->private_key);
   sign = malloc(sign_len);
   if (!sign)
     {
        err = EET_ERROR_OUT_OF_MEMORY;
        goto on_error;
     }

   /* Do the signature. */
# if (LIBRESSL_VERSION_NUMBER >= 0x3050000fL) || ((OPENSSL_VERSION_NUMBER >= 0x10100000L) && (!defined(LIBRESSL_VERSION_NUMBER)))
   md_ctx = EVP_MD_CTX_new();
   if (!md_ctx)
     {
        err = EET_ERROR_OUT_OF_MEMORY;
        goto on_error;
     }
   EVP_SignInit(md_ctx, EVP_sha1());
   EVP_SignUpdate(md_ctx, data, st_buf.st_size);
   err = EVP_SignFinal(md_ctx,
                       sign,
                       (unsigned int *)&sign_len,
                       key->private_key);
   EVP_MD_CTX_free(md_ctx);
# else
   EVP_SignInit(&md_ctx, EVP_sha1());
   EVP_SignUpdate(&md_ctx, data, st_buf.st_size);
   err = EVP_SignFinal(&md_ctx,
                       sign,
                       (unsigned int *)&sign_len,
                       key->private_key);
   EVP_MD_CTX_cleanup(&md_ctx);
# endif
   if (err != 1)
     {
        ERR_print_errors_fp(stdout);
        err = EET_ERROR_SIGNATURE_FAILED;
        goto on_error;
     }

   /* Give me the der (binary form for X509). */
   cert_len = i2d_X509(key->certificate, &cert);
   if (cert_len < 0)
     {
        ERR_print_errors_fp(stdout);
        err = EET_ERROR_X509_ENCODING_FAILED;
        goto on_error;
     }

   /* Append the signature at the end of the file. */
   head[0] = (int)eina_htonl ((unsigned int)EET_MAGIC_SIGN);
   head[1] = (int)eina_htonl ((unsigned int)sign_len);
   head[2] = (int)eina_htonl ((unsigned int)cert_len);

   if (fwrite(head, sizeof(head), 1, fp) != 1)
     {
        err = EET_ERROR_WRITE_ERROR;
        goto on_error;
     }

   if (fwrite(sign, sign_len, 1, fp) != 1)
     {
        err = EET_ERROR_WRITE_ERROR;
        goto on_error;
     }

   if (fwrite(cert, cert_len, 1, fp) != 1)
     {
        err = EET_ERROR_WRITE_ERROR;
        goto on_error;
     }

on_error:
   if (sign)
     free(sign);

   munmap(data, st_buf.st_size);
   return err;
#else /* ifdef HAVE_SIGNATURE */
   fp = NULL;
   key = NULL;
   return EET_ERROR_NOT_IMPLEMENTED;
#endif /* ifdef HAVE_SIGNATURE */
}

const void *
eet_identity_check(const void   *data_base,
                   unsigned int  data_length,
                   void        **sha1,
                   int          *sha1_length,
                   const void   *signature_base,
                   unsigned int  signature_length,
                   const void  **raw_signature_base,
                   unsigned int *raw_signature_length,
                   int          *x509_length)
{
#ifdef HAVE_SIGNATURE
   const int *header = signature_base;
   const unsigned char *sign;
   const unsigned char *cert_der;
   int sign_len;
   int cert_len;
   int magic;

   /* At least the header size */
   if (signature_length < sizeof(int) * 3)
     return NULL;

   if (!emile_cipher_init()) return NULL;

   /* Get the header */
   memcpy(&magic,    header, sizeof(int));
   memcpy(&sign_len, header+1, sizeof(int));
   memcpy(&cert_len, header+2, sizeof(int));

   magic = eina_ntohl(magic);
   sign_len = eina_ntohl(sign_len);
   cert_len = eina_ntohl(cert_len);

   /* Verify the header */
   if (magic != EET_MAGIC_SIGN)
     return NULL;

   if (sign_len + cert_len + sizeof(int) * 3 > signature_length)
     return NULL;

   /* Update the signature and certificate pointer */
   sign = (unsigned char *)signature_base + sizeof(int) * 3;
   cert_der = sign + sign_len;

   const unsigned char *tmp;
   EVP_PKEY *pkey;
   X509 *x509;
# if (LIBRESSL_VERSION_NUMBER >= 0x3050000fL) || ((OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER))
   EVP_MD_CTX *md_ctx;
# else
   EVP_MD_CTX md_ctx;
# endif
   int err;

   /* Strange but d2i_X509 seems to put 0 all over the place. */
   tmp = alloca(cert_len);
   memcpy((char *)tmp, cert_der, cert_len);
   x509 = d2i_X509(NULL, &tmp, cert_len);
   if (!x509)
     return NULL;

   /* Get public key - eay */
   pkey = X509_get_pubkey(x509);
   if (!pkey)
     {
        X509_free(x509);
        return NULL;
     }

   /* Verify the signature */
# if (LIBRESSL_VERSION_NUMBER >= 0x3050000fL) || ((OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER))
   md_ctx = EVP_MD_CTX_new();
   if (!md_ctx)
     {
        err = EET_ERROR_OUT_OF_MEMORY;

        X509_free(x509);
        EVP_PKEY_free(pkey);

        return NULL;
     }

   EVP_VerifyInit(md_ctx, EVP_sha1());
   EVP_VerifyUpdate(md_ctx, data_base, data_length);
   err = EVP_VerifyFinal(md_ctx, sign, sign_len, pkey);
   EVP_MD_CTX_free(md_ctx);
# else
   EVP_VerifyInit(&md_ctx, EVP_sha1());
   EVP_VerifyUpdate(&md_ctx, data_base, data_length);
   err = EVP_VerifyFinal(&md_ctx, sign, sign_len, pkey);
   EVP_MD_CTX_cleanup(&md_ctx);
# endif

   X509_free(x509);
   EVP_PKEY_free(pkey);

   if (sha1)
     {
        *sha1 = NULL;
        *sha1_length = -1;
     }

   if (err != 1)
     return NULL;

   if (x509_length)
     *x509_length = cert_len;

   if (raw_signature_base)
     *raw_signature_base = sign;

   if (raw_signature_length)
     *raw_signature_length = sign_len;

   return cert_der;
#else /* ifdef HAVE_SIGNATURE */
   data_base = NULL;
   data_length = 0;
   sha1 = NULL;
   sha1_length = NULL;
   signature_base = NULL;
   signature_length = 0;
   raw_signature_base = NULL;
   raw_signature_length = NULL;
   x509_length = NULL;
   return NULL;
#endif /* ifdef HAVE_SIGNATURE */
}

EAPI void
eet_identity_certificate_print(const unsigned char *certificate,
                               int                  der_length,
                               FILE                *out)
{
#ifdef HAVE_SIGNATURE
   if (!certificate || !out || der_length <= 0)
     {
        ERR("No certificate provided.");
        return;
     }

   if (!emile_cipher_init()) return ;

   const unsigned char *tmp;
   X509 *x509;

   /* Strange but d2i_X509 seems to put 0 all over the place. */
   tmp = alloca(der_length);
   memcpy((char *)tmp, certificate, der_length);
   x509 = d2i_X509(NULL, &tmp, der_length);
   if (!x509)
     {
        INF("Not a valid certificate.");
        return;
     }

   INF("Public certificate :");
   X509_print_fp(out, x509);

   X509_free(x509);
#else /* ifdef HAVE_SIGNATURE */
   certificate = NULL;
   der_length = 0;
   out = NULL;
   ERR("You need to compile signature support in EET.");
#endif /* ifdef HAVE_SIGNATURE */
}

Eet_Error
eet_cipher(const void   *data,
           unsigned int  size,
           const char   *key,
           unsigned int  length,
           void        **result,
           unsigned int *result_length)
{
   Eina_Binbuf *out;
   Eina_Binbuf *in;

   in = eina_binbuf_manage_new(data, size, EINA_TRUE);
   out = emile_binbuf_cipher(EMILE_AES256_CBC, in, key, length);

   if (result_length) *result_length = out ? eina_binbuf_length_get(out) : 0;
   if (result) *result = out ? eina_binbuf_string_steal(out) : NULL;

   eina_binbuf_free(out);
   eina_binbuf_free(in);
   return out ? EET_ERROR_NONE : EET_ERROR_ENCRYPT_FAILED;
}

Eet_Error
eet_decipher(const void   *data,
             unsigned int  size,
             const char   *key,
             unsigned int  length,
             void        **result,
             unsigned int *result_length)
{
   Eina_Binbuf *out;
   Eina_Binbuf *in;

   in = eina_binbuf_manage_new(data, size, EINA_TRUE);
   out = emile_binbuf_decipher(EMILE_AES256_CBC, in, key, length);

   if (result_length) *result_length = out ? eina_binbuf_length_get(out) : 0;
   if (result) *result = out ? eina_binbuf_string_steal(out) : NULL;

   eina_binbuf_free(out);
   eina_binbuf_free(in);
   return out ? EET_ERROR_NONE : EET_ERROR_DECRYPT_FAILED;
}
