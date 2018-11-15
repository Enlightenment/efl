#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef _WIN32
# include <winsock2.h>
#endif /* ifdef _WIN32 */

#include <Eina.h>

#include "Eet.h"
#include "Eet_private.h"

/*
 * routines for doing data -> struct and struct -> data conversion
 *
 * types:
 *
 * basic types:
 *   a sequence of...
 *
 *   char
 *   short
 *   int
 *   long long
 *   float
 *   double
 *   unsigned char
 *   unsigned short
 *   unsigned int
 *   unsgined long long
 *   string
 *
 * groupings:
 *   multiple entries ordered as...
 *
 *   fixed size array    [ of basic types ]
 *   variable size array [ of basic types ]
 *   linked list         [ of basic types ]
 *   hash table          [ of basic types ]
 *
 * need to provide builder/accessor funcs for:
 *
 *   list_next
 *   list_append
 *
 *   hash_foreach
 *   hash_add
 *
 */

/*---*/

typedef struct _Eet_Data_Element          Eet_Data_Element;
typedef struct _Eet_Data_Basic_Type_Codec Eet_Data_Basic_Type_Codec;
typedef struct _Eet_Data_Group_Type_Codec Eet_Data_Group_Type_Codec;
typedef struct _Eet_Data_Chunk            Eet_Data_Chunk;
typedef struct _Eet_Data_Stream           Eet_Data_Stream;
typedef struct _Eet_Data_Descriptor_Hash  Eet_Data_Descriptor_Hash;
typedef struct _Eet_Data_Encode_Hash_Info Eet_Data_Encode_Hash_Info;
typedef struct _Eet_Free                  Eet_Free;
typedef struct _Eet_Free_Context          Eet_Free_Context;
typedef struct _Eet_Variant_Unknow        Eet_Variant_Unknow;

/*---*/

/* TODO:
 * Eet_Data_Basic_Type_Codec (Coder, Decoder)
 * Eet_Data_Group_Type_Codec (Coder, Decoder)
 */
struct _Eet_Data_Basic_Type_Codec
{
   int         size;
   const char *name;
   int         (*get)(const Eet_Dictionary *ed,
                      const void           *src,
                      const void           *src_end,
                      void                 *dest);
   void      * (*put)(Eet_Dictionary *ed, const void *src, int *size_ret);
};

struct _Eet_Data_Group_Type_Codec
{
   int  (*get)(Eet_Free_Context *context,
               const             Eet_Dictionary *ed,
               Eet_Data_Descriptor *edd,
               Eet_Data_Element *ede,
               Eet_Data_Chunk *echnk,
               int               type,
               int               group_type,
               void             *data_in,
               char            **p,
               int              *size);
   void (*put)(Eet_Dictionary *ed,
               Eet_Data_Descriptor *edd,
               Eet_Data_Element *ede,
               Eet_Data_Stream *ds,
               void           *data_in);
};

struct _Eet_Data_Chunk
{
   const char   *name;
   int           len;
   int           size;
   int           hash;
   void         *data;
   unsigned char type;
   unsigned char group_type;
};

struct _Eet_Data_Stream
{
   void *data;
   int   size;
   int   pos;
};

struct _Eet_Data_Descriptor_Hash
{
   Eet_Data_Element         *element;
   Eet_Data_Descriptor_Hash *next;
};

struct _Eet_Data_Descriptor
{
   const char           *name;
   const Eet_Dictionary *ed;
   int                   size;
   struct
   {
      void      * (*mem_alloc)(size_t size);
      void        (*mem_free)(void *mem);
      char      * (*str_alloc)(const char *str);
      char      * (*str_direct_alloc)(const char *str);
      void        (*str_free)(const char *str);
      void        (*str_direct_free)(const char *str);
      void      * (*list_next)(void *l);
      void      * (*list_append)(void *l, void *d);
      void      * (*list_data)(void *l);
      void      * (*list_free)(void *l);
      void        (*hash_foreach)(void *h,
                                  int (*func)(void *h,
					      const char *k,
					      void       *dt,
					      void       *fdt),
                                  void *fdt);
      void      * (*hash_add)(void *h, const char *k, void *d);
      void        (*hash_free)(void *h);
      const char *(*type_get)(const void *data, Eina_Bool *unknow);
      Eina_Bool   (*type_set)(const char *type,
                              void       *data,
                              Eina_Bool   unknow);
      void      * (*array_alloc)(size_t size);
      void        (*array_free)(void *mem);
   } func;
   struct
   {
      int               num;
      Eet_Data_Element *set;
      struct
      {
         int                       size;
         Eet_Data_Descriptor_Hash *buckets;
      } hash;
   } elements;

   Eina_Bool unified_type : 1;
//   char *strings;
//   int   strings_len;
};

struct _Eet_Data_Element
{
   const char          *name;
   const char          *counter_name;
   const char          *directory_name_ptr;
   Eet_Data_Descriptor *subtype;
   int                  offset;  /* offset in bytes from the base element */
   int                  count;  /* number of elements for a fixed array */
   int                  counter_offset;  /* for a variable array we need the offset of the count variable */
   unsigned char        type;  /* EET_T_XXX */
   unsigned char        group_type;  /* EET_G_XXX */
   Eina_Bool            subtype_free : 1;
};

struct _Eet_Data_Encode_Hash_Info
{
   Eet_Data_Stream  *ds;
   Eet_Data_Element *ede;
   Eet_Dictionary   *ed;
};

struct _Eet_Free
{
   int        ref;
   Eina_Array list;
};

struct _Eet_Free_Context
{
   Eet_Free freelist;
   Eet_Free freelist_array;
   Eet_Free freelist_list;
   Eet_Free freelist_hash;
   Eet_Free freelist_str;
   Eet_Free freelist_direct_str;
};

struct _Eet_Variant_Unknow
{
   EINA_MAGIC

   int  size;
   char data[1];
};

/*---*/

static void
 eet_free_context_init(Eet_Free_Context *context);
static void
 eet_free_context_shutdown(Eet_Free_Context *context);

static int
eet_data_get_char(const Eet_Dictionary *ed,
                  const void           *src,
                  const void           *src_end,
                  void                 *dest);
static void *
eet_data_put_char(Eet_Dictionary *ed,
                  const void     *src,
                  int            *size_ret);
static int
eet_data_get_short(const Eet_Dictionary *ed,
                   const void           *src,
                   const void           *src_end,
                   void                 *dest);
static void *
eet_data_put_short(Eet_Dictionary *ed,
                   const void     *src,
                   int            *size_ret);
static inline int
eet_data_get_int(const Eet_Dictionary *ed,
                 const void           *src,
                 const void           *src_end,
                 void                 *dest);
static void *
eet_data_put_int(Eet_Dictionary *ed,
                 const void     *src,
                 int            *size_ret);
static int
eet_data_get_long_long(const Eet_Dictionary *ed,
                       const void           *src,
                       const void           *src_end,
                       void                 *dest);
static void *
eet_data_put_long_long(Eet_Dictionary *ed,
                       const void     *src,
                       int            *size_ret);
static int
eet_data_get_float(const Eet_Dictionary *ed,
                   const void           *src,
                   const void           *src_end,
                   void                 *dest);
static void *
eet_data_put_float(Eet_Dictionary *ed,
                   const void     *src,
                   int            *size_ret);
static int
eet_data_get_double(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dest);
static void *
eet_data_put_double(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret);
static int
eet_data_get_f32p32(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dest);
static void *
eet_data_put_f32p32(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret);
static int
eet_data_get_f16p16(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dest);
static void *
eet_data_put_f16p16(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret);
static int
eet_data_get_f8p24(const Eet_Dictionary *ed,
                   const void           *src,
                   const void           *src_end,
                   void                 *dest);
static void *
eet_data_put_f8p24(Eet_Dictionary *ed,
                   const void     *src,
                   int            *size_ret);
static inline int
eet_data_get_string(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dest);
static void *
eet_data_put_string(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret);
static int
eet_data_get_istring(const Eet_Dictionary *ed,
                     const void           *src,
                     const void           *src_end,
                     void                 *dest);
static void *
eet_data_put_istring(Eet_Dictionary *ed,
                     const void     *src,
                     int            *size_ret);
static int
eet_data_get_null(const Eet_Dictionary *ed,
                  const void           *src,
                  const void           *src_end,
                  void                 *dest);
static void *
eet_data_put_null(Eet_Dictionary *ed,
                  const void     *src,
                  int            *size_ret);

static int
eet_data_get_value(const Eet_Dictionary *ed,     
		   const void           *src,    
		   const void           *src_end,
		   void                 *dst);

static void *
eet_data_put_value(Eet_Dictionary *ed,
                   const void     *src,
                   int            *size_ret);

static int
eet_data_get_type(const Eet_Dictionary *ed,
                  int                   type,
                  const void           *src,
                  const void           *src_end,
                  void                 *dest);
static void *
eet_data_put_type(Eet_Dictionary *ed,
                  int             type,
                  const void     *src,
                  int            *size_ret);

static Eet_Node *
eet_data_node_simple_type(int         type,
                          const char *name,
                          void       *dd);

static int
eet_data_get_unknown(Eet_Free_Context     *context,
                     const Eet_Dictionary *ed,
                     Eet_Data_Descriptor  *edd,
                     Eet_Data_Element     *ede,
                     Eet_Data_Chunk       *echnk,
                     int                   type,
                     int                   group_type,
                     void                 *data_in,
                     char                **p,
                     int                  *size);
static void
eet_data_put_unknown(Eet_Dictionary      *ed,
                     Eet_Data_Descriptor *edd,
                     Eet_Data_Element    *ede,
                     Eet_Data_Stream     *ds,
                     void                *data_in);
static void
eet_data_put_array(Eet_Dictionary      *ed,
                   Eet_Data_Descriptor *edd,
                   Eet_Data_Element    *ede,
                   Eet_Data_Stream     *ds,
                   void                *data_in);
static int
eet_data_get_array(Eet_Free_Context     *context,
                   const Eet_Dictionary *ed,
                   Eet_Data_Descriptor  *edd,
                   Eet_Data_Element     *ede,
                   Eet_Data_Chunk       *echnk,
                   int                   type,
                   int                   group_type,
                   void                 *data,
                   char                **p,
                   int                  *size);
static int
eet_data_get_list(Eet_Free_Context     *context,
                  const Eet_Dictionary *ed,
                  Eet_Data_Descriptor  *edd,
                  Eet_Data_Element     *ede,
                  Eet_Data_Chunk       *echnk,
                  int                   type,
                  int                   group_type,
                  void                 *data_in,
                  char                **p,
                  int                  *size);
static void
eet_data_put_list(Eet_Dictionary      *ed,
                  Eet_Data_Descriptor *edd,
                  Eet_Data_Element    *ede,
                  Eet_Data_Stream     *ds,
                  void                *data_in);
static void
eet_data_put_hash(Eet_Dictionary      *ed,
                  Eet_Data_Descriptor *edd,
                  Eet_Data_Element    *ede,
                  Eet_Data_Stream     *ds,
                  void                *data_in);
static int
eet_data_get_hash(Eet_Free_Context     *context,
                  const Eet_Dictionary *ed,
                  Eet_Data_Descriptor  *edd,
                  Eet_Data_Element     *ede,
                  Eet_Data_Chunk       *echnk,
                  int                   type,
                  int                   group_type,
                  void                 *data,
                  char                **p,
                  int                  *size);
static void
eet_data_put_union(Eet_Dictionary      *ed,
                   Eet_Data_Descriptor *edd,
                   Eet_Data_Element    *ede,
                   Eet_Data_Stream     *ds,
                   void                *data_in);
static int
eet_data_get_union(Eet_Free_Context     *context,
                   const Eet_Dictionary *ed,
                   Eet_Data_Descriptor  *edd,
                   Eet_Data_Element     *ede,
                   Eet_Data_Chunk       *echnk,
                   int                   type,
                   int                   group_type,
                   void                 *data,
                   char                **p,
                   int                  *size);
static void
eet_data_put_variant(Eet_Dictionary      *ed,
                     Eet_Data_Descriptor *edd,
                     Eet_Data_Element    *ede,
                     Eet_Data_Stream     *ds,
                     void                *data_in);
static int
eet_data_get_variant(Eet_Free_Context     *context,
                     const Eet_Dictionary *ed,
                     Eet_Data_Descriptor  *edd,
                     Eet_Data_Element     *ede,
                     Eet_Data_Chunk       *echnk,
                     int                   type,
                     int                   group_type,
                     void                 *data,
                     char                **p,
                     int                  *size);

static void
eet_data_chunk_get(const Eet_Dictionary *ed,
                   Eet_Data_Chunk       *chnk,
                   const void           *src,
                   int                   size);
static Eet_Data_Chunk *
eet_data_chunk_new(void       *data,
                   int         size,
                   const char *name,
                   int         type,
                   int         group_type);
static void
eet_data_chunk_free(Eet_Data_Chunk *chnk);

static Eet_Data_Stream *
 eet_data_stream_new(void);
static void
 eet_data_stream_write(Eet_Data_Stream *ds,
                      const void       *data,
                      int               size);
static void
eet_data_stream_free(Eet_Data_Stream *ds);

static void
eet_data_chunk_put(Eet_Dictionary  *ed,
                   Eet_Data_Chunk  *chnk,
                   Eet_Data_Stream *ds);

static int
eet_data_descriptor_encode_hash_cb(void       *hash,
                                   const char *key,
                                   void       *hdata,
                                   void       *fdata);
static void *_eet_data_descriptor_encode(Eet_Dictionary      *ed,
                                         Eet_Data_Descriptor *edd,
                                         const void          *data_in,
                                         int                 *size_ret);
static void *_eet_data_descriptor_decode(Eet_Free_Context     *context,
                                         const Eet_Dictionary *ed,
                                         Eet_Data_Descriptor  *edd,
                                         const void           *data_in,
                                         int                   size_in,
                                         void                 *data_out,
                                         int                   size_out);

/*---*/

static const Eet_Data_Basic_Type_Codec eet_basic_codec[] =
{
   {sizeof(char), "char", eet_data_get_char, eet_data_put_char     },
   {sizeof(short), "short", eet_data_get_short, eet_data_put_short    },
   {sizeof(int), "int", eet_data_get_int, eet_data_put_int      },
   {sizeof(long long), "long_long", eet_data_get_long_long, eet_data_put_long_long},
   {sizeof(float), "float", eet_data_get_float, eet_data_put_float    },
   {sizeof(double), "double", eet_data_get_double, eet_data_put_double   },
   {sizeof(char), "uchar", eet_data_get_char, eet_data_put_char     },
   {sizeof(short), "ushort", eet_data_get_short, eet_data_put_short    },
   {sizeof(int), "uint", eet_data_get_int, eet_data_put_int      },
   {sizeof(long long), "ulong_long", eet_data_get_long_long, eet_data_put_long_long},
   {sizeof(char *), "string", eet_data_get_string, eet_data_put_string   },
   {sizeof(char *), "inlined", eet_data_get_istring, eet_data_put_istring  },
   {sizeof(void *), "NULL", eet_data_get_null, eet_data_put_null     },
   {sizeof(Eina_F32p32), "f32p32", eet_data_get_f32p32, eet_data_put_f32p32   },
   {sizeof(Eina_F16p16), "f16p16", eet_data_get_f16p16, eet_data_put_f16p16   },
   {sizeof(Eina_F8p24), "f8p24", eet_data_get_f8p24, eet_data_put_f8p24    },
   {sizeof(Eina_Value*), "eina_value*", eet_data_get_value, eet_data_put_value }
};

static const Eet_Data_Group_Type_Codec eet_group_codec[] =
{
   { eet_data_get_unknown, eet_data_put_unknown },
   { eet_data_get_array, eet_data_put_array },
   { eet_data_get_array, eet_data_put_array },
   { eet_data_get_list, eet_data_put_list },
   { eet_data_get_hash, eet_data_put_hash },
   { eet_data_get_union, eet_data_put_union },
   { eet_data_get_variant, eet_data_put_variant },
   { eet_data_get_unknown, eet_data_put_unknown }
};

static int _eet_data_words_bigendian = -1;

/*---*/

#define SWAP64(x) (x) =                                        \
  ((((unsigned long long)(x) & 0x00000000000000ffULL) << 56) | \
   (((unsigned long long)(x) & 0x000000000000ff00ULL) << 40) | \
   (((unsigned long long)(x) & 0x0000000000ff0000ULL) << 24) | \
   (((unsigned long long)(x) & 0x00000000ff000000ULL) << 8) |  \
   (((unsigned long long)(x) & 0x000000ff00000000ULL) >> 8) |  \
   (((unsigned long long)(x) & 0x0000ff0000000000ULL) >> 24) | \
   (((unsigned long long)(x) & 0x00ff000000000000ULL) >> 40) | \
   (((unsigned long long)(x) & 0xff00000000000000ULL) >> 56))
#define SWAP32(x) (x) =              \
  ((((int)(x) & 0x000000ff) << 24) | \
   (((int)(x) & 0x0000ff00) << 8) |  \
   (((int)(x) & 0x00ff0000) >> 8) |  \
   (((int)(x) & 0xff000000) >> 24))
#define SWAP16(x) (x) =           \
  ((((short)(x) & 0x00ff) << 8) | \
   (((short)(x) & 0xff00) >> 8))

#ifdef CONV8
# undef CONV8
#endif /* ifdef CONV8 */
#ifdef CONV16
# undef CONV16
#endif /* ifdef CONV16 */
#ifdef CONV32
# undef CONV32
#endif /* ifdef CONV32 */
#ifdef CONV64
# undef CONV64
#endif /* ifdef CONV64 */

#define CONV8(x)
#define CONV16(x)             {if (_eet_data_words_bigendian) {SWAP16(x); }}
#define CONV32(x)             {if (_eet_data_words_bigendian) {SWAP32(x); }}
#define CONV64(x)             {if (_eet_data_words_bigendian) {SWAP64(x); }}

