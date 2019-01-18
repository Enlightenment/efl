#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#include <Eina.h>
#include <Emile.h>

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
};
struct _Eet_Dictionary
{
   Eet_String    *all;
   unsigned char *all_hash;
   unsigned char *all_allocated;

   Eina_Hash     *converts;
   Eina_RWLock    rwlock;

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

typedef struct _Eet_File_Header    Eet_File_Header;
typedef struct _Eet_File_Node      Eet_File_Node;
typedef struct _Eet_File_Directory Eet_File_Directory;

struct _Eet_File
{
   const char          *path;
   Eina_File           *readfp;
   Eet_File_Header     *header;
   Eet_Dictionary      *ed;
   Eet_Key             *key;
   const unsigned char *data;
   const void          *x509_der;
   const void          *signature;
   void                *sha1;

   Eet_File_Mode        mode;

   int                  magic;
   int                  references;

   unsigned long int    data_size;
   int                  x509_length;
   unsigned int         signature_length;
   int                  sha1_length;

   Eina_Lock            file_lock;

   unsigned char        writes_pending : 1;
   unsigned char        delete_me_now : 1;
   unsigned char        readfp_owned : 1;
};

struct _Eet_File_Header
{
   int                 magic;
   Eet_File_Directory *directory;
};

struct _Eet_File_Directory
{
   int             size;
   Eet_File_Node **nodes;
   unsigned int free_count;
};

struct _Eet_File_Node
{
   char             *name;
   void             *data;
   Eet_File_Node    *next; /* FIXME: make buckets linked lists */

   unsigned int      offset;
   unsigned int      name_size;
   unsigned int      size;
   unsigned int      data_size;

   unsigned char     compression_type;

   unsigned char     free_name : 1;
   unsigned char     compression : 1;
   unsigned char     ciphered : 1;
   unsigned char     alias : 1;
};

#if 0
/* Version 2 */
/* NB: all int's are stored in network byte order on disk */
/* file format: */
int magic; /* magic number ie 0x1ee7ff00 */
int num_directory_entries; /* number of directory entries to follow */
int bytes_directory_entries; /* bytes of directory entries to follow */
struct
{
   int  offset; /* bytes offset into file for data chunk */
   int  flags; /* flags - for now 0 = uncompressed and clear, 1 = compressed and clear, 2 = uncompressed and ciphered, 3 = compressed and ciphered */
   int  size; /* size of the data chunk */
   int  data_size; /* size of the (uncompressed) data chunk */
   int  name_size; /* length in bytes of the name field */
   char name[name_size]; /* name string (variable length) and \0 terminated */
} directory[num_directory_entries];
/* and now startes the data stream... */
#endif /* if 0 */

#if 0
/* Version 3 */
/* NB: all int's are stored in network byte order on disk */
/* file format: */
int magic; /* magic number ie 0x1ee70f42 */
int num_directory_entries; /* number of directory entries to follow */
int num_dictionary_entries; /* number of dictionary entries to follow */
struct
{
   int data_offset; /* bytes offset into file for data chunk */
   int size; /* size of the data chunk */
   int data_size; /* size of the (uncompressed) data chunk */
   int name_offset; /* bytes offset into file for name string */
   int name_size; /* length in bytes of the name field */
   int flags; /* bit flags - for now:
                 bit 0 => compresion on/off
                 bit 1 => ciphered on/off
                 bit 2 => alias
               */
} directory[num_directory_entries];
struct
{
   int hash;
   int offset;
   int size;
   int prev;
   int next;
} dictionary[num_dictionary_entries];
/* now start the string stream. */
/* and right after them the data stream. */
int magic_sign; /* Optional, only if the eet file is signed. */
int signature_length; /* Signature length. */
int x509_length; /* Public certificate that signed the file. */
char signature[signature_length]; /* The signature. */
char x509[x509_length]; /* The public certificate. */
#endif /* if 0 */

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
#ifdef CRI
# undef CRI
#endif /* ifdef CRI */
#define CRI(...) EINA_LOG_DOM_CRIT(_eet_log_dom_global, __VA_ARGS__)

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

static inline Emile_Compressor_Type
eet_2_emile_compressor(int comp)
{
   switch (comp)
     {
      case EET_COMPRESSION_VERYFAST: return EMILE_LZ4HC;
      case EET_COMPRESSION_SUPERFAST: return EMILE_LZ4HC;
      default: return EMILE_ZLIB;
     }
}

#define GENERIC_ALLOC_FREE_HEADER(TYPE, Type) \
  TYPE *Type##_malloc(unsigned int);		      \
  TYPE *Type##_calloc(unsigned int);		      \
  void Type##_mp_free(TYPE *e);

GENERIC_ALLOC_FREE_HEADER(Eet_File_Directory, eet_file_directory);
GENERIC_ALLOC_FREE_HEADER(Eet_File_Node, eet_file_node);
GENERIC_ALLOC_FREE_HEADER(Eet_File_Header, eet_file_header);
GENERIC_ALLOC_FREE_HEADER(Eet_Dictionary, eet_dictionary);
GENERIC_ALLOC_FREE_HEADER(Eet_File, eet_file);

Eina_Bool eet_mempool_init(void);
void eet_mempool_shutdown(void);

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif /* ifndef PATH_MAX */

#ifdef DNDEBUG
# define EET_ASSERT(Test, Do) if (Test == 0) {Do; }
#else /* ifdef DNDEBUG */
# define EET_ASSERT(Test, Do) if (Test == 0) {abort(); }
#endif /* ifdef DNDEBUG */

#define EET_MAGIC_SIGN 0x1ee74271

#endif /* ifndef _EET_PRIVATE_H */
