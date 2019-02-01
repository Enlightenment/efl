#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_SIGNATURE
# include <sys/mman.h>
# ifdef HAVE_GNUTLS
#  include <gnutls/gnutls.h>
#  include <gnutls/x509.h>
# else /* ifdef HAVE_GNUTLS */
#  include <openssl/rsa.h>
#  include <openssl/objects.h>
#  include <openssl/err.h>
#  include <openssl/ssl.h>
#  include <openssl/dh.h>
#  include <openssl/dsa.h>
#  include <openssl/evp.h>
#  include <openssl/x509.h>
#  include <openssl/pem.h>
# endif /* ifdef HAVE_GNUTLS */
#endif /* ifdef HAVE_SIGNATURE */

#ifdef HAVE_OPENSSL
# include <openssl/sha.h>
#endif /* ifdef HAVE_OPENSSL */

#ifdef HAVE_CIPHER
# ifdef HAVE_GNUTLS
#  include <gnutls/abstract.h>
#  include <gnutls/x509.h>
#  include <gcrypt.h>
# else /* ifdef HAVE_GNUTLS */
#  include <openssl/evp.h>
#  include <openssl/hmac.h>
#  include <openssl/rand.h>
# endif /* ifdef HAVE_GNUTLS */
#endif /* ifdef HAVE_CIPHER */

#include <Emile.h>

#include "Eet.h"
#include "Eet_private.h"

#ifdef HAVE_GNUTLS
# define MAX_KEY_LEN   32
# define MAX_IV_LEN    16
#else /* ifdef HAVE_GNUTLS */
# define MAX_KEY_LEN   EVP_MAX_KEY_LENGTH
# define MAX_IV_LEN    EVP_MAX_IV_LENGTH
#endif /* ifdef HAVE_GNUTLS */

struct _Eet_Key
{
   int                   references;
#ifdef HAVE_SIGNATURE
# ifdef HAVE_GNUTLS
   gnutls_x509_crt_t     certificate;
   gnutls_x509_privkey_t private_key;
# else /* ifdef HAVE_GNUTLS */
   X509                 *certificate;
   EVP_PKEY             *private_key;
# endif /* ifdef HAVE_GNUTLS */
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
# ifdef HAVE_GNUTLS
   /* Gnutls private declarations */
   Eina_File *f = NULL;
   void *data = NULL;
   gnutls_datum_t load_file = { NULL, 0 };
   char pass[1024];

   if (!emile_cipher_init()) return NULL;

   /* Init */
   if (!(key = malloc(sizeof(Eet_Key))))
     goto on_error;

   key->references = 1;

   if (gnutls_x509_crt_init(&(key->certificate)))
     goto on_error;

   if (gnutls_x509_privkey_init(&(key->private_key)))
     goto on_error;

   /* Mmap certificate_file */
   f = eina_file_open(certificate_file, 0);
   if (!f)
     goto on_error;

   /* let's make mmap safe and just get 0 pages for IO erro */
   eina_mmap_safety_enabled_set(EINA_TRUE);

   data = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!data) goto on_error;

   /* Import the certificate in Eet_Key structure */
   load_file.data = data;
   load_file.size = eina_file_size_get(f);
   if (gnutls_x509_crt_import(key->certificate, &load_file,
                              GNUTLS_X509_FMT_PEM) < 0)
     goto on_error;

   eina_file_map_free(f, data);

   /* Reset values */
   eina_file_close(f);
   f = NULL;
   data = NULL;
   load_file.data = NULL;
   load_file.size = 0;

   /* Mmap private_key_file */
   f = eina_file_open(private_key_file, 0);
   if (!f)
     goto on_error;

   /* let's make mmap safe and just get 0 pages for IO erro */
   eina_mmap_safety_enabled_set(EINA_TRUE);

   data = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!data)
     goto on_error;

   /* Import the private key in Eet_Key structure */
   load_file.data = data;
   load_file.size = eina_file_size_get(f);
   /* Try to directly import the PEM encoded private key */
   if (gnutls_x509_privkey_import(key->private_key, &load_file,
                                  GNUTLS_X509_FMT_PEM) < 0)
     {
        /* Else ask for the private key pass */
         if (cb && cb(pass, 1024, 0, NULL))
           {
     /* If pass then try to decode the pkcs 8 private key */
               if (gnutls_x509_privkey_import_pkcs8(key->private_key, &load_file,
                                                    GNUTLS_X509_FMT_PEM, pass, 0))
                 goto on_error;
           }
         else
         /* Else try to import the pkcs 8 private key without pass */
         if (gnutls_x509_privkey_import_pkcs8(key->private_key, &load_file,
                                              GNUTLS_X509_FMT_PEM, NULL, 1))
           goto on_error;
     }

   eina_file_map_free(f, data);
   eina_file_close(f);

   return key;

