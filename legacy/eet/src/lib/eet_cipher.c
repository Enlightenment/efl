#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_SIGNATURE
# ifdef HAVE_GNUTLS
#  include <gnutls/gnutls.h>
#  include <gnutls/x509.h>
# else
#  include <openssl/rsa.h>
#  include <openssl/objects.h>
#  include <openssl/err.h>
#  include <openssl/ssl.h>
#  include <openssl/dh.h>
#  include <openssl/dsa.h>
#  include <openssl/evp.h>
#  include <openssl/x509.h>
#  include <openssl/pem.h>
# endif
#endif

#ifdef HAVE_OPENSSL
#  include <openssl/sha.h>
#endif

#ifdef HAVE_CIPHER
# ifdef HAVE_GNUTLS
#  include <gnutls/x509.h>
#  include <gcrypt.h>
# else
#  include <openssl/evp.h>
#  include <openssl/hmac.h>
#  include <openssl/rand.h>
# endif
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include "Eet.h"
#include "Eet_private.h"

#define EET_MAGIC_SIGN 0x1ee74271

#ifdef HAVE_GNUTLS
# define MAX_KEY_LEN 32
# define MAX_IV_LEN 16
#else
# define MAX_KEY_LEN EVP_MAX_KEY_LENGTH
# define MAX_IV_LEN EVP_MAX_IV_LENGTH
#endif

#ifdef HAVE_CIPHER
# ifdef HAVE_GNUTLS
static Eet_Error eet_hmac_sha1(const void *key, size_t key_len, const void *data, size_t data_len, unsigned char *res);
# endif
static Eet_Error eet_pbkdf2_sha1(const char *key, int key_len, const unsigned char *salt, unsigned int salt_len, int iter, unsigned char *res, int res_len);
#endif

struct _Eet_Key
{
   int          references;
#ifdef HAVE_SIGNATURE
# ifdef HAVE_GNUTLS
   gnutls_x509_crt_t		certificate;
   gnutls_x509_privkey_t	private_key;
# else
   X509	       *certificate;
   EVP_PKEY    *private_key;
# endif
#endif
};