#define IS_SIMPLE_TYPE(Type)  (Type > EET_T_UNKNOW && Type < EET_T_LAST)
#define IS_POINTER_TYPE(Type) ((Type >= EET_T_STRING && Type <= EET_T_NULL) || Type == EET_T_VALUE)

#define POINTER_TYPE_DECODE(Context,                    \
                            Ed,                         \
                            Edd,                        \
                            Ede,                        \
                            Echnk,                      \
                            Type,                       \
                            Data,                       \
                            P,                          \
                            Size,                       \
                            Label)                      \
  do {                                                  \
       int ___r;                                        \
       ___r = eet_data_get_unknown(Context,             \
                                   Ed,                  \
                                   Edd, Ede,            \
                                   Echnk,               \
                                   Type, EET_G_UNKNOWN, \
                                   Data, P, Size);      \
       if (!___r) { goto Label; }                       \
    } while (0)

#define STRUCT_TYPE_DECODE(Data_Ret, Context, Ed, Ede, Data, Size, SubSize, Label) \
  do {                                                                             \
       Data_Ret = _eet_data_descriptor_decode(Context,                             \
                                              Ed,                                  \
                                              Ede,                                 \
                                              Data,                                \
                                              Size,                                \
                                              SubSize > 0 ? Data_Ret : NULL,       \
                                              SubSize);                            \
       EINA_SAFETY_ON_NULL_GOTO(Data_Ret, Label);                                 \
    } while (0)

#define EET_I_STRING         1 << 4
#define EET_I_INLINED_STRING 2 << 4
#define EET_I_NULL           3 << 4
#define EET_I_VALUE          4 << 4

#define EET_MAGIC_VARIANT    0xF1234BC
/*---*/

/* CHAR TYPE */
static int
eet_data_get_char(const Eet_Dictionary *ed EINA_UNUSED,
                  const void           *src,
                  const void           *src_end,
                  void                 *dst)
{
   char *s, *d;

   if (((char *)src + sizeof(char)) > (char *)src_end)
     return -1;

   s = (char *)src;
   d = (char *)dst;
   *d = *s;
   CONV8(*d);
   return sizeof(char);
}

static void *
eet_data_put_char(Eet_Dictionary *ed EINA_UNUSED,
                  const void     *src,
                  int            *size_ret)
{
   char *s, *d;

   d = (char *)malloc(sizeof(char));
   if (!d)
     return NULL;

   s = (char *)src;
   *d = *s;
   CONV8(*d);
   *size_ret = sizeof(char);
   return d;
}

/* SHORT TYPE */
static int
eet_data_get_short(const Eet_Dictionary *ed EINA_UNUSED,
                   const void           *src,
                   const void           *src_end,
                   void                 *dst)
{
   short *d;

   if (((char *)src + sizeof(short)) > (char *)src_end)
     return -1;

   memcpy(dst, src, sizeof(short));
   d = (short *)dst;
   CONV16(*d);
   return sizeof(short);
}

static void *
eet_data_put_short(Eet_Dictionary *ed EINA_UNUSED,
                   const void     *src,
                   int            *size_ret)
{
   short *s, *d;

   d = (short *)malloc(sizeof(short));
   if (!d)
     return NULL;

   s = (short *)src;
   *d = *s;
   CONV16(*d);
   *size_ret = sizeof(short);
   return d;
}

/* INT TYPE */
static inline int
eet_data_get_int(const Eet_Dictionary *ed EINA_UNUSED,
                 const void           *src,
                 const void           *src_end,
                 void                 *dst)
{
   int *d;

   if (((char *)src + sizeof(int)) > (char *)src_end)
     return -1;

   memcpy(dst, src, sizeof(int));
   d = (int *)dst;
   CONV32(*d);
   return sizeof(int);
}

static void *
eet_data_put_int(Eet_Dictionary *ed EINA_UNUSED,
                 const void     *src,
                 int            *size_ret)
{
   int *s, *d;

   d = (int *)malloc(sizeof(int));
   if (!d)
     return NULL;

   s = (int *)src;
   *d = *s;
   CONV32(*d);
   *size_ret = sizeof(int);
   return d;
}

/* LONG LONG TYPE */
static int
eet_data_get_long_long(const Eet_Dictionary *ed EINA_UNUSED,
                       const void           *src,
                       const void           *src_end,
                       void                 *dst)
{
   unsigned long long *d;

   if (((char *)src + sizeof(unsigned long long)) > (char *)src_end)
     return -1;

   memcpy(dst, src, sizeof(unsigned long long));
   d = (unsigned long long *)dst;
   CONV64(*d);
   return sizeof(unsigned long long);
}

static void *
eet_data_put_long_long(Eet_Dictionary *ed EINA_UNUSED,
                       const void     *src,
                       int            *size_ret)
{
   unsigned long long *s, *d;

   d = (unsigned long long *)malloc(sizeof(unsigned long long));
   if (!d)
     return NULL;

   s = (unsigned long long *)src;
   *d = *s;
   CONV64(*d);
   *size_ret = sizeof(unsigned long long);
   return d;
}

/* STRING TYPE */
static inline int
eet_data_get_string_hash(const Eet_Dictionary *ed,
                         const void           *src,
                         const void           *src_end)
{
   if (ed)
     {
        int idx;

        if (eet_data_get_int(ed, src, src_end, &idx) < 0)
          return -1;

        return eet_dictionary_string_get_hash(ed, idx);
     }

   return -1;
}

static inline int
eet_data_get_string(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dst)
{
   char *s, **d;

   d = (char **)dst;

   if (ed)
     {
        const char *str;
        int idx;

        if (eet_data_get_int(ed, src, src_end, &idx) < 0)
          return -1;

        str = eet_dictionary_string_get_char(ed, idx);
        if (!str)
          return -1;

        *d = (char *)str;
        return eet_dictionary_string_get_size(ed, idx);
     }

   s = (char *)src;
   if (!s)
     {
        *d = NULL;
        return 0;
     }

   *d = s;
   return strlen(s) + 1;
}

static void *
eet_data_put_string(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret)
{
   char *s, *d;
   int len;

   if (ed)
     {
        const char *str;
        int idx;

        str = *((const char **)src);
        if (!str)
          return NULL;

        idx = eet_dictionary_string_add(ed, str);
        if (idx == -1)
          return NULL;

        return eet_data_put_int(ed, &idx, size_ret);
     }

   s = (char *)(*((char **)src));
   if (!s)
     return NULL;

   len = strlen(s);
   d = malloc(len + 1);
   if (!d)
     return NULL;

   memcpy(d, s, len + 1);
   *size_ret = len + 1;
   return d;
}

/* ALWAYS INLINED STRING TYPE */
static int
eet_data_get_istring(const Eet_Dictionary *ed EINA_UNUSED,
                     const void           *src,
                     const void           *src_end,
                     void                 *dst)
{
   return eet_data_get_string(NULL, src, src_end, dst);
}

static void *
eet_data_put_istring(Eet_Dictionary *ed EINA_UNUSED,
                     const void     *src,
                     int            *size_ret)
{
   return eet_data_put_string(NULL, src, size_ret);
}

/* ALWAYS NULL TYPE */
static int
eet_data_get_null(const Eet_Dictionary *ed EINA_UNUSED,
                  const void           *src EINA_UNUSED,
                  const void           *src_end EINA_UNUSED,
                  void                 *dst)
{
   char **d;

   d = (char **)dst;

   *d = NULL;
   return 1;
}

static void *
eet_data_put_null(Eet_Dictionary *ed EINA_UNUSED,
                  const void     *src EINA_UNUSED,
                  int            *size_ret)
{
   *size_ret = 0;
   return NULL;
}

/**
 * Fast lookups of simple doubles/floats.
 *
 * These aren't properly a cache because they don't store pre-calculated
 * values, but have a so simple math that is almost as fast.
 */
static inline int
_eet_data_float_cache_get(const char *s,
                          int         len,
                          float      *d)
{
   /* fast handle of simple case 0xMp+E*/
    if ((len == 6) && (s[0] == '0') && (s[1] == 'x') && (s[3] == 'p'))
      {
         int mantisse = (s[2] >= 'a') ? (s[2] - 'a' + 10) : (s[2] - '0');
         int exponent = (s[5] - '0');

         if (s[4] == '+')
           *d = (float)(mantisse << exponent);
         else
           *d = (float)mantisse / (float)(1 << exponent);

         return 1;
      }

    return 0;
}

static inline int
_eet_data_double_cache_get(const char *s,
                           int         len,
                           double     *d)
{
   /* fast handle of simple case 0xMp+E*/
    if ((len == 6) && (s[0] == '0') && (s[1] == 'x') && (s[3] == 'p'))
      {
         int mantisse = (s[2] >= 'a') ? (s[2] - 'a' + 10) : (s[2] - '0');
         int exponent = (s[5] - '0');

         if (s[4] == '+')
           *d = (double)(mantisse << exponent);
         else
           *d = (double)mantisse / (double)(1 << exponent);

         return 1;
      }

    return 0;
}

/* FLOAT TYPE */
static int
eet_data_get_float(const Eet_Dictionary *ed,
                   const void           *src,
                   const void           *src_end,
                   void                 *dst)
{
   float *d;
   int idx;

   d = (float *)dst;
   if (!ed)
     {
        const char *s, *p;
        long long mantisse;
        long exponent;
        int len;

        s = (const char *)src;
        p = s;
        len = 0;
        while ((p < (const char *)src_end) && (*p != 0)) {len++; p++; }

        if (_eet_data_float_cache_get(s, len, d) != 0)
          return len + 1;

        if (eina_convert_atod(s, len, &mantisse, &exponent) == EINA_FALSE)
          return -1;

        *d = (float)ldexp((double)mantisse, exponent);

        return len + 1;
     }

   if (eet_data_get_int(ed, src, src_end, &idx) < 0)
     return -1;

   if (!eet_dictionary_string_get_float(ed, idx, d))
     return -1;

   return 1;
}

static void *
eet_data_put_float(Eet_Dictionary *ed,
                   const void     *src,
                   int            *size_ret)
{
   char buf[128];
   int idx;

   if (!eina_convert_dtoa((double)(*(float *)src), buf))
     return NULL;

   if (!ed)
     {
        char *d;
        int len;

        len = strlen(buf);
        d = malloc(len + 1);
        if (!d)
          return NULL;

        memcpy(d, buf, len + 1);
        *size_ret = len + 1;
        return d;
     }

   idx = eet_dictionary_string_add(ed, buf);
   if (idx == -1)
     return NULL;

   return eet_data_put_int(ed, &idx, size_ret);
}

/* DOUBLE TYPE */
static int
eet_data_get_double(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dst)
{
   double *d;
   int idx;

   d = (double *)dst;

   if (!ed)
     {
        const char *s, *p;
        long long mantisse = 0;
        long exponent = 0;
        int len;

        s = (const char *)src;
        p = s;
        len = 0;
        while ((p < (const char *)src_end) && (*p != 0)) {len++; p++; }

        if (_eet_data_double_cache_get(s, len, d) != 0)
          return len + 1;

        if (eina_convert_atod(s, len, &mantisse, &exponent) == EINA_FALSE)
          return -1;

        *d = ldexp((double)mantisse, exponent);

        return len + 1;
     }

   if (eet_data_get_int(ed, src, src_end, &idx) < 0)
     return -1;

   if (!eet_dictionary_string_get_double(ed, idx, d))
     return -1;

   return 1;
}

static void *
eet_data_put_double(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret)
{
   char buf[128];
   int idx;

   if (!eina_convert_dtoa((double)(*(double *)src), buf))
     return NULL;

   if (!ed)
     {
        char *d;
        int len;

        len = strlen(buf);
        d = malloc(len + 1);
        if (!d)
          return NULL;

        memcpy(d, buf, len + 1);
        *size_ret = len + 1;

        return d;
     }

   idx = eet_dictionary_string_add(ed, buf);
   if (idx == -1)
     return NULL;

   return eet_data_put_int(ed, &idx, size_ret);
}

static int
eet_data_get_f32p32(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dst)
{
   Eina_F32p32 *fp;
   int idx;

   fp = (Eina_F32p32 *)dst;

   if (!ed)
     {
        const char *s, *p;
        int len;

        s = (const char *)src;
        p = s;
        len = 0;
        while ((p < (const char *)src_end) && (*p != 0)) { len++; p++; }

        if (!(eina_convert_atofp(s, len, fp)))
          return -1;

        return 1;
     }

   if (eet_data_get_int(ed, src, src_end, &idx) < 0)
     return -1;

   if (!eet_dictionary_string_get_fp(ed, idx, fp))
     return -1;

   return 1;
}

static void *
eet_data_put_f32p32(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret)
{
   char buf[128];
   int idx;

   eina_convert_fptoa((Eina_F32p32)(*(Eina_F32p32 *)src), buf);

   if (!ed)
     {
        char *d;
        int len;

        len = strlen(buf);
        d = malloc(len + 1);
        if (!d)
          return NULL;

        memcpy(d, buf, len + 1);
        *size_ret = len + 1;

        return d;
     }

   idx = eet_dictionary_string_add(ed, buf);
   if (idx == -1)
     return NULL;

   return eet_data_put_int(ed, &idx, size_ret);
}

static int
eet_data_get_f16p16(const Eet_Dictionary *ed,
                    const void           *src,
                    const void           *src_end,
                    void                 *dst)
{
   Eina_F32p32 tmp;
   Eina_F16p16 *fp;

   fp = (Eina_F16p16 *)dst;

   if (eet_data_get_f32p32(ed, src, src_end, &tmp) < 0)
     return -1;

   *fp = eina_f32p32_to_f16p16(tmp);
   return 1;
}

static void *
eet_data_put_f16p16(Eet_Dictionary *ed,
                    const void     *src,
                    int            *size_ret)
{
   Eina_F32p32 tmp;

   tmp = eina_f16p16_to_f32p32((Eina_F16p16)(*(Eina_F16p16 *)src));
   return eet_data_put_f32p32(ed, &tmp, size_ret);
}

static int
eet_data_get_f8p24(const Eet_Dictionary *ed,
                   const void           *src,
                   const void           *src_end,
                   void                 *dst)
{
   Eina_F32p32 tmp;
   Eina_F8p24 *fp;

   fp = (Eina_F8p24 *)dst;

   if (eet_data_get_f32p32(ed, src, src_end, &tmp) < 0)
     return -1;

   *fp = eina_f32p32_to_f8p24(tmp);
   return 1;
}

static void *
eet_data_put_f8p24(Eet_Dictionary *ed,
                   const void     *src,
                   int            *size_ret)
{
   Eina_F32p32 tmp;

   tmp = eina_f8p24_to_f32p32((Eina_F8p24)(*(Eina_F8p24 *)src));
   return eet_data_put_f32p32(ed, &tmp, size_ret);
}

static const Eina_Value_Type *
_eet_type_to_eina_value_get(int eet_type)
{
   switch (eet_type)
     {
      case EET_T_UCHAR: return EINA_VALUE_TYPE_UCHAR;
      case EET_T_USHORT: return EINA_VALUE_TYPE_USHORT;
      case EET_T_UINT: return EINA_VALUE_TYPE_UINT;
#if SIZEOF_LONG == SIZEOF_INT
      /* case EET_T_UINT: return EINA_VALUE_TYPE_ULONG; */
      /* case EET_T_UINT: return EINA_VALUE_TYPE_TIMESTAMP; */
#else
      /* case EET_T_ULONG_LONG: return EINA_VALUE_TYPE_ULONG; */
      /* case EET_T_ULONG_LONG: return EINA_VALUE_TYPE_TIMESTAMP; */
#endif
      case EET_T_ULONG_LONG: return EINA_VALUE_TYPE_UINT64;
      case EET_T_CHAR: return EINA_VALUE_TYPE_CHAR;
      case EET_T_SHORT: return EINA_VALUE_TYPE_SHORT;
      case EET_T_INT: return EINA_VALUE_TYPE_INT;
#if SIZEOF_LONG == SIZEOF_INT
      /* case EET_T_INT: return EINA_VALUE_TYPE_LONG; */
#else
      /* case EET_T_LONG_LONG: return EINA_VALUE_TYPE_LONG; */
#endif
      case EET_T_LONG_LONG: return EINA_VALUE_TYPE_INT64;
      case EET_T_FLOAT: return EINA_VALUE_TYPE_FLOAT;
      case EET_T_DOUBLE: return EINA_VALUE_TYPE_DOUBLE;
      case EET_T_STRING: return EINA_VALUE_TYPE_STRING;
      /* case EET_T_STRING: return EINA_VALUE_TYPE_STRINGSHARE; */
     }

   return NULL;
}

static int
_eina_value_to_eet_type_get(const Eina_Value_Type *eina_type)
{
   if (eina_type == EINA_VALUE_TYPE_UCHAR) return EET_T_UCHAR;
   else if (eina_type == EINA_VALUE_TYPE_USHORT) return EET_T_USHORT;
   else if (eina_type == EINA_VALUE_TYPE_UINT) return EET_T_UINT;
#if SIZEOF_LONG == SIZEOF_INT
   else if (eina_type == EINA_VALUE_TYPE_ULONG) return EET_T_UINT;
   else if (eina_type == EINA_VALUE_TYPE_TIMESTAMP) return EET_T_UINT;
#else
   else if (eina_type == EINA_VALUE_TYPE_ULONG) return EET_T_ULONG_LONG;
   else if (eina_type == EINA_VALUE_TYPE_TIMESTAMP) return EET_T_ULONG_LONG;
#endif
   else if (eina_type == EINA_VALUE_TYPE_UINT64) return EET_T_ULONG_LONG;
   else if (eina_type == EINA_VALUE_TYPE_CHAR) return EET_T_CHAR;
   else if (eina_type == EINA_VALUE_TYPE_SHORT) return EET_T_SHORT;
   else if (eina_type == EINA_VALUE_TYPE_INT) return EET_T_INT;
#if SIZEOF_LONG == SIZEOF_INT
   else if (eina_type == EINA_VALUE_TYPE_LONG) return EET_T_INT;
#else
   else if (eina_type == EINA_VALUE_TYPE_LONG) return EET_T_LONG_LONG;
#endif
   else if (eina_type == EINA_VALUE_TYPE_INT64) return EET_T_LONG_LONG;
   else if (eina_type == EINA_VALUE_TYPE_FLOAT) return EET_T_FLOAT;
   else if (eina_type == EINA_VALUE_TYPE_DOUBLE) return EET_T_DOUBLE;
   else if (eina_type == EINA_VALUE_TYPE_STRING) return EET_T_STRING;
   else if (eina_type == EINA_VALUE_TYPE_STRINGSHARE) return EET_T_STRING;
   // always fallback to try a conversion to string if possible
   return EET_T_STRING;
}

