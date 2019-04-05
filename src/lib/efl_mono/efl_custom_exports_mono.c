#include "Eo.h"
#include "Eina.h"
#include "Ecore.h"

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

typedef void (*Efl_Mono_Free_GCHandle_Cb)(void *gchandle);
typedef void (*Efl_Mono_Remove_Events_Cb)(Eo *obj, void *gchandle);

static Efl_Mono_Free_GCHandle_Cb _efl_mono_free_gchandle_call = NULL;
static Efl_Mono_Remove_Events_Cb _efl_mono_remove_events_call = NULL;

EAPI void efl_mono_gchandle_callbacks_set(Efl_Mono_Free_GCHandle_Cb free_gchandle_cb, Efl_Mono_Remove_Events_Cb remove_events_cb)
{
    _efl_mono_free_gchandle_call = free_gchandle_cb;
    _efl_mono_remove_events_call = remove_events_cb;
}

EAPI void efl_mono_native_dispose(Eo *obj, void* gchandle)
{
   if (gchandle) _efl_mono_remove_events_call(obj, gchandle);
   efl_unref(obj);
   if (gchandle) _efl_mono_free_gchandle_call(gchandle);
}

typedef struct _Efl_Mono_Native_Dispose_Data
{
   Eo *obj;
   void *gchandle;
} Efl_Mono_Native_Dispose_Data;

static void _efl_mono_native_dispose_cb(void *data)
{
   Efl_Mono_Native_Dispose_Data *dd = data;
   efl_mono_native_dispose(dd->obj, dd->gchandle);
   free(dd);
}

EAPI void efl_mono_thread_safe_native_dispose(Eo *obj, void* gchandle)
{
   Efl_Mono_Native_Dispose_Data *dd = malloc(sizeof(Efl_Mono_Native_Dispose_Data));
   dd->obj = obj;
   dd->gchandle = gchandle;
   ecore_main_loop_thread_safe_call_async(_efl_mono_native_dispose_cb, dd);
}

static void _efl_mono_unref_cb(void *obj)
{
   efl_unref(obj);
}

EAPI void efl_mono_thread_safe_efl_unref(Eo* obj)
{
   ecore_main_loop_thread_safe_call_async(_efl_mono_unref_cb, obj);
}

EAPI void efl_mono_thread_safe_free_cb_exec(Eina_Free_Cb free_cb, void* cb_data)
{
   ecore_main_loop_thread_safe_call_async(free_cb, cb_data);
}

static void _efl_mono_list_free_cb(void *l)
{
   eina_list_free(l);
}

EAPI void efl_mono_thread_safe_eina_list_free(Eina_List* list)
{
   ecore_main_loop_thread_safe_call_async(_efl_mono_list_free_cb, list);
}

typedef struct _Efl_Mono_Promise_Reject_Data
{
   Eina_Promise *promise;
   Eina_Error err;
} Efl_Mono_Promise_Reject_Data;

static void _efl_mono_promise_reject_cb(void *data)
{
   Efl_Mono_Promise_Reject_Data *d = data;
   eina_promise_reject(d->promise, d->err);
   free(d);
}

EAPI void efl_mono_thread_safe_promise_reject(Eina_Promise *p, Eina_Error err)
{
   Efl_Mono_Promise_Reject_Data *d = malloc(sizeof(Efl_Mono_Promise_Reject_Data));
   d->promise = p;
   d->err = err;
   ecore_main_loop_thread_safe_call_async(_efl_mono_promise_reject_cb, d);
}

EAPI void *efl_mono_native_alloc(unsigned int size)
{
   return malloc(size);
}

EAPI void efl_mono_native_memset(void *ptr, unsigned int fill, unsigned int count)
{
   memset(ptr, fill, count);
}

EAPI void efl_mono_native_free(void *ptr)
{
   free(ptr);
}