on_error:
   if (data) eina_file_map_free(f, data);
   if (f) eina_file_close(f);

   if (key)
     {
        if (key->certificate)
          gnutls_x509_crt_deinit(key->certificate);

        if (key->private_key)
          gnutls_x509_privkey_deinit(key->private_key);

        free(key);
     }

# else /* ifdef HAVE_GNUTLS */
   /* Openssl private declarations */
   FILE *fp;
   EVP_PKEY *pkey = NULL;
   X509 *cert = NULL;

   if (!emile_cipher_init()) return NULL;

   /* Load the X509 certificate in memory. */
   fp = fopen(certificate_file, "rb");
   if (!fp)
     return NULL;

   cert = PEM_read_X509(fp, NULL, NULL, NULL);
   fclose(fp);
   if (!cert)
     goto on_error;

   /* Check the presence of the public key. Just in case. */
   pkey = X509_get_pubkey(cert);
   if (!pkey)
     goto on_error;

   /* Load the private key in memory. */
   fp = fopen(private_key_file, "rb");
   if (!fp)
     goto on_error;

   pkey = PEM_read_PrivateKey(fp, NULL, cb, NULL);
   fclose(fp);
   if (!pkey)
     goto on_error;

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

# endif /* ifdef HAVE_GNUTLS */
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

# ifdef HAVE_GNUTLS
   gnutls_x509_crt_deinit(key->certificate);
   gnutls_x509_privkey_deinit(key->private_key);
# else /* ifdef HAVE_GNUTLS */
   X509_free(key->certificate);
   EVP_PKEY_free(key->private_key);
# endif /* ifdef HAVE_GNUTLS */
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
# ifdef HAVE_GNUTLS
   const char *names[6] = {
      "Modulus",
      "Public exponent",
      "Private exponent",
      "First prime",
      "Second prime",
      "Coefficient"
   };
   int err = 0;
   gnutls_datum_t data = { NULL, 0 };
   gnutls_datum_t rsa_raw[6];
   size_t size = 128;
   char *res = NULL;
   char buf[33];
   unsigned int i, j;

   if (!key)
     return;

   if (!emile_cipher_init()) return ;

   if (key->private_key)
     {
        if (gnutls_x509_privkey_export_rsa_raw(key->private_key,
                                               rsa_raw + 0, /* Modulus */
                                               rsa_raw + 1, /* Public exponent */
                                               rsa_raw + 2, /* Private exponent */
                                               rsa_raw + 3, /* First prime */
                                               rsa_raw + 4, /* Second prime */
                                               rsa_raw + 5)) /* Coefficient */
          goto on_error;

        if (!(res = malloc(size)))
          goto on_error;

        fprintf(out, "Private Key:\n");
        buf[32] = '\0';

        for (i = 0; i < 6; i++)
          {
             while ((err = gnutls_hex_encode(rsa_raw + i, res, &size)) ==
                    GNUTLS_E_SHORT_MEMORY_BUFFER)
               {
                  char *temp;

                  size += 128;
                  if (!(temp = realloc(res, size)))
                    goto on_error;
                  res = temp;
               }
             if (err)
               goto on_error;

             fprintf(out, "\t%s:\n", names[i]);
             for (j = 0; strlen(res) > j; j += 32)
               {
                  snprintf(buf, 32, "%s", res + j);
                  fprintf(out, "\t\t%s\n", buf);
               }
          }
        free(res);
        res = NULL;
     }

   if (key->certificate)
     {
        fprintf(out, "Public certificate:\n");
        if (gnutls_x509_crt_print(key->certificate, GNUTLS_X509_CRT_FULL,
                                  &data))
          goto on_error;

        fprintf(out, "%s\n", data.data);
        gnutls_free(data.data);
        data.data = NULL;
     }

on_error:
   if (res)
     free(res);

   if (data.data)
     gnutls_free(data.data);

   return;
