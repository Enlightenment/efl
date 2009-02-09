/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

typedef struct _Eet_String              Eet_String;

struct _Eet_String
{
  const char            *mmap;
  char                  *str;

  int                    hash;
  int                    len;

  int                    next;
  int                    prev;

  union
  {
    float                f;
    double               d;
  } convert;

  struct
  {
    unsigned int         converted : 1;
    unsigned int         is_float : 1;
  } flags;
};
struct _Eet_Dictionary
{
  Eet_String   *all;

  int           size;
  int           offset;

  int           hash[256];

  int           count;
  int           total;

  const char   *start;
  const char   *end;
};

Eet_Dictionary  *eet_dictionary_add(void);
void             eet_dictionary_free(Eet_Dictionary *ed);
int              eet_dictionary_string_add(Eet_Dictionary *ed, const char *string);
int              eet_dictionary_string_get_size(const Eet_Dictionary *ed, int index);
const char      *eet_dictionary_string_get_char(const Eet_Dictionary *ed, int index);
int              eet_dictionary_string_get_float(const Eet_Dictionary *ed, int index, float *result);
int              eet_dictionary_string_get_double(const Eet_Dictionary *ed, int index, double *result);
int              eet_dictionary_string_get_hash(const Eet_Dictionary *ed, int index);

int   _eet_hash_gen(const char *key, int hash_size);

const void* eet_identity_check(const void *data_base, unsigned int data_length,
			       const void *signature_base, unsigned int signature_length,
			       const void **raw_signature_base, unsigned int *raw_signature_length,
			       int *x509_length);
void *eet_identity_compute_sha1(const void *data_base, unsigned int data_length,
				int *sha1_length);
Eet_Error eet_cipher(const void *data, unsigned int size, const char *key, unsigned int length, void **result, unsigned int *result_length);
Eet_Error eet_decipher(const void *data, unsigned int size, const char *key, unsigned int length, void **result, unsigned int *result_length);
Eet_Error eet_identity_sign(FILE *fp, Eet_Key *key);
void eet_identity_unref(Eet_Key *key);
void eet_identity_ref(Eet_Key *key);

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef DNDEBUG
#define EET_ASSERT(Test, Do) if (Test == 0) Do;
#else
#define EET_ASSERT(Test, Do) if (Test == 0) abort();
#endif

#endif