EAPI void efl_mono_native_free_ref(void **ptr)
{
   if (!ptr) return;
   free(*ptr);
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

EAPI Eina_Free_Cb efl_mono_native_free_addr_get()
{
    return (Eina_Free_Cb)free;
}

EAPI Eina_Free_Cb efl_mono_native_efl_unref_addr_get()
{
    return (Eina_Free_Cb)efl_mono_thread_safe_efl_unref;
}

// Iterator Wrapper //

typedef struct _Eina_Iterator_Wrapper_Mono
{
   Eina_Iterator iterator; // Must be the first
   Eina_Iterator *internal;
} Eina_Iterator_Wrapper_Mono;

static void *eina_iterator_wrapper_get_container_mono(Eina_Iterator_Wrapper_Mono *it)
{
   return eina_iterator_container_get(it->internal);
}

static void eina_iterator_wrapper_free_mono(Eina_Iterator_Wrapper_Mono *it)
{
   eina_iterator_free(it->internal);
   free(it);
}


static Eina_Iterator *eina_iterator_wrapper_new_mono(Eina_Iterator *internal, Eina_Iterator_Next_Callback next_cb)
{
   if (!internal) return NULL;

   Eina_Iterator_Wrapper_Mono *it = calloc(1, sizeof(Eina_Iterator_Wrapper_Mono));
   if (!it)
     {
        eina_iterator_free(internal);
        return NULL;
     }

   it->internal = internal;

   it->iterator.next = next_cb;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(eina_iterator_wrapper_get_container_mono);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_iterator_wrapper_free_mono);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   return &it->iterator;
}

// Array //

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

// Inlist //

typedef struct _Inlist_Node_Mono
{
   EINA_INLIST;
   char mem_start;
} Inlist_Node_Mono;

static Eina_Bool eina_inlist_iterator_wrapper_next_mono(Eina_Iterator_Wrapper_Mono *it, void **data)
{
   Inlist_Node_Mono *node = NULL;

   if (!eina_iterator_next(it->internal, (void**)&node))
     return EINA_FALSE;

   if (data)
     *data = &node->mem_start;

   return EINA_TRUE;
}

EAPI Eina_Iterator *eina_inlist_iterator_wrapper_new_custom_export_mono(const Eina_Inlist *in_list)
{
   return eina_iterator_wrapper_new_mono(eina_inlist_iterator_new(in_list), FUNC_ITERATOR_NEXT(eina_inlist_iterator_wrapper_next_mono));
}

EAPI Eina_Inlist *eina_inlist_first_custom_export_mono(const Eina_Inlist *list)
{
   return eina_inlist_first(list);
}

EAPI Eina_Inlist *eina_inlist_last_custom_export_mono(const Eina_Inlist *list)
{
   return eina_inlist_last(list);
}

EAPI Eina_Inlist *eina_inlist_next_custom_export_mono(const Eina_Inlist *list)
{
   if (list)
     return list->next;
   return NULL;
}

EAPI Eina_Inlist *eina_inlist_prev_custom_export_mono(const Eina_Inlist *list)
{
   if (list)
     return list->prev;
   return NULL;
}

// Hash //

static Eina_Bool eina_hash_iterator_ptr_key_wrapper_next_mono(Eina_Iterator_Wrapper_Mono *it, void **data)
{
   void **ptr = NULL;

   if (!eina_iterator_next(it->internal, (void**)&ptr))
     return EINA_FALSE;

   if (data)
     *data = *ptr;

   return EINA_TRUE;
}

EAPI Eina_Iterator *eina_hash_iterator_ptr_key_wrapper_new_custom_export_mono(const Eina_Hash *hash)
{
   return eina_iterator_wrapper_new_mono(eina_hash_iterator_key_new(hash), FUNC_ITERATOR_NEXT(eina_hash_iterator_ptr_key_wrapper_next_mono));
}

// Eina Value //
EAPI const Eina_Value_Type *type_byte() {
   return EINA_VALUE_TYPE_UCHAR;
}
EAPI const Eina_Value_Type *type_sbyte() {
   return EINA_VALUE_TYPE_CHAR;
}
EAPI const Eina_Value_Type *type_short() {
   return EINA_VALUE_TYPE_SHORT;
}
EAPI const Eina_Value_Type *type_ushort() {
   return EINA_VALUE_TYPE_USHORT;
}
EAPI const Eina_Value_Type *type_int32() {
   return EINA_VALUE_TYPE_INT;
}
EAPI const Eina_Value_Type *type_uint32() {
   return EINA_VALUE_TYPE_UINT;
}
EAPI const Eina_Value_Type *type_long() {
   return EINA_VALUE_TYPE_LONG;
}
EAPI const Eina_Value_Type *type_ulong() {
   return EINA_VALUE_TYPE_ULONG;
}
EAPI const Eina_Value_Type *type_int64() {
   return EINA_VALUE_TYPE_INT64;
}
EAPI const Eina_Value_Type *type_uint64() {
   return EINA_VALUE_TYPE_UINT64;
}
EAPI const Eina_Value_Type *type_string() {
   return EINA_VALUE_TYPE_STRING;
}
EAPI const Eina_Value_Type *type_float() {
   return EINA_VALUE_TYPE_FLOAT;
}
EAPI const Eina_Value_Type *type_double() {
   return EINA_VALUE_TYPE_DOUBLE;
}
EAPI const Eina_Value_Type *type_array() {
   return EINA_VALUE_TYPE_ARRAY;
}
EAPI const Eina_Value_Type *type_list() {
   return EINA_VALUE_TYPE_LIST;
}
EAPI const Eina_Value_Type *type_error() {
   return EINA_VALUE_TYPE_ERROR;
}