EAPI Eet_Key*
eet_identity_open(const char *certificate_file, const char *private_key_file, Eet_Key_Password_Callback cb)
{
#ifdef HAVE_SIGNATURE
  /* Signature declarations */
  Eet_Key *key = NULL;
  FILE *fp = NULL;
# ifdef HAVE_GNUTLS
  /* Gnutls private declarations */
  int fd = -1;
  struct stat st;
  void *data = NULL;
  gnutls_datum_t load_file = { NULL, 0 };
  char pass[1024];

  /* Init */
  if (!(key = malloc(sizeof(Eet_Key)))) goto on_error;
  if (gnutls_x509_crt_init(&(key->certificate))) goto on_error;
  if (gnutls_x509_privkey_init(&(key->private_key))) goto on_error;

  /* Mmap certificate_file */
  if (!(fp = fopen(certificate_file, "r"))) goto on_error;
  if ((fd = fileno(fp)) == -1) goto on_error;
  if (fstat(fd, &st)) goto on_error;
  if ((data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) goto on_error;

  /* Import the certificate in Eet_Key structure */
  load_file.data = data;
  load_file.size = st.st_size;
  if (gnutls_x509_crt_import(key->certificate, &load_file, GNUTLS_X509_FMT_PEM) < 0)
    goto on_error;
  if (munmap(data, st.st_size)) goto on_error;

  /* Reset values */
  fclose(fp);
  fp = NULL;
  data = NULL;
  load_file.data = NULL;
  load_file.size = 0;

  /* Mmap private_key_file */
  if (!(fp = fopen(private_key_file, "r"))) goto on_error;
  if ((fd = fileno(fp)) == -1) goto on_error;
  if (fstat(fd, &st)) goto on_error;
  if ((data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) goto on_error;

  /* Import the private key in Eet_Key structure */
  load_file.data = data;
  load_file.size = st.st_size;
  /* Try to directly import the PEM encoded private key */
  if (gnutls_x509_privkey_import(key->private_key, &load_file, GNUTLS_X509_FMT_PEM) < 0)
    {
      /* Else ask for the private key pass */
      if (cb && cb(pass, 1024, 0, NULL))
	{
	  /* If pass then try to decode the pkcs 8 private key */
	  if (gnutls_x509_privkey_import_pkcs8(key->private_key, &load_file, GNUTLS_X509_FMT_PEM, pass, 0))
	    goto on_error;
	}
      else
	{
	  /* Else try to import the pkcs 8 private key without pass */
	  if (gnutls_x509_privkey_import_pkcs8(key->private_key, &load_file, GNUTLS_X509_FMT_PEM, NULL, 1))
	    goto on_error;
	}
    }
  if (munmap(data, st.st_size)) goto on_error;
  fclose(fp);

  return key;

 on_error:
  if (fp) fclose(fp);
  if (key)
    {
      if (key->certificate) gnutls_x509_crt_deinit(key->certificate);
      if (key->private_key) gnutls_x509_privkey_deinit(key->private_key);
      free(key);
    }
  if (data) munmap(data, st.st_size);
# else
  /* Openssl private declarations */
  EVP_PKEY *pkey = NULL;
  X509 *cert = NULL;

  /* Load the X509 certificate in memory. */
  fp = fopen(certificate_file, "r");
  if (!fp) return NULL;
  cert = PEM_read_X509(fp, NULL, NULL, NULL);
  fclose(fp);
  if (!cert) goto on_error;

  /* Check the presence of the public key. Just in case. */
  pkey = X509_get_pubkey(cert);
  if (!pkey) goto on_error;

  /* Load the private key in memory. */
  fp = fopen(private_key_file, "r");
  if (!fp) goto on_error;
  pkey = PEM_read_PrivateKey(fp, NULL, cb, NULL);
  fclose(fp);
  if (!pkey) goto on_error;

  /* Load the certificate and the private key in Eet_Key structure */
  key = malloc(sizeof(Eet_Key));
  if (!key) goto on_error;
  key->references = 1;
  key->certificate = cert;
  key->private_key = pkey;

  return key;

 on_error:
  if (cert) X509_free(cert);
  if (pkey) EVP_PKEY_free(pkey);
# endif
#endif
  return NULL;
}

EAPI void
eet_identity_close(Eet_Key *key)
{
#ifdef HAVE_SIGNATURE
  if (!key || (key->references > 0)) return ;
# ifdef HAVE_GNUTLS
  gnutls_x509_crt_deinit(key->certificate);
  gnutls_x509_privkey_deinit(key->private_key);
# else
  X509_free(key->certificate);
  EVP_PKEY_free(key->private_key);
# endif
  free(key);
#endif
}

EAPI void
eet_identity_print(Eet_Key *key, FILE *out)
{
#ifdef HAVE_SIGNATURE
# ifdef HAVE_GNUTLS
  const char	*names[6] = {
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

  if (!key) return ;

  if (key->private_key)
    {
      if (gnutls_x509_privkey_export_rsa_raw(key->private_key,
					     rsa_raw + 0,  /* Modulus */
					     rsa_raw + 1,  /* Public exponent */
					     rsa_raw + 2,  /* Private exponent */
					     rsa_raw + 3,  /* First prime */
					     rsa_raw + 4,  /* Second prime */
					     rsa_raw + 5)) /* Coefficient */
	goto on_error;
      if (!(res = malloc(size))) goto on_error;

      fprintf(out, "Private Key:\n");
      buf[32] = '\0';

      for (i = 0; i < 6; i++)
	{
	  while ((err = gnutls_hex_encode(rsa_raw + i, res, &size)) == GNUTLS_E_SHORT_MEMORY_BUFFER)
	    {
	      size += 128;
	      if (!(res = realloc(res, size))) goto on_error;
	    }
	  if (err) goto on_error;

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
      if (gnutls_x509_crt_print(key->certificate, GNUTLS_X509_CRT_FULL, &data)) goto on_error;
      fprintf(out, "%s", data.data);
      gnutls_free(data.data);
      data.data = NULL;
    }

 on_error:
  if (res) free(res);
  if (data.data) gnutls_free(data.data);
  return ;
# else
  RSA *rsa;
  DSA *dsa;
  DH *dh;

  if (!key) return ;

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
# endif
#else
  fprintf(out, "You need to compile signature support in EET.\n");
#endif
}

void
eet_identity_ref(Eet_Key *key)
{
   if (key == NULL) return ;
   key->references++;
}

void
eet_identity_unref(Eet_Key *key)
{
   if (key == NULL) return ;
   key->references--;
   eet_identity_close(key);
}

void *
eet_identity_compute_sha1(const void *data_base, unsigned int data_length,
			  int *sha1_length)
{
   void *result;

#ifdef HAVE_GNUTLS
   result = malloc(gcry_md_get_algo_dlen(GCRY_MD_SHA1));
   if (!result) return NULL;

   gcry_md_hash_buffer(GCRY_MD_SHA1, result, data_base, data_length);
#else
# ifdef HAVE_OPENSSL
   result = malloc(SHA_DIGEST_LENGTH);
   if (!result) return NULL;

   SHA1(data_base, data_length, result);
# else
   result = NULL;
# endif
#endif

   return result;
}

Eet_Error
eet_identity_sign(FILE *fp, Eet_Key *key)
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
# else
   EVP_MD_CTX md_ctx;
   unsigned int sign_len = 0;
   int cert_len = 0;
# endif

   /* A few check and flush pending write. */
   if (!fp || !key || !key->certificate || !key->private_key)
     return EET_ERROR_BAD_OBJECT;

   /* Get the file size. */
   fd = fileno(fp);
   if (fd < 0) return EET_ERROR_BAD_OBJECT;
   if (fstat(fd, &st_buf) < 0) return EET_ERROR_MMAP_FAILED;

   /* Map the file in memory. */
   data = mmap(NULL, st_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
   if (data == MAP_FAILED) return EET_ERROR_MMAP_FAILED;

# ifdef HAVE_GNUTLS
   datum.data = data;
   datum.size = st_buf.st_size;

   /* Get the signature length */
   if (gnutls_x509_privkey_sign_data(key->private_key, GNUTLS_DIG_SHA1, 0, &datum, sign, &sign_len) &&
       !sign_len)
     {
       err = EET_ERROR_SIGNATURE_FAILED;
       goto on_error;
     }

   /* Get the signature */
   sign = malloc(sign_len);
   if (!sign || gnutls_x509_privkey_sign_data(key->private_key, GNUTLS_DIG_SHA1, 0, &datum, sign, &sign_len))
     {
       if (!sign) err = EET_ERROR_OUT_OF_MEMORY;
       else err = EET_ERROR_SIGNATURE_FAILED;
       goto on_error;
     }

   /* Get the certificate length */
   if (gnutls_x509_crt_export(key->certificate, GNUTLS_X509_FMT_DER, cert, &cert_len) &&
       !cert_len)
     {
       err = EET_ERROR_SIGNATURE_FAILED;
       goto on_error;
     }

   /* Get the certificate */
   cert = malloc(cert_len);
   if (!cert || gnutls_x509_crt_export(key->certificate, GNUTLS_X509_FMT_DER, cert, &cert_len))
     {
       if (!cert) err = EET_ERROR_OUT_OF_MEMORY;
       else err = EET_ERROR_SIGNATURE_FAILED;
       goto on_error;
     }
# else
   sign_len = EVP_PKEY_size(key->private_key);
   sign = malloc(sign_len);
   if (sign == NULL)
     {
	err = EET_ERROR_OUT_OF_MEMORY;
	goto on_error;
     }

   /* Do the signature. */
   EVP_SignInit(&md_ctx, EVP_sha1());
   EVP_SignUpdate(&md_ctx, data, st_buf.st_size);
   err = EVP_SignFinal(&md_ctx, sign, (unsigned int *)&sign_len, key->private_key);
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
# endif
   /* Append the signature at the end of the file. */
   head[0] = (int) htonl ((unsigned int) EET_MAGIC_SIGN);
   head[1] = (int) htonl ((unsigned int) sign_len);
   head[2] = (int) htonl ((unsigned int) cert_len);

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
   if (cert) free(cert);
# else
   if (cert) OPENSSL_free(cert);
# endif
   if (sign) free(sign);
   munmap(data, st_buf.st_size);
   return err;
#else
   return EET_ERROR_NOT_IMPLEMENTED;
#endif
}

const void*
eet_identity_check(const void *data_base, unsigned int data_length,
		   const void *signature_base, unsigned int signature_length,
		   const void **raw_signature_base, unsigned int *raw_signature_length,
		   int *x509_length)
{
#ifdef HAVE_SIGNATURE
   const int *header = signature_base;
   const unsigned char *sign;
   const unsigned char *cert_der;
   int sign_len;
   int cert_len;
   int magic;

   /* At least the header size */
   if (signature_length < sizeof(int) * 3) return NULL;

   /* Get the header */
   magic = ntohl(header[0]);
   sign_len = ntohl(header[1]);
   cert_len = ntohl(header[2]);

   /* Verify the header */
   if (magic != EET_MAGIC_SIGN) return NULL;
   if (sign_len + cert_len + sizeof(int) * 3 > signature_length) return NULL;

   /* Update the signature and certificate pointer */
   sign = signature_base + sizeof(int) * 3;
   cert_der = sign + sign_len;

# ifdef HAVE_GNUTLS
   gnutls_x509_crt_t cert;
   gnutls_datum_t datum;
   gnutls_datum_t signature;

   /* Create an understanding certificate structure for gnutls */
   datum.data = (void *)cert_der;
   datum.size = cert_len;
   gnutls_x509_crt_init(&cert);
   gnutls_x509_crt_import(cert, &datum, GNUTLS_X509_FMT_DER);

   signature.data = (void *)sign;
   signature.size = sign_len;

   /* Verify the signature */
   datum.data = (void *)data_base;
   datum.size = data_length;
   if (!gnutls_x509_crt_verify_data(cert, 0, &datum, &signature))
     return NULL;
# else
   const unsigned char *tmp;
   EVP_PKEY *pkey;
   X509 *x509;
   EVP_MD_CTX md_ctx;
   int err;

   /* Strange but d2i_X509 seems to put 0 all over the place. */
   tmp = alloca(cert_len);
   memcpy((char*) tmp, cert_der, cert_len);
   x509 = d2i_X509(NULL, &tmp, cert_len);
   if (x509 == NULL) return NULL;

   /* Get public key - eay */
   pkey=X509_get_pubkey(x509);
   if (pkey == NULL)
     {
	X509_free(x509);
	return NULL;
     }

   /* Verify the signature */
   EVP_VerifyInit(&md_ctx, EVP_sha1());
   EVP_VerifyUpdate(&md_ctx, data_base, data_length);
   err = EVP_VerifyFinal(&md_ctx, sign, sign_len, pkey);

   X509_free(x509);
   EVP_PKEY_free(pkey);

   if (err != 1)
     return NULL;
# endif
   if (x509_length) *x509_length = cert_len;
   if (raw_signature_base) *raw_signature_base = sign;
   if (raw_signature_length) *raw_signature_length = sign_len;
   return cert_der;
#else
   return NULL;
#endif
}

EAPI void
eet_identity_certificate_print(const unsigned char *certificate, int der_length, FILE *out)
{
#ifdef HAVE_SIGNATURE
  if (!certificate || !out || der_length <= 0)
     {
	fprintf(out, "No certificate provided.\n");
	return ;
     }
# ifdef HAVE_GNUTLS
  gnutls_datum_t datum;
  gnutls_x509_crt_t cert;

  /* Create an understanding certificate structure for gnutls */
  datum.data = (void *)certificate;
  datum.size = der_length;
  if (gnutls_x509_crt_init(&cert)) goto on_error;
  if (gnutls_x509_crt_import(cert, &datum, GNUTLS_X509_FMT_DER)) goto on_error;

  /* Pretty print the certificate */
  datum.data = NULL;
  datum.size = 0;
  if (gnutls_x509_crt_print(cert, GNUTLS_X509_CRT_FULL, &datum)) goto on_error;
  fprintf(out, "Public certificate :\n");
  fprintf(out, "%s", datum.data);

 on_error:
  if (datum.data) gnutls_free(datum.data);
  gnutls_x509_crt_deinit(cert);
# else
   const unsigned char *tmp;
   X509 *x509;

   /* Strange but d2i_X509 seems to put 0 all over the place. */
   tmp = alloca(der_length);
   memcpy((char*) tmp, certificate, der_length);
   x509 = d2i_X509(NULL, &tmp, der_length);
   if (x509 == NULL)
     {
	fprintf(out, "Not a valid certificate.\n");
	return ;
     }

   fprintf(out, "Public certificate :\n");
   X509_print_fp(out, x509);

   X509_free(x509);
# endif
#else
   fprintf(out, "You need to compile signature support in EET.\n");
#endif
}

Eet_Error
eet_cipher(const void *data, unsigned int size, const char *key, unsigned int length, void **result, unsigned int *result_length)
{
#ifdef HAVE_CIPHER
  /* Cipher declarations */
   unsigned int *ret = NULL;
   unsigned char iv[MAX_IV_LEN];
   unsigned char ik[MAX_KEY_LEN];
   unsigned char key_material[MAX_IV_LEN + MAX_KEY_LEN];
   unsigned int salt;
   unsigned int tmp = 0;
   int crypted_length;
   int opened = 0;
# ifdef HAVE_GNUTLS
   /* Gcrypt declarations */
   gcry_error_t err = 0;
   gcry_cipher_hd_t cipher;
# else
   /* Openssl declarations*/
   EVP_CIPHER_CTX ctx;
   unsigned int *buffer;
   int tmp_len;
# endif

# ifdef HAVE_GNUTLS
   /* Gcrypt salt generation */
   gcry_create_nonce((unsigned char *)&salt, sizeof(salt));
# else
   /* Openssl salt generation */
   if (!RAND_bytes((unsigned char*)&salt, sizeof (unsigned int)))
     return EET_ERROR_PRNG_NOT_SEEDED;
#endif

   eet_pbkdf2_sha1(key, length, (unsigned char *)&salt, sizeof(unsigned int), 2048, key_material, MAX_KEY_LEN + MAX_IV_LEN);

   memcpy(iv, key_material, MAX_IV_LEN);
   memcpy(ik, key_material + MAX_IV_LEN, MAX_KEY_LEN);

   crypted_length = ((((size + sizeof (unsigned int)) >> 5) + 1) << 5);
   ret = malloc(crypted_length + sizeof(unsigned int));
   if (!ret) return EET_ERROR_OUT_OF_MEMORY;

   *ret = salt;
   tmp = htonl(size);

#ifdef HAVE_GNUTLS
   *(ret + 1) = tmp;
   memcpy(ret + 2, data, size);

   /* Gcrypt create the corresponding cipher
      AES with a 256 bit key, Cipher Block Chaining mode */
   err = gcry_cipher_open(&cipher, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, 0);
   if (err) goto on_error;
   opened = 1;
   err = gcry_cipher_setiv(cipher, iv, MAX_IV_LEN);
   if (err) goto on_error;
   err = gcry_cipher_setkey(cipher, ik, MAX_KEY_LEN);
   if (err) goto on_error;

   /* Gcrypt encrypt */
   err = gcry_cipher_encrypt(cipher, (unsigned char *)(ret + 1), crypted_length, NULL, 0);
   if (err) goto on_error;

   /* Gcrypt close the cipher */
   gcry_cipher_close(cipher);
# else
   buffer = alloca(crypted_length);
   *buffer = tmp;

   memcpy(buffer + 1, data, size);

   /* Openssl create the corresponding cipher
      AES with a 256 bit key, Cipher Block Chaining mode */
   EVP_CIPHER_CTX_init(&ctx);
   if (!EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, ik, iv)) goto on_error;
   opened = 1;

   /* Openssl encrypt */
   if (!EVP_EncryptUpdate(&ctx, (unsigned char*)(ret + 1), &tmp_len, (unsigned char*) buffer, size + sizeof (unsigned int)))
     goto on_error;

   /* Openssl close the cipher */
   if (!EVP_EncryptFinal_ex(&ctx, ((unsigned char*)(ret + 1)) + tmp_len, &tmp_len))
     goto on_error;
   EVP_CIPHER_CTX_cleanup(&ctx);
# endif

   /* Set return values */
   if (result_length) *result_length = crypted_length + sizeof(unsigned int);
   if (result) *result = ret;
   else free(ret);

   return EET_ERROR_NONE;

 on_error:
# ifdef HAVE_GNUTLS
   /* Gcrypt error */
   if (opened) gcry_cipher_close(cipher);
# else
   /* Openssl error */
   if (opened) EVP_CIPHER_CTX_cleanup(&ctx);
# endif
   /* General error */
   if (ret) free(ret);
   if (result) *result = NULL;
   if (result_length) *result_length = 0;

   return EET_ERROR_ENCRYPT_FAILED;
#else
   /* Cipher not supported */
   (void) data;
   (void) size;
   (void) key;
   (void) length;
   (void) result;
   (void) result_length;
   return EET_ERROR_NOT_IMPLEMENTED;
#endif
}

Eet_Error
eet_decipher(const void *data, unsigned int size, const char *key, unsigned int length, void **result, unsigned int *result_length)
{
#ifdef HAVE_CIPHER
   const unsigned int *over = data;
   unsigned int *ret = NULL;
   unsigned char ik[MAX_KEY_LEN];
   unsigned char iv[MAX_IV_LEN];
   unsigned char key_material[MAX_KEY_LEN + MAX_IV_LEN];
   unsigned int salt;
   int tmp_len;
   int tmp = 0;

   /* At least the salt and an AES block */
   if (size < sizeof(unsigned int) + 16) return EET_ERROR_BAD_OBJECT;

   /* Get the salt */
   salt = *over;

   /* Generate the iv and the key with the salt */
   eet_pbkdf2_sha1(key, length, (unsigned char *)&salt, sizeof(unsigned int), 2048, key_material, MAX_KEY_LEN + MAX_IV_LEN);
   memcpy(iv, key_material, MAX_IV_LEN);
   memcpy(ik, key_material + MAX_IV_LEN, MAX_KEY_LEN);

   /* Align to AES block size if size is not align */
   tmp_len = size - sizeof (unsigned int);
   if ((tmp_len & 0x1F) != 0) goto on_error;

   ret = malloc(tmp_len);
   if (!ret) goto on_error;

# ifdef HAVE_GNUTLS
   gcry_error_t err = 0;
   gcry_cipher_hd_t cipher;

   /* Gcrypt create the corresponding cipher */
   err = gcry_cipher_open(&cipher, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, 0);
   if (err) return EET_ERROR_DECRYPT_FAILED;
   err = gcry_cipher_setiv(cipher, iv, MAX_IV_LEN);
   if (err) goto on_error;
   err = gcry_cipher_setkey(cipher, ik, MAX_KEY_LEN);
   if (err) goto on_error;

   /* Gcrypt decrypt */
   err = gcry_cipher_decrypt(cipher, ret, tmp_len, ((unsigned int *)data) + 1, tmp_len);
   if (err) goto on_error;

   /* Gcrypt close the cipher */
   gcry_cipher_close(cipher);

# else
   EVP_CIPHER_CTX ctx;
   int opened = 0;

   /* Openssl create the corresponding cipher */
   EVP_CIPHER_CTX_init(&ctx);
   opened = 1;

   if (!EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, ik, iv))
     goto on_error;

   /* Openssl decrypt */
   if (!EVP_DecryptUpdate(&ctx, (unsigned char *) ret, &tmp,
			  (unsigned char *) (over + 1), tmp_len))
     goto on_error;

   /* Openssl close the cipher*/
   EVP_CIPHER_CTX_cleanup(&ctx);
# endif
   /* Get the decrypted data size */
   tmp = *ret;
   tmp = ntohl(tmp);
   if (tmp > tmp_len) goto on_error;

   /* Update the return values */
   if (result_length) *result_length = tmp;
   if (result)
     {
       *result = NULL;
       *result = malloc(tmp);
       if (!*result)
	 goto on_error;
       memcpy(*result, ret + 1, tmp);
     }
   free(ret);

   return EET_ERROR_NONE;

 on_error:
# ifdef HAVE_GNUTLS
# else
   if (opened) EVP_CIPHER_CTX_cleanup(&ctx);
# endif
   if (result) *result = NULL;
   if (result_length) *result_length = 0;
   if (ret) free(ret);

   return EET_ERROR_DECRYPT_FAILED;
#else
   (void) data;
   (void) size;
   (void) key;
   (void) length;
   (void) result;
   (void) result_length;
   return EET_ERROR_NOT_IMPLEMENTED;
#endif
}

#ifdef HAVE_CIPHER
# ifdef HAVE_GNUTLS
static Eet_Error
eet_hmac_sha1(const void *key, size_t key_len,
              const void *data, size_t data_len, unsigned char *res)
{
  size_t hlen = gcry_md_get_algo_dlen (GCRY_MD_SHA1);
  gcry_md_hd_t mdh;
  unsigned char *hash;
  gpg_error_t err;

  err = gcry_md_open(&mdh, GCRY_MD_SHA1, GCRY_MD_FLAG_HMAC);
  if (err != GPG_ERR_NO_ERROR)
    return 1;

  err = gcry_md_setkey(mdh, key, key_len);
  if (err != GPG_ERR_NO_ERROR)
    {
      gcry_md_close(mdh);
      return 1;
    }

  gcry_md_write(mdh, data, data_len);

  hash = gcry_md_read(mdh, GCRY_MD_SHA1);
  if (hash == NULL)
    {
      gcry_md_close(mdh);
      return 1;
    }

  memcpy(res, hash, hlen);

  gcry_md_close(mdh);

  return 0;
}
# endif

static Eet_Error
eet_pbkdf2_sha1(const char          *key,
		int                  key_len,
		const unsigned char *salt,
		unsigned int         salt_len,
		int                  iter,
		unsigned char       *res,
		int                  res_len)
{
  unsigned char         digest[20];
  unsigned char         tab[4];
  unsigned char	       *p = res;
  unsigned char        *buf;
  unsigned long         i;
  int                   digest_len = 20;
  int                   len = res_len;
  int                   tmp_len;
  int                   j, k;
# ifdef HAVE_GNUTLS
# else
  HMAC_CTX              hctx;
# endif

  buf = malloc(salt_len + 4);
  if (!buf) return 1;

  for (i = 1; len; len -= tmp_len, p += tmp_len, i++)
    {
      if (len > digest_len) tmp_len = digest_len;
      else tmp_len = len;

      tab[0] = (unsigned char)(i & 0xff000000) >> 24;
      tab[1] = (unsigned char)(i & 0x00ff0000) >> 16;
      tab[2] = (unsigned char)(i & 0x0000ff00) >> 8;
      tab[3] = (unsigned char)(i & 0x000000ff) >> 0;

# ifdef HAVE_GNUTLS
      memcpy(buf, salt, salt_len);
      memcpy(buf + salt_len, tab, 4);
      eet_hmac_sha1(key, key_len, buf, salt_len + 4, digest);
# else
      HMAC_Init(&hctx, key, key_len, EVP_sha1());
      HMAC_Update(&hctx, salt, salt_len);
      HMAC_Update(&hctx, tab, 4);
      HMAC_Final(&hctx, digest, NULL);
# endif
      memcpy(p, digest, tmp_len);

      for (j = 1; j < iter; j++)
	{
# ifdef HAVE_GNUTLS
	  eet_hmac_sha1(key, key_len, digest, 20, digest);
# else
	  HMAC(EVP_sha1(), key, key_len, digest, 20, digest, NULL);
# endif
	  for (k = 0; k < tmp_len; k++)
	    p[k] ^= digest[k];
	}
    }

# ifdef HAVE_GNUTLS
# else
  HMAC_cleanup(&hctx);
# endif
  return 0;
}
#endif
