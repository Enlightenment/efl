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

/**
 * @file
 * @brief These routines are used for Evas data types.
 */

typedef unsigned char Evas_Bool;

typedef struct _Evas_Array       Evas_Array;             /**< A generic vector */
typedef struct _Evas_Array_Hash  Evas_Array_Hash;
typedef struct _Evas_Hash        Evas_Hash;              /**< A Hash table handle */
typedef struct _Evas_List        Evas_List;              /**< A generic linked list node handle */
typedef struct _Evas_Object_List Evas_Object_List;


struct _Evas_Array /** An array of data */
{
   void		**data;   /**< Pointer to a vector of pointer to payload */
   unsigned int   total;  /**< Total number of slot in the vector */
   unsigned int   count;  /**< Number of activ slot in the vector */
   unsigned int	  step;   /**< How much must we grow the vector When it is full */
};

struct _Evas_Hash
{
   int               population;
   Evas_Object_List *buckets[256];
};

struct _Evas_List                             /** A linked list node */
{
   void      *data;                           /**< Pointer to list element payload */
   Evas_List *next;                           /**< Next member in the list */
   Evas_List *prev;                           /**< Previous member in the list */
   struct _Evas_List_Accounting *accounting;  /**< Private list accounting info - don't touch */
};

struct _Evas_Object_List
{
   Evas_Object_List  *next, *prev;
   Evas_Object_List  *last;
};


#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Evas Array functions
   */

   EAPI Evas_Array *evas_array_new                  (unsigned int step);
   EAPI void        evas_array_setup                (Evas_Array *array, unsigned int step);
   EAPI void        evas_array_free                 (Evas_Array *array);
   EAPI void        evas_array_append               (Evas_Array *array, void *data);
   EAPI void       *evas_array_get                  (Evas_Array *array, unsigned int index);
   EAPI void        evas_array_clean                (Evas_Array *array);
   EAPI void        evas_array_flush                (Evas_Array *array);
   EAPI void        evas_array_remove               (Evas_Array *array, Evas_Bool (*keep)(void *data, void *gdata), void *gdata);


  /*
   * Evas Array functions
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

   EAPI Evas_List  *evas_list_append                (Evas_List *list, const void *data);
   EAPI Evas_List  *evas_list_prepend               (Evas_List *list, const void *data);
   EAPI Evas_List  *evas_list_append_relative       (Evas_List *list, const void *data, const void *relative);
   EAPI Evas_List  *evas_list_append_relative_list  (Evas_List *list, const void *data, Evas_List *relative);
   EAPI Evas_List  *evas_list_prepend_relative      (Evas_List *list, const void *data, const void *relative);
   EAPI Evas_List  *evas_list_prepend_relative_list (Evas_List *list, const void *data, Evas_List *relative);
   EAPI Evas_List  *evas_list_remove                (Evas_List *list, const void *data);
   EAPI Evas_List  *evas_list_remove_list           (Evas_List *list, Evas_List *remove_list);
   EAPI Evas_List  *evas_list_promote_list          (Evas_List *list, Evas_List *move_list);
   EAPI void       *evas_list_find                  (const Evas_List *list, const void *data);
   EAPI Evas_List  *evas_list_find_list             (const Evas_List *list, const void *data);
   EAPI Evas_List  *evas_list_free                  (Evas_List *list);
   EAPI Evas_List  *evas_list_last                  (const Evas_List *list);
   EAPI Evas_List  *evas_list_next                  (const Evas_List *list);
   EAPI Evas_List  *evas_list_prev                  (const Evas_List *list);
   EAPI void       *evas_list_data                  (const Evas_List *list);
   EAPI int         evas_list_count                 (const Evas_List *list);
   EAPI void       *evas_list_nth                   (const Evas_List *list, int n);
   EAPI Evas_List  *evas_list_nth_list              (const Evas_List *list, int n);
   EAPI Evas_List  *evas_list_reverse               (Evas_List *list);
   EAPI Evas_List  *evas_list_sort                  (Evas_List *list, int size, int(*func)(void*,void*));
   EAPI int         evas_list_alloc_error           (void);


  /*
   * Evas Stringshare functions
   */

   EAPI void        evas_stringshare_init           (void);           /* not implemented */
   EAPI void        evas_stringshare_shutdown       (void);           /* not implemented */
   EAPI const char *evas_stringshare_add            (const char *str);
   EAPI void        evas_stringshare_del            (const char *str);

#ifdef __cplusplus
}
#endif

#endif /* _EVAS_DATA_H */