static int
eet_data_get_value(const Eet_Dictionary *ed,     
		   const void           *src,    
		   const void           *src_end,
		   void                 *dst)    
{
   const Eina_Value_Type *eina_type;
   void *tmp;
   int eet_type;
   int eet_size, type_size;

   eet_size = eet_data_get_int(ed, src, src_end, &eet_type);
   if (eet_size < 0 ||
       eet_type <= EET_T_UNKNOW ||
       eet_type >= EET_T_VALUE)
     return -1;

   tmp = alloca(eet_basic_codec[eet_type - 1].size);
   type_size = eet_basic_codec[eet_type - 1].get(ed, (char*) src + eet_size, src_end, tmp);

   if (eet_type == EET_T_NULL)
     {
        Eina_Value **value = dst;

        *value = NULL;

        return eet_size + type_size;
     }

   eina_type = _eet_type_to_eina_value_get(eet_type);
   if (eina_type)
     {
        Eina_Value **value = dst;

        *value = eina_value_new(eina_type);
        if (!eina_value_pset(*value, tmp)) return -1;

        return eet_size + type_size;
     }

   return -1;
}

static void *
eet_data_put_value(Eet_Dictionary *ed,       
		   const void     *src,      
		   int            *size_ret) 
{
   const Eina_Value *value = *(Eina_Value **)src;
   const Eina_Value_Type *value_type;
   void *int_data;
   void *type_data;
   int int_size, type_size;
   int eet_type;
   void *tmp;
   Eina_Bool v2s = EINA_FALSE;

   // map empty Eina_Value to EET_T_NULL;
   if (!value)
     {
        eet_type = EET_T_NULL;
        goto lookup_done;
     }

   value_type = eina_value_type_get(value);
   eet_type = _eina_value_to_eet_type_get(value_type);

 lookup_done:
   tmp = alloca(eet_basic_codec[eet_type - 1].size);
   if (value) eina_value_get(value, tmp);
   else *(void**) tmp = NULL;

   // handle non simple case by forcing them to convert to string
   if ((eet_type == EET_T_STRING) &&
       (*(char**)tmp == NULL))
     {
        *(char**)tmp = eina_value_to_string(value);
        v2s = EINA_TRUE;
     }

   int_data = eet_data_put_int(ed, &eet_type, &int_size);
   type_data = eet_basic_codec[eet_type - 1].put(ed, tmp, &type_size);

   // free temporary string as it is not needed anymore
   if (v2s) free(*(char**)tmp);

   // pack data with type first, then the data
   *size_ret = int_size + type_size;
   tmp = malloc(*size_ret);
   memcpy(tmp, int_data, int_size);
   memcpy(((char*)tmp) + int_size, type_data, type_size);

   free(int_data);
   free(type_data);

   return tmp;
}

static inline int
eet_data_get_type(const Eet_Dictionary *ed,
                  int                   type,
                  const void           *src,
                  const void           *src_end,
                  void                 *dest)
{
   int ret;

   ret = eet_basic_codec[type - 1].get(ed, src, src_end, dest);
   return ret;
}

static inline void *
eet_data_put_type(Eet_Dictionary *ed,
                  int             type,
                  const void     *src,
                  int            *size_ret)
{
   void *ret;

   ret = eet_basic_codec[type - 1].put(ed, src, size_ret);
   return ret;
}

static inline Eina_Bool
eet_data_type_match(int type1,
                    int type2)
{
   if (type1 == type2)
     return EINA_TRUE;

   /* Note: All floating point type are equivalent and could be read
      without problem by any other floating point getter. */
   switch (type1)
     {
      case EET_T_FLOAT:
      case EET_T_DOUBLE:
      case EET_T_F32P32:
      case EET_T_F16P16:
      case EET_T_F8P24:
        switch (type2)
          {
           case EET_T_FLOAT:
           case EET_T_DOUBLE:
           case EET_T_F32P32:
           case EET_T_F16P16:
           case EET_T_F8P24:
             return EINA_TRUE;

           default:
             break;
          }  /* switch */
        break;

      default:
        break;
     }

   return EINA_FALSE;
}

/* chunk format...
 *
 * char[4] = "CHnK"; // untyped data ... or
 * char[4] = "CHKx"; // typed data - x == type
 *
 * int     = chunk size (including magic string);
 * char[]  = chunk magic/name string (0 byte terminated);
 * ... sub-chunks (a chunk can contain chuncks recusrively) ...
 * or
 * ... payload data ...
 *
 */

static inline void
eet_data_chunk_get(const Eet_Dictionary *ed,
                   Eet_Data_Chunk       *chnk,
                   const void           *src,
                   int                   size)
{
   const char *s;
   int ret1, ret2;

   if (!src)
     return;

   if (size <= 8)
     return;

   if (!chnk)
     return;

   s = src;
   if (s[2] == 'K')
     {
        if ((s[0] != 'C') || (s[1] != 'H') || (s[2] != 'K'))
          return;

        chnk->type = (unsigned char)(s[3]);
        if (chnk->type >= EET_I_LIMIT)
          {
             chnk->group_type =
               ((chnk->type - EET_I_LIMIT) & 0xF) + EET_G_UNKNOWN;
             switch ((chnk->type - EET_I_LIMIT) & 0xF0)
               {
#define EET_UNMATCH_TYPE(Type) \
case EET_I_ ## Type: chnk->type = EET_T_ ## Type; break;

                  EET_UNMATCH_TYPE(STRING);
                  EET_UNMATCH_TYPE(INLINED_STRING);
                  EET_UNMATCH_TYPE(VALUE);
                  EET_UNMATCH_TYPE(NULL);

                default:
                  return;
               }
          }
        else if (chnk->type > EET_T_LAST)
          {
             chnk->group_type = chnk->type;
             chnk->type = EET_T_UNKNOW;
          }
        else
          chnk->group_type = EET_G_UNKNOWN;
        if ((chnk->type >= EET_T_LAST) ||
            (chnk->group_type >=
             EET_G_LAST))
          {
             chnk->type = 0;
             chnk->group_type = 0;
          }
     }
   else if ((s[0] != 'C') || (s[1] != 'H') || (s[2] != 'n') || (s[3] != 'K'))
     return;

   ret1 = eet_data_get_type(ed, EET_T_INT, (s + 4), (s + size), &(chnk->size));

   if (ret1 <= 0)
     return;

   if ((chnk->size < 0) || ((chnk->size + 8) > size))
     return;

   ret2 = eet_data_get_type(ed, EET_T_STRING, (s + 8), (s + size), &(chnk->name));

   if (ret2 <= 0)
     return;

   chnk->len = ret2;

   /* Precalc hash */
   chnk->hash = eet_data_get_string_hash(ed, (s + 8), (s + size));

   if (ed)
     {
        chnk->data = (char *)src + 4 + ret1 + sizeof(int);
        chnk->size -= sizeof(int);
     }
   else
     {
        chnk->data = (char *)src + 4 + ret1 + chnk->len;
        chnk->size -= chnk->len;
     }

   return;
}

static inline Eet_Data_Chunk *
eet_data_chunk_new(void       *data,
                   int         size,
                   const char *name,
                   int         type,
                   int         group_type)
{
   Eet_Data_Chunk *chnk;

   if (!name)
     return NULL;

   chnk = calloc(1, sizeof(Eet_Data_Chunk));
   if (!chnk)
     return NULL;

   /* Note: Another security, so older eet library could read file
      saved with fixed point value. */
   if (type == EET_T_F32P32
       || type == EET_T_F16P16
       || type == EET_T_F8P24)
     type = EET_T_DOUBLE;

   chnk->name = name;
   chnk->len = strlen(name) + 1;
   chnk->size = size;
   chnk->data = data;
   chnk->type = type;
   chnk->group_type = group_type;
   return chnk;
}

static inline void
eet_data_chunk_free(Eet_Data_Chunk *chnk)
{
   free(chnk);
}

static inline Eet_Data_Stream *
eet_data_stream_new(void)
{
   Eet_Data_Stream *ds;

   ds = calloc(1, sizeof(Eet_Data_Stream));
   if (!ds)
     return NULL;

   return ds;
}

static inline void
eet_data_stream_free(Eet_Data_Stream *ds)
{
   if (ds->data)
     free(ds->data);

   free(ds);
}

static inline void
eet_data_stream_flush(Eet_Data_Stream *ds)
{
   free(ds);
}

static inline void
eet_data_stream_write(Eet_Data_Stream *ds,
                      const void      *data,
                      int              size)
{
   char *p;

   if ((ds->pos + size) > ds->size)
     {
        ds->data = realloc(ds->data, ds->size + size + 512);
        if (!ds->data)
          {
             ds->pos = 0;
             ds->size = 0;
             return;
          }

        ds->size = ds->size + size + 512;
     }
   p = ds->data;
   if (!p) return;
   memcpy(p + ds->pos, data, size);
   ds->pos += size;
}

static void
eet_data_chunk_put(Eet_Dictionary  *ed,
                   Eet_Data_Chunk  *chnk,
                   Eet_Data_Stream *ds)
{
   int *size;
   void *string;
   int s;
   int size_ret = 0;
   int string_ret = 0;
   unsigned char buf[4] = "CHK";

   /* disable this check - it will allow empty chunks to be written. this is
    * right for corner-cases when y have a struct with empty fields (empty
    * strings or empty list ptrs etc.) */
   /* if (!chnk->data && chnk->type != EET_T_NULL) return; */
   /* chunk head */

/*   eet_data_stream_write(ds, "CHnK", 4);*/
   if (chnk->type != EET_T_UNKNOW)
     {
        if (chnk->group_type != EET_G_UNKNOWN)
          {
             int type = EET_I_LIMIT + chnk->group_type - EET_G_UNKNOWN;

             switch (chnk->type)
               {
     /* Only make sense with pointer type. */
#define EET_MATCH_TYPE(Type) \
case EET_T_ ## Type: type += EET_I_ ## Type; break;

                   EET_MATCH_TYPE(STRING);
                   EET_MATCH_TYPE(INLINED_STRING);
                   EET_MATCH_TYPE(VALUE);
                   EET_MATCH_TYPE(NULL);

                 default:
                   return;
               }

             buf[3] = type;
          }
        else
          buf[3] = chnk->type;
     }
   else
     buf[3] = chnk->group_type;

   string = eet_data_put_string(ed, &chnk->name, &string_ret);
   if (!string)
     return;

   /* size of chunk payload data + name */
   s = chnk->size + string_ret;
   size = eet_data_put_int(ed, &s, &size_ret);

   /* FIXME: If something goes wrong the resulting file will be corrupted. */
   EINA_SAFETY_ON_TRUE_GOTO(!size, on_error);

   eet_data_stream_write(ds, buf, 4);

   /* write chunk length */
   eet_data_stream_write(ds, size, size_ret);

   /* write chunk name */
   eet_data_stream_write(ds, string, string_ret);

   /* write payload */
   if (chnk->data)
     eet_data_stream_write(ds, chnk->data, chnk->size);

   free(size);
on_error:
   free(string);
}

/*---*/

static void
_eet_descriptor_hash_new(Eet_Data_Descriptor *edd)
{
   int i;

   edd->elements.hash.size = 1 << 6;
   edd->elements.hash.buckets = calloc(
       1,
       sizeof(Eet_Data_Descriptor_Hash) *
       edd->elements.hash.size);
   for (i = 0; i < edd->elements.num; i++)
     {
        Eet_Data_Element *ede;
        int hash;

        ede = &(edd->elements.set[i]);
        hash = _eet_hash_gen((char *)ede->name, 6);
        if (!edd->elements.hash.buckets[hash].element)
          edd->elements.hash.buckets[hash].element = ede;
        else
          {
             Eet_Data_Descriptor_Hash *bucket;

             bucket = calloc(1, sizeof(Eet_Data_Descriptor_Hash));
             bucket->element = ede;
             bucket->next = edd->elements.hash.buckets[hash].next;
             edd->elements.hash.buckets[hash].next = bucket;
          }
     }
}

static void
_eet_descriptor_hash_free(Eet_Data_Descriptor *edd)
{
   int i;

   for (i = 0; i < edd->elements.hash.size; i++)
     {
        Eet_Data_Descriptor_Hash *bucket, *pbucket;

        bucket = edd->elements.hash.buckets[i].next;
        while (bucket)
          {
             pbucket = bucket;
             bucket = bucket->next;
             free(pbucket);
          }
     }
   if (edd->elements.hash.buckets)
     free(edd->elements.hash.buckets);
}

static Eet_Data_Element *
_eet_descriptor_hash_find(Eet_Data_Descriptor *edd,
                          const char          *name,
                          int                  hash)
{
   Eet_Data_Descriptor_Hash *bucket;

   if (hash < 0)
     hash = _eet_hash_gen(name, 6);
   else
     hash &= 0x3f;

   if (!edd->elements.hash.buckets[hash].element)
     return NULL;  /*
                      When we use the dictionary as a source for chunk name, we will always
                      have the same pointer in name. It's a good idea to just compare pointer
                      instead of running strcmp on both string.
                    */

   if (edd->elements.hash.buckets[hash].element->directory_name_ptr == name)
     return edd->elements.hash.buckets[hash].element;

   if (!strcmp(edd->elements.hash.buckets[hash].element->name, name))
     {
        edd->elements.hash.buckets[hash].element->directory_name_ptr = name;
        return edd->elements.hash.buckets[hash].element;
     }

   bucket = edd->elements.hash.buckets[hash].next;
   while (bucket)
     {
        if (bucket->element->directory_name_ptr == name)
          return bucket->element;

        if (!strcmp(bucket->element->name, name))
          {
             bucket->element->directory_name_ptr = name;
             return bucket->element;
          }

        bucket = bucket->next;
     }
   return NULL;
}

static void *
_eet_mem_alloc(size_t size)
{
   return calloc(1, size);
}

static void
_eet_mem_free(void *mem)
{
   free(mem);
}

static char *
_eet_str_alloc(const char *str)
{
   return strdup(str);
}

static void
_eet_str_free(const char *str)
{
   free((char *)str);
}

static Eina_Hash *
_eet_eina_hash_add_alloc(Eina_Hash  *hash,
                         const char *key,
                         void       *data)
{
   if (!hash)
     hash = eina_hash_string_small_new(NULL);

   if (!hash)
     return NULL;

   eina_hash_add(hash, key, data);
   return hash;
}

static Eina_Hash *
_eet_eina_hash_direct_add_alloc(Eina_Hash  *hash,
                                const char *key,
                                void       *data)
{
   if (!hash)
     hash = eina_hash_string_small_new(NULL);

   if (!hash)
     return NULL;

   eina_hash_direct_add(hash, key, data);
   return hash;
}

static char *
_eet_str_direct_alloc(const char *str)
{
   return (char *)str;
}

static void
_eet_str_direct_free(const char *str EINA_UNUSED)
{
}

static void
_eet_eina_hash_foreach(void             *hash,
                       Eina_Hash_Foreach cb,
                       void             *fdata)
{
   if (hash)
     eina_hash_foreach(hash, cb, fdata);
}

static void
_eet_eina_hash_free(void *hash)
{
   if (hash)
     eina_hash_free(hash);
}

/*---*/
EAPI Eina_Bool
eet_eina_stream_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
     /* When we change the structure content in the future, we need to handle old structure type too */
                                          unsigned int               eddc_size,
                                          const char                *name,
                                          int                        size)
{
   if (!eddc || !name || eddc_size != sizeof (Eet_Data_Descriptor_Class))
     return EINA_FALSE;

   eddc->name = name;
   eddc->size = size;
   eddc->version = EET_DATA_DESCRIPTOR_CLASS_VERSION;

   eddc->func.mem_alloc = _eet_mem_alloc;
   eddc->func.mem_free = _eet_mem_free;
   eddc->func.str_alloc = (char *(*)(const char *))eina_stringshare_add;
   eddc->func.str_free = eina_stringshare_del;
   eddc->func.str_direct_alloc = NULL;
   eddc->func.str_direct_free = NULL;
   eddc->func.list_next = (void *(*)(void *))eina_list_next;
   eddc->func.list_append = (void *(*)(void *, void *))eina_list_append;
   eddc->func.list_data = (void *(*)(void *))eina_list_data_get;
   eddc->func.list_free = (void *(*)(void *))eina_list_free;
   eddc->func.hash_foreach = (void (*)(void *, int (*)(void *, const char *, void *, void *), void *))_eet_eina_hash_foreach;
   eddc->func.hash_add = (void *(*)(void *, const char *, void *))_eet_eina_hash_add_alloc;
   eddc->func.hash_free = (void (*)(void *))_eet_eina_hash_free;

   /* This will cause an ABI incompatibility */
   eddc->func.array_alloc = _eet_mem_alloc;
   eddc->func.array_free = _eet_mem_free;

   return EINA_TRUE;
}