# else /* ifdef HAVE_GNUTLS */
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
# endif /* ifdef HAVE_GNUTLS */
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
# ifdef HAVE_GNUTLS
   result = malloc(gcry_md_get_algo_dlen(GCRY_MD_SHA1));
   if (!result)
     return NULL;

   gcry_md_hash_buffer(GCRY_MD_SHA1, result, data_base, data_length);
   if (sha1_length)
     *sha1_length = gcry_md_get_algo_dlen(GCRY_MD_SHA1);

# else /* ifdef HAVE_GNUTLS */
#  ifdef HAVE_OPENSSL
   result = malloc(SHA_DIGEST_LENGTH);
   if (!result)
     return NULL;

   SHA1(data_base, data_length, result);
   if (sha1_length)
     *sha1_length = SHA_DIGEST_LENGTH;

#  else /* ifdef HAVE_OPENSSL */
   result = NULL;
#  endif /* ifdef HAVE_OPENSSL */
# endif /* ifdef HAVE_GNUTLS */
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
# ifdef HAVE_GNUTLS
   gnutls_datum_t datum = { NULL, 0 };
   size_t sign_len = 0;
   size_t cert_len = 0;
   gnutls_datum_t signum = { NULL, 0 };
   gnutls_privkey_t privkey;
# else /* ifdef HAVE_GNUTLS */
#  if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
   EVP_MD_CTX *md_ctx;
#  else
   EVP_MD_CTX md_ctx;
#  endif
   unsigned int sign_len = 0;
   int cert_len = 0;
# endif /* ifdef HAVE_GNUTLS */

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

# ifdef HAVE_GNUTLS
   datum.data = data;
   datum.size = st_buf.st_size;

   /* Get the signature length */
   if (gnutls_privkey_init(&privkey) < 0)
     {
        err = EET_ERROR_SIGNATURE_FAILED;
        goto on_error;
     }

   if (gnutls_privkey_import_x509(privkey, key->private_key, 0) < 0)
     {
        err = EET_ERROR_SIGNATURE_FAILED;
        goto on_error;
     }

   if (gnutls_privkey_sign_data(privkey, GNUTLS_DIG_SHA1, 0, &datum, &signum) < 0)
     {
        err = EET_ERROR_SIGNATURE_FAILED;
        goto on_error;
     }

   sign = signum.data;
   sign_len = signum.size;

   /* Get the certificate length */
   if (gnutls_x509_crt_export(key->certificate, GNUTLS_X509_FMT_DER, cert,
                              &cert_len) &&
       !cert_len)
     {
        err = EET_ERROR_SIGNATURE_FAILED;
        goto on_error;
     }

   /* Get the certificate */
   cert = malloc(cert_len);
   if (!cert ||
       gnutls_x509_crt_export(key->certificate, GNUTLS_X509_FMT_DER, cert,
                              &cert_len))
     {
        if (!cert)
          err = EET_ERROR_OUT_OF_MEMORY;
        else
          err = EET_ERROR_SIGNATURE_FAILED;

        goto on_error;
     }

# else /* ifdef HAVE_GNUTLS */
   sign_len = EVP_PKEY_size(key->private_key);
   sign = malloc(sign_len);
   if (!sign)
     {
        err = EET_ERROR_OUT_OF_MEMORY;
        goto on_error;
     }

   /* Do the signature. */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
   md_ctx = EVP_MD_CTX_new();
   EVP_SignInit(md_ctx, EVP_sha1());
   EVP_SignUpdate(md_ctx, data, st_buf.st_size);
   err = EVP_SignFinal(md_ctx,
                       sign,
                       (unsigned int *)&sign_len,
                       key->private_key);
   EVP_MD_CTX_free(md_ctx);
#else
   EVP_SignInit(&md_ctx, EVP_sha1());
   EVP_SignUpdate(&md_ctx, data, st_buf.st_size);
   err = EVP_SignFinal(&md_ctx,
                       sign,
                       (unsigned int *)&sign_len,
                       key->private_key);
   EVP_MD_CTX_cleanup(&md_ctx);
#endif
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

# endif /* ifdef HAVE_GNUTLS */
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
# ifdef HAVE_GNUTLS
   if (cert)
     free(cert);

# else /* ifdef HAVE_GNUTLS */
   if (cert)
     OPENSSL_free(cert);

