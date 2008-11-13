#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef HAVE_SIGNATURE
# include <openssl/rsa.h>
# include <openssl/objects.h>
# include <openssl/err.h>
# include <openssl/ssl.h>
# include <openssl/dh.h>
# include <openssl/dsa.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#include "Eet.h"
#include "Eet_private.h"

#define EET_MAGIC_SIGN 0x1ee74271

EAPI Eet_Key*
eet_identity_open(const char *certificate_file, const char *private_key_file, Eet_Key_Password_Callback cb)
{
#ifdef HAVE_SIGNATURE
   EVP_PKEY *pkey = NULL;
   X509 *cert = NULL;
   Eet_Key *key;
   FILE *fp;

   /* Load the X509 certificate in memory. */
   fp = fopen(certificate_file, "r");
   if (!fp) return NULL;
   cert = PEM_read_X509(fp, NULL, NULL, NULL);
   fclose(fp);
   if (!cert) return NULL;

   /* Check the presence of the public key. Just in case. */
   pkey = X509_get_pubkey(cert);
   if (!pkey) goto on_error;

   /* Load the private key in memory. */
   fp = fopen(private_key_file, "r");
   if (!fp) goto on_error;
   pkey = PEM_read_PrivateKey(fp, NULL, cb, NULL);
   fclose(fp);
   if (!pkey) goto on_error;

   key = malloc(sizeof(Eet_Key));
   if (!key) goto on_error;

   key->references = 1;
   key->certificate = cert;
   key->private_key = pkey;

   return key;

 on_error:
   if (cert) X509_free(cert);
   if (pkey) EVP_PKEY_free(pkey);
#endif
   return NULL;
}

EAPI void
eet_identity_print(Eet_Key *key, FILE *out)
{
#ifdef HAVE_SIGNATURE
   RSA *rsa;
   DSA *dsa;
   DH *dh;

   if (!key) return ;

   rsa = EVP_PKEY_get1_RSA(key->private_key);
   if (rsa)
     {
	fprintf(out, "Private key (RSA) :\n");
	RSA_print_fp(out, rsa, 0);
     }

   dsa = EVP_PKEY_get1_DSA(key->private_key);
   if (dsa)
     {
	fprintf(out, "Private key (DSA) :\n");
	DSA_print_fp(out, dsa, 0);
     }

   dh = EVP_PKEY_get1_DH(key->private_key);
   if (dh)
     {
	fprintf(out, "Private key (DH) :\n");
	DHparams_print_fp(out, dh);
     }

   fprintf(out, "Public certificate :\n");
   X509_print_fp(out, key->certificate);
#else
   fprintf(out, "You need to compile signature support in EET.\n");
#endif
}