EAPI Eina_Bool
eet_eina_file_data_descriptor_class_set(Eet_Data_Descriptor_Class *eddc,
     /* When we change the structure content in the future, we need to handle old structure type too */
                                        unsigned int               eddc_size,
                                        const char                *name,
                                        int                        size)
{
   if (!eet_eina_stream_data_descriptor_class_set(eddc, eddc_size, name, size))
     return EINA_FALSE;

   eddc->version = EET_DATA_DESCRIPTOR_CLASS_VERSION;

   eddc->func.hash_add = (void *(*)(void *, const char *, void *))_eet_eina_hash_direct_add_alloc;
   eddc->func.str_direct_alloc = _eet_str_direct_alloc;
   eddc->func.str_direct_free = _eet_str_direct_free;

   return EINA_TRUE;
}

static Eet_Data_Descriptor *
_eet_data_descriptor_new(const Eet_Data_Descriptor_Class *eddc,
                         int                              version)
{
   Eet_Data_Descriptor *edd;

   if (!eddc)
     return NULL;

   edd = calloc(1, sizeof (Eet_Data_Descriptor));
   if (!edd)
     return NULL;

   edd->name = eddc->name;
   edd->ed = NULL;
   edd->size = eddc->size;
   edd->func.mem_alloc = _eet_mem_alloc;
   edd->func.mem_free = _eet_mem_free;
   edd->func.str_alloc = _eet_str_alloc;
   edd->func.str_free = _eet_str_free;
   if (eddc->func.mem_alloc)
     edd->func.mem_alloc = eddc->func.mem_alloc;

   if (eddc->func.mem_free)
     edd->func.mem_free = eddc->func.mem_free;

   if (eddc->func.str_alloc)
     edd->func.str_alloc = eddc->func.str_alloc;

   if (eddc->func.str_free)
     edd->func.str_free = eddc->func.str_free;

   edd->func.list_next = eddc->func.list_next;
   edd->func.list_append = eddc->func.list_append;
   edd->func.list_data = eddc->func.list_data;
   edd->func.list_free = eddc->func.list_free;
   edd->func.hash_foreach = eddc->func.hash_foreach;
   edd->func.hash_add = eddc->func.hash_add;
   edd->func.hash_free = eddc->func.hash_free;

   if (eddc->version > 1 && version > 1)
     {
        edd->func.str_direct_alloc = eddc->func.str_direct_alloc;
        edd->func.str_direct_free = eddc->func.str_direct_free;
     }

   if (eddc->version > 2)
     {
        edd->func.type_get = eddc->func.type_get;
        edd->func.type_set = eddc->func.type_set;
     }

   if (eddc->version > 3)
     {
        edd->func.array_alloc = eddc->func.array_alloc;
        edd->func.array_free = eddc->func.array_free;
     }

   return edd;
}

EAPI Eet_Data_Descriptor *
eet_data_descriptor_new(const char                          *name,
                        int                                  size,
                        Eet_Descriptor_List_Next_Callback    func_list_next,
                        Eet_Descriptor_List_Append_Callback  func_list_append,
                        Eet_Descriptor_List_Data_Callback    func_list_data,
                        Eet_Descriptor_List_Free_Callback    func_list_free,
                        Eet_Descriptor_Hash_Foreach_Callback func_hash_foreach,
                        Eet_Descriptor_Hash_Add_Callback     func_hash_add,
                        Eet_Descriptor_Hash_Free_Callback    func_hash_free)
{
   Eet_Data_Descriptor_Class eddc;

   if (!name)
     return NULL;

   memset(&eddc, 0, sizeof (Eet_Data_Descriptor_Class));

   eddc.name = name;
   eddc.size = size;
   eddc.version = 0;

   eddc.func.list_next = func_list_next;
   eddc.func.list_append = func_list_append;
   eddc.func.list_data = func_list_data;
   eddc.func.list_free = func_list_free;
   eddc.func.hash_foreach = func_hash_foreach;
   eddc.func.hash_add = func_hash_add;
   eddc.func.hash_free = func_hash_free;

   return _eet_data_descriptor_new(&eddc, 0);
}

EAPI Eet_Data_Descriptor *
eet_data_descriptor2_new(const Eet_Data_Descriptor_Class *eddc)
{
   return _eet_data_descriptor_new(eddc, 1);
}

EAPI Eet_Data_Descriptor *
eet_data_descriptor3_new(const Eet_Data_Descriptor_Class *eddc)
{
   return _eet_data_descriptor_new(eddc, 2);
}

EAPI Eet_Data_Descriptor *
eet_data_descriptor_stream_new(const Eet_Data_Descriptor_Class *eddc)
{
   return _eet_data_descriptor_new(eddc, 1);
}

EAPI Eet_Data_Descriptor *
eet_data_descriptor_file_new(const Eet_Data_Descriptor_Class *eddc)
{
   return _eet_data_descriptor_new(eddc, 2);
}

EAPI const char *
eet_data_descriptor_name_get(const Eet_Data_Descriptor *edd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);
   return edd->name;
}


EAPI void
eet_data_descriptor_free(Eet_Data_Descriptor *edd)
{
   if (!edd)
     return;

   _eet_descriptor_hash_free(edd);
   if (edd->elements.set)
     {
        int i;
        for (i = 0; i < edd->elements.num; i++)
          {
             if (edd->elements.set[i].subtype_free)
               eet_data_descriptor_free(edd->elements.set[i].subtype);
          }
        free(edd->elements.set);
     }

   free(edd);
}

EAPI void
eet_data_descriptor_element_add(Eet_Data_Descriptor *edd,
                                const char          *name,
                                int                  type,
                                int                  group_type,
                                int                  offset,
                                int                  count,
/*        int counter_offset, */
                                const char          *counter_name /* FIXME: Useless should go on a major release */,
                                Eet_Data_Descriptor *subtype)
{
   Eet_Data_Element *ede;
   Eet_Data_Element *tmp;

   EINA_SAFETY_ON_NULL_RETURN(edd);

   /* Sanity check to avoid crash later at runtime */
   if (type < EET_T_UNKNOW ||
       type >= EET_T_LAST)
     {
        CRI("Preventing later bug due to unknown type: %i", type);
        return;
     }
   if (offset < 0)
     {
        CRI("Preventing later buffer underrun : offset = %i", offset);
        return;
     }
   if (offset > edd->size)
     {
        CRI("Preventing later buffer overrun : offset = %i in a structure of %i bytes", offset, edd->size);
        return;
     }
   if (group_type == EET_G_UNKNOWN && type != EET_T_UNKNOW)
     {
        if (offset + eet_basic_codec[type - 1].size > edd->size)
          {
             CRI("Preventing later buffer overrun : offset = %i, size = %i in a structure of %i bytes", offset, eet_basic_codec[type - 1].size, edd->size);
             return;
          }
     }
   else if ((offset + sizeof (void*)) > (unsigned int) edd->size)
     {
        CRI("Preventing later buffer overrun : offset = %i, estimated size = %zu in a structure of %i bytes", offset, sizeof (void*), edd->size);
        return;
     }

   /* UNION, VARIANT type would not work with simple type, we need a way to map the type. */
   if ((group_type == EET_G_UNION
        || group_type == EET_G_VARIANT)
       &&
       (type != EET_T_UNKNOW
        || !subtype
        || !subtype->func.type_get
        || !subtype->func.type_set))
     return;

   /* VARIANT type will only work if the map only contains EET_G_*, but not UNION, VARIANT and ARRAY. */
   if (group_type == EET_G_VARIANT)
     {
        int i;

        for (i = 0; i < subtype->elements.num; ++i)
          if (subtype->elements.set[i].type != EET_T_UNKNOW
              && subtype->elements.set[i].group_type > EET_G_VAR_ARRAY
              && subtype->elements.set[i].group_type < EET_G_UNION)
            return;

        subtype->unified_type = EINA_TRUE;
     }

   if (subtype
       && subtype->unified_type
       && (type != EET_T_UNKNOW
           || group_type < EET_G_UNION))
     return;

   /* Sanity check done, let allocate ! */
   edd->elements.num++;
   tmp = realloc(edd->elements.set, edd->elements.num * sizeof(Eet_Data_Element));
   if (!tmp)
     return;

   edd->elements.set = tmp;
   ede = &(edd->elements.set[edd->elements.num - 1]);
   ede->name = name;
   ede->directory_name_ptr = NULL;
   ede->subtype_free = EINA_FALSE;

   /*
    * We do a special case when we do list,hash or whatever group of simple type.
    * Instead of handling it in encode/decode/dump/undump, we create an
    * implicit structure with only the simple type.
    */
   if ((group_type > EET_G_UNKNOWN)
       && (group_type < EET_G_LAST)
       && (((type >= EET_T_CHAR) && (type <= EET_T_ULONG_LONG))
           || ((type >= EET_T_F32P32) && (type <= EET_T_F8P24)))
       && (!subtype))
     {
        subtype = calloc(1, sizeof (Eet_Data_Descriptor));
        if (!subtype)
          return;

        subtype->name = "implicit";
        subtype->size = eet_basic_codec[type - 1].size;
        memcpy(&subtype->func, &edd->func, sizeof(subtype->func));

        eet_data_descriptor_element_add(subtype,
                                        eet_basic_codec[type - 1].name,
                                        type,
                                        EET_G_UNKNOWN,
                                        0,
                                        0,
                                        /* 0,  */ NULL,
                                        NULL);
        type = EET_T_UNKNOW;
        ede->subtype_free = EINA_TRUE;
     }

   ede->type = type;
   ede->group_type = group_type;
   ede->offset = offset;
   ede->count = count;
   /* FIXME: For the time being, VAR_ARRAY, UNION and VARIANT  will put the counter_offset in count. */
   ede->counter_offset = count;
/*    ede->counter_offset = counter_offset; */
   ede->counter_name = counter_name;

   if (subtype)
     INF("Adding '%s' of size %i to '%s' at offset %i.", 
         subtype->name, subtype->size,
         edd->name, offset);

   ede->subtype = subtype;
}

EAPI void *
eet_data_read_cipher(Eet_File            *ef,
                     Eet_Data_Descriptor *edd,
                     const char          *name,
                     const char          *cipher_key)
{
   const Eet_Dictionary *ed = NULL;
   const void *data = NULL;
   void *data_dec;
   Eet_Free_Context context;
   int required_free = 0;
   int size;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);
   ed = eet_dictionary_get(ef);

   if (!cipher_key)
     data = eet_read_direct(ef, name, &size);

   if (!data)
     {
        required_free = 1;
        data = eet_read_cipher(ef, name, &size, cipher_key);
        if (!data)
          return NULL;
     }

   eet_free_context_init(&context);
   data_dec = _eet_data_descriptor_decode(&context, ed, edd, data, size, NULL, 0);
   eet_free_context_shutdown(&context);

   if (required_free)
     free((void *)data);

   return data_dec;
}

EAPI void *
eet_data_read_cipher_buffer(Eet_File            *ef,
                            Eet_Data_Descriptor *edd,
                            const char          *name,
                            const char          *cipher_key, 
                            char* buffer,
                            int buffer_size)
{
   const Eet_Dictionary *ed = NULL;
   const void *data = NULL;
   void *data_dec;
   Eet_Free_Context context;
   int required_free = 0;
   int size;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);
   ed = eet_dictionary_get(ef);

   if (!cipher_key)
     data = eet_read_direct(ef, name, &size);

   if (!data)
     {
        required_free = 1;
        data = eet_read_cipher(ef, name, &size, cipher_key);
        if (!data)
          return NULL;
     }

   eet_free_context_init(&context);
   data_dec = _eet_data_descriptor_decode(&context, ed, edd, data, size, buffer, buffer_size);
   eet_free_context_shutdown(&context);

   if (required_free)
     free((void *)data);

   return data_dec;
}

EAPI Eet_Node *
eet_data_node_read_cipher(Eet_File   *ef,
                          const char *name,
                          const char *cipher_key)
{
   const Eet_Dictionary *ed = NULL;
   const void *data = NULL;
   Eet_Node *result;
   Eet_Free_Context context;
   int required_free = 0;
   int size;

   ed = eet_dictionary_get(ef);

   if (!cipher_key)
     data = eet_read_direct(ef, name, &size);

   if (!data)
     {
        required_free = 1;
        data = eet_read_cipher(ef, name, &size, cipher_key);
        if (!data)
          return NULL;
     }

   eet_free_context_init(&context);
   result = _eet_data_descriptor_decode(&context, ed, NULL, data, size, NULL, 0);
   eet_free_context_shutdown(&context);

   if (required_free)
     free((void *)data);

   return result;
}

EAPI void *
eet_data_read(Eet_File            *ef,
              Eet_Data_Descriptor *edd,
              const char          *name)
{
   return eet_data_read_cipher(ef, edd, name, NULL);
}

EAPI int
eet_data_write_cipher(Eet_File            *ef,
                      Eet_Data_Descriptor *edd,
                      const char          *name,
                      const char          *cipher_key,
                      const void          *data,
                      int                  comp)
{
   Eet_Dictionary *ed;
   void *data_enc;
   int size;
   int val;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, 0);

   ed = eet_dictionary_get(ef);

   data_enc = _eet_data_descriptor_encode(ed, edd, data, &size);
   if (!data_enc)
     return 0;

   val = eet_write_cipher(ef, name, data_enc, size, comp, cipher_key);
   free(data_enc);
   return val;
}

EAPI int
eet_data_write(Eet_File            *ef,
               Eet_Data_Descriptor *edd,
               const char          *name,
               const void          *data,
               int                  comp)
{
   return eet_data_write_cipher(ef, edd, name, NULL, data, comp);
}

static void
eet_free_context_init(Eet_Free_Context *context)
{
   memset(context, 0, sizeof (Eet_Free_Context));

   eina_array_step_set(&context->freelist.list,
                       sizeof (context->freelist.list),
                       32);
   eina_array_step_set(&context->freelist_array.list,
                       sizeof (context->freelist.list),
                       32);
   eina_array_step_set(&context->freelist_list.list,
                       sizeof (context->freelist.list),
                       32);
   eina_array_step_set(&context->freelist_hash.list,
                       sizeof (context->freelist.list),
                       32);
   eina_array_step_set(&context->freelist_str.list,
                       sizeof (context->freelist.list),
                       32);
   eina_array_step_set(&context->freelist_direct_str.list,
                       sizeof (context->freelist.list),
                       32);
}

static void
eet_free_context_shutdown(Eet_Free_Context *context)
{
   eina_array_flush(&context->freelist.list);
   eina_array_flush(&context->freelist_array.list);
   eina_array_flush(&context->freelist_list.list);
   eina_array_flush(&context->freelist_hash.list);
   eina_array_flush(&context->freelist_str.list);
   eina_array_flush(&context->freelist_direct_str.list);
}

static void
_eet_free_add(Eet_Free *ef,
              void     *data)
{
   eina_array_push(&ef->list, data);
}

static void
_eet_free_reset(Eet_Free *ef)
{
   if (ef->ref > 0)
     return;

   eina_array_clean(&ef->list);
}

static void
_eet_free_ref(Eet_Free *ef)
{
   ef->ref++;
}

static void
_eet_free_unref(Eet_Free *ef)
{
   ef->ref--;
}

#define _eet_freelist_add(Ctx, Data) _eet_free_add(&Ctx->freelist, Data);
#define _eet_freelist_reset(Ctx)     _eet_free_reset(&Ctx->freelist);
#define _eet_freelist_ref(Ctx)       _eet_free_ref(&Ctx->freelist);
#define _eet_freelist_unref(Ctx)     _eet_free_unref(&Ctx->freelist);

static void
_eet_freelist_free(Eet_Free_Context    *context,
                   Eet_Data_Descriptor *edd)
{
   void *track;
   Eina_Array_Iterator it;
   unsigned int i;

   if (context->freelist.ref > 0)
     return;

   EINA_ARRAY_ITER_NEXT(&context->freelist.list, i, track, it)
     if (track)
       {
          if (edd)
            edd->func.mem_free(track);
          else
            free(track);
       }
   _eet_free_reset(&context->freelist);
}

#define _eet_freelist_array_add(Ctx, Data) _eet_free_add(&Ctx->freelist_array, Data);
#define _eet_freelist_array_reset(Ctx)     _eet_free_reset(&Ctx->freelist_array);
#define _eet_freelist_array_ref(Ctx)       _eet_free_ref(&Ctx->freelist_array);
#define _eet_freelist_array_unref(Ctx)     _eet_free_unref(&Ctx->freelist_array);

static void
_eet_freelist_array_free(Eet_Free_Context    *context,
                         Eet_Data_Descriptor *edd)
{
   void *track;
   Eina_Array_Iterator it;
   unsigned int i;

   if (context->freelist_array.ref > 0)
     return;

   EINA_ARRAY_ITER_NEXT(&context->freelist_array.list, i, track, it)
     if (track)
       {
          if (edd)
            {
               if (edd->func.array_free)
                 edd->func.array_free(track);
               else
                 edd->func.mem_free(track);
            }
          else
            free(track);
       }
   _eet_free_reset(&context->freelist_array);
}

#define _eet_freelist_list_add(Ctx, Data) _eet_free_add(&Ctx->freelist_list, Data);
#define _eet_freelist_list_reset(Ctx)     _eet_free_reset(&Ctx->freelist_list);
#define _eet_freelist_list_ref(Ctx)       _eet_free_ref(&Ctx->freelist_list);
#define _eet_freelist_list_unref(Ctx)     _eet_free_unref(&Ctx->freelist_list);

static void
_eet_freelist_list_free(Eet_Free_Context    *context,
                        Eet_Data_Descriptor *edd)
{
   void *track;
   Eina_Array_Iterator it;
   unsigned int i;

   if (context->freelist_list.ref > 0)
     return;

   EINA_ARRAY_ITER_NEXT(&context->freelist_list.list, i, track, it)
     if (track)
       {
          if (edd)
            edd->func.list_free(*((void **)(track)));
       }
   _eet_free_reset(&context->freelist_list);
}