EAPI const Eina_Value_Type *type_optional() {
   return EINA_VALUE_TYPE_OPTIONAL;
}

EAPI size_t eina_value_sizeof()
{
   return sizeof(Eina_Value);
}

#define EINA_SET_WRAPPER(N, T) EAPI Eina_Bool eina_value_set_wrapper_##N(Eina_Value *value, T new_value) \
{ \
    return eina_value_set(value, new_value); \
}

EINA_SET_WRAPPER(char, char)
EINA_SET_WRAPPER(uchar, unsigned char)
EINA_SET_WRAPPER(short, short)
EINA_SET_WRAPPER(ushort, unsigned short)
EINA_SET_WRAPPER(int, int)
EINA_SET_WRAPPER(uint, unsigned int)
EINA_SET_WRAPPER(long, long)
EINA_SET_WRAPPER(ulong, unsigned long)
EINA_SET_WRAPPER(float, float)
EINA_SET_WRAPPER(double, double)
EINA_SET_WRAPPER(string, const char *)
EINA_SET_WRAPPER(ptr, void *)

#define EINA_CONTAINER_SET_WRAPPER(N, T) EAPI Eina_Bool eina_value_container_set_wrapper_##N(Eina_Value *value, int i, T new_value) \
{ \
    const Eina_Value_Type *tp = eina_value_type_get(value); \
    if (tp == EINA_VALUE_TYPE_ARRAY) \
        return eina_value_array_set(value, i, new_value); \
    else if (tp == EINA_VALUE_TYPE_LIST) \
        return eina_value_list_set(value, i, new_value); \
    else \
        return EINA_FALSE; \
}

EINA_CONTAINER_SET_WRAPPER(char, char)
EINA_CONTAINER_SET_WRAPPER(uchar, unsigned char)
EINA_CONTAINER_SET_WRAPPER(short, short)
EINA_CONTAINER_SET_WRAPPER(ushort, unsigned short)
EINA_CONTAINER_SET_WRAPPER(int, int)
EINA_CONTAINER_SET_WRAPPER(uint, unsigned int)
EINA_CONTAINER_SET_WRAPPER(long, long)
EINA_CONTAINER_SET_WRAPPER(ulong, unsigned long)
EINA_CONTAINER_SET_WRAPPER(float, float)
EINA_CONTAINER_SET_WRAPPER(double, double)
EINA_CONTAINER_SET_WRAPPER(string, const char *)
EINA_CONTAINER_SET_WRAPPER(ptr, void *)

#define EINA_CONTAINER_APPEND_WRAPPER(N, T) EAPI Eina_Bool eina_value_container_append_wrapper_##N(Eina_Value *value, T new_value) \
{ \
    const Eina_Value_Type *tp = eina_value_type_get(value); \
    if (tp == EINA_VALUE_TYPE_ARRAY) \
        return eina_value_array_append(value, new_value); \
    else if (tp == EINA_VALUE_TYPE_LIST) \
        return eina_value_list_append(value, new_value); \
    else \
        return EINA_FALSE; \
}