EAPI void
eet_identity_close(Eet_Key *key)
{
#ifdef HAVE_SIGNATURE
   if (key->references > 0) return ;

   X509_free(key->certificate);
   EVP_PKEY_free(key->private_key);
   free(key);
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

Eet_Error
eet_identity_sign(FILE *fp, Eet_Key *key)
{
#ifdef HAVE_SIGNATURE
   Eet_Error err = EET_ERROR_NONE;
   unsigned char *x509_der = NULL;
   void *sign = NULL;
   void *data;
   int head[3];
   unsigned int sign_length;
   int x509_length;
   int fd;
   EVP_MD_CTX md_ctx;
   struct stat st_buf;

   /* A few check and flush pending write. */
   if (!fp
       || !key
       || !key->certificate
       || !key->private_key)
     return EET_ERROR_BAD_OBJECT;

   /* Get the file size. */
   fd = fileno(fp);
   if (fd < 0) return EET_ERROR_BAD_OBJECT;
   if (fstat(fd, &st_buf) < 0) return EET_ERROR_MMAP_FAILED;

   /* Map the file in memory. */
   data = mmap(NULL, st_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
   if (data == MAP_FAILED) return EET_ERROR_MMAP_FAILED;

   sign_length = EVP_PKEY_size(key->private_key);
   sign = malloc(sign_length);
   if (sign == NULL)
     {
	err = EET_ERROR_OUT_OF_MEMORY;
	goto on_error;
     }

   /* Do the signature. */
   EVP_SignInit(&md_ctx, EVP_sha1());
   EVP_SignUpdate (&md_ctx, data, st_buf.st_size);
   err = EVP_SignFinal (&md_ctx, sign, &sign_length, key->private_key);
   if (err != 1)
     {
	ERR_print_errors_fp(stdout);
	err = EET_ERROR_SIGNATURE_FAILED;
	goto on_error;
     }

   /* Give me the der (binary form for X509). */
   x509_length = i2d_X509(key->certificate, &x509_der);
   if (x509_length < 0)
     {
	ERR_print_errors_fp(stdout);
	err = EET_ERROR_X509_ENCODING_FAILED;
	goto on_error;
     }

   /* Append the signature at the end of the file. */
   head[0] = (int) htonl ((unsigned int) EET_MAGIC_SIGN);
   head[1] = (int) htonl ((unsigned int) sign_length);
   head[2] = (int) htonl ((unsigned int) x509_length);

   if (fwrite(head, sizeof(head), 1, fp) != 1)
     {
	err = EET_ERROR_WRITE_ERROR;
	goto on_error;
     }
   if (fwrite(sign, sign_length, 1, fp) != 1)
     {
	err = EET_ERROR_WRITE_ERROR;
	goto on_error;
     }
   if (fwrite(x509_der, x509_length, 1, fp) != 1)
     {
	err = EET_ERROR_WRITE_ERROR;
	goto on_error;
     }

 on_error:
   if (x509_der) OPENSSL_free(x509_der);
   if (sign != NULL) free(sign);
   munmap(data, st_buf.st_size);
   return err;
#else
   return EET_ERROR_NOT_IMPLEMENTED;
#endif
}

const void*
eet_identity_check(const void *data_base, unsigned int data_length,
		   const void *signature_base, unsigned int signature_length,
		   int *x509_length)
{
#ifdef HAVE_SIGNATURE
   const int *header = signature_base;
   const unsigned char *sign;
   const unsigned char *cert_der;
   const unsigned char *tmp;
   EVP_PKEY *pkey;
   X509 *x509;
   EVP_MD_CTX md_ctx;
   int sign_length;
   int cert_length;
   int magic;
   int err;

   if (signature_length < sizeof(int) * 3) return NULL;

   magic = ntohl(header[0]);
   sign_length = ntohl(header[1]);
   cert_length = ntohl(header[2]);

   if (magic != EET_MAGIC_SIGN) return NULL;
   if (sign_length + cert_length + sizeof(int) * 3 > signature_length) return NULL;

   sign = signature_base + sizeof(int) * 3;
   cert_der = sign + sign_length;

   /* Strange but d2i_X509 seems to put 0 all over the place. */
   tmp = alloca(cert_length);
   memcpy((char*) tmp, cert_der, cert_length);
   x509 = d2i_X509(NULL, &tmp, cert_length);
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
   err = EVP_VerifyFinal(&md_ctx, sign, sign_length, pkey);

   X509_free(x509);
   EVP_PKEY_free(pkey);

   if (err != 1)
     return NULL;

   if (x509_length) *x509_length = cert_length;
   return cert_der;
#else
   return NULL;
#endif
}

EAPI void
eet_identity_certificate_print(const unsigned char *certificate, int der_length, FILE *out)
{
#ifdef HAVE_SIGNATURE
   const unsigned char *tmp;
   X509 *x509;

   if (certificate == NULL
       || out == NULL
       || der_length <= 0)
     {
	fprintf(out, "No certificate provided.\n");
	return ;
     }

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
#else
   fprintf(out, "You need to compile signature support in EET.\n");
#endif
}

Eet_Error
eet_cypher(void *data, unsigned int size, const char *key, unsigned int length)
{
#ifdef HAVE_CYPHER
   (void) data;
   (void) size;
   (void) key;
   (void) length;
   return EET_ERROR_NOT_IMPLEMENTED;
#else
   (void) data;
   (void) size;
   (void) key;
   (void) length;
   return EET_ERROR_NOT_IMPLEMENTED;
#endif
}

Eet_Error
eet_decypher(void *data, unsigned int size, const char *key, unsigned int length)
{
#ifdef HAVE_CYPHER
   (void) data;
   (void) size;
   (void) key;
   (void) length;
   return EET_ERROR_NOT_IMPLEMENTED;
#else
   (void) data;
   (void) size;
   (void) key;
   (void) length;
   return EET_ERROR_NOT_IMPLEMENTED;
#endif
}
