#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#include <Eina.h>

typedef enum _Eet_Convert_Type Eet_Convert_Type;

enum _Eet_Convert_Type
{
   EET_D_NOTHING       = 0,
   EET_D_FLOAT         = 1 << 1,
   EET_D_DOUBLE        = 1 << 2,
   EET_D_FIXED_POINT   = 1 << 4
};

typedef struct _Eet_String  Eet_String;
typedef struct _Eet_Convert Eet_Convert;

struct _Eet_Convert
{
   float            f;
   double           d;
   Eina_F32p32      fp;

   Eet_Convert_Type type;
};

struct _Eet_String
{
   const char   *str;

   int           len;

   int           next;
   int           prev;

   unsigned char hash;
   unsigned char allocated : 1;
};
struct _Eet_Dictionary
{
   Eet_String *all;
   Eina_Hash  *converts;

   int         size;
   int         offset;

   int         hash[256];

   int         count;
   int         total;

   const char *start;
   const char *end;
};

struct _Eet_Node
{
   int           type;
   int           count;
   const char   *name;
   const char   *key;
   Eet_Node     *values;
   Eet_Node     *next;
   Eet_Node     *parent;
   Eet_Node_Data data;
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
#endif /* ifdef EET_DEFAULT_LOG_COLOR */
#define EET_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...)  EINA_LOG_DOM_ERR(_eet_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...)  EINA_LOG_DOM_DBG(_eet_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...)  EINA_LOG_DOM_INFO(_eet_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...)  EINA_LOG_DOM_WARN(_eet_log_dom_global, __VA_ARGS__)
#ifdef CRIT
# undef CRIT
#endif /* ifdef CRIT */
#define CRIT(...) EINA_LOG_DOM_CRIT(_eet_log_dom_global, __VA_ARGS__)

Eet_Dictionary *
 eet_dictionary_add(void);
void
 eet_dictionary_free(Eet_Dictionary *ed);
int
 eet_dictionary_string_add(Eet_Dictionary *ed,
                          const char *string);
int
eet_dictionary_string_get_size(const Eet_Dictionary *ed,
                               int index);
const char *
eet_dictionary_string_get_char(const Eet_Dictionary *ed,
                               int index);
Eina_Bool
eet_dictionary_string_get_float(const Eet_Dictionary *ed,
                                int index,
                                float *result);
Eina_Bool
eet_dictionary_string_get_double(const Eet_Dictionary *ed,
                                 int index,
                                 double *result);
Eina_Bool
eet_dictionary_string_get_fp(const Eet_Dictionary *ed,
                             int index,
                             Eina_F32p32 *result);
int
eet_dictionary_string_get_hash(const Eet_Dictionary *ed,
                               int index);

int _eet_hash_gen(const char *key,
                  int hash_size);

const void *
eet_identity_check(const void *data_base,
                   unsigned int data_length,
                   void **sha1,
                   int *sha1_length,
                   const void *signature_base,
                   unsigned int signature_length,
                   const void **raw_signature_base,
                   unsigned int *raw_signature_length,
                   int *x509_length);
void *
eet_identity_compute_sha1(const void *data_base,
                          unsigned int data_length,
                          int *sha1_length);
Eet_Error
eet_cipher(const void *data,
           unsigned int size,
           const char *key,
           unsigned int length,
           void **result,
           unsigned int *result_length);
Eet_Error
eet_decipher(const void *data,
             unsigned int size,
             const char *key,
             unsigned int length,
             void **result,
             unsigned int *result_length);
Eet_Error
eet_identity_sign(FILE *fp,
                  Eet_Key *key);
void
 eet_identity_unref(Eet_Key *key);
void
 eet_identity_ref(Eet_Key *key);

void
 eet_node_shutdown(void);
int
 eet_node_init(void);
Eet_Node *
 eet_node_new(void);
void
 eet_node_free(Eet_Node *node);

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif /* ifndef PATH_MAX */

#ifdef DNDEBUG
# define EET_ASSERT(Test, Do) if (Test == 0) {Do; }
#else /* ifdef DNDEBUG */
# define EET_ASSERT(Test, Do) if (Test == 0) {abort(); }
#endif /* ifdef DNDEBUG */

#endif /* ifndef _EET_PRIVATE_H */