EINA_CONTAINER_APPEND_WRAPPER(char, char)
EINA_CONTAINER_APPEND_WRAPPER(uchar, unsigned char)
EINA_CONTAINER_APPEND_WRAPPER(short, short)
EINA_CONTAINER_APPEND_WRAPPER(ushort, unsigned short)
EINA_CONTAINER_APPEND_WRAPPER(int, int)
EINA_CONTAINER_APPEND_WRAPPER(uint, unsigned int)
EINA_CONTAINER_APPEND_WRAPPER(long, long)
EINA_CONTAINER_APPEND_WRAPPER(ulong, unsigned long)
EINA_CONTAINER_APPEND_WRAPPER(float, float)
EINA_CONTAINER_APPEND_WRAPPER(double, double)
EINA_CONTAINER_APPEND_WRAPPER(string, const char *)
EINA_CONTAINER_APPEND_WRAPPER(ptr, void *)

EAPI void eina_value_container_get_wrapper(const Eina_Value *value, int i, void *output)
{
    const Eina_Value_Type *tp = eina_value_type_get(value);
    if (tp == EINA_VALUE_TYPE_ARRAY)
        eina_value_array_get(value, i, output);
    else if (tp == EINA_VALUE_TYPE_LIST)
        eina_value_list_get(value, i, output);
}

EAPI Eina_Bool eina_value_setup_wrapper(Eina_Value *value,
                                   const Eina_Value_Type *type)
{
   return eina_value_setup(value, type);
}

EAPI void eina_value_flush_wrapper(Eina_Value *value)
{
   eina_value_flush(value);
}

EAPI const Eina_Value_Type *eina_value_type_get_wrapper(const Eina_Value *value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, NULL);

   // Can't pass null value type (for Empty values) to value_type_get.
   if (value->type == NULL)
     return NULL;
   return eina_value_type_get(value);
}

EAPI Eina_Bool eina_value_get_wrapper(const Eina_Value *value, void *output)
{
   return eina_value_get(value, output);
}

EAPI int eina_value_compare_wrapper(const Eina_Value *this, const Eina_Value *other)
{
   return eina_value_compare(this, other);
}

EAPI Eina_Bool eina_value_array_setup_wrapper(Eina_Value *array, const Eina_Value_Type *subtype, unsigned int step)
{
   return eina_value_array_setup(array, subtype, step);
}

EAPI Eina_Bool eina_value_list_setup_wrapper(Eina_Value *list, const Eina_Value_Type *subtype)
{
   return eina_value_list_setup(list, subtype);
}

EAPI Eina_Bool eina_value_array_append_wrapper(Eina_Value *array, va_list argp)
{
   return eina_value_array_append(array, argp);
}

EAPI Eina_Bool eina_value_list_append_wrapper(Eina_Value *list, va_list argp)
{
   return eina_value_list_append(list, argp);
}

EAPI Eina_Bool eina_value_array_get_wrapper(const Eina_Value *array, int i, void *output)
{
   return eina_value_array_get(array, i, output);
}

EAPI Eina_Bool eina_value_list_get_wrapper(const Eina_Value *list, int i, void *output)
{
   return eina_value_list_get(list, i, output);
}

EAPI Eina_Bool eina_value_array_set_wrapper(Eina_Value *array, int i, void *value)
{
   return eina_value_array_set(array, i, value);
}

EAPI Eina_Bool eina_value_list_set_wrapper(Eina_Value *list, int i, void *value)
{
   return eina_value_list_set(list, i, value);
}

// Not actually a wrapper, but keeping the naming convention for functions on this file.
EAPI const Eina_Value_Type* eina_value_array_subtype_get_wrapper(const Eina_Value *array)
{
   Eina_Value_Array array_value;
   eina_value_get(array, &array_value);
   return array_value.subtype;
}

EAPI const Eina_Value_Type* eina_value_list_subtype_get_wrapper(const Eina_Value *list)
{
   Eina_Value_List list_value;
   eina_value_get(list, &list_value);
   return list_value.subtype;
}

EAPI unsigned int eina_value_array_count_wrapper(const Eina_Value *array)
{
   return eina_value_array_count(array);
}

EAPI unsigned int eina_value_list_count_wrapper(const Eina_Value *list)
{
   return eina_value_list_count(list);
}

EAPI Eina_Bool eina_value_optional_empty_is_wrapper(const Eina_Value *value, Eina_Bool *empty)
{
   return eina_value_optional_empty_is(value, empty);
}

EAPI const Eina_Value_Type *eina_value_optional_type_get_wrapper(Eina_Value *value)
{
   return eina_value_optional_type_get(value);
}

EAPI Eina_Bool eina_value_pset_wrapper(Eina_Value *value, void *ptr)
{
   return eina_value_pset(value, ptr);
}
