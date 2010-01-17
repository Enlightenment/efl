/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#include <Eina.h>

typedef enum _Eet_Convert_Type Eet_Convert_Type;

enum _Eet_Convert_Type
{
  EET_D_NOT_CONVERTED = 0,
  EET_D_FLOAT = 1 << 1,
  EET_D_DOUBLE = 1 << 2,
  EET_D_FIXED_POINT = 1 << 4
};

typedef struct _Eet_String              Eet_String;

struct _Eet_String
{
  const char            *mmap;
  char                  *str;

  int                    hash;
  int                    len;

  int                    next;
  int                    prev;

  float                f;
  double               d;
  Eina_F32p32		 fp;

  Eet_Convert_Type	 type;
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

struct _Eet_Node
{
   int         type;
   int         count;
   const char *name;
   const char *key;
   Eet_Node   *values;
   Eet_Node   *next;
   Eet_Node   *parent;
   union {
      char                c;
      short               s;
      int                 i;
      long long           l;
      float               f;
      double              d;
      unsigned char       uc;
      unsigned short      us;
      unsigned int        ui;
      unsigned long long  ul;
      const char         *str;
   } data;
};

/*
 * variable and macros used for the eina_log module
 */
extern int _eet_log_dom_global;

/*
 * Macros that are used everywhere
 *
 * the first four macros are the general macros for the lib
 */
#ifdef EET_DEFAULT_LOG_COLOR
# undef EET_DEFAULT_LOG_COLOR
#endif
#define EET_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eet_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eet_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eet_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eet_log_dom_global, __VA_ARGS__)
#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_eet_log_dom_global, __VA_ARGS__)

Eet_Dictionary  *eet_dictionary_add(void);
void             eet_dictionary_free(Eet_Dictionary *ed);
int              eet_dictionary_string_add(Eet_Dictionary *ed, const char *string);
int              eet_dictionary_string_get_size(const Eet_Dictionary *ed, int index);
const char      *eet_dictionary_string_get_char(const Eet_Dictionary *ed, int index);
Eina_Bool        eet_dictionary_string_get_float(const Eet_Dictionary *ed, int index, float *result);
Eina_Bool        eet_dictionary_string_get_double(const Eet_Dictionary *ed, int index, double *result);
Eina_Bool        eet_dictionary_string_get_fp(const Eet_Dictionary *ed, int index, Eina_F32p32 *result);
int              eet_dictionary_string_get_hash(const Eet_Dictionary *ed, int index);

int   _eet_hash_gen(const char *key, int hash_size);

const void* eet_identity_check(const void *data_base, unsigned int data_length,
			       void **sha1, int *sha1_length,
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

void eet_node_shutdown(void);
int eet_node_init(void);
Eet_Node *eet_node_new(void);
void eet_node_free(Eet_Node *node);

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#ifdef DNDEBUG
# define EET_ASSERT(Test, Do) if (Test == 0) Do;
#else
# define EET_ASSERT(Test, Do) if (Test == 0) abort();
#endif

#endif