# endif /* ifdef HAVE_GNUTLS */
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

# ifdef HAVE_GNUTLS
   gnutls_x509_crt_t cert;
   gnutls_datum_t datum;
   gnutls_datum_t signature;
   gnutls_pubkey_t pubkey;
   unsigned char *hash;
   gcry_md_hd_t md;
   int err;

   /* Create an understanding certificate structure for gnutls */
   datum.data = (void *)cert_der;
   datum.size = cert_len;
   gnutls_x509_crt_init(&cert);
   gnutls_x509_crt_import(cert, &datum, GNUTLS_X509_FMT_DER);

   signature.data = (void *)sign;
   signature.size = sign_len;

   /* Verify the signature */
   /*
      I am waiting for my patch being accepted in GnuTLS release.
      But we now have a way to prevent double computation of SHA1.
    */
   err = gcry_md_open (&md, GCRY_MD_SHA1, 0);
   if (err < 0)
     return NULL;

   gcry_md_write(md, data_base, data_length);

   hash = gcry_md_read(md, GCRY_MD_SHA1);
   if (!hash)
     goto on_error;

   datum.size = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
   datum.data = hash;

   if (gnutls_pubkey_init(&pubkey) < 0)
     goto on_error;

   if (gnutls_pubkey_import_x509(pubkey, cert, 0) < 0)
     goto on_error;

   if (gnutls_pubkey_verify_hash2(pubkey,
                                  gnutls_x509_crt_get_signature_algorithm(cert),
                                  0,
                                  &datum, &signature) < 0)
     goto on_error;

   if (sha1)
     {
        *sha1 = malloc(datum.size);
        if (!*sha1) goto on_error;

        memcpy(*sha1, hash, datum.size);
        *sha1_length = datum.size;
     }

   gcry_md_close(md);
   gnutls_x509_crt_deinit(cert);

# else /* ifdef HAVE_GNUTLS */
   const unsigned char *tmp;
   EVP_PKEY *pkey;
   X509 *x509;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
   EVP_MD_CTX *md_ctx;
#else
   EVP_MD_CTX md_ctx;
#endif
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
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
   md_ctx = EVP_MD_CTX_new();
   EVP_VerifyInit(md_ctx, EVP_sha1());
   EVP_VerifyUpdate(md_ctx, data_base, data_length);
   err = EVP_VerifyFinal(md_ctx, sign, sign_len, pkey);
   EVP_MD_CTX_free(md_ctx);
#else
   EVP_VerifyInit(&md_ctx, EVP_sha1());
   EVP_VerifyUpdate(&md_ctx, data_base, data_length);
   err = EVP_VerifyFinal(&md_ctx, sign, sign_len, pkey);
   EVP_MD_CTX_cleanup(&md_ctx);
#endif

   X509_free(x509);
   EVP_PKEY_free(pkey);

   if (sha1)
     {
        *sha1 = NULL;
        *sha1_length = -1;
     }

   if (err != 1)
     return NULL;

# endif /* ifdef HAVE_GNUTLS */
   if (x509_length)
     *x509_length = cert_len;

   if (raw_signature_base)
     *raw_signature_base = sign;

   if (raw_signature_length)
     *raw_signature_length = sign_len;

   return cert_der;
# ifdef HAVE_GNUTLS
 on_error:
   gcry_md_close(md);
   return NULL;
# endif
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

# ifdef HAVE_GNUTLS
   gnutls_datum_t datum;
   gnutls_x509_crt_t cert;

   /* Create an understanding certificate structure for gnutls */
   datum.data = (void *)certificate;
   datum.size = der_length;
   if (gnutls_x509_crt_init(&cert))
     goto on_error;

   if (gnutls_x509_crt_import(cert, &datum, GNUTLS_X509_FMT_DER))
     goto on_error;

   /* Pretty print the certificate */
   datum.data = NULL;
   datum.size = 0;
   if (gnutls_x509_crt_print(cert, GNUTLS_X509_CRT_FULL, &datum))
     goto on_error;

   INF("Public certificate :");
   INF("%s", datum.data);

on_error:
   if (datum.data)
     gnutls_free(datum.data);

   gnutls_x509_crt_deinit(cert);
# else /* ifdef HAVE_GNUTLS */
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
# endif /* ifdef HAVE_GNUTLS */
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