#define _eet_freelist_str_add(Ctx, Data) _eet_free_add(&Ctx->freelist_str, Data);
#define _eet_freelist_str_reset(Ctx)     _eet_free_reset(&Ctx->freelist_str);
#define _eet_freelist_str_ref(Ctx)       _eet_free_ref(&Ctx->freelist_str);
#define _eet_freelist_str_unref(Ctx)     _eet_free_unref(&Ctx->freelist_str);

static void
_eet_freelist_str_free(Eet_Free_Context    *context,
                       Eet_Data_Descriptor *edd)
{
   void *track;
   Eina_Array_Iterator it;
   unsigned int i;

   if (context->freelist_str.ref > 0)
     return;

   EINA_ARRAY_ITER_NEXT(&context->freelist_str.list, i, track, it)
     if (track)
       {
          if (edd)
            edd->func.str_free(track);
          else
            free(track);
       }
   _eet_free_reset(&context->freelist_str);
}

#define _eet_freelist_direct_str_add(Ctx, Data) _eet_free_add(&Ctx->freelist_direct_str, Data);
#define _eet_freelist_direct_str_reset(Ctx)     _eet_free_reset(&Ctx->freelist_direct_str);
#define _eet_freelist_direct_str_ref(Ctx)       _eet_free_ref(&Ctx->freelist_direct_str);
#define _eet_freelist_direct_str_unref(Ctx)     _eet_free_unref(&Ctx->freelist_direct_str);

static void
_eet_freelist_direct_str_free(Eet_Free_Context    *context,
                              Eet_Data_Descriptor *edd)
{
   void *track;
   Eina_Array_Iterator it;
   unsigned int i;

   if (context->freelist_direct_str.ref > 0)
     return;

   EINA_ARRAY_ITER_NEXT(&context->freelist_str.list, i, track, it)
     if (track)
       {
          if (edd)
            edd->func.str_direct_free(track);
          else
            free(track);
       }
   _eet_free_reset(&context->freelist_direct_str);
}

#define _eet_freelist_hash_add(Ctx, Data) _eet_free_add(&Ctx->freelist_hash, Data);
#define _eet_freelist_hash_reset(Ctx)     _eet_free_reset(&Ctx->freelist_hash);
#define _eet_freelist_hash_ref(Ctx)       _eet_free_ref(&Ctx->freelist_hash);
#define _eet_freelist_hash_unref(Ctx)     _eet_free_unref(&Ctx->freelist_hash);

static void
_eet_freelist_hash_free(Eet_Free_Context    *context,
                        Eet_Data_Descriptor *edd)
{
   void *track;
   Eina_Array_Iterator it;
   unsigned int i;

   if (context->freelist_hash.ref > 0)
     return;

   EINA_ARRAY_ITER_NEXT(&context->freelist_hash.list, i, track, it)
     if (track)
       {
          if (edd)
            edd->func.hash_free(track);
          else
            free(track);
       }
   _eet_free_reset(&context->freelist_hash);
}

static void
_eet_freelist_all_ref(Eet_Free_Context *freelist_context)
{
   _eet_freelist_ref(freelist_context);
   _eet_freelist_str_ref(freelist_context);
   _eet_freelist_list_ref(freelist_context);
   _eet_freelist_hash_ref(freelist_context);
   _eet_freelist_direct_str_ref(freelist_context);
}

static void
_eet_freelist_all_unref(Eet_Free_Context *freelist_context)
{
   _eet_freelist_unref(freelist_context);
   _eet_freelist_str_unref(freelist_context);
   _eet_freelist_list_unref(freelist_context);
   _eet_freelist_hash_unref(freelist_context);
   _eet_freelist_direct_str_unref(freelist_context);
}

static int
eet_data_descriptor_encode_hash_cb(void       *hash EINA_UNUSED,
                                   const char *cipher_key,
                                   void       *hdata,
                                   void       *fdata)
{
   Eet_Dictionary *ed;
   Eet_Data_Encode_Hash_Info *edehi;
   Eet_Data_Stream *ds;
   Eet_Data_Element *ede;
   Eet_Data_Chunk *echnk;
   void *data = NULL;
   int size;

   edehi = fdata;
   ede = edehi->ede;
   ds = edehi->ds;
   ed = edehi->ed;

   /* Store key */
   data = eet_data_put_type(ed,
                            EET_T_STRING,
                            &cipher_key,
                            &size);
   if (data)
     {
        echnk = eet_data_chunk_new(data,
                                   size,
                                   ede->name,
                                   ede->type,
                                   ede->group_type);
        eet_data_chunk_put(ed, echnk, ds);
        eet_data_chunk_free(echnk);
        free(data);
        data = NULL;
     }

   EET_ASSERT(!((ede->type > EET_T_UNKNOW) && (ede->type < EET_T_STRING)), return );

   /* Store data */
   if (ede->type >= EET_T_STRING)
     eet_data_put_unknown(ed, NULL, ede, ds, &hdata);
   else
     {
        if (ede->subtype)
          data = _eet_data_descriptor_encode(ed,
                                             ede->subtype,
                                             hdata,
                                             &size);

        if (data)
          {
             echnk = eet_data_chunk_new(data,
                                        size,
                                        ede->name,
                                        ede->type,
                                        ede->group_type);
             eet_data_chunk_put(ed, echnk, ds);
             eet_data_chunk_free(echnk);
             free(data);
             data = NULL;
          }
     }

   return 1;
}

static char *
_eet_data_dump_token_get(const char *src,
                         int        *len)
{
   const char *p;
   char *tok = NULL, *temp;
   int in_token = 0;
   int in_quote = 0;
   int in_escape = 0;
   int tlen = 0, tsize = 0;

#define TOK_ADD(x)                        \
  do {                                    \
       tlen++;                            \
       if (tlen >= tsize)                 \
         {                                \
            tsize += 32;                  \
            temp = tok;                   \
            tok = realloc(tok, tsize);    \
            if (!tok)                     \
              {                           \
                 tok = temp;              \
                 ERR("Realloc failed\n"); \
                 goto realloc_error;      \
              }                           \
         }                                \
       tok[tlen - 1] = x;                 \
    } while (0)

   for (p = src; *len > 0; p++, (*len)--)
     {
        if (in_token)
          {
             if (in_escape)
               {
                  switch (p[0]) {
                   case 'n':
                      TOK_ADD('\n');
                      break;
                   case '"':
                   case '\'':
                   case '\\':
                      TOK_ADD(p[0]);
                      break;
                   default:
                      ERR("Unknown escape character %#x (%c). Append as is",
                          p[0], p[0]);
                      TOK_ADD(p[0]);
                  }
                  in_escape = 0;
               }
             else if (p[0] == '\\')
               {
                  in_escape = 1;
               }
             else if (in_quote)
               {
                  if (p[0] == '\"')
                    in_quote = 0;
                  else
                    TOK_ADD(p[0]);
               }
             else
               {
                  if (p[0] == '\"')
                    in_quote = 1;
                  else
                    {
                       if ((isspace(p[0])) || (p[0] == ';')) /* token ends here */
                         {
                            TOK_ADD(0);
                            (*len)--;
                            return tok;
                         }
                       else
                         TOK_ADD(p[0]);
                    }
               }
          }
        else if (!((isspace(p[0])) || (p[0] == ';')))
          {
             in_token = 1;
             (*len)++;
             p--;
          }
     }
   if (in_token)
     {
        TOK_ADD(0);
        return tok;
     }

realloc_error:
   free(tok);

   return NULL;
}

static void
eet_data_encode(Eet_Dictionary  *ed,
                Eet_Data_Stream *ds,
                void            *data,
                const char      *name,
                int              size,
                int              type,
                int              group_type,
                Eina_Bool        free_data)
{
   Eet_Data_Chunk *echnk;

   if (!data)
     type = EET_T_NULL;

   if (group_type != EET_G_UNKNOWN)
     if (type >= EET_T_LAST)
       type = EET_T_UNKNOW;

   echnk = eet_data_chunk_new(data, size, name, type, group_type);
   eet_data_chunk_put(ed, echnk, ds);
   eet_data_chunk_free(echnk);
   if (free_data) free(data);
}

static void *
_eet_data_dump_encode(int             parent_type,
                      Eet_Dictionary *ed,
                      Eet_Node       *node,
                      int            *size_ret)
{
   Eet_Data_Chunk *chnk = NULL;
   Eet_Data_Stream *ds;
   void *cdata, *data;
   int csize, size = 0;
   int count;
   int child_type;
   Eet_Node *n;

   if (_eet_data_words_bigendian == -1)
     {
        unsigned long int v;

        v = htonl(0x12345678);
        if (v == 0x12345678)
          _eet_data_words_bigendian = 1;
        else
          _eet_data_words_bigendian = 0;
     }

   if (!node)
     return NULL;

   ds = eet_data_stream_new();
   if (!ds)
     return NULL;

   switch (node->type)
     {
      case EET_G_UNKNOWN:
        for (n = node->values; n; n = n->next)
          {
             data = _eet_data_dump_encode(node->type, ed, n, &size);
             if (data)
               {
                  eet_data_stream_write(ds, data, size);
                  free(data);
               }
          }
        break;

      case EET_G_ARRAY:
      case EET_G_VAR_ARRAY:
        for (child_type = EET_T_NULL, n = node->values; n; n = n->next)
          {
             if (n->type != EET_T_NULL)
               {
                  child_type = n->type;
                  break;
               }
          }

        data = eet_data_put_type(ed,
                                 EET_T_INT,
                                 &node->count,
                                 &size);
        eet_data_encode(ed,
                        ds,
                        data,
                        node->name,
                        size,
                        child_type,
                        node->type,
                        EINA_TRUE);

        count = node->count;

        for (n = node->values; n; n = n->next)
          {
             int pos = ds->pos;

             switch (n->type)
               {
                case EET_T_STRING:
                case EET_T_INLINED_STRING:
                  data = eet_data_put_type(ed,
                                           n->type,
                                           &(n->data.value.str),
                                           &size);
                  if (data)
                    eet_data_encode(ed,
                                    ds,
                                    data,
                                    node->name,
                                    size,
                                    n->type,
                                    node->type,
                                    EINA_TRUE);

                  break;

                case EET_T_VALUE:
                case EET_T_NULL:
                  continue;

                default:
                  data = _eet_data_dump_encode(n->type, ed, n, &size);
                  eet_data_encode(ed,
                                  ds,
                                  data,
                                  node->name,
                                  size,
                                  n->type,
                                  node->type,
                                  EINA_TRUE);
                  break;
               }  /* switch */
             if (ds->pos != pos)
               count--;
          }

        for (; count; count--)
          {
             eet_data_encode(ed,
                             ds,
                             NULL,
                             node->name,
                             0,
                             EET_T_NULL,
                             node->type,
                             EINA_TRUE);
          }

        /* Array is somekind of special case, so we should embed it inside another chunk. */
        *size_ret = ds->pos;
        cdata = ds->data;

        ds->data = NULL;
        ds->size = 0;
        eet_data_stream_free(ds);

        return cdata;
        break;

      case EET_G_LIST:
        for (n = node->values; n; n = n->next)
          {
             switch (n->type)
               {
                case EET_T_STRING:
                case EET_T_INLINED_STRING:
                  data = eet_data_put_type(ed,
                                           n->type,
                                           &(n->data.value.str),
                                           &size);
                  if (data)
                    eet_data_encode(ed,
                                    ds,
                                    data,
                                    node->name,
                                    size,
                                    n->type,
                                    node->type,
                                    EINA_TRUE);

                  break;

                case EET_T_VALUE:
                case EET_T_NULL:
                  continue;

                default:
                  data = _eet_data_dump_encode(node->type, ed, n, &size);
                  eet_data_encode(ed,
                                  ds,
                                  data,
                                  node->name,
                                  size,
                                  n->type,
                                  node->type,
                                  EINA_TRUE);
               }  /* switch */
          }

        /* List is another somekind of special case, every chunk is embed inside a list chunk. */
        *size_ret = ds->pos;
        cdata = ds->data;

        ds->data = NULL;
        ds->size = 0;
        eet_data_stream_free(ds);

        return cdata;
        break;

      case EET_G_HASH:
        if (node->key)
          {
             data = eet_data_put_type(ed,
                                      EET_T_STRING,
                                      &node->key,
                                      &size);
             eet_data_encode(ed,
                             ds,
                             data,
                             node->name,
                             size,
                             node->type,
                             node->type,
                             EINA_TRUE);
          }
        else
          {
             /* A Hash without key will not decode correctly. */

             ds->data = NULL;
             ds->size = 0;
             eet_data_stream_free(ds);

             return NULL;
          }

        for (n = node->values; n; n = n->next)
          {
             switch (n->type)
               {
                case EET_T_STRING:
                case EET_T_INLINED_STRING:
                  data = eet_data_put_type(ed,
                                           n->type,
                                           &(n->data.value.str),
                                           &size);
                  if (data)
                    eet_data_encode(ed,
                                    ds,
                                    data,
                                    node->name,
                                    size,
                                    n->type,
                                    node->type,
                                    EINA_TRUE);

                  break;

                case EET_T_VALUE:
                case EET_T_NULL:
                  continue;

                default:
                  data = _eet_data_dump_encode(node->type, ed, n, &size);
                  eet_data_encode(ed,
                                  ds,
                                  data,
                                  node->name,
                                  size,
                                  n->type,
                                  node->type,
                                  EINA_TRUE);
               }  /* switch */
          }

        /* Hash is somekind of special case, so we should embed it inside another chunk. */
        *size_ret = ds->pos;
        cdata = ds->data;

        eet_data_stream_flush(ds);

        return cdata;

      case EET_T_VALUE:
      case EET_T_NULL:
        break;

#define EET_DATA_NODE_ENCODE(Eet_Type, Type)                                 \
case Eet_Type:                                                               \
  data = eet_data_put_type(ed, node->type, &(node->data.value.Type), &size); \
  if (data)                                                                  \
    {                                                                        \
       eet_data_encode(ed,                                                   \
                       ds,                                                   \
                       data,                                                 \
                       node->name,                                           \
                       size,                                                 \
                       node->type,                                           \
                       parent_type,                                          \
                       EINA_TRUE);                                           \
       cdata = ds->data;                                                     \
       *size_ret = ds->pos;                                                  \
       eet_data_stream_flush(ds);                                            \
       return cdata;                                                         \
    }   /* switch */                                                         \
  break;

        EET_DATA_NODE_ENCODE(EET_T_CHAR, c);
        EET_DATA_NODE_ENCODE(EET_T_SHORT, s);
        EET_DATA_NODE_ENCODE(EET_T_INT, i);
        EET_DATA_NODE_ENCODE(EET_T_LONG_LONG, l);
        EET_DATA_NODE_ENCODE(EET_T_FLOAT, f);
        EET_DATA_NODE_ENCODE(EET_T_DOUBLE, d);
        EET_DATA_NODE_ENCODE(EET_T_UCHAR, uc);
        EET_DATA_NODE_ENCODE(EET_T_USHORT, us);
        EET_DATA_NODE_ENCODE(EET_T_UINT, ui);
        EET_DATA_NODE_ENCODE(EET_T_ULONG_LONG, ul);
        EET_DATA_NODE_ENCODE(EET_T_INLINED_STRING, str);
        EET_DATA_NODE_ENCODE(EET_T_STRING, str);

      default:
        break;
     }

   if ((node->type >= EET_G_UNKNOWN) && (node->type < EET_G_LAST))
     chnk = eet_data_chunk_new(ds->data,
                               ds->pos,
                               node->name,
                               EET_T_UNKNOW,
                               node->type);
   else
     chnk = eet_data_chunk_new(ds->data,
                               ds->pos,
                               node->name,
                               node->type,
                               EET_G_UNKNOWN);

   eet_data_stream_flush(ds);

   ds = eet_data_stream_new();
   eet_data_chunk_put(ed, chnk, ds);
   cdata = ds->data;
   csize = ds->pos;

   eet_data_stream_flush(ds);
   *size_ret = csize;

   free(chnk->data);
   eet_data_chunk_free(chnk);

   return cdata;
}

