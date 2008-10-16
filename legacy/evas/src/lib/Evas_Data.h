#ifndef _EVAS_DATA_H
#define _EVAS_DATA_H

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

#include <Eina.h>

/**
 * @file
 * @brief These routines are used for Evas data types.
 */

typedef unsigned char Evas_Bool;

#define Evas_List Eina_List
#define Evas_Object_List Eina_Inlist

typedef struct _Evas_Array_Hash  Evas_Array_Hash;
typedef struct _Evas_Hash        Evas_Hash;              /**< A Hash table handle */

struct _Evas_Hash
{
   int               population;
   Evas_Object_List *buckets[256];
};

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
   EAPI Evas_Hash  *evas_hash_add                   (Evas_Hash *hash, const char *key, const void *data);
   EAPI Evas_Hash  *evas_hash_direct_add            (Evas_Hash *hash, const char *key, const void *data);
   EAPI Evas_Hash  *evas_hash_del                   (Evas_Hash *hash, const char *key, const void *data);
   EAPI void       *evas_hash_find                  (const Evas_Hash *hash, const char *key);
   EAPI void       *evas_hash_modify                (Evas_Hash *hash, const char *key, const void *data);
   EAPI int         evas_hash_size                  (const Evas_Hash *hash);
   EAPI void        evas_hash_free                  (Evas_Hash *hash);
   EAPI void        evas_hash_foreach               (const Evas_Hash *hash, Evas_Bool (*func) (const Evas_Hash *hash, const char *key, void *data, void *fdata), const void *fdata);
   EAPI int         evas_hash_alloc_error           (void);


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
   * Evas Object List functions
   */
  #define evas_object_list_append eina_inlist_append
  #define evas_object_list_prepend eina_inlist_prepend
  #define evas_object_list_append_relative eina_inlist_append_relative
  #define evas_object_list_prepend_relative eina_inlist_prepend_relative
  #define evas_object_list_remove eina_inlist_remove
  #define evas_object_list_find eina_inlist_find

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
