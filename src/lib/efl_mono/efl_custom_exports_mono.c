#include "Eo.h"
#include "Eina.h"

#include <stdlib.h>
#include <string.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
#  define EAPI __declspec(dllexport)
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

EAPI void *efl_mono_native_alloc(unsigned int size)
{
   return malloc(size);
}

EAPI void efl_mono_native_free(void *ptr)
{
   free(ptr);
}

EAPI void *efl_mono_native_alloc_copy(const void *val, unsigned int size)
{
    if (!val) return NULL;
    void *r = malloc(size);
    memcpy(r, val, size);
    return r;
}

EAPI const char *efl_mono_native_strdup(const char *str)
{
    if (!str) return NULL;
    return strdup(str);
}

EAPI int efl_mono_native_ptr_compare(const void *ptr1, const void *ptr2)
{
    uintptr_t addr1 = (uintptr_t)ptr1;
    uintptr_t addr2 = (uintptr_t)ptr2;
    return (addr1 > addr2) - (addr1 < addr2);
}

EAPI Eina_Compare_Cb efl_mono_native_ptr_compare_addr_get()
{
    return efl_mono_native_ptr_compare;
}

EAPI Eina_Compare_Cb efl_mono_native_str_compare_addr_get()
{
    return (Eina_Compare_Cb)strcmp;
}

// Array //

EAPI void eina_array_free_generic_custom_export_mono(Eina_Array *array) EINA_ARG_NONNULL(1)
{
   unsigned int i;
   void *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(array, i, ele, it)
     free(ele);

   eina_array_clean(array);
}

EAPI void eina_array_free_string_custom_export_mono(Eina_Array *array) EINA_ARG_NONNULL(1)
{
   eina_array_free_generic_custom_export_mono(array);
}

EAPI void eina_array_free_obj_custom_export_mono(Eina_Array *array) EINA_ARG_NONNULL(1)
{
   unsigned int i;
   Eo *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(array, i, ele, it)
     efl_unref(ele);

   eina_array_clean(array);
}

EAPI void eina_array_clean_custom_export_mono(Eina_Array *array) EINA_ARG_NONNULL(1)
{
   eina_array_clean(array);
}

EAPI Eina_Bool eina_array_push_custom_export_mono(Eina_Array *array, const void *data) EINA_ARG_NONNULL(1, 2)
{
   return eina_array_push(array, data);
}

EAPI void *eina_array_pop_custom_export_mono(Eina_Array *array) EINA_ARG_NONNULL(1)
{
   return eina_array_pop(array);
}

EAPI void *eina_array_data_get_custom_export_mono(const Eina_Array *array, unsigned int idx) EINA_ARG_NONNULL(1)
{
   return eina_array_data_get(array, idx);
}

EAPI void eina_array_data_set_custom_export_mono(const Eina_Array *array, unsigned int idx, const void *data) EINA_ARG_NONNULL(1)
{
   eina_array_data_set(array, idx, data);
}

EAPI unsigned int eina_array_count_custom_export_mono(const Eina_Array *array) EINA_ARG_NONNULL(1)
{
   return eina_array_count(array);
}

EAPI Eina_Bool eina_array_foreach_custom_export_mono(Eina_Array *array, Eina_Each_Cb cb, void *fdata)
{
   return eina_array_foreach(array, cb, fdata);
}

// List //

EAPI Eina_List *eina_list_last_custom_export_mono(const Eina_List *list)
{
   return eina_list_last(list);
}

EAPI Eina_List *eina_list_next_custom_export_mono(const Eina_List *list)
{
   return eina_list_next(list);
}

EAPI Eina_List *eina_list_prev_custom_export_mono(const Eina_List *list)
{
   return eina_list_prev(list);
}

EAPI void *eina_list_data_get_custom_export_mono(const Eina_List *list)
{
   return eina_list_data_get(list);
}

EAPI void *eina_list_data_set_custom_export_mono(Eina_List *list, const void *data)
{
   return eina_list_data_set(list, data);
}

EAPI unsigned int eina_list_count_custom_export_mono(const Eina_List *list)
{
   return eina_list_count(list);
}

EAPI void *eina_list_last_data_get_custom_export_mono(const Eina_List *list)
{
   return eina_list_last_data_get(list);
}