static void *
_eet_data_dump_parse(Eet_Dictionary *ed,
                     int            *size_ret,
                     const char     *src,
                     int             size)
{
   void *cdata = NULL;
   const char *p = NULL;
#define M_NONE   0
#define M_STRUCT 1
#define M_       2
   int left, jump;
   Eet_Node *node_base = NULL;
   Eet_Node *node = NULL;
   Eet_Node *n = NULL, *nn = NULL;

   /* FIXME; handle parse errors */
#define TOK_GET(t) \
  jump = left; t = _eet_data_dump_token_get(p, &left); p += jump - left;
   left = size;
   for (p = src; p < (src + size); )
     {
        char *tok1, *tok2, *tok3, *tok4;

        TOK_GET(tok1);
        if (tok1)
          {
             if (!strcmp(tok1, "group"))
               {
                  TOK_GET(tok2);
                  if (tok2)
                    {
                       TOK_GET(tok3);
                       if (tok3)
                         {
                            TOK_GET(tok4);
                            if (tok4)
                              {
                                 if (!strcmp(tok4, "{"))
                                   {
/* we have 'group NAM TYP {' */
                                       n = eet_node_new();
                                       if (n)
                                         {
                                            n->parent = node;
                                            if (!node_base)
                                              node_base = n;

                                            if (node)
                                              {
/* append node */
                                                  if (!node->values)
                                                    node->values = n;
                                                  else
                                                    for (nn = node->values; nn;
                                                         nn = nn->next)
                                                      {
                                                         if (!nn->next)
                                                           {
                                                              nn->next = n;
                                                              break;
                                                           }
                                                      }
                                              }

                                            n->name = eina_stringshare_add(tok2);
                                            if      (!strcmp(tok3, "struct"))
                                              n->type = EET_G_UNKNOWN;
                                            else if (!strcmp(tok3, "array"))
                                              n->type = EET_G_ARRAY;
                                            else if (!strcmp(tok3, "var_array"))
                                              n->type = EET_G_VAR_ARRAY;
                                            else if (!strcmp(tok3, "list"))
                                              n->type = EET_G_LIST;
                                            else if (!strcmp(tok3, "hash"))
                                              n->type = EET_G_HASH;
                                            else
                                              ERR(
                                                "ERROR: group type '%s' invalid.",
                                                tok3);

                                            node = n;
                                         }
                                   }

                                 free(tok4);
                              }

                            free(tok3);
                         }

                       free(tok2);
                    }
               }
             else if (!strcmp(tok1, "value"))
               {
                  TOK_GET(tok2);
                  if (tok2)
                    {
                       TOK_GET(tok3);
                       if (tok3)
                         {
                            TOK_GET(tok4);
                            if (tok4)
                              {
/* we have 'value NAME TYP XXX' */
                                  if (node_base)
                                    {
                                       n = eet_node_new();
                                       if (n)
                                         {
                                            n->parent = node;
/* append node */
                                            if (!node->values)
                                              node->values = n;
                                            else
                                              for (nn = node->values; nn;
                                                   nn = nn->next)
                                                {
                                                   if (!nn->next)
                                                     {
                                                        nn->next = n;
                                                        break;
                                                     }
                                                }

                                            n->name = eina_stringshare_add(tok2);
                                            if      (!strcmp(tok3, "char:"))
                                              {
                                                 n->type = EET_T_CHAR;
                                                 sscanf(tok4, "%hhi",
                                                        &(n->data.value.c));
                                              }
                                            else if (!strcmp(tok3, "short:"))
                                              {
                                                 n->type = EET_T_SHORT;
                                                 sscanf(tok4, "%hi",
                                                        &(n->data.value.s));
                                              }
                                            else if (!strcmp(tok3, "int:"))
                                              {
                                                 n->type = EET_T_INT;
                                                 sscanf(tok4, "%i",
                                                        &(n->data.value.i));
                                              }
                                            else if (!strcmp(tok3, "long_long:"))
                                              {
                                                 n->type = EET_T_LONG_LONG;
                                                 sscanf(tok4, "%lli",
                                                        &(n->data.value.l));
                                              }
                                            else if (!strcmp(tok3, "float:"))
                                              {
                                                 n->type = EET_T_FLOAT;
                                                 sscanf(tok4, "%f",
                                                        &(n->data.value.f));
                                              }
                                            else if (!strcmp(tok3, "double:"))
                                              {
                                                 n->type = EET_T_DOUBLE;
                                                 sscanf(tok4, "%lf",
                                                        &(n->data.value.d));
                                              }
                                            else if (!strcmp(tok3, "uchar:"))
                                              {
                                                 n->type = EET_T_UCHAR;
                                                 sscanf(tok4, "%hhu",
                                                        &(n->data.value.uc));
                                              }
                                            else if (!strcmp(tok3, "ushort:"))
                                              {
                                                 n->type = EET_T_USHORT;
                                                 sscanf(tok4, "%hu",
                                                        &(n->data.value.us));
                                              }
                                            else if (!strcmp(tok3, "uint:"))
                                              {
                                                 n->type = EET_T_UINT;
                                                 sscanf(tok4, "%u",
                                                        &(n->data.value.ui));
                                              }
                                            else if (!strcmp(tok3, "ulong_long:"))
                                              {
                                                 n->type = EET_T_ULONG_LONG;
                                                 sscanf(tok4, "%llu",
                                                        &(n->data.value.ul));
                                              }
                                            else if (!strcmp(tok3, "string:"))
                                              {
                                                 n->type = EET_T_STRING;
                                                 n->data.value.str =
                                                   eina_stringshare_add(tok4);
                                              }
                                            else if (!strcmp(tok3, "inlined:"))
                                              {
                                                 n->type = EET_T_INLINED_STRING;
                                                 n->data.value.str =
                                                   eina_stringshare_add(tok4);
                                              }
                                            else if (!strcmp(tok3, "null"))
                                              {
                                                 n->type = EET_T_NULL;
                                                 n->data.value.str = NULL;
                                              }
                                            else
                                              ERR(
                                                "ERROR: value type '%s' invalid.",
                                                tok4);
                                         }
                                    }

                                  free(tok4);
                              }

                            free(tok3);
                         }

                       free(tok2);
                    }
               }
             else if (!strcmp(tok1, "key"))
               {
                  TOK_GET(tok2);
                  if (tok2)
                    {
/* we have 'key NAME' */
                        if (node)
                          node->key = eina_stringshare_add(tok2);

                        free(tok2);
                    }
               }
             else if (!strcmp(tok1, "count"))
               {
                  TOK_GET(tok2);
                  if (tok2)
                    {
/* we have a 'count COUNT' */
                        if (node)
                          sscanf(tok2, "%i", &(node->count));

                        free(tok2);
                    }
               }
             else if (!strcmp(tok1, "}"))
               /* we have an end of the group */
               if (node)
                 node = node->parent;

             free(tok1);
          }
     }

   if (node_base)
     {
        cdata = _eet_data_dump_encode(EET_G_UNKNOWN, ed, node_base, size_ret);
        eet_node_del(node_base);
     }

   return cdata;
}

#define NEXT_CHUNK(P, Size, Echnk, Ed)                    \
  {                                                       \
     int __tmp;                                           \
     __tmp = Ed ? (int)(sizeof(int) * 2) : Echnk.len + 4; \
     P += (4 + Echnk.size + __tmp);                       \
     Size -= (4 + Echnk.size + __tmp);                    \
  }

static void *
_eet_data_descriptor_decode(Eet_Free_Context     *context,
                            const Eet_Dictionary *ed,
                            Eet_Data_Descriptor  *edd,
                            const void           *data_in,
                            int                   size_in,
                            void                 *data_out,
                            int                   size_out)
{
   Eet_Node *result = NULL;
   void *data = NULL;
   char *p;
   int size, i;
   Eet_Data_Chunk chnk;
   Eina_Bool need_free = EINA_FALSE;

   if (_eet_data_words_bigendian == -1)
     {
        unsigned long int v;

        v = htonl(0x12345678);
        if (v == 0x12345678)
          _eet_data_words_bigendian = 1;
        else
          _eet_data_words_bigendian = 0;
     }

   if (edd)
     {
        if (data_out)
          {
             if (size_out <= edd->size)
               data = data_out;
          }
        else
          {
             data = edd->func.mem_alloc(edd->size);
             need_free = EINA_TRUE;
          }

        if (!data)
          return NULL;

        if (edd->ed != ed)
          {
             for (i = 0; i < edd->elements.num; i++)
               edd->elements.set[i].directory_name_ptr = NULL;
             edd->ed = ed;
          }
     }

   _eet_freelist_all_ref(context);

   memset(&chnk, 0, sizeof(Eet_Data_Chunk));
   eet_data_chunk_get(ed, &chnk, data_in, size_in);
   EINA_SAFETY_ON_NULL_GOTO(chnk.name, error);

   if (edd)
     {
        EINA_SAFETY_ON_TRUE_GOTO(strcmp(chnk.name, edd->name), error);
     }

   p = chnk.data;
   if (ed)
     size = size_in - (4 + sizeof(int) * 2);
   else
     size = size_in - (4 + 4 + chnk.len);

   if (edd)
     {
        if (!edd->elements.hash.buckets)
          _eet_descriptor_hash_new(edd);
     }
   else
     {
        switch (chnk.group_type)
          {
           case EET_G_UNKNOWN:
             switch (chnk.type)
               {
                case EET_T_STRING:
                  return eet_node_string_new(chnk.name, chnk.data);

                case EET_T_INLINED_STRING:
                  return eet_node_inlined_string_new(chnk.name, chnk.data);

                case EET_T_VALUE:
                case EET_T_NULL:
                  return eet_node_null_new(chnk.name);

                default:
                  result = eet_node_struct_new(chnk.name, NULL);
               }  /* switch */
             break;

           case EET_G_VAR_ARRAY:
             return eet_node_var_array_new(chnk.name, NULL);

           case EET_G_LIST:
           case EET_G_HASH:
           case EET_G_ARRAY:
           case EET_G_UNION:
           case EET_G_VARIANT:
           default:
             ERR("Decoding error!");
             goto error;
          }
     }

   while (size > 0)
     {
        Eet_Data_Chunk echnk;
        Eet_Data_Element *ede = NULL;
        Eet_Node *child = NULL;
        int group_type = EET_G_UNKNOWN, type = EET_T_UNKNOW;
        int ret = 0;

        /* get next data chunk */
        memset(&echnk, 0, sizeof(Eet_Data_Chunk));
        eet_data_chunk_get(ed, &echnk, p, size);
        EINA_SAFETY_ON_NULL_GOTO(echnk.name, error); /* FIXME: don't REPLY on edd - work without */

        if (edd)
          {
             ede = _eet_descriptor_hash_find(edd, echnk.name, echnk.hash);
             if (ede)
               {
                  group_type = ede->group_type;
                  type = ede->type;
                  if ((echnk.type == 0) && (echnk.group_type == 0))
                    {
                       type = ede->type;
                       group_type = ede->group_type;
                    }
                  else
                    {
                       if (IS_SIMPLE_TYPE(echnk.type) &&
                           eet_data_type_match(echnk.type, ede->type))
/* Needed when converting on the fly from FP to Float */
                         type = ede->type;
                       else if (IS_SIMPLE_TYPE(echnk.type) &&
                                echnk.type == EET_T_NULL &&
                                ede->type == EET_T_VALUE)
/* EET_T_NULL can become an EET_T_VALUE as EET_T_VALUE are pointer to */
                         type = echnk.type;
                       else if ((echnk.group_type > EET_G_UNKNOWN) &&
                                (echnk.group_type < EET_G_LAST) &&
                                (echnk.group_type == ede->group_type))
                         group_type = echnk.group_type;
                    }
               }
          }
        /*...... dump to node */
        else
          {
             type = echnk.type;
             group_type = echnk.group_type;
          }

        if (!edd && group_type == EET_G_UNKNOWN && IS_SIMPLE_TYPE(type))
          {
             unsigned long long dd[128];

             ret = eet_data_get_type(ed,
                                     type,
                                     echnk.data,
                                     ((char *)echnk.data) + echnk.size,
                                     dd);
             EINA_SAFETY_ON_TRUE_GOTO(ret <= 0, error);

             child = eet_data_node_simple_type(type, echnk.name, dd);

             eet_node_struct_append(result, echnk.name, child);
          }
        else
          {
             ret = eet_group_codec[group_type - 100].get(
                 context,
                 ed,
                 edd,
                 ede,
                 &echnk,
                 type,
                 group_type,
                 ede ? (void *)(((char *)data) + ede->offset) : (void **)&result,
                 &p,
                 &size);

             EINA_SAFETY_ON_TRUE_GOTO(ret <= 0, error);
          }

        /* advance to next chunk */
        NEXT_CHUNK(p, size, echnk, ed);
     }

   _eet_freelist_all_unref(context);
   if (!edd)
     {
        _eet_freelist_str_free(context, edd);
        _eet_freelist_direct_str_free(context, edd);
        _eet_freelist_list_free(context, edd);
        _eet_freelist_hash_free(context, edd);
        _eet_freelist_array_free(context, edd);
        _eet_freelist_free(context, edd);
     }
   else
     {
        _eet_freelist_reset(context);
        _eet_freelist_str_reset(context);
        _eet_freelist_list_reset(context);
        _eet_freelist_hash_reset(context);
        _eet_freelist_direct_str_reset(context);
        _eet_freelist_array_reset(context);
     }

   if (!edd)
     return result;

   return data;

error:
   if (need_free) free(data);
   eet_node_del(result);

   _eet_freelist_all_unref(context);
   _eet_freelist_str_free(context, edd);
   _eet_freelist_direct_str_free(context, edd);
   _eet_freelist_list_free(context, edd);
   _eet_freelist_hash_free(context, edd);
   _eet_freelist_array_free(context, edd);
   _eet_freelist_free(context, edd);

   /* FIXME: Warn that something goes wrong here. */
   return NULL;
}

static int
eet_data_get_list(Eet_Free_Context     *context,
                  const Eet_Dictionary *ed,
                  Eet_Data_Descriptor  *edd,
                  Eet_Data_Element     *ede,
                  Eet_Data_Chunk       *echnk,
                  int                   type,
                  int                   group_type EINA_UNUSED,
                  void                 *data,
                  char                **p,
                  int                  *size)
{
   Eet_Data_Descriptor *subtype = NULL;
   void *list = NULL;
   void **ptr;
   void *data_ret;

   EET_ASSERT(!((type > EET_T_UNKNOW) && (type < EET_T_STRING)), return 0);

   if (ede)
     {
        subtype = ede->subtype;

        if (type != ede->type)
          return 0;
     }

   ptr = (void **)data;
   list = *ptr;
   data_ret = NULL;

   if (IS_POINTER_TYPE(type))
     POINTER_TYPE_DECODE(context,
                         ed,
                         edd,
                         ede,
                         echnk,
                         type,
                         &data_ret,
                         p,
                         size,
                         on_error);
   else
     {
        STRUCT_TYPE_DECODE(data_ret,
                           context,
                           ed,
                           subtype,
                           echnk->data,
                           echnk->size,
                           -1,
                           on_error);
        if (subtype) _eet_freelist_add(context, data_ret);
     }

   if (edd)
     {
        void *oldlist = list;

        list = edd->func.list_append(list, data_ret);
        *ptr = list;
        if (oldlist != list)
          _eet_freelist_list_add(context, ptr);
     }
   else
     eet_node_list_append(*((Eet_Node **)data), echnk->name, data_ret);

   return 1;

on_error:
   return 0;
}

static int
eet_data_get_hash(Eet_Free_Context     *context,
                  const Eet_Dictionary *ed,
                  Eet_Data_Descriptor  *edd,
                  Eet_Data_Element     *ede,
                  Eet_Data_Chunk       *echnk,
                  int                   type,
                  int                   group_type EINA_UNUSED,
                  void                 *data,
                  char                **p,
                  int                  *size)
{
   void **ptr;
   void *hash = NULL;
   char *key = NULL;
   void *data_ret = NULL;
   int ret = 0;

   EET_ASSERT(!((type > EET_T_UNKNOW) && (type < EET_T_STRING)), return 0);

   ptr = (void **)data;
   hash = *ptr;

   /* Read key */
   ret = eet_data_get_type(ed,
                           EET_T_STRING,
                           echnk->data,
                           ((char *)echnk->data) + echnk->size,
                           &key);
   EINA_SAFETY_ON_TRUE_GOTO(ret <= 0, on_error);

   EINA_SAFETY_ON_NULL_GOTO(key, on_error);

   /* Advance to next chunk */
   NEXT_CHUNK((*p), (*size), (*echnk), ed);
   memset(echnk, 0, sizeof(Eet_Data_Chunk));

   /* Read value */
   eet_data_chunk_get(ed, echnk, *p, *size);
   EINA_SAFETY_ON_NULL_GOTO(echnk->name, on_error);

   if (ede)
     if ((ede->group_type != echnk->group_type)
         || (ede->type != echnk->type))
       {
          ERR("ERROR!");
          goto on_error;
       }

   if (IS_POINTER_TYPE(echnk->type))
     POINTER_TYPE_DECODE(context,
                         ed,
                         edd,
                         ede,
                         echnk,
                         echnk->type,
                         &data_ret,
                         p,
                         size,
                         on_error);
   else
     {
        STRUCT_TYPE_DECODE(data_ret,
                           context,
                           ed,
                           ede ? ede->subtype : NULL,
                           echnk->data,
                           echnk->size,
                           -1,
                           on_error);

        if (ede ? ede->subtype : NULL) _eet_freelist_add(context, data_ret);
     }

   if (edd)
     {
        void *oldhash = hash;

        hash = edd->func.hash_add(hash, key, data_ret);
        *ptr = hash;
        if (oldhash != hash)
          _eet_freelist_hash_add(context, hash);
     }
   else
     eet_node_hash_add(*((Eet_Node **)data), echnk->name, key, data_ret);

   return 1;

on_error:
   return 0;
}

/* var arrays and fixed arrays have to
 * get all chunks at once. for fixed arrays
 * we can get each chunk and increment a
 * counter stored on the element itself but
 * it wont be thread safe. for var arrays
 * we still need a way to get the number of
 * elements from the data, so storing the
 * number of elements and the element data on
 * each chunk is pointless.
 */
