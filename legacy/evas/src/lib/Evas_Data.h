#ifndef _EVAS_DATA_H
#define _EVAS_DATA_H

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

/**
 * @file
 * @brief These routines are used for Evas data types.
 */

typedef unsigned char Evas_Bool;

#define Evas_List Eina_List
#define Evas_Hash Eina_Hash

typedef struct _Evas_Array_Hash  Evas_Array_Hash;

#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Evas Array Hash functions
   */

   EAPI Evas_Array_Hash *evas_array_hash_new        (void);
   EAPI void             evas_array_hash_free       (Evas_Array_Hash *hash);
   EAPI void             evas_array_hash_add        (Evas_Array_Hash *hash, int key, int data);
   EAPI int              evas_array_hash_search     (Evas_Array_Hash *hash, int key);


  /*
   * Evas Hash functions
   */

   /* FIXME: add:
    * api to add find, del members by data, size not just string and also
    * provide hash generation functions settable by the app
    *
    * do we really need this? hmmm - let me think... there may be a better way
    */
  static inline Eina_Hash *evas_hash_add(Eina_Hash *hash, const char *key, const void *data)
  {
     if (!hash) hash = eina_hash_string_superfast_new(NULL);
     if (!hash) return NULL;

     eina_hash_add(hash, key, data);
     return hash;
  }

  static inline Eina_Hash *evas_hash_direct_add(Eina_Hash *hash, const char *key, const void *data)
  {
     if (!hash) hash = eina_hash_string_superfast_new(NULL);
     if (!hash) return NULL;

     eina_hash_direct_add(hash, key, data);
     return hash;
  }

  static inline Eina_Hash *evas_hash_del(Eina_Hash *hash, const char *key, const void *data)
  {
     if (!hash) return NULL;
     eina_hash_del(hash, key, data);

     if (eina_hash_population(hash) == 0)
       {
	  eina_hash_free(hash);
	  return NULL;
       }

     return hash;
  }
  static inline int evas_hash_size(const Eina_Hash *hash)
  {
     if (!hash) return 0;
     return 255;
  }

  static inline void *evas_hash_find(const Eina_Hash *hash, const void *key)
  {
     if (!hash) return NULL;
     return eina_hash_find(hash, key);
  }

  static inline void *evas_hash_modify(Eina_Hash *hash, const void *key, const void *data)
  {
     if (!hash) return NULL;
     return eina_hash_modify(hash, key, data);
  }
  static inline void evas_hash_free(Eina_Hash *hash)
  {
     if (!hash) return NULL;
     return eina_hash_free(hash);
  }

  static inline void evas_hash_foreach(const Eina_Hash *hash, Eina_Hash_Foreach cb, const void *fdata)
  {
     if (!hash) return;
     eina_hash_foreach(hash, cb, fdata);
  }

  #define evas_hash_alloc_error eina_error_get

  /*
   * Evas List functions
   */

  #define evas_list_append eina_list_append
  #define evas_list_prepend eina_list_prepend
  #define evas_list_append_relative eina_list_append_relative
  #define evas_list_append_relative_list eina_list_append_relative_list
  #define evas_list_prepend_relative eina_list_prepend_relative
  #define evas_list_prepend_relative_list eina_list_prepend_relative_list
  #define evas_list_remove eina_list_remove
  #define evas_list_remove_list eina_list_remove_list
  #define evas_list_promote_list eina_list_promote_list
  #define evas_list_find eina_list_data_find
  #define evas_list_find_list eina_list_data_find_list
  #define evas_list_free eina_list_free
  #define evas_list_last eina_list_last
  #define evas_list_next eina_list_next
  #define evas_list_prev eina_list_prev
  #define evas_list_data eina_list_data_get
  #define evas_list_count eina_list_count
  #define evas_list_nth eina_list_nth
  #define evas_list_nth_list eina_list_nth_list
  #define evas_list_reverse eina_list_reverse
  #define evas_list_sort eina_list_sort
  #define evas_list_alloc_error eina_error_get

  /*
   * Evas Stringshare functions
   */

  #define evas_stringshare_init eina_stringshare_init
  #define evas_stringshare_shutdown eina_stringshare_shutdown
  #define evas_stringshare_add eina_stringshare_add
  #define evas_stringshare_del eina_stringshare_del

#ifdef __cplusplus
}
#endif

#endif /* _EVAS_DATA_H */