static int
eet_data_get_array(Eet_Free_Context     *context,
                   const Eet_Dictionary *ed,
                   Eet_Data_Descriptor  *edd,
                   Eet_Data_Element     *ede,
                   Eet_Data_Chunk       *echnk,
                   int                   type,
                   int                   group_type,
                   void                 *data,
                   char                **p,
                   int                  *size)
{
   Eina_List *childs = NULL;
   const char *name;
   Eet_Node *tmp;
   void *ptr;
   int count;
   int ret;
   int subsize = 0;
   int i;

   EET_ASSERT(!((type > EET_T_UNKNOW) && (type < EET_T_STRING)), return 0);

   ptr = data;
   /* read the number of elements */
   ret = eet_data_get_type(ed,
                           EET_T_INT,
                           echnk->data,
                           ((char *)echnk->data) + echnk->size,
                           &count);
   if (ret <= 0)
     return ret;

   name = echnk->name;

   if (ede)
     {
        if (IS_POINTER_TYPE(type))
          subsize = eet_basic_codec[ede->type - 1].size;
        else
          subsize = ede->subtype->size;

        if (group_type == EET_G_VAR_ARRAY)
          {
             /* store the number of elements
              * on the counter offset */
               *(int *)(((char *)data) + ede->count - ede->offset) = count;
     /* allocate space for the array of elements */
               if (edd->func.array_alloc)
                 *(void **)ptr = edd->func.array_alloc(count * subsize);
               else
                 *(void **)ptr = edd->func.mem_alloc(count * subsize);

               if (!*(void **)ptr)
                 return 0;

               memset(*(void **)ptr, 0, count * subsize);

               _eet_freelist_array_add(context, *(void **)ptr);
          }
     }

   /* get all array elements */
   for (i = 0; i < count; i++)
     {
        void *dst = NULL;

        /* Advance to next chunk */
        NEXT_CHUNK((*p), (*size), (*echnk), ed);
        memset(echnk, 0, sizeof(Eet_Data_Chunk));

        eet_data_chunk_get(ed, echnk, *p, *size);
        if (!echnk->name || strcmp(echnk->name, name) != 0)
          {
             ERR("ERROR!");
             goto on_error;
          }

        if ((echnk->group_type != group_type)
            || ((echnk->type != type) && (echnk->type != EET_T_NULL)))
          {
             ERR("ERROR!");
             goto on_error;
          }

        if (ede)
          if ((ede->group_type != echnk->group_type)
              || ((echnk->type != ede->type) && (echnk->type != EET_T_NULL)))
            {
               ERR("ERROR!");
               goto on_error;
            }
        /* get the data */
        /* get the destination pointer */
        if (ede)
          {
             if (group_type == EET_G_ARRAY)
               dst = (char *)ptr + (subsize * i);
             else
               dst = *(char **)ptr + (subsize * i);
          }

        if (IS_POINTER_TYPE(echnk->type))
          {
             void *data_ret = NULL;

             POINTER_TYPE_DECODE(context,
                                 ed,
                                 edd,
                                 ede,
                                 echnk,
                                 echnk->type,
                                 &data_ret,
                                 p,
                                 size,
                                 on_error);
             if (dst)
               memcpy(dst, &data_ret, subsize);

             if (!edd)
               childs = eina_list_append(childs, data_ret);
          }
        else
          {
             STRUCT_TYPE_DECODE(dst,
                                context,
                                ed,
                                ede ? ede->subtype : NULL,
                                echnk->data,
                                echnk->size,
                                subsize,
                                on_error);

             if (!edd)
               childs = eina_list_append(childs, dst);
          }
     }

   if (!edd)
     {
        Eet_Node *parent = *((Eet_Node **)data);
        Eet_Node *array;

        if (group_type == EET_G_ARRAY)
          array = eet_node_array_new(name, count, childs);
        else
          array = eet_node_var_array_new(name, childs);

        EINA_SAFETY_ON_NULL_GOTO(array, on_error);

        eet_node_struct_append(parent, name, array);
     }

   return 1;

on_error:
   EINA_LIST_FREE(childs, tmp)
     eet_node_del(tmp);

   return 0;
}

static void
eet_data_put_union(Eet_Dictionary      *ed,
                   Eet_Data_Descriptor *edd EINA_UNUSED,
                   Eet_Data_Element    *ede,
                   Eet_Data_Stream     *ds,
                   void                *data_in)
{
   const char *union_type;
   int i;

   EET_ASSERT(!((ede->type != EET_T_UNKNOW) || (!ede->subtype)), return );

   union_type = ede->subtype->func.type_get(
       ((char *)data_in) + ede->count - ede->offset,
       NULL);

   if (!union_type)
     return;

   /* Search the structure of the union to encode. */
   for (i = 0; i < ede->subtype->elements.num; ++i)
     if (strcmp(ede->subtype->elements.set[i].name, union_type) == 0)
       {
          Eet_Data_Element *sede;
          void *data;
          int size;

          /* Yeah we found it ! */
          data = eet_data_put_type(ed, EET_T_STRING, &union_type, &size);
          if (data)
            eet_data_encode(ed,
                            ds,
                            data,
                            ede->name,
                            size,
                            ede->type,
                            ede->group_type,
                            EINA_TRUE);

          sede = &(ede->subtype->elements.set[i]);

          if (IS_SIMPLE_TYPE(sede->type))
            data = eet_data_put_type(ed, sede->type, data_in, &size);
          else
            data = _eet_data_descriptor_encode(ed,
                                             sede->subtype,
                                             data_in,
                                             &size);
          if (data)
            eet_data_encode(ed,
                            ds,
                            data,
                            ede->name,
                            size,
                            ede->type,
                            ede->group_type,
                            EINA_TRUE);

          break;
       }
}

static int
eet_data_get_union(Eet_Free_Context     *context,
                   const Eet_Dictionary *ed,
                   Eet_Data_Descriptor  *edd EINA_UNUSED,
                   Eet_Data_Element     *ede,
                   Eet_Data_Chunk       *echnk,
                   int                   type,
                   int                   group_type,
                   void                 *data,
                   char                **p,
                   int                  *size)
{
   const char *union_type;
   void *data_ret = NULL;
   int ret = 0;
   int i;

   /* Read type */
   ret = eet_data_get_type(ed,
                           EET_T_STRING,
                           echnk->data,
                           ((char *)echnk->data) + echnk->size,
                           &union_type);
   EINA_SAFETY_ON_TRUE_GOTO(ret <= 0, on_error);

   /* Advance to next chunk */
   NEXT_CHUNK((*p), (*size), (*echnk), ed);
   memset(echnk, 0, sizeof(Eet_Data_Chunk));

   /* Read value */
   eet_data_chunk_get(ed, echnk, *p, *size);
   EINA_SAFETY_ON_NULL_GOTO(echnk->name, on_error);

   if (ede)
     {
        EET_ASSERT(!(ede->group_type != group_type || ede->type != type),
                   ERR("ERROR!"); goto on_error);

        /* Search the structure of the union to decode */
        for (i = 0; i < ede->subtype->elements.num; ++i)
          if (strcmp(ede->subtype->elements.set[i].name, union_type) == 0)
            {
               Eet_Data_Element *sede;
               char *ut;

               /* Yeah we found it ! */
               sede = &(ede->subtype->elements.set[i]);

               if (IS_SIMPLE_TYPE(sede->type))
                 {
                    ret = eet_data_get_type(ed,
                          sede->type,
                          echnk->data,
                          ((char *)echnk->data) + echnk->size,
                          (char *)data);

                    if (ret <= 0)
                      return ret;
                 }
               else
                 {
                  EET_ASSERT(sede->subtype, ERR("ERROR!"); goto on_error);
                  data_ret = _eet_data_descriptor_decode(context,
                                                      ed,
                                                      sede->subtype,
                                                      echnk->data,
                                                      echnk->size,
                                                      data,
                                                      sede->subtype->size);
                  EINA_SAFETY_ON_NULL_GOTO(data_ret, on_error);
               }

               /* Set union type. */
               if ((!ed) || (!ede->subtype->func.str_direct_alloc))
                 {
                    ut = ede->subtype->func.str_alloc(union_type);
                    _eet_freelist_str_add(context, ut);
                 }
               else
                 {
                    ut = ede->subtype->func.str_direct_alloc(union_type);
                    _eet_freelist_direct_str_add(context, ut);
                 }

               ede->subtype->func.type_set(
                 ut,
                 ((char *)data) + ede->count -
                 ede->offset,
                 EINA_FALSE);

               break;
            }
     }
   else
     {
        /* FIXME: generate node structure. */
        _eet_data_descriptor_decode(context,
                                    ed, NULL,
                                    echnk->data, echnk->size,
                                    NULL, 0);
        ERR("ERROR!");
        goto on_error;
     }

   return 1;

on_error:
   return 0;
}

static void
eet_data_put_variant(Eet_Dictionary      *ed,
                     Eet_Data_Descriptor *edd EINA_UNUSED,
                     Eet_Data_Element    *ede,
                     Eet_Data_Stream     *ds,
                     void                *data_in)
{
   const char *union_type;
   void *data;
   Eina_Bool unknow = EINA_FALSE;
   int size;
   int i;

   EET_ASSERT(!((ede->type != EET_T_UNKNOW) || (!ede->subtype)), return );

   union_type = ede->subtype->func.type_get(
       ((char *)data_in) + ede->count - ede->offset,
       &unknow);

   if (!union_type && unknow == EINA_FALSE)
     return;

   if (unknow)
     {
        /* Handle opaque internal representation */
         Eet_Variant_Unknow *evu;

         data = eet_data_put_type(ed, EET_T_STRING, &union_type, &size);
         if (data)
           eet_data_encode(ed,
                           ds,
                           data,
                           ede->name,
                           size,
                           ede->type,
                           ede->group_type,
                           EINA_TRUE);

         evu = (Eet_Variant_Unknow *)data_in;
         if (evu && EINA_MAGIC_CHECK(evu, EET_MAGIC_VARIANT))
           eet_data_encode(ed,
                           ds,
                           evu->data,
                           ede->name,
                           evu->size,
                           ede->type,
                           ede->group_type,
                           EINA_FALSE);
     }
   else
     /* Search the structure of the union to encode. */
     for (i = 0; i < ede->subtype->elements.num; ++i)
       if (strcmp(ede->subtype->elements.set[i].name, union_type) == 0)
         {
            Eet_Data_Element *sede;

            /* Yeah we found it ! */
            data = eet_data_put_type(ed, EET_T_STRING, &union_type, &size);
            if (data)
              eet_data_encode(ed,
                              ds,
                              data,
                              ede->name,
                              size,
                              ede->type,
                              ede->group_type,
                              EINA_TRUE);

            sede = &(ede->subtype->elements.set[i]);

            if (sede->group_type != EET_G_UNKNOWN)
              {
                 Eet_Data_Stream *lds;

                 lds = eet_data_stream_new();
                 eet_group_codec[sede->group_type - 100].put(ed,
                                                             sede->subtype,
                                                             sede,
                                                             lds,
                                                             data_in);
                 if (lds->size != 0)
                   {
                      eet_data_encode(ed, ds, lds->data, ede->name, lds->pos,
                                      ede->type, ede->group_type,
                                      EINA_TRUE);

                      lds->data = NULL;
                      lds->size = 0;
                   }
                 else
                   eet_data_encode(ed, ds, NULL, ede->name, 0,
                                   EET_T_NULL, ede->group_type,
                                   EINA_TRUE);

                 eet_data_stream_free(lds);
              }
            else
              {
                 data = _eet_data_descriptor_encode(ed,
                                                    sede->subtype,
                                                    *(void **)data_in,
                                                    &size);
                 if (data)
                   eet_data_encode(ed,
                                   ds,
                                   data,
                                   ede->name,
                                   size,
                                   ede->type,
                                   ede->group_type,
                                   EINA_TRUE);
              }

            break;
         }
}

static int
eet_data_get_variant(Eet_Free_Context     *context,
                     const Eet_Dictionary *ed,
                     Eet_Data_Descriptor  *edd EINA_UNUSED,
                     Eet_Data_Element     *ede,
                     Eet_Data_Chunk       *echnk,
                     int                   type EINA_UNUSED,
                     int                   group_type EINA_UNUSED,
                     void                 *data,
                     char                **p,
                     int                  *size)
{
   const char *union_type;
   void *data_ret = NULL;
   int ret = 0;
   int i;

   /* Read type */
   ret = eet_data_get_type(ed,
                           EET_T_STRING,
                           echnk->data,
                           ((char *)echnk->data) + echnk->size,
                           &union_type);
   EINA_SAFETY_ON_TRUE_GOTO(ret <= 0, on_error);

   /* Advance to next chunk */
   NEXT_CHUNK((*p), (*size), (*echnk), ed);
   memset(echnk, 0, sizeof(Eet_Data_Chunk));

   /* Read value */
   eet_data_chunk_get(ed, echnk, *p, *size);
   EINA_SAFETY_ON_NULL_GOTO(echnk->name, on_error);

   if (ede)
     {
        char *ut;

        EET_ASSERT(ede->subtype, ERR("ERROR!"); goto on_error);

        if ((!ed) || (!ede->subtype->func.str_direct_alloc))
          {
             ut = ede->subtype->func.str_alloc(union_type);
             _eet_freelist_str_add(context, ut);
          }
        else
          {
             ut = ede->subtype->func.str_direct_alloc(union_type);
             _eet_freelist_direct_str_add(context, ut);
          }

        /* Search the structure of the union to decode */
        for (i = 0; i < ede->subtype->elements.num; ++i)
          if (strcmp(ede->subtype->elements.set[i].name, union_type) == 0)
            {
               Eet_Data_Element *sede;

               /* Yeah we found it ! */
               sede = &(ede->subtype->elements.set[i]);

               if (sede->group_type != EET_G_UNKNOWN)
                 {
                    Eet_Data_Chunk chnk;
                    char *p2;
                    int size2;

                    p2 = echnk->data;
                    size2 = echnk->size;

     /* Didn't find a proper way to provide this
        without duplicating code */
                    while (size2 > 0)
                      {
                         memset(&chnk, 0, sizeof(Eet_Data_Chunk));
                         eet_data_chunk_get(ed, &chnk, p2, size2);

                         EINA_SAFETY_ON_NULL_GOTO(chnk.name, on_error);

                         ret = eet_group_codec[sede->group_type - 100].get
                             (context, ed, sede->subtype, sede, &chnk, sede->type,
                             sede->group_type, data, &p2, &size2);

                         EINA_SAFETY_ON_TRUE_GOTO(ret <= 0, on_error);

/* advance to next chunk */
                         NEXT_CHUNK(p2, size2, chnk, ed);
                      }

     /* Put garbage so that we will not put eet_variant_unknow in it */
                    data_ret = (void *)data;

     /* Set variant type. */
                    ede->subtype->func.type_set
                      (ut, ((char *)data) + ede->count - ede->offset,
                      EINA_FALSE);
                    break;
                 }

               data_ret = _eet_data_descriptor_decode(context,
                                                      ed,
                                                      sede->subtype,
                                                      echnk->data,
                                                      echnk->size,
                                                      NULL, 0);
               EINA_SAFETY_ON_TRUE_GOTO(!data_ret, on_error);

               /* And point to the variant data. */
               *(void **)data = data_ret;

               /* Set variant type. */
               ede->subtype->func.type_set
                 (ut, ((char *)data) + ede->count - ede->offset, EINA_FALSE);
               break;
            }

        if (!data_ret)
          {
             Eet_Variant_Unknow *evu;

             evu = calloc(1, sizeof (Eet_Variant_Unknow) + echnk->size - 1);
             EINA_SAFETY_ON_NULL_GOTO(evu, on_error);

             evu->size = echnk->size;
             memcpy(evu->data, echnk->data, evu->size);
             EINA_MAGIC_SET(evu, EET_MAGIC_VARIANT);

             /* And point to the opaque internal data scructure */
             *(void **)data = evu;

             /* Set variant type. */
             ede->subtype->func.type_set
               (ut, ((char *)data) + ede->count - ede->offset, EINA_TRUE);
          }
     }
   else
     {
        /* FIXME: dump node structure. */
        _eet_data_descriptor_decode(context,
                                    ed, NULL,
                                    echnk->data, echnk->size,
                                    NULL, 0);
        ERR("ERROR!");
        goto on_error;
     }

   return 1;

on_error:
   return 0;
}

static Eet_Node *
eet_data_node_simple_type(int         type,
                          const char *name,
                          void       *dd)
{
#ifdef EET_T_TYPE
# undef EET_T_TYPE
#endif /* ifdef EET_T_TYPE */

#define EET_T_TYPE(Eet_Type, Eet_Node_Type, Type)                 \
case Eet_Type:                                                    \
  return eet_node_ ## Eet_Node_Type ## _new(name, *((Type *)dd)); \

   switch (type)
     {
        EET_T_TYPE(EET_T_CHAR, char, char);
        EET_T_TYPE(EET_T_SHORT, short, short);
        EET_T_TYPE(EET_T_INT, int, int);
        EET_T_TYPE(EET_T_LONG_LONG, long_long, long long);
        EET_T_TYPE(EET_T_FLOAT, float, float);
        EET_T_TYPE(EET_T_DOUBLE, double, double);
        EET_T_TYPE(EET_T_UCHAR, unsigned_char, unsigned char);
        EET_T_TYPE(EET_T_USHORT, unsigned_short, unsigned short);
        EET_T_TYPE(EET_T_UINT, unsigned_int, unsigned int);
        EET_T_TYPE(EET_T_ULONG_LONG, unsigned_long_long, unsigned long long);
        EET_T_TYPE(EET_T_STRING, string, char *);
        EET_T_TYPE(EET_T_INLINED_STRING, inlined_string, char *);

      case EET_T_NULL:
        return eet_node_null_new(name);

      case EET_T_VALUE:
         return eet_node_null_new(name);

      default:
        ERR("Unknown type passed to eet_data_node_simple_type");
        return NULL;
     }
}

static int
eet_data_get_unknown(Eet_Free_Context     *context,
                     const Eet_Dictionary *ed,
                     Eet_Data_Descriptor  *edd,
                     Eet_Data_Element     *ede,
                     Eet_Data_Chunk       *echnk,
                     int                   type,
                     int                   group_type EINA_UNUSED,
                     void                 *data,
                     char                **p EINA_UNUSED,
                     int                  *size EINA_UNUSED)
{
   int ret;
   void *data_ret;

   if (IS_SIMPLE_TYPE(type))
     {
        unsigned long long dd[128];

        ret = eet_data_get_type(ed,
                                type,
                                echnk->data,
                                ((char *)echnk->data) + echnk->size,
                                edd ? (char *)data : (char *)dd);
        if (ret <= 0)
          return ret;

        if (!edd)
          {
             Eet_Node **parent = data;
             Eet_Node *node;

             node = eet_data_node_simple_type(type, echnk->name, dd);

             if (*parent)
               eet_node_struct_append(*parent, echnk->name, node);
             else
               *parent = node;
          }
        else
          {
             if (type == EET_T_STRING)
               {
                  char **str = data;

                  if (*str)
                    {
                       if ((!ed) || (!edd->func.str_direct_alloc))
                         {
                            *str = edd->func.str_alloc(*str);
                            _eet_freelist_str_add(context, *str);
                         }
                       else
                         {
                            *str = edd->func.str_direct_alloc(*str);
                            _eet_freelist_direct_str_add(context, *str);
                         }
                    }
               }
             else if (edd && type == EET_T_INLINED_STRING)
               {
                  char **str = data;

                  if (*str)
                    {
                       *str = edd->func.str_alloc(*str);
                       _eet_freelist_str_add(context, *str);
                    }
               }
          }
     }
   else
     {
        Eet_Data_Descriptor *subtype;

        subtype = ede ? ede->subtype : NULL;

        if (subtype || !edd)
          {
             Eet_Node **parent = data;
             void **ptr;

             data_ret = _eet_data_descriptor_decode(context,
                                                    ed,
                                                    subtype,
                                                    echnk->data,
                                                    echnk->size,
                                                    NULL, 0);
             if (!data_ret)
               return 0;

             if (edd)
               {
                  if (subtype && ede->group_type == EET_G_UNKNOWN_NESTED)
                    {
                       memcpy(data, data_ret, subtype->size);
                       free(data_ret);
                    }
                  else 
                    {
                       ptr = data;
                       *ptr = (void *)data_ret;
                       _eet_freelist_add(context, data_ret);
                    }
               }
             else
               {
                  Eet_Node *node = data_ret;

                  if (*parent)
                    {
                       node = eet_node_struct_child_new(echnk->name, node);
                       eet_node_struct_append(*parent, echnk->name, node);
                    }
                  else
                    *parent = node;
               }
          }
     }

   return 1;
}

static void
eet_data_put_array(Eet_Dictionary      *ed,
                   Eet_Data_Descriptor *edd EINA_UNUSED,
                   Eet_Data_Element    *ede,
                   Eet_Data_Stream     *ds,
                   void                *data_in)
{
   void *data;
   int offset = 0;
   int subsize;
   int count;
   int size;
   int j;

   EET_ASSERT(!((ede->type > EET_T_UNKNOW) && (ede->type < EET_T_STRING)),
              return );

   if (ede->group_type == EET_G_ARRAY)
     count = ede->counter_offset;
   else
     count = *(int *)(((char *)data_in) + ede->count - ede->offset);

   if (count <= 0)
     return;  /* Store number of elements */

   data = eet_data_put_type(ed, EET_T_INT, &count, &size);
   if (data)
     eet_data_encode(ed, ds, data, ede->name, size, ede->type, ede->group_type, EINA_TRUE);

   if (IS_POINTER_TYPE(ede->type))
     subsize = eet_basic_codec[ede->type - 1].size;
   else
     subsize = ede->subtype->size;

   for (j = 0; j < count; j++)
     {
        void *d;
        int pos = ds->pos;

        if (ede->group_type == EET_G_ARRAY)
          d = ((char *)data_in) + offset;
        else
          d = *(((char **)data_in)) + offset;

        if (IS_POINTER_TYPE(ede->type))
          {
             if (*(char **)d)
               eet_data_put_unknown(ed, NULL, ede, ds, d);
          }
        else
          {
             data = _eet_data_descriptor_encode(ed, ede->subtype, d, &size);
             if (data)
               eet_data_encode(ed,
                               ds,
                               data,
                               ede->name,
                               size,
                               ede->type,
                               ede->group_type,
                               EINA_TRUE);
          }

        if (pos == ds->pos)
          /* Add a NULL element just to have the correct array layout. */
          eet_data_encode(ed,
                          ds,
                          NULL,
                          ede->name,
                          0,
                          EET_T_NULL,
                          ede->group_type,
                          EINA_TRUE);

        offset += subsize;
     }
}

static void
eet_data_put_unknown(Eet_Dictionary      *ed,
                     Eet_Data_Descriptor *edd EINA_UNUSED,
                     Eet_Data_Element    *ede,
                     Eet_Data_Stream     *ds,
                     void                *data_in)
{
   void *data = NULL;
   int size;

   if (IS_SIMPLE_TYPE(ede->type))
     data = eet_data_put_type(ed, ede->type, data_in, &size);
   else if (ede->subtype)
     {
        if (ede->group_type == EET_G_UNKNOWN_NESTED)
          data = _eet_data_descriptor_encode(ed,
                                             ede->subtype,
                                             data_in,
                                             &size);
        else if (*((char **)data_in))
          data = _eet_data_descriptor_encode(ed,
                                             ede->subtype,
                                             *((char **)((char *)(data_in))),
                                             &size);
     }

   if (data)
     eet_data_encode(ed,
                     ds,
                     data,
                     ede->name,
                     size,
                     ede->type,
                     ede->group_type,
                     EINA_TRUE);
}

static void
eet_data_put_list(Eet_Dictionary      *ed,
                  Eet_Data_Descriptor *edd,
                  Eet_Data_Element    *ede,
                  Eet_Data_Stream     *ds,
                  void                *data_in)
{
   void *data;
   void *l;
   int size;

   EET_ASSERT(!(((ede->type > EET_T_UNKNOW) && (ede->type < EET_T_STRING))
                || ((ede->type > EET_T_NULL) && (ede->type < EET_T_LAST))),
              return );

   l = *((void **)(((char *)data_in)));
   for (; l; l = edd->func.list_next(l))
     {
        if (IS_POINTER_TYPE(ede->type))
          {
             const void *str = edd->func.list_data(l);
             eet_data_put_unknown(ed, NULL, ede, ds, &str);
          }
        else
          {
             data = _eet_data_descriptor_encode(ed,
                                                ede->subtype,
                                                edd->func.list_data(l),
                                                &size);
             if (data)
               eet_data_encode(ed,
                               ds,
                               data,
                               ede->name,
                               size,
                               ede->type,
                               ede->group_type,
                               EINA_TRUE);
          }
     }
}

static void
eet_data_put_hash(Eet_Dictionary      *ed,
                  Eet_Data_Descriptor *edd,
                  Eet_Data_Element    *ede,
                  Eet_Data_Stream     *ds,
                  void                *data_in)
{
   Eet_Data_Encode_Hash_Info fdata;
   void *l;

   l = *((void **)(((char *)data_in)));
   fdata.ds = ds;
   fdata.ede = ede;
   fdata.ed = ed;
   edd->func.hash_foreach(l, eet_data_descriptor_encode_hash_cb, &fdata);
}

EAPI int
eet_data_dump_cipher(Eet_File         *ef,
                     const char       *name,
                     const char       *cipher_key,
                     Eet_Dump_Callback dumpfunc,
                     void             *dumpdata)
{
   const Eet_Dictionary *ed = NULL;
   const void *data = NULL;
   Eet_Node *result;
   Eet_Free_Context context;
   int required_free = 0;
   int size;

   ed = eet_dictionary_get(ef);

   if (!cipher_key)
     data = eet_read_direct(ef, name, &size);

   if (!data)
     {
        required_free = 1;
        data = eet_read_cipher(ef, name, &size, cipher_key);
        if (!data)
          return 0;
     }

   eet_free_context_init(&context);
   result = _eet_data_descriptor_decode(&context, ed, NULL, data, size, NULL, 0);
   eet_free_context_shutdown(&context);

   eet_node_dump(result, 0, dumpfunc, dumpdata);

   eet_node_del(result);

   if (required_free)
     free((void *)data);

   return result ? 1 : 0;
}

EAPI int
eet_data_dump(Eet_File         *ef,
              const char       *name,
              Eet_Dump_Callback dumpfunc,
              void             *dumpdata)
{
   return eet_data_dump_cipher(ef, name, NULL, dumpfunc, dumpdata);
}

EAPI int
eet_data_text_dump_cipher(const void       *data_in,
                          const char       *cipher_key,
                          int               size_in,
                          Eet_Dump_Callback dumpfunc,
                          void             *dumpdata)
{
   void *ret = NULL;
   Eet_Node *result;
   Eet_Free_Context context;
   unsigned int ret_len = 0;

   if (!data_in)
     return 0;

   if (cipher_key)
     {
        if (eet_decipher(data_in, size_in, cipher_key,
                         strlen(cipher_key), &ret, &ret_len))
          {
             if (ret)
               free(ret);

             return 0;
          }
     }
   else
     {
        ret = (void *)data_in;
        ret_len = size_in;
     }

   eet_free_context_init(&context);
   result = _eet_data_descriptor_decode(&context, NULL, NULL, ret, ret_len, NULL, 0);
   eet_free_context_shutdown(&context);

   eet_node_dump(result, 0, dumpfunc, dumpdata);

   eet_node_del(result);
   if (cipher_key)
     free(ret);

   return result ? 1 : 0;
}

EAPI int
eet_data_text_dump(const void       *data_in,
                   int               size_in,
                   Eet_Dump_Callback dumpfunc,
                   void             *dumpdata)
{
   return eet_data_text_dump_cipher(data_in, NULL, size_in, dumpfunc, dumpdata);
}

EAPI void *
eet_data_text_undump_cipher(const char *text,
                            const char *cipher_key,
                            int         textlen,
                            int        *size_ret)
{
   void *ret = NULL;

   ret = _eet_data_dump_parse(NULL, size_ret, text, textlen);
   if (ret && cipher_key)
     {
        void *ciphered = NULL;
        unsigned int ciphered_len;

        if (eet_cipher(ret, *size_ret, cipher_key,
                       strlen(cipher_key), &ciphered, &ciphered_len))
          {
             if (ciphered)
               free(ciphered);

             size_ret = 0;
             free(ret);
             return NULL;
          }

        free(ret);
        *size_ret = ciphered_len;
        ret = ciphered;
     }

   return ret;
}

EAPI void *
eet_data_text_undump(const char *text,
                     int         textlen,
                     int        *size_ret)
{
   return eet_data_text_undump_cipher(text, NULL, textlen, size_ret);
}

EAPI int
eet_data_undump_cipher(Eet_File   *ef,
                       const char *name,
                       const char *cipher_key,
                       const char *text,
                       int         textlen,
                       int         comp)
{
   Eet_Dictionary *ed;
   void *data_enc;
   int size;
   int val;

   ed = eet_dictionary_get(ef);

   data_enc = _eet_data_dump_parse(ed, &size, text, textlen);
   if (!data_enc)
     return 0;

   val = eet_write_cipher(ef, name, data_enc, size, comp, cipher_key);
   free(data_enc);
   return val;
}

EAPI int
eet_data_undump(Eet_File   *ef,
                const char *name,
                const char *text,
                int         textlen,
                int         comp)
{
   return eet_data_undump_cipher(ef, name, NULL, text, textlen, comp);
}

EAPI void *
eet_data_descriptor_decode_cipher(Eet_Data_Descriptor *edd,
                                  const void          *data_in,
                                  const char          *cipher_key,
                                  int                  size_in)
{
   void *deciphered = (void *)data_in;
   void *ret;
   Eet_Free_Context context;
   unsigned int deciphered_len = size_in;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);

   if (cipher_key && data_in)
     if (eet_decipher(data_in, size_in, cipher_key,
                      strlen(cipher_key), &deciphered, &deciphered_len))
       {
          if (deciphered)
            free(deciphered);

          return NULL;
       }

   eet_free_context_init(&context);
   ret = _eet_data_descriptor_decode(&context,
                                     NULL,
                                     edd,
                                     deciphered,
                                     deciphered_len,
                                     NULL, 0);
   eet_free_context_shutdown(&context);

   if (data_in != deciphered)
     free(deciphered);

   return ret;
}

EAPI void *
eet_data_descriptor_decode(Eet_Data_Descriptor *edd,
                           const void          *data_in,
                           int                  size_in)
{
   return eet_data_descriptor_decode_cipher(edd, data_in, NULL, size_in);
}

EAPI Eet_Node *
eet_data_node_decode_cipher(const void *data_in,
                            const char *cipher_key,
                            int         size_in)
{
   void *deciphered = (void *)data_in;
   Eet_Node *ret;
   Eet_Free_Context context;
   unsigned int deciphered_len = size_in;

   if (cipher_key && data_in)
     if (eet_decipher(data_in, size_in, cipher_key,
                      strlen(cipher_key), &deciphered, &deciphered_len))
       {
          if (deciphered)
            free(deciphered);

          return NULL;
       }

   eet_free_context_init(&context);
   ret = _eet_data_descriptor_decode(&context,
                                     NULL,
                                     NULL,
                                     deciphered,
                                     deciphered_len,
                                     NULL, 0);
   eet_free_context_shutdown(&context);

   if (data_in != deciphered)
     free(deciphered);

   return ret;
}

static void *
_eet_data_descriptor_encode(Eet_Dictionary      *ed,
                            Eet_Data_Descriptor *edd,
                            const void          *data_in,
                            int                 *size_ret)
{
   Eet_Data_Stream *ds;
   Eet_Data_Chunk *chnk;
   void *cdata;
   int csize;
   int i;

   if (_eet_data_words_bigendian == -1)
     {
        unsigned long int v;

        v = htonl(0x12345678);
        if (v == 0x12345678)
          _eet_data_words_bigendian = 1;
        else
          _eet_data_words_bigendian = 0;
     }

   ds = eet_data_stream_new();
   for (i = 0; i < edd->elements.num; i++)
     {
        Eet_Data_Element *ede;

        ede = &(edd->elements.set[i]);
        eet_group_codec[ede->group_type - 100].put(
          ed,
          edd,
          ede,
          ds,
          ((char *)data_in) +
          ede->offset);
     }
   chnk = eet_data_chunk_new(ds->data,
                             ds->pos,
                             edd->name,
                             EET_T_UNKNOW,
                             EET_G_UNKNOWN);
   ds->data = NULL;
   ds->size = 0;
   eet_data_stream_free(ds);

   ds = eet_data_stream_new();
   eet_data_chunk_put(ed, chnk, ds);
   cdata = ds->data;
   csize = ds->pos;

   ds->data = NULL;
   ds->size = 0;
   eet_data_stream_free(ds);
   *size_ret = csize;

   free(chnk->data);
   eet_data_chunk_free(chnk);

   return cdata;
}

EAPI int
eet_data_node_write_cipher(Eet_File   *ef,
                           const char *name,
                           const char *cipher_key,
                           Eet_Node   *node,
                           int         comp)
{
   Eet_Dictionary *ed;
   void *data_enc;
   int size;
   int val;

   ed = eet_dictionary_get(ef);

   data_enc = _eet_data_dump_encode(EET_G_UNKNOWN, ed, node, &size);
   if (!data_enc)
     return 0;

   val = eet_write_cipher(ef, name, data_enc, size, comp, cipher_key);
   free(data_enc);
   return val;
}

EAPI void *
eet_data_node_encode_cipher(Eet_Node   *node,
                            const char *cipher_key,
                            int        *size_ret)
{
   void *ret = NULL;
   void *ciphered = NULL;
   unsigned int ciphered_len = 0;
   int size;

   ret = _eet_data_dump_encode(EET_G_UNKNOWN, NULL, node, &size);
   if (cipher_key && ret)
     {
        if (eet_cipher(ret, size, cipher_key,
                       strlen(cipher_key), &ciphered, &ciphered_len))
          {
             if (ciphered)
               free(ciphered);

             if (size_ret)
               *size_ret = 0;

             free(ret);
             return NULL;
          }

        free(ret);
        size = (int)ciphered_len;
        ret = ciphered;
     }

   if (size_ret)
     *size_ret = size;

   return ret;
}

EAPI void *
eet_data_descriptor_encode_cipher(Eet_Data_Descriptor *edd,
                                  const void          *data_in,
                                  const char          *cipher_key,
                                  int                 *size_ret)
{
   void *ret = NULL;
   void *ciphered = NULL;
   unsigned int ciphered_len = 0;
   int size;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data_in, NULL);

   ret = _eet_data_descriptor_encode(NULL, edd, data_in, &size);
   if (cipher_key && ret)
     {
        if (eet_cipher(ret, size, cipher_key,
                       strlen(cipher_key), &ciphered, &ciphered_len))
          {
             if (ciphered)
               free(ciphered);

             if (size_ret)
               *size_ret = 0;

             free(ret);
             return NULL;
          }

        free(ret);
        size = ciphered_len;
        ret = ciphered;
     }

   if (size_ret)
     *size_ret = size;

   return ret;
}

EAPI void *
eet_data_descriptor_encode(Eet_Data_Descriptor *edd,
                           const void          *data_in,
                           int                 *size_ret)
{
   return eet_data_descriptor_encode_cipher(edd, data_in, NULL, size_ret);
}

EAPI void *
eet_data_xattr_cipher_get(const char          *filename,
                          const char          *attribute,
                          Eet_Data_Descriptor *edd,
                          const char          *cipher_key)
{
   void *blob;
   void *ret;
   ssize_t size;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);

   blob = eina_xattr_get(filename, attribute, &size);
   if (!blob) return NULL;

   ret = eet_data_descriptor_decode_cipher(edd, blob, cipher_key, size);
   free(blob);

   return ret;
}

EAPI Eina_Bool
eet_data_xattr_cipher_set(const char          *filename,
                          const char          *attribute,
                          Eet_Data_Descriptor *edd,
                          const char          *cipher_key,
                          const void          *data,
                          Eina_Xattr_Flags     flags)
{
   void *blob;
   int size;
   Eina_Bool ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, EINA_FALSE);

   blob = eet_data_descriptor_encode_cipher(edd, data, cipher_key, &size);
   if (!blob) return EINA_FALSE;

   ret = eina_xattr_set(filename, attribute, blob, size, flags);
   free(blob);

   return ret;
}
