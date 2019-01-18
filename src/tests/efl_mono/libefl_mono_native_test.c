
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define EFL_PART_PROTECTED

#include <Ecore.h>
#include <Eo.h>

#ifdef EOAPI
#undef EOAPI
#endif

#ifdef EWAPI
#undef EWAPI
#endif

#ifdef EAPI
#undef EAPI
#endif

#define EOAPI EAPI EAPI_WEAK
#define EWAPI EAPI EAPI_WEAK

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

#include "dummy_numberwrapper.eo.h"
#include "dummy_test_object.eo.h"
#include "dummy_child.eo.h"
#include "dummy_test_iface.eo.h"
#include "dummy_another_iface.eo.h"
#include "dummy_inherit_iface.eo.h"
#include "dummy_inherit_helper.eo.h"

#include <interfaces/efl_part.eo.h>

#define EQUAL(a, b) ((a) == (b) ? 1 : (fprintf(stderr, "NOT EQUAL! %s:%i (%s)", __FILE__, __LINE__, __FUNCTION__), fflush(stderr), 0))
#define STR_EQUAL(a, b) (strcmp((a), (b)) == 0 ? 1 : (fprintf(stderr, "NOT EQUAL! %s:%i (%s) '%s' != '%s'", __FILE__, __LINE__, __FUNCTION__, (a), (b)), fflush(stderr), 0))

typedef struct Dummy_Test_Object_Data
{
  Dummy_SimpleCb cb;
  void *cb_data;
  Eina_Free_Cb free_cb;
  Eina_Error error_code;
  Eina_Value *stored_value;
  Dummy_StructSimple stored_struct;
  int stored_int;
  Eo *part_one;
  Eo *part_two;
  Eina_Promise *promise;
  Eina_List *list_for_accessor;
  int setter_only;
  int iface_prop;
} Dummy_Test_Object_Data;

typedef struct Dummy_Numberwrapper_Data
{
   int number;
} Dummy_Numberwrapper_Data;

typedef struct Dummy_Child_Data
{
} Dummy_Child_Data;

typedef struct Dummy_Inherit_Helper_Data
{
} Dummy_Inherit_Helper_Data;

typedef struct Dummy_Inherit_Iface_Data
{
} Dummy_Inherit_Iface_Data;

static
void *_new_int(int v)
{
   int *r = malloc(sizeof(int));
   *r = v;
   return r;
}

static
int *_int_ref(int n)
{
   static int r;
   r = n;
   return &r;
}

static
char **_new_str_ref(const char* str)
{
   static char *r;
   r = strdup(str);
   return &r;
}

static
Dummy_Numberwrapper *_new_obj(int n)
{
   return efl_add_ref(DUMMY_NUMBERWRAPPER_CLASS, NULL, dummy_numberwrapper_number_set(efl_added, n));
}

static
Dummy_Numberwrapper **_new_obj_ref(int n)
{
   static Dummy_Numberwrapper *r;
   r = _new_obj(n);
   return &r;
}

// ############ //
// Test.Testing //
// ############ //

static Efl_Object*
_dummy_test_object_efl_object_constructor(Eo *obj, Dummy_Test_Object_Data *pd)
{
   efl_constructor(efl_super(obj, DUMMY_TEST_OBJECT_CLASS));

   // To avoid an infinite loop calling the same constructor
   if (!efl_parent_get(obj))
     {
        pd->part_one = efl_add(DUMMY_TEST_OBJECT_CLASS, obj, efl_name_set(efl_added, "part_one"));
        pd->part_two = efl_add(DUMMY_TEST_OBJECT_CLASS, obj, efl_name_set(efl_added, "part_two"));
     }

   return obj;
}

Efl_Object *_dummy_test_object_return_object(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return obj;
}

void _dummy_test_object_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, int x, int *y)
{
    *y = -x;
}

void _dummy_test_object_int_ptr_out(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, int x, int **y)
{
    pd->stored_int = x * 2;
    *y = &pd->stored_int;
}

const char *_dummy_test_object_in_string(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const char *str)
{
  const char *ret = malloc(sizeof(char)*(strlen(str) + 1));
  return strcpy((char*)ret, str);
}

char *_dummy_test_object_in_own_string(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, char *str)
{
  char *ret = malloc(sizeof(char)*(strlen(str) + 1));
  strcpy(ret, str);
  free((void*)str); // Actually take ownership of it.
  return ret;
}

Eina_Stringshare *_dummy_test_object_return_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Stringshare *str = eina_stringshare_add("stringshare");
  return str;
}

Eina_Stringshare *_dummy_test_object_return_own_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Stringshare *str = eina_stringshare_add("own_stringshare");
  return str;
}

const char *_dummy_test_object_return_string(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return "string";
}

const char *_dummy_test_object_return_own_string(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  static const char* reference = "own_string";
  const char *ret = malloc(sizeof(char)*(strlen(reference) + 1));
  return strcpy((char*)ret, reference);
}

void _dummy_test_object_out_string(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const char**str)
{
  *str = "out_string";
}

void _dummy_test_object_out_own_string(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const char**str)
{
  static const char* reference = "out_own_string";
  *str = malloc(sizeof(char)*(strlen(reference) + 1));
  strcpy((char*)*str, reference);
}

void _dummy_test_object_call_in_string(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const char *str)
{
  dummy_test_object_in_string(obj, str);
}

void _dummy_test_object_call_in_own_string(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, char *str)
{
  dummy_test_object_in_own_string(obj, str);
}

const char *_dummy_test_object_call_return_string(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return dummy_test_object_return_string(obj);
}

const char *_dummy_test_object_call_return_own_string(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return dummy_test_object_return_own_string(obj);
}

const char *_dummy_test_object_call_out_string(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  const char *ret = NULL;
  dummy_test_object_out_string(obj, &ret);
  return ret;
}

const char *_dummy_test_object_call_out_own_string(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  const char *ret = NULL;
  dummy_test_object_out_own_string(obj, &ret);
  return ret;
}

// Stringshare virtual test helpers
void _dummy_test_object_call_in_stringshare(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Stringshare *str)
{
  dummy_test_object_in_stringshare(obj, str);
}

void _dummy_test_object_call_in_own_stringshare(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Stringshare *str)
{
  str = dummy_test_object_in_own_stringshare(obj, str);
  eina_stringshare_del(str);
}

Eina_Stringshare *_dummy_test_object_in_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Stringshare *str)
{
    return eina_stringshare_add(str);
}

Eina_Stringshare *_dummy_test_object_in_own_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Stringshare *str)
{
    return str;
}

void _dummy_test_object_out_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const char**str)
{
  // Returning simple string but the binding shouldn't del it as it is not owned by the caller
  *str = "out_stringshare";
}

void _dummy_test_object_out_own_stringshare(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const char**str)
{
  *str = eina_stringshare_add("out_own_stringshare");
}

Eina_Stringshare *_dummy_test_object_call_return_stringshare(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return dummy_test_object_return_stringshare(obj);
}

Eina_Stringshare *_dummy_test_object_call_return_own_stringshare(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return dummy_test_object_return_own_stringshare(obj);
}

Eina_Stringshare *_dummy_test_object_call_out_stringshare(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Stringshare *ret = NULL;
  dummy_test_object_out_stringshare(obj, &ret);
  return ret;
}

Eina_Stringshare *_dummy_test_object_call_out_own_stringshare(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Stringshare *ret = NULL;
  dummy_test_object_out_own_stringshare(obj, &ret);
  return ret;
}


static const uint8_t base_seq[] = {0x0,0x2A,0x42};
// static const size_t base_seq_size = 3; // TODO: Use it!!!

static void *memdup(const void* mem, size_t size)
{
  void *out = malloc(size);
  memcpy(out, mem, size);
  return out;
}

Eina_Bool _dummy_test_object_eina_slice_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Slice slice)
{
  uint8_t *buf = memdup(slice.mem, slice.len);
  free(buf);
  return 0 == memcmp(slice.mem, base_seq, slice.len);
}

Eina_Bool _dummy_test_object_eina_rw_slice_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Rw_Slice slice)
{
  Eina_Bool r = (0 == memcmp(slice.mem, base_seq, slice.len));
  unsigned char *buf = memdup(slice.mem, slice.len);
  free(buf);
  for (unsigned i = 0; i < slice.len; ++i)
    slice.bytes[i] += 1;
  return r;
}

Eina_Bool _dummy_test_object_eina_slice_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Slice *slice)
{
  if (!slice) return EINA_FALSE;
  static const Eina_Slice slc = EINA_SLICE_ARRAY(base_seq);
  slice->len = slc.len;
  slice->mem = slc.mem;
  return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_rw_slice_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Rw_Slice *slice)
{
  if (!slice) return EINA_FALSE;
  slice->len = 3;
  slice->mem = memdup(base_seq, 3);
  return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_rw_slice_inout(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Rw_Slice *slice)
{
  if (!slice) return EINA_FALSE;
  for (size_t i = 0; i < slice->len; i++)
    slice->bytes[i] += (uint8_t)i;
  return EINA_TRUE;
}

Eina_Slice _dummy_test_object_eina_slice_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Slice slc = EINA_SLICE_ARRAY(base_seq);
  return slc;
}

Eina_Rw_Slice _dummy_test_object_eina_rw_slice_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Rw_Slice slc = { .len = 3, .mem = memdup(base_seq, 3) };
  return slc;
}

Eina_Bool _dummy_test_object_eina_binbuf_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Binbuf *binbuf)
{
  Eina_Bool r = (0 == memcmp(eina_binbuf_string_get(binbuf), base_seq, eina_binbuf_length_get(binbuf)));
  eina_binbuf_insert_char(binbuf, 42, 0);
  eina_binbuf_insert_char(binbuf, 43, 0);
  eina_binbuf_append_char(binbuf, 33);
  return r;
}

Eina_Bool _dummy_test_object_call_eina_binbuf_in(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Binbuf *binbuf)
{
  return dummy_test_object_eina_binbuf_in(obj, binbuf);
}

Eina_Binbuf *_binbuf_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_binbuf_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Binbuf *binbuf)
{
  Eina_Bool r = (0 == memcmp(eina_binbuf_string_get(binbuf), base_seq, eina_binbuf_length_get(binbuf)));
  eina_binbuf_insert_char(binbuf, 42, 0);
  eina_binbuf_insert_char(binbuf, 43, 0);
  eina_binbuf_append_char(binbuf, 33);
  _binbuf_in_own_to_check = binbuf;
  return r;
}

Eina_Bool _dummy_test_object_call_eina_binbuf_in_own(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Binbuf *binbuf)
{
    return dummy_test_object_eina_binbuf_in_own(obj, binbuf);
}

Eina_Bool _dummy_test_object_check_binbuf_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    if (!_binbuf_in_own_to_check) return EINA_FALSE;
    const uint8_t mod_seq[] = {43,42,0x0,0x2A,0x42,33};
    Eina_Bool r = (0 == memcmp(eina_binbuf_string_get(_binbuf_in_own_to_check), mod_seq, eina_binbuf_length_get(_binbuf_in_own_to_check)));
    eina_binbuf_string_free(_binbuf_in_own_to_check);
    return r;
}

Eina_Binbuf *_binbuf_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_binbuf_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Binbuf **binbuf)
{
  if (!binbuf) return EINA_FALSE;
  *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(*binbuf, 33);
  _binbuf_out_to_check = *binbuf;
  return EINA_TRUE;
}

Eina_Binbuf *_dummy_test_object_call_eina_binbuf_out(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Binbuf *binbuf = NULL;
  dummy_test_object_eina_binbuf_out(obj, &binbuf);
  return binbuf;
}

Eina_Bool _dummy_test_object_check_binbuf_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    if (!_binbuf_out_to_check) return EINA_FALSE;
    return 0 == memcmp(eina_binbuf_string_get(_binbuf_out_to_check), base_seq, eina_binbuf_length_get(_binbuf_out_to_check));
}

Eina_Bool _dummy_test_object_eina_binbuf_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Binbuf **binbuf)
{
  if (!binbuf) return EINA_FALSE;
  *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(*binbuf, 33);
  return EINA_TRUE;
}

Eina_Binbuf *_dummy_test_object_call_eina_binbuf_out_own(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Binbuf *binbuf = NULL;
  dummy_test_object_eina_binbuf_out_own(obj, &binbuf);
  return binbuf;
}

Eina_Binbuf *_binbuf_return_to_check = NULL;

Eina_Binbuf *_dummy_test_object_eina_binbuf_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Binbuf *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(binbuf, 33);
  _binbuf_return_to_check = binbuf;
  return binbuf;
}

Eina_Binbuf *_dummy_test_object_call_eina_binbuf_return(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return dummy_test_object_eina_binbuf_return(obj);
}

Eina_Bool _dummy_test_object_check_binbuf_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    if (!_binbuf_return_to_check) return EINA_FALSE;
    return 0 == memcmp(eina_binbuf_string_get(_binbuf_return_to_check), base_seq, eina_binbuf_length_get(_binbuf_return_to_check));
}

Eina_Binbuf *_dummy_test_object_eina_binbuf_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  Eina_Binbuf *binbuf = eina_binbuf_new();
  eina_binbuf_append_char(binbuf, 33);
  return binbuf;
}

Eina_Binbuf *_dummy_test_object_call_eina_binbuf_return_own(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
  return dummy_test_object_eina_binbuf_return_own(obj);
}


static const int base_seq_int[] = {0x0,0x2A,0x42};
static const unsigned int base_seq_int_size = EINA_C_ARRAY_LENGTH(base_seq_int);
static const int modified_seq_int[] = {0x0,0x2A,0x42,42,43,33};
static const unsigned int modified_seq_int_size = EINA_C_ARRAY_LENGTH(modified_seq_int);

static const char * const base_seq_str[] = {"0x0","0x2A","0x42"};
static const unsigned int base_seq_str_size = EINA_C_ARRAY_LENGTH(base_seq_str);
static const char * const modified_seq_str[] = {"0x0","0x2A","0x42","42","43","33"};
static const unsigned int modified_seq_str_size = EINA_C_ARRAY_LENGTH(modified_seq_str);

static const Dummy_Numberwrapper *base_seq_obj[] = {NULL,NULL,NULL};
static const unsigned int base_seq_obj_size = EINA_C_ARRAY_LENGTH(base_seq_str);
static const Dummy_Numberwrapper *modified_seq_obj[] = {NULL,NULL,NULL,NULL,NULL,NULL};
static const unsigned int modified_seq_obj_size = EINA_C_ARRAY_LENGTH(modified_seq_str);

// //
// Array
//

static const unsigned int default_step = 32;

// Integer

Eina_Bool _array_int_equal(const Eina_Array *arr, const int base[], unsigned int len)
{
   if (eina_array_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        if (*((int*)eina_array_data_get(arr, i)) != base[i])
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_array_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_int_equal(arr, base_seq_int, base_seq_int_size);
   eina_array_push(arr, _new_int(42));
   eina_array_push(arr, _new_int(43));
   eina_array_push(arr, _new_int(33));
   return r;
}

static Eina_Array *_array_int_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_array_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_int_equal(arr, base_seq_int, base_seq_int_size);
   eina_array_push(arr, _new_int(42));
   eina_array_push(arr, _new_int(43));
   eina_array_push(arr, _new_int(33));
   _array_int_in_own_to_check = arr;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_array_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    if (!_array_int_in_own_to_check) return EINA_FALSE;

    Eina_Bool r = _array_int_equal(_array_int_in_own_to_check, modified_seq_int, modified_seq_int_size);

    unsigned int i;
    int *ele;
    Eina_Array_Iterator it;
    EINA_ARRAY_ITER_NEXT(_array_int_in_own_to_check, i, ele, it)
      free(ele);

    eina_array_free(_array_int_in_own_to_check);
    _array_int_in_own_to_check = NULL;
    return r;
}

Eina_Array *_array_int_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_array_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array **arr)
{
    if (!arr) return EINA_FALSE;
    *arr = eina_array_new(default_step);
    eina_array_push(*arr, _new_int(0x0));
    eina_array_push(*arr, _new_int(0x2A));
    eina_array_push(*arr, _new_int(0x42));
    _array_int_out_to_check = *arr;
    return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_array_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_int_out_to_check) return EINA_FALSE;

   Eina_Bool r = _array_int_equal(_array_int_out_to_check, modified_seq_int, modified_seq_int_size);

   unsigned int i;
   int *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_int_out_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_int_out_to_check);
   _array_int_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_array_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, _new_int(0x0));
   eina_array_push(*arr, _new_int(0x2A));
   eina_array_push(*arr, _new_int(0x42));
   return EINA_TRUE;
}

Eina_Array *_array_int_return_to_check = NULL;

Eina_Array *_dummy_test_object_eina_array_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_int(0x0));
   eina_array_push(arr, _new_int(0x2A));
   eina_array_push(arr, _new_int(0x42));
   _array_int_return_to_check = arr;
   return arr;
}
Eina_Bool _dummy_test_object_check_eina_array_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_int_return_to_check) return EINA_FALSE;

   Eina_Bool r = _array_int_equal(_array_int_return_to_check, modified_seq_int, modified_seq_int_size);

   unsigned int i;
   int *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_int_return_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_int_return_to_check);
   _array_int_return_to_check = NULL;
   return r;
}

Eina_Array *_dummy_test_object_eina_array_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_int(0x0));
   eina_array_push(arr, _new_int(0x2A));
   eina_array_push(arr, _new_int(0x42));
   return arr;
}

// String

Eina_Bool _array_str_equal(const Eina_Array *arr, const char * const base[], unsigned int len)
{
   if (eina_array_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        if (0 != strcmp(eina_array_data_get(arr, i), base[i]))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_array_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_str_equal(arr, base_seq_str, base_seq_str_size);
   eina_array_push(arr, strdup("42"));
   eina_array_push(arr, strdup("43"));
   eina_array_push(arr, strdup("33"));
   return r;
}

static Eina_Array *_array_str_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_array_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_str_equal(arr, base_seq_str, base_seq_str_size);
   eina_array_push(arr, strdup("42"));
   eina_array_push(arr, strdup("43"));
   eina_array_push(arr, strdup("33"));
   _array_str_in_own_to_check = arr;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_array_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_str_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _array_str_equal(_array_str_in_own_to_check, modified_seq_str, modified_seq_str_size);

   unsigned int i;
   char *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_str_in_own_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_str_in_own_to_check);
   _array_str_in_own_to_check = NULL;
   return r;
}

Eina_Array *_array_str_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_array_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, strdup("0x0"));
   eina_array_push(*arr, strdup("0x2A"));
   eina_array_push(*arr, strdup("0x42"));
   _array_str_out_to_check = *arr;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_array_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_str_out_to_check) return EINA_FALSE;

   Eina_Bool r = _array_str_equal(_array_str_out_to_check, modified_seq_str, modified_seq_str_size);

   unsigned int i;
   char *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_str_out_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_str_out_to_check);
   _array_str_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_array_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, strdup("0x0"));
   eina_array_push(*arr, strdup("0x2A"));
   eina_array_push(*arr, strdup("0x42"));
   return EINA_TRUE;
}

Eina_Array *_array_str_return_to_check = NULL;

Eina_Array *_dummy_test_object_eina_array_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, strdup("0x0"));
   eina_array_push(arr, strdup("0x2A"));
   eina_array_push(arr, strdup("0x42"));
   _array_str_return_to_check = arr;
   return arr;
}
Eina_Bool _dummy_test_object_check_eina_array_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_str_return_to_check) return EINA_FALSE;

   Eina_Bool r = _array_str_equal(_array_str_return_to_check, modified_seq_str, modified_seq_str_size);

   unsigned int i;
   char *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_str_return_to_check, i, ele, it)
     free(ele);

   eina_array_free(_array_str_return_to_check);
   _array_str_return_to_check = NULL;
   return r;
}

Eina_Array *_dummy_test_object_eina_array_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, strdup("0x0"));
   eina_array_push(arr, strdup("0x2A"));
   eina_array_push(arr, strdup("0x42"));
   return arr;
}

// Object

Eina_Bool _array_obj_equal(const Eina_Array *arr, const Dummy_Numberwrapper * const base[], unsigned int len)
{
   if (eina_array_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        const Dummy_Numberwrapper *eo = eina_array_data_get(arr, i);
        int a = dummy_numberwrapper_number_get(eo);
        int b = dummy_numberwrapper_number_get(base[i]);
        if (a != b)
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_array_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_obj_equal(arr, base_seq_obj, base_seq_obj_size);
   if (!r) return r;
   eina_array_push(arr, _new_obj(42));
   eina_array_push(arr, _new_obj(43));
   eina_array_push(arr, _new_obj(33));
   return r;
}

static Eina_Array *_array_obj_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_array_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array *arr)
{
   Eina_Bool r = _array_obj_equal(arr, base_seq_obj, base_seq_obj_size);
   if (!r) return r;
   eina_array_push(arr, _new_obj(42));
   eina_array_push(arr, _new_obj(43));
   eina_array_push(arr, _new_obj(33));
   _array_obj_in_own_to_check = arr;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_array_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_obj_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _array_obj_equal(_array_obj_in_own_to_check, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   unsigned int i;
   Dummy_Numberwrapper *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_obj_in_own_to_check, i, ele, it)
     efl_unref(ele);

   eina_array_free(_array_obj_in_own_to_check);
   _array_obj_in_own_to_check = NULL;
   return r;
}

Eina_Array *_array_obj_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_array_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, _new_obj(0x0));
   eina_array_push(*arr, _new_obj(0x2A));
   eina_array_push(*arr, _new_obj(0x42));
   _array_obj_out_to_check = *arr;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_array_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_obj_out_to_check) return EINA_FALSE;

   Eina_Bool r = _array_obj_equal(_array_obj_out_to_check, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   unsigned int i;
   Dummy_Numberwrapper *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_obj_out_to_check, i, ele, it)
     efl_unref(ele);

   eina_array_free(_array_obj_out_to_check);
   _array_obj_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_array_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_array_new(default_step);
   eina_array_push(*arr, _new_obj(0x0));
   eina_array_push(*arr, _new_obj(0x2A));
   eina_array_push(*arr, _new_obj(0x42));
   return EINA_TRUE;
}

Eina_Array *_array_obj_return_to_check = NULL;

Eina_Array *_dummy_test_object_eina_array_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_obj(0x0));
   eina_array_push(arr, _new_obj(0x2A));
   eina_array_push(arr, _new_obj(0x42));
   _array_obj_return_to_check = arr;
   return arr;
}
Eina_Bool _dummy_test_object_check_eina_array_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_array_obj_return_to_check) return EINA_FALSE;

   Eina_Bool r = _array_obj_equal(_array_obj_return_to_check, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   unsigned int i;
   Dummy_Numberwrapper *ele;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(_array_obj_return_to_check, i, ele, it)
     efl_unref(ele);

   eina_array_free(_array_obj_return_to_check);
   _array_obj_return_to_check = NULL;
   return r;
}

Eina_Array *_dummy_test_object_eina_array_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = eina_array_new(default_step);
   eina_array_push(arr, _new_obj(0x0));
   eina_array_push(arr, _new_obj(0x2A));
   eina_array_push(arr, _new_obj(0x42));
   return arr;
}

Eina_Array *_dummy_test_object_eina_array_obj_return_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Array *arr)
{
   return arr;
}

// //
// Inarray
//

// Integer

Eina_Bool _inarray_int_equal(const Eina_Inarray *arr, const int base[], unsigned int len)
{
   if (eina_inarray_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        if (*((int*)eina_inarray_nth(arr, i)) != base[i])
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_inarray_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray *arr)
{
   Eina_Bool r = _inarray_int_equal(arr, base_seq_int, base_seq_int_size);
   eina_inarray_push(arr, _int_ref(42));
   eina_inarray_push(arr, _int_ref(43));
   eina_inarray_push(arr, _int_ref(33));
   return r;
}

static Eina_Inarray *_inarray_int_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inarray_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray *arr)
{
   Eina_Bool r = _inarray_int_equal(arr, base_seq_int, base_seq_int_size);
   eina_inarray_push(arr, _int_ref(42));
   eina_inarray_push(arr, _int_ref(43));
   eina_inarray_push(arr, _int_ref(33));
   _inarray_int_in_own_to_check = arr;
   return r;
}
Eina_Bool _dummy_test_object_check_eina_inarray_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_int_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_int_equal(_inarray_int_in_own_to_check, modified_seq_int, modified_seq_int_size);

   eina_inarray_free(_inarray_int_in_own_to_check);
   _inarray_int_in_own_to_check = NULL;
   return r;
}

Eina_Inarray *_inarray_int_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inarray_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_inarray_new(sizeof(int), 0);
   eina_inarray_push(*arr, _int_ref(0x0));
   eina_inarray_push(*arr, _int_ref(0x2A));
   eina_inarray_push(*arr, _int_ref(0x42));
   _inarray_int_out_to_check = *arr;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_inarray_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_int_out_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_int_equal(_inarray_int_out_to_check, modified_seq_int, modified_seq_int_size);

   eina_inarray_free(_inarray_int_out_to_check);
   _inarray_int_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_inarray_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_inarray_new(sizeof(int), 0);
   eina_inarray_push(*arr, _int_ref(0x0));
   eina_inarray_push(*arr, _int_ref(0x2A));
   eina_inarray_push(*arr, _int_ref(0x42));
   return EINA_TRUE;
}

Eina_Inarray *_inarray_int_return_to_check = NULL;

Eina_Inarray *_dummy_test_object_eina_inarray_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inarray *arr = eina_inarray_new(sizeof(int), 0);
   eina_inarray_push(arr, _int_ref(0x0));
   eina_inarray_push(arr, _int_ref(0x2A));
   eina_inarray_push(arr, _int_ref(0x42));
   _inarray_int_return_to_check = arr;
   return arr;
}
Eina_Bool _dummy_test_object_check_eina_inarray_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_int_return_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_int_equal(_inarray_int_return_to_check, modified_seq_int, modified_seq_int_size);

   eina_inarray_free(_inarray_int_return_to_check);
   _inarray_int_return_to_check = NULL;
   return r;
}

Eina_Inarray *_dummy_test_object_eina_inarray_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inarray *arr = eina_inarray_new(sizeof(int), 0);
   eina_inarray_push(arr, _int_ref(0x0));
   eina_inarray_push(arr, _int_ref(0x2A));
   eina_inarray_push(arr, _int_ref(0x42));
   return arr;
}

// String

Eina_Bool _inarray_str_equal(const Eina_Inarray *arr, const char * const base[], unsigned int len)
{
   if (eina_inarray_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        if (0 != strcmp(*((char**)eina_inarray_nth(arr, i)), base[i]))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_inarray_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray *arr)
{
   Eina_Bool r = _inarray_str_equal(arr, base_seq_str, base_seq_str_size);
   eina_inarray_push(arr, _new_str_ref("42"));
   eina_inarray_push(arr, _new_str_ref("43"));
   eina_inarray_push(arr, _new_str_ref("33"));
   return r;
}

static Eina_Inarray *_inarray_str_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inarray_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray *arr)
{
   Eina_Bool r = _inarray_str_equal(arr, base_seq_str, base_seq_str_size);
   eina_inarray_push(arr, _new_str_ref("42"));
   eina_inarray_push(arr, _new_str_ref("43"));
   eina_inarray_push(arr, _new_str_ref("33"));
   _inarray_str_in_own_to_check = arr;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_inarray_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_str_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_str_equal(_inarray_str_in_own_to_check, modified_seq_str, modified_seq_str_size);

   char **ele;
   EINA_INARRAY_FOREACH(_inarray_str_in_own_to_check, ele)
     free(*ele);

   eina_inarray_free(_inarray_str_in_own_to_check);
   _inarray_str_in_own_to_check = NULL;
   return r;
}

Eina_Inarray *_inarray_str_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inarray_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_inarray_new(sizeof(char*), 0);
   eina_inarray_push(*arr, _new_str_ref("0x0"));
   eina_inarray_push(*arr, _new_str_ref("0x2A"));
   eina_inarray_push(*arr, _new_str_ref("0x42"));
   _inarray_str_out_to_check = *arr;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_inarray_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_str_out_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_str_equal(_inarray_str_out_to_check, modified_seq_str, modified_seq_str_size);

   char **ele;
   EINA_INARRAY_FOREACH(_inarray_str_out_to_check, ele)
     free(*ele);

   eina_inarray_free(_inarray_str_out_to_check);
   _inarray_str_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_inarray_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_inarray_new(sizeof(char*), 0);
   eina_inarray_push(*arr, _new_str_ref("0x0"));
   eina_inarray_push(*arr, _new_str_ref("0x2A"));
   eina_inarray_push(*arr, _new_str_ref("0x42"));
   return EINA_TRUE;
}

Eina_Inarray *_inarray_str_return_to_check = NULL;

Eina_Inarray *_dummy_test_object_eina_inarray_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inarray *arr = eina_inarray_new(sizeof(char*), 0);
   eina_inarray_push(arr, _new_str_ref("0x0"));
   eina_inarray_push(arr, _new_str_ref("0x2A"));
   eina_inarray_push(arr, _new_str_ref("0x42"));
   _inarray_str_return_to_check = arr;
   return arr;
}
Eina_Bool _dummy_test_object_check_eina_inarray_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_str_return_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_str_equal(_inarray_str_return_to_check, modified_seq_str, modified_seq_str_size);

   char **ele;
   EINA_INARRAY_FOREACH(_inarray_str_return_to_check, ele)
     free(*ele);

   eina_inarray_free(_inarray_str_return_to_check);
   _inarray_str_return_to_check = NULL;
   return r;
}

Eina_Inarray *_dummy_test_object_eina_inarray_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inarray *arr = eina_inarray_new(sizeof(char*), 0);
   eina_inarray_push(arr, _new_str_ref("0x0"));
   eina_inarray_push(arr, _new_str_ref("0x2A"));
   eina_inarray_push(arr, _new_str_ref("0x42"));
   return arr;
}

// Object

Eina_Bool _inarray_obj_equal(const Eina_Inarray *arr, const Dummy_Numberwrapper * const base[], unsigned int len)
{
   if (eina_inarray_count(arr) != len)
     return EINA_FALSE;
   for (unsigned int i = 0; i < len; ++i)
     {
        const Dummy_Numberwrapper **eo = eina_inarray_nth(arr, i);
        int a = dummy_numberwrapper_number_get(*eo);
        int b = dummy_numberwrapper_number_get(base[i]);
        if (a != b)
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_inarray_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray *arr)
{
   Eina_Bool r = _inarray_obj_equal(arr, base_seq_obj, base_seq_obj_size);
   if (!r) return r;
   eina_inarray_push(arr, _new_obj_ref(42));
   eina_inarray_push(arr, _new_obj_ref(43));
   eina_inarray_push(arr, _new_obj_ref(33));
   return r;
}

static Eina_Inarray *_inarray_obj_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inarray_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray *arr)
{
   Eina_Bool r = _inarray_obj_equal(arr, base_seq_obj, base_seq_obj_size);
   if (!r) return r;
   eina_inarray_push(arr, _new_obj_ref(42));
   eina_inarray_push(arr, _new_obj_ref(43));
   eina_inarray_push(arr, _new_obj_ref(33));
   _inarray_obj_in_own_to_check = arr;
   return r;
}
Eina_Bool _dummy_test_object_check_eina_inarray_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_obj_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_obj_equal(_inarray_obj_in_own_to_check, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   Dummy_Numberwrapper **ele;
   EINA_INARRAY_FOREACH(_inarray_obj_in_own_to_check, ele)
     efl_unref(*ele);

   eina_inarray_free(_inarray_obj_in_own_to_check);
   _inarray_obj_in_own_to_check = NULL;
   return r;
}

Eina_Inarray *_inarray_obj_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inarray_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_inarray_new(sizeof(Eo*), 0);
   eina_inarray_push(*arr, _new_obj_ref(0x0));
   eina_inarray_push(*arr, _new_obj_ref(0x2A));
   eina_inarray_push(*arr, _new_obj_ref(0x42));
   _inarray_obj_out_to_check = *arr;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_inarray_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_obj_out_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_obj_equal(_inarray_obj_out_to_check, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   Dummy_Numberwrapper **ele;
   EINA_INARRAY_FOREACH(_inarray_obj_out_to_check, ele)
     efl_unref(*ele);

   eina_inarray_free(_inarray_obj_out_to_check);
   _inarray_obj_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_inarray_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray **arr)
{
   if (!arr) return EINA_FALSE;
   *arr = eina_inarray_new(sizeof(Eo*), 0);
   eina_inarray_push(*arr, _new_obj_ref(0x0));
   eina_inarray_push(*arr, _new_obj_ref(0x2A));
   eina_inarray_push(*arr, _new_obj_ref(0x42));
   return EINA_TRUE;
}

Eina_Inarray *_inarray_obj_return_to_check = NULL;

Eina_Inarray *_dummy_test_object_eina_inarray_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inarray *arr = eina_inarray_new(sizeof(Eo*), 0);
   eina_inarray_push(arr, _new_obj_ref(0x0));
   eina_inarray_push(arr, _new_obj_ref(0x2A));
   eina_inarray_push(arr, _new_obj_ref(0x42));
   _inarray_obj_return_to_check = arr;
   return arr;
}
Eina_Bool _dummy_test_object_check_eina_inarray_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_inarray_obj_return_to_check) return EINA_FALSE;

   Eina_Bool r = _inarray_obj_equal(_inarray_obj_return_to_check, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   Dummy_Numberwrapper **ele;
   EINA_INARRAY_FOREACH(_inarray_obj_return_to_check, ele)
     efl_unref(*ele);

   eina_inarray_free(_inarray_obj_return_to_check);
   _inarray_obj_return_to_check = NULL;
   return r;
}

Eina_Inarray *_dummy_test_object_eina_inarray_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inarray *arr = eina_inarray_new(sizeof(Eo*), 0);
   eina_inarray_push(arr, _new_obj_ref(0x0));
   eina_inarray_push(arr, _new_obj_ref(0x2A));
   eina_inarray_push(arr, _new_obj_ref(0x42));
   return arr;
}

Eina_Inarray *_dummy_test_object_eina_inarray_obj_return_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inarray *arr)
{
   return arr;
}

// //
// List
//

// Integer

Eina_Bool _list_int_equal(const Eina_List *lst, const int base[], unsigned int len)
{
   if (eina_list_count(lst) != len)
     return EINA_FALSE;

   const Eina_List *l;
   int *data;
   int i = 0;
   EINA_LIST_FOREACH(lst, l, data)
     {
        if (*data != base[i])
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_list_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_int_equal(lst, base_seq_int, base_seq_int_size);
   return r;
}

static Eina_List *_list_int_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_list_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_int_equal(lst, base_seq_int, base_seq_int_size);
   if (!r) return r;

   lst = eina_list_append(lst, _new_int(42));
   lst = eina_list_append(lst, _new_int(43));
   lst = eina_list_append(lst, _new_int(33));
   _list_int_in_own_to_check = lst;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_list_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_int_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _list_int_equal(_list_int_in_own_to_check, modified_seq_int, modified_seq_int_size);
   if (!r) return r;

   int *ele;
   EINA_LIST_FREE(_list_int_in_own_to_check, ele)
     free(ele);
   _list_int_in_own_to_check = NULL;
   return r;
}

Eina_List *_list_int_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_list_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List **lst)
{
    if (!lst) return EINA_FALSE;
    *lst = eina_list_append(*lst, _new_int(0x0));
    *lst = eina_list_append(*lst, _new_int(0x2A));
    *lst = eina_list_append(*lst, _new_int(0x42));
    _list_int_out_to_check = *lst;
    return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_list_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_int_out_to_check) return EINA_FALSE;

   Eina_Bool r = _list_int_equal(_list_int_out_to_check, base_seq_int, base_seq_int_size);
   if (!r) return r;

   int *ele;
   EINA_LIST_FREE(_list_int_out_to_check, ele)
     free(ele);

   _list_int_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_list_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, _new_int(0x0));
   *lst = eina_list_append(*lst, _new_int(0x2A));
   *lst = eina_list_append(*lst, _new_int(0x42));
   return EINA_TRUE;
}

Eina_List *_list_int_return_to_check = NULL;

Eina_List *_dummy_test_object_eina_list_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_int(0x0));
   lst = eina_list_append(lst, _new_int(0x2A));
   lst = eina_list_append(lst, _new_int(0x42));
   _list_int_return_to_check = lst;
   return lst;
}
Eina_Bool _dummy_test_object_check_eina_list_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_int_return_to_check) return EINA_FALSE;

   Eina_Bool r = _list_int_equal(_list_int_return_to_check, base_seq_int, base_seq_int_size);
   if (!r) return r;

   int *ele;
   EINA_LIST_FREE(_list_int_return_to_check, ele)
     free(ele);

   _list_int_return_to_check = NULL;
   return r;
}

Eina_List *_dummy_test_object_eina_list_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_int(0x0));
   lst = eina_list_append(lst, _new_int(0x2A));
   lst = eina_list_append(lst, _new_int(0x42));
   return lst;
}

// String

Eina_Bool _list_str_equal(const Eina_List *lst, const char * const base[], unsigned int len)
{
   if (eina_list_count(lst) != len)
     return EINA_FALSE;

   const Eina_List *l;
   char *data;
   int i = 0;
   EINA_LIST_FOREACH(lst, l, data)
     {
        if (0 != strcmp(data, base[i]))
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_list_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_str_equal(lst, base_seq_str, base_seq_str_size);
   return r;
}

static Eina_List *_list_str_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_list_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_str_equal(lst, base_seq_str, base_seq_str_size);
   if (!r) return r;
   lst = eina_list_append(lst, strdup("42"));
   lst = eina_list_append(lst, strdup("43"));
   lst = eina_list_append(lst, strdup("33"));
   _list_str_in_own_to_check = lst;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_list_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_str_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _list_str_equal(_list_str_in_own_to_check, modified_seq_str, modified_seq_str_size);
   if (!r) return r;

   char *ele;
   EINA_LIST_FREE(_list_str_in_own_to_check, ele)
     free(ele);

   _list_str_in_own_to_check = NULL;
   return r;
}

Eina_List *_list_str_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_list_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, strdup("0x0"));
   *lst = eina_list_append(*lst, strdup("0x2A"));
   *lst = eina_list_append(*lst, strdup("0x42"));
   _list_str_out_to_check = *lst;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_list_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_str_out_to_check) return EINA_FALSE;

   Eina_Bool r = _list_str_equal(_list_str_out_to_check, base_seq_str, base_seq_str_size);
   if (!r) return r;

   char *ele;
   EINA_LIST_FREE(_list_str_out_to_check, ele)
     free(ele);

   _list_str_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_list_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, strdup("0x0"));
   *lst = eina_list_append(*lst, strdup("0x2A"));
   *lst = eina_list_append(*lst, strdup("0x42"));
   return EINA_TRUE;
}

Eina_List *_list_str_return_to_check = NULL;

Eina_List *_dummy_test_object_eina_list_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, strdup("0x0"));
   lst = eina_list_append(lst, strdup("0x2A"));
   lst = eina_list_append(lst, strdup("0x42"));
   _list_str_return_to_check = lst;
   return lst;
}
Eina_Bool _dummy_test_object_check_eina_list_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_str_return_to_check) return EINA_FALSE;

   Eina_Bool r = _list_str_equal(_list_str_return_to_check, base_seq_str, base_seq_str_size);
   if (!r) return r;

   char *ele;
   EINA_LIST_FREE(_list_str_return_to_check, ele)
     free(ele);

   _list_str_return_to_check = NULL;
   return r;
}

Eina_List *_dummy_test_object_eina_list_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, strdup("0x0"));
   lst = eina_list_append(lst, strdup("0x2A"));
   lst = eina_list_append(lst, strdup("0x42"));
   return lst;
}

// Object

Eina_Bool _list_obj_equal(const Eina_List *lst, const Dummy_Numberwrapper * const base[], unsigned int len)
{
   if (eina_list_count(lst) != len)
     return EINA_FALSE;

   const Eina_List *l;
   Dummy_Numberwrapper *eo;
   int i = 0;
   EINA_LIST_FOREACH(lst, l, eo)
     {
        int a = dummy_numberwrapper_number_get(eo);
        int b = dummy_numberwrapper_number_get(base[i]);
        if (a != b)
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_list_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_obj_equal(lst, base_seq_obj, base_seq_obj_size);
   return r;
}

static Eina_List *_list_obj_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_list_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List *lst)
{
   Eina_Bool r = _list_obj_equal(lst, base_seq_obj, base_seq_obj_size);
   if (!r) return r;
   lst = eina_list_append(lst, _new_obj(42));
   lst = eina_list_append(lst, _new_obj(43));
   lst = eina_list_append(lst, _new_obj(33));
   _list_obj_in_own_to_check = lst;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_list_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_obj_in_own_to_check) return EINA_FALSE;

   Eina_Bool r = _list_obj_equal(_list_obj_in_own_to_check, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   Dummy_Numberwrapper *ele;
   EINA_LIST_FREE(_list_obj_in_own_to_check, ele)
     efl_unref(ele);

   _list_obj_in_own_to_check = NULL;
   return r;
}

Eina_List *_list_obj_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_list_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, _new_obj(0x0));
   *lst = eina_list_append(*lst, _new_obj(0x2A));
   *lst = eina_list_append(*lst, _new_obj(0x42));
   _list_obj_out_to_check = *lst;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_list_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_obj_out_to_check) return EINA_FALSE;

   Eina_Bool r = _list_obj_equal(_list_obj_out_to_check, base_seq_obj, base_seq_obj_size);
   if (!r) return r;

   Dummy_Numberwrapper *ele;
   EINA_LIST_FREE(_list_obj_out_to_check, ele)
     efl_unref(ele);

   _list_obj_out_to_check = NULL;
   return r;
}

Eina_Bool _dummy_test_object_eina_list_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_list_append(*lst, _new_obj(0x0));
   *lst = eina_list_append(*lst, _new_obj(0x2A));
   *lst = eina_list_append(*lst, _new_obj(0x42));
   return EINA_TRUE;
}

Eina_List *_list_obj_return_to_check = NULL;

Eina_List *_dummy_test_object_eina_list_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_obj(0x0));
   lst = eina_list_append(lst, _new_obj(0x2A));
   lst = eina_list_append(lst, _new_obj(0x42));
   _list_obj_return_to_check = lst;
   return lst;
}
Eina_Bool _dummy_test_object_check_eina_list_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_list_obj_return_to_check) return EINA_FALSE;

   Eina_Bool r = _list_obj_equal(_list_obj_return_to_check, base_seq_obj, base_seq_obj_size);
   if (!r) return r;

   Dummy_Numberwrapper *ele;
   EINA_LIST_FREE(_list_obj_return_to_check, ele)
     efl_unref(ele);

   _list_obj_return_to_check = NULL;
   return r;
}

Eina_List *_dummy_test_object_eina_list_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_List *lst = NULL;
   lst = eina_list_append(lst, _new_obj(0x0));
   lst = eina_list_append(lst, _new_obj(0x2A));
   lst = eina_list_append(lst, _new_obj(0x42));
   return lst;
}

Eina_List *_dummy_test_object_eina_list_obj_return_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_List *lst)
{
   return lst;
}


// //
// Inlist
//

// Integer

typedef struct _Dummy_Inlist_Node_Int
{
   EINA_INLIST;
   int val;
} Dummy_Inlist_Node_Int;


Eina_Inlist *_new_inlist_int(int v)
{
   Dummy_Inlist_Node_Int *node = malloc(sizeof(Dummy_Inlist_Node_Int));
   node->val = v;
   return EINA_INLIST_GET(node);
}

Eina_Bool _inlist_int_equal(const Eina_Inlist *lst, const int base[], unsigned int len)
{
   if (eina_inlist_count(lst) != len)
     return EINA_FALSE;

   const Dummy_Inlist_Node_Int *node;
   int i = 0;
   EINA_INLIST_FOREACH(lst, node)
     {
        if (node->val != base[i])
          return EINA_FALSE;
        ++i;
     }

   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_inlist_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist *lst)
{
   Eina_Bool r = _inlist_int_equal(lst, base_seq_int, base_seq_int_size);
   return r;
}

static Eina_Inlist *_inlist_int_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inlist_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist *lst)
{
   Eina_Bool r = _inlist_int_equal(lst, base_seq_int, base_seq_int_size);
   if (!r) return r;

   lst = eina_inlist_append(lst, _new_inlist_int(42));
   lst = eina_inlist_append(lst, _new_inlist_int(43));
   lst = eina_inlist_append(lst, _new_inlist_int(33));
   _inlist_int_in_own_to_check = lst;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_inlist_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_int_in_own_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_int_in_own_to_check = NULL;

   Eina_Bool r = _inlist_int_equal(lst, modified_seq_int, modified_seq_int_size);
   if (!r) return r;

   Dummy_Inlist_Node_Int *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        free(node);
     }
   return r;
}

Eina_Inlist *_inlist_int_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inlist_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist **lst)
{
    if (!lst) return EINA_FALSE;
    *lst = eina_inlist_append(*lst, _new_inlist_int(0x0));
    *lst = eina_inlist_append(*lst, _new_inlist_int(0x2A));
    *lst = eina_inlist_append(*lst, _new_inlist_int(0x42));
    _inlist_int_out_to_check = *lst;
    return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_inlist_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_int_out_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_int_out_to_check = NULL;

   Eina_Bool r = _inlist_int_equal(lst, base_seq_int, base_seq_int_size);
   if (!r) return r;

   Dummy_Inlist_Node_Int *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        free(node);
     }

   return r;
}

Eina_Bool _dummy_test_object_eina_inlist_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_inlist_append(*lst, _new_inlist_int(0x0));
   *lst = eina_inlist_append(*lst, _new_inlist_int(0x2A));
   *lst = eina_inlist_append(*lst, _new_inlist_int(0x42));
   return EINA_TRUE;
}

Eina_Inlist *_inlist_int_return_to_check = NULL;

Eina_Inlist *_dummy_test_object_eina_inlist_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = NULL;
   lst = eina_inlist_append(lst, _new_inlist_int(0x0));
   lst = eina_inlist_append(lst, _new_inlist_int(0x2A));
   lst = eina_inlist_append(lst, _new_inlist_int(0x42));
   _inlist_int_return_to_check = lst;
   return lst;
}
Eina_Bool _dummy_test_object_check_eina_inlist_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_int_return_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_int_return_to_check = NULL;

   Eina_Bool r = _inlist_int_equal(lst, base_seq_int, base_seq_int_size);
   if (!r) return r;

   Dummy_Inlist_Node_Int *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        free(node);
     }

   return r;
}

Eina_Inlist *_dummy_test_object_eina_inlist_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = NULL;
   lst = eina_inlist_append(lst, _new_inlist_int(0x0));
   lst = eina_inlist_append(lst, _new_inlist_int(0x2A));
   lst = eina_inlist_append(lst, _new_inlist_int(0x42));
   return lst;
}

// String

typedef struct _Dummy_Inlist_Node_Str
{
   EINA_INLIST;
   char *val;
} Dummy_Inlist_Node_Str;


Eina_Inlist *_new_inlist_str(const char *v)
{
   Dummy_Inlist_Node_Str *node = malloc(sizeof(Dummy_Inlist_Node_Str));
   node->val = strdup(v);
   return EINA_INLIST_GET(node);
}

Eina_Bool _inlist_str_equal(const Eina_Inlist *lst, const char * const base[], unsigned int len)
{
   if (eina_inlist_count(lst) != len)
     return EINA_FALSE;

   const Dummy_Inlist_Node_Str *node;
   int i = 0;
   EINA_INLIST_FOREACH(lst, node)
     {
        if (0 != strcmp(node->val, base[i]))
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_inlist_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist *lst)
{
   Eina_Bool r = _inlist_str_equal(lst, base_seq_str, base_seq_str_size);
   return r;
}

static Eina_Inlist *_inlist_str_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inlist_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist *lst)
{
   Eina_Bool r = _inlist_str_equal(lst, base_seq_str, base_seq_str_size);
   if (!r) return r;
   lst = eina_inlist_append(lst, _new_inlist_str("42"));
   lst = eina_inlist_append(lst, _new_inlist_str("43"));
   lst = eina_inlist_append(lst, _new_inlist_str("33"));
   _inlist_str_in_own_to_check = lst;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_inlist_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_str_in_own_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_str_in_own_to_check = NULL;

   Eina_Bool r = _inlist_str_equal(lst, modified_seq_str, modified_seq_str_size);
   if (!r) return r;

   Dummy_Inlist_Node_Str *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        free(node->val);
        free(node);
     }

   return r;
}

Eina_Inlist *_inlist_str_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inlist_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_inlist_append(*lst, _new_inlist_str("0x0"));
   *lst = eina_inlist_append(*lst, _new_inlist_str("0x2A"));
   *lst = eina_inlist_append(*lst, _new_inlist_str("0x42"));
   _inlist_str_out_to_check = *lst;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_inlist_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_str_out_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_str_out_to_check = NULL;

   Eina_Bool r = _inlist_str_equal(lst, base_seq_str, base_seq_str_size);
   if (!r) return r;

   Dummy_Inlist_Node_Str *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        free(node->val);
        free(node);
     }

   return r;
}

Eina_Bool _dummy_test_object_eina_inlist_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_inlist_append(*lst, _new_inlist_str("0x0"));
   *lst = eina_inlist_append(*lst, _new_inlist_str("0x2A"));
   *lst = eina_inlist_append(*lst, _new_inlist_str("0x42"));
   return EINA_TRUE;
}

Eina_Inlist *_inlist_str_return_to_check = NULL;

Eina_Inlist *_dummy_test_object_eina_inlist_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = NULL;
   lst = eina_inlist_append(lst, _new_inlist_str("0x0"));
   lst = eina_inlist_append(lst, _new_inlist_str("0x2A"));
   lst = eina_inlist_append(lst, _new_inlist_str("0x42"));
   _inlist_str_return_to_check = lst;
   return lst;
}
Eina_Bool _dummy_test_object_check_eina_inlist_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_str_return_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_str_return_to_check = NULL;

   Eina_Bool r = _inlist_str_equal(lst, base_seq_str, base_seq_str_size);
   if (!r) return r;

   Dummy_Inlist_Node_Str *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        free(node->val);
        free(node);
     }

   return r;
}

Eina_Inlist *_dummy_test_object_eina_inlist_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = NULL;
   lst = eina_inlist_append(lst, _new_inlist_str("0x0"));
   lst = eina_inlist_append(lst, _new_inlist_str("0x2A"));
   lst = eina_inlist_append(lst, _new_inlist_str("0x42"));
   return lst;
}

// Object

typedef struct _Dummy_Inlist_Node_Obj
{
   EINA_INLIST;
   Dummy_Numberwrapper *val;
} Dummy_Inlist_Node_Obj;


Eina_Inlist *_new_inlist_obj(int v)
{
   Dummy_Inlist_Node_Obj *node = malloc(sizeof(Dummy_Inlist_Node_Obj));
   node->val = _new_obj(v);
   return EINA_INLIST_GET(node);
}

Eina_Bool _inlist_obj_equal(const Eina_Inlist *lst, const Dummy_Numberwrapper * const base[], unsigned int len)
{
   if (eina_inlist_count(lst) != len)
     return EINA_FALSE;

   const Dummy_Inlist_Node_Obj *node;
   int i = 0;
   EINA_INLIST_FOREACH(lst, node)
     {
        int a = dummy_numberwrapper_number_get(node->val);
        int b = dummy_numberwrapper_number_get(base[i]);
        if (a != b)
          return EINA_FALSE;
        ++i;
     }
   return EINA_TRUE;
}

Eina_Bool _dummy_test_object_eina_inlist_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist *lst)
{
   Eina_Bool r = _inlist_obj_equal(lst, base_seq_obj, base_seq_obj_size);
   return r;
}

static Eina_Inlist *_inlist_obj_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inlist_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist *lst)
{
   Eina_Bool r = _inlist_obj_equal(lst, base_seq_obj, base_seq_obj_size);
   if (!r) return r;
   lst = eina_inlist_append(lst, _new_inlist_obj(42));
   lst = eina_inlist_append(lst, _new_inlist_obj(43));
   lst = eina_inlist_append(lst, _new_inlist_obj(33));
   _inlist_obj_in_own_to_check = lst;
   return r;
}

Eina_Bool _dummy_test_object_check_eina_inlist_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_obj_in_own_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_obj_in_own_to_check = NULL;

   Eina_Bool r = _inlist_obj_equal(lst, modified_seq_obj, modified_seq_obj_size);
   if (!r) return r;

   Dummy_Inlist_Node_Obj *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        efl_unref(node->val);
        free(node);
     }

   return r;
}

Eina_Inlist *_inlist_obj_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_inlist_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_inlist_append(*lst, _new_inlist_obj(0x0));
   *lst = eina_inlist_append(*lst, _new_inlist_obj(0x2A));
   *lst = eina_inlist_append(*lst, _new_inlist_obj(0x42));
   _inlist_obj_out_to_check = *lst;
   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_inlist_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_obj_out_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_obj_out_to_check = NULL;

   Eina_Bool r = _inlist_obj_equal(lst, base_seq_obj, base_seq_obj_size);
   if (!r) return r;

   Dummy_Inlist_Node_Obj *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        efl_unref(node->val);
        free(node);
     }

   return r;
}

Eina_Bool _dummy_test_object_eina_inlist_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist **lst)
{
   if (!lst) return EINA_FALSE;
   *lst = eina_inlist_append(*lst, _new_inlist_obj(0x0));
   *lst = eina_inlist_append(*lst, _new_inlist_obj(0x2A));
   *lst = eina_inlist_append(*lst, _new_inlist_obj(0x42));
   return EINA_TRUE;
}

Eina_Inlist *_inlist_obj_return_to_check = NULL;

Eina_Inlist *_dummy_test_object_eina_inlist_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = NULL;
   lst = eina_inlist_append(lst, _new_inlist_obj(0x0));
   lst = eina_inlist_append(lst, _new_inlist_obj(0x2A));
   lst = eina_inlist_append(lst, _new_inlist_obj(0x42));
   _inlist_obj_return_to_check = lst;
   return lst;
}
Eina_Bool _dummy_test_object_check_eina_inlist_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = _inlist_obj_return_to_check;
   if (!lst) return EINA_FALSE;
   _inlist_obj_return_to_check = NULL;

   Eina_Bool r = _inlist_obj_equal(lst, base_seq_obj, base_seq_obj_size);
   if (!r) return r;

   Dummy_Inlist_Node_Obj *node;
   EINA_INLIST_FREE(lst, node)
     {
        lst = eina_inlist_remove(lst, EINA_INLIST_GET(node));
        efl_unref(node->val);
        free(node);
     }

   return r;
}

Eina_Inlist *_dummy_test_object_eina_inlist_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Inlist *lst = NULL;
   lst = eina_inlist_append(lst, _new_inlist_obj(0x0));
   lst = eina_inlist_append(lst, _new_inlist_obj(0x2A));
   lst = eina_inlist_append(lst, _new_inlist_obj(0x42));
   return lst;
}

Eina_Inlist *_dummy_test_object_eina_inlist_obj_return_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Inlist *lst)
{
   return lst;
}


//      //
// Hash //
//      //

// Integer //

Eina_Bool _hash_int_check(const Eina_Hash *hsh, int key, int expected_val)
{
   int *val = eina_hash_find(hsh, &key);
   return val && (*val == expected_val);
}


// int in

Eina_Bool _dummy_test_object_eina_hash_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash *hsh)
{
   if (!_hash_int_check(hsh, 22, 222))
     return EINA_FALSE;

   int key = 44;
   return eina_hash_add(hsh, &key, _new_int(444));
}


// int in own

static Eina_Bool _hash_int_in_own_free_flag = EINA_FALSE;
static void _hash_int_in_own_free_cb(void *data)
{
   _hash_int_in_own_free_flag = EINA_TRUE;
   free(data);
}
static Eina_Hash *_hash_int_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_hash_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash *hsh)
{
   eina_hash_free_cb_set(hsh, _hash_int_in_own_free_cb);

   if (!_hash_int_check(hsh, 22, 222))
     return EINA_FALSE;

   _hash_int_in_own_to_check = hsh;

   int key = 44;
   return eina_hash_add(hsh, &key, _new_int(444));
}
Eina_Bool _dummy_test_object_check_eina_hash_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_hash_int_in_own_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_int_in_own_to_check;

   if (!_hash_int_check(hsh, 22, 222)
       || !_hash_int_check(hsh, 44, 444)
       || !_hash_int_check(hsh, 88, 888))
     return EINA_FALSE;

   eina_hash_free(hsh);

   return _hash_int_in_own_free_flag;
}


// int out

static Eina_Bool _hash_int_out_free_flag = EINA_FALSE;
static void _hash_int_out_free_cb(void *data)
{
   _hash_int_out_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_int_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_hash_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_int32_new(_hash_int_out_free_cb);

   _hash_int_out_to_check = *hsh;

   int key = 22;
   return eina_hash_add(*hsh, &key, _new_int(222));
}
Eina_Bool _dummy_test_object_check_eina_hash_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_hash_int_out_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_int_out_to_check;

   if (!_hash_int_check(hsh, 22, 222)
       || !_hash_int_check(hsh, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_int_out_to_check = NULL;
   return _hash_int_out_free_flag;
}


// int out own

static Eina_Bool _hash_int_out_own_free_flag = EINA_FALSE;
static void _hash_int_out_own_free_cb(void *data)
{
   _hash_int_out_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Bool _dummy_test_object_eina_hash_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_int32_new(_hash_int_out_own_free_cb);

   int key = 22;
   return eina_hash_add(*hsh, &key, _new_int(222));
}
Eina_Bool _dummy_test_object_check_eina_hash_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   return !_hash_int_out_own_free_flag;
}


// int return

static Eina_Bool _hash_int_return_free_flag = EINA_FALSE;
static void _hash_int_return_free_cb(void *data)
{
   _hash_int_return_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_int_return_to_check = NULL;

Eina_Hash *_dummy_test_object_eina_hash_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Hash *hsh = eina_hash_int32_new(_hash_int_return_free_cb);

   int key = 22;
   eina_hash_add(hsh, &key, _new_int(222));

   _hash_int_return_to_check = hsh;

   return hsh;
}
Eina_Bool _dummy_test_object_check_eina_hash_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_hash_int_return_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_int_return_to_check;

   if (!_hash_int_check(hsh, 22, 222)
       || !_hash_int_check(hsh, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_int_return_to_check = NULL;
   return _hash_int_return_free_flag;
}


// int return own

static Eina_Bool _hash_int_return_own_free_flag = EINA_FALSE;
static void _hash_int_return_own_free_cb(void *data)
{
   _hash_int_return_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_dummy_test_object_eina_hash_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Hash *hsh = eina_hash_int32_new(_hash_int_return_own_free_cb);

   int key = 22;
   eina_hash_add(hsh, &key, _new_int(222));

   return hsh;
}
Eina_Bool _dummy_test_object_check_eina_hash_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   return !_hash_int_return_own_free_flag;
}


// String //

Eina_Bool _hash_str_check(const Eina_Hash *hsh, const char *key, const char *expected_val)
{
   const char *val = eina_hash_find(hsh, key);
   return val && 0 == strcmp(val, expected_val);
}


// str in

Eina_Bool _dummy_test_object_eina_hash_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash *hsh)
{
   if (!_hash_str_check(hsh, "aa", "aaa"))
     return EINA_FALSE;

   return eina_hash_add(hsh, "bb", strdup("bbb"));
}


// str in own

static Eina_Bool _hash_str_in_own_free_flag = EINA_FALSE;
static void _hash_str_in_own_free_cb(void *data)
{
   _hash_str_in_own_free_flag = EINA_TRUE;
   free(data);
}
static Eina_Hash *_hash_str_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_hash_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash *hsh)
{
   eina_hash_free_cb_set(hsh, _hash_str_in_own_free_cb);

   if (!_hash_str_check(hsh, "aa", "aaa"))
     return EINA_FALSE;

   _hash_str_in_own_to_check = hsh;

   return eina_hash_add(hsh, "bb", strdup("bbb"));
}
Eina_Bool _dummy_test_object_check_eina_hash_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_hash_str_in_own_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_str_in_own_to_check;

   if (!_hash_str_check(hsh, "aa", "aaa")
       || !_hash_str_check(hsh, "bb", "bbb")
       || !_hash_str_check(hsh, "cc", "ccc"))
     return EINA_FALSE;

   eina_hash_free(hsh);

   return _hash_str_in_own_free_flag;
}


// str out

static Eina_Bool _hash_str_out_free_flag = EINA_FALSE;
static void _hash_str_out_free_cb(void *data)
{
   _hash_str_out_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_str_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_hash_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_string_superfast_new(_hash_str_out_free_cb);

   _hash_str_out_to_check = *hsh;

   return eina_hash_add(*hsh, "aa", strdup("aaa"));
}
Eina_Bool _dummy_test_object_check_eina_hash_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_hash_str_out_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_str_out_to_check;

   if (!_hash_str_check(hsh, "aa", "aaa")
       || !_hash_str_check(hsh, "bb", "bbb"))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_str_out_to_check = NULL;
   return _hash_str_out_free_flag;
}


// str out own

static Eina_Bool _hash_str_out_own_free_flag = EINA_FALSE;
static void _hash_str_out_own_free_cb(void *data)
{
   _hash_str_out_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Bool _dummy_test_object_eina_hash_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash **hsh)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_string_superfast_new(_hash_str_out_own_free_cb);

   return eina_hash_add(*hsh, "aa", strdup("aaa"));
}
Eina_Bool _dummy_test_object_check_eina_hash_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   return !_hash_str_out_own_free_flag;
}


// str return

static Eina_Bool _hash_str_return_free_flag = EINA_FALSE;
static void _hash_str_return_free_cb(void *data)
{
   _hash_str_return_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_hash_str_return_to_check = NULL;

Eina_Hash *_dummy_test_object_eina_hash_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Hash *hsh = eina_hash_string_superfast_new(_hash_str_return_free_cb);

   eina_hash_add(hsh, "aa", strdup("aaa"));

   _hash_str_return_to_check = hsh;

   return hsh;
}
Eina_Bool _dummy_test_object_check_eina_hash_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   if (!_hash_str_return_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_str_return_to_check;

   if (!_hash_str_check(hsh, "aa", "aaa")
       || !_hash_str_check(hsh, "bb", "bbb"))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_str_return_to_check = NULL;
   return _hash_str_return_free_flag;
}


// str return own

static Eina_Bool _hash_str_return_own_free_flag = EINA_FALSE;
static void _hash_str_return_own_free_cb(void *data)
{
   _hash_str_return_own_free_flag = EINA_TRUE;
   free(data);
}
Eina_Hash *_dummy_test_object_eina_hash_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Hash *hsh = eina_hash_string_superfast_new(_hash_str_return_own_free_cb);

   eina_hash_add(hsh, "aa", strdup("aaa"));

   return hsh;
}
Eina_Bool _dummy_test_object_check_eina_hash_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   return !_hash_str_return_own_free_flag;
}


// Object //

Eina_Bool _hash_obj_check(const Eina_Hash *hsh, Dummy_Numberwrapper *key, Dummy_Numberwrapper *expected_val, int knum, int vnum)
{
   Dummy_Numberwrapper *val = eina_hash_find(hsh, &key);
   return val && (val == expected_val) && (dummy_numberwrapper_number_get(key) == knum) && (dummy_numberwrapper_number_get(val) == vnum);
}


// obj in

Eina_Bool _dummy_test_object_eina_hash_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash *hsh, Dummy_Numberwrapper *nwk1, Dummy_Numberwrapper *nwv1, Dummy_Numberwrapper **nwk2, Dummy_Numberwrapper **nwv2)
{
   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222))
     return EINA_FALSE;

   *nwk2 = _new_obj(44);
   *nwv2 = _new_obj(444);

   return eina_hash_add(hsh, nwk2, *nwv2);
}


// obj in own

static Eina_Bool _hash_obj_in_own_free_flag = EINA_FALSE;
static void _hash_obj_in_own_free_cb(void *data)
{
   _hash_obj_in_own_free_flag = EINA_TRUE;
   efl_unref(data);
}
static Eina_Hash *_hash_obj_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_hash_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash *hsh, Dummy_Numberwrapper *nwk1, Dummy_Numberwrapper *nwv1, Dummy_Numberwrapper **nwk2, Dummy_Numberwrapper **nwv2)
{
   eina_hash_free_cb_set(hsh, _hash_obj_in_own_free_cb);

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222))
     return EINA_FALSE;

   _hash_obj_in_own_to_check = hsh;

   *nwk2 = _new_obj(44);
   *nwv2 = _new_obj(444);

   return eina_hash_add(hsh, nwk2, *nwv2);
}
Eina_Bool _dummy_test_object_check_eina_hash_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_Numberwrapper *nwk1, Dummy_Numberwrapper *nwv1, Dummy_Numberwrapper *nwk2, Dummy_Numberwrapper *nwv2)
{
   if (!_hash_obj_in_own_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_obj_in_own_to_check;

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222)
       || !_hash_obj_check(hsh, nwk2, nwv2, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   return _hash_obj_in_own_free_flag;
}


// obj out

static Eina_Bool _hash_obj_out_free_flag = EINA_FALSE;
static void _hash_obj_out_free_cb(void *data)
{
   _hash_obj_out_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Hash *_hash_obj_out_to_check = NULL;

Eina_Bool _dummy_test_object_eina_hash_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash **hsh, Dummy_Numberwrapper **nwk, Dummy_Numberwrapper **nwv)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_pointer_new(_hash_obj_out_free_cb);

   _hash_obj_out_to_check = *hsh;

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);
   return eina_hash_add(*hsh, nwk, *nwv);
}
Eina_Bool _dummy_test_object_check_eina_hash_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_Numberwrapper *nwk1, Dummy_Numberwrapper *nwv1, Dummy_Numberwrapper *nwk2, Dummy_Numberwrapper *nwv2)
{
   if (!_hash_obj_out_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_obj_out_to_check;

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222)
       || !_hash_obj_check(hsh, nwk2, nwv2, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_obj_out_to_check = NULL;
   return _hash_obj_out_free_flag;
}


// obj out own

static Eina_Bool _hash_obj_out_own_free_flag = EINA_FALSE;
static void _hash_obj_out_own_free_cb(void *data)
{
   _hash_obj_out_own_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Bool _dummy_test_object_eina_hash_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Hash **hsh, Dummy_Numberwrapper **nwk, Dummy_Numberwrapper **nwv)
{
   if (!hsh) return EINA_FALSE;

   *hsh = eina_hash_pointer_new(_hash_obj_out_own_free_cb);

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);
   return eina_hash_add(*hsh, nwk, *nwv);
}
Eina_Bool _dummy_test_object_check_eina_hash_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   return !_hash_obj_out_own_free_flag;
}


// obj return

static Eina_Bool _hash_obj_return_free_flag = EINA_FALSE;
static void _hash_obj_return_free_cb(void *data)
{
   _hash_obj_return_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Hash *_hash_obj_return_to_check = NULL;

Eina_Hash *_dummy_test_object_eina_hash_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_Numberwrapper **nwk, Dummy_Numberwrapper **nwv)
{
   Eina_Hash *hsh = eina_hash_pointer_new(_hash_obj_return_free_cb);

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);

   eina_hash_add(hsh, nwk, *nwv);

   _hash_obj_return_to_check = hsh;

   return hsh;
}
Eina_Bool _dummy_test_object_check_eina_hash_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_Numberwrapper *nwk1, Dummy_Numberwrapper *nwv1, Dummy_Numberwrapper *nwk2, Dummy_Numberwrapper *nwv2)
{
   if (!_hash_obj_return_to_check) return EINA_FALSE;

   Eina_Hash *hsh = _hash_obj_return_to_check;

   if (!_hash_obj_check(hsh, nwk1, nwv1, 22, 222)
       || !_hash_obj_check(hsh, nwk2, nwv2, 44, 444))
     return EINA_FALSE;

   eina_hash_free(hsh);

   _hash_obj_return_to_check = NULL;
   return _hash_obj_return_free_flag;
}


// obj return own

static Eina_Bool _hash_obj_return_own_free_flag = EINA_FALSE;
static void _hash_obj_return_own_free_cb(void *data)
{
   _hash_obj_return_own_free_flag = EINA_TRUE;
   efl_unref(data);
}
Eina_Hash *_dummy_test_object_eina_hash_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_Numberwrapper **nwk, Dummy_Numberwrapper **nwv)
{
   Eina_Hash *hsh = eina_hash_pointer_new(_hash_obj_return_own_free_cb);

   *nwk = _new_obj(22);
   *nwv = _new_obj(222);
   eina_hash_add(hsh, nwk, *nwv);

   return hsh;
}
Eina_Bool _dummy_test_object_check_eina_hash_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   return !_hash_obj_return_own_free_flag;
}


//          //
// Iterator //
//          //

// Integer //

Eina_Bool _iterator_int_equal(Eina_Iterator *itr, const int base[], unsigned int len, Eina_Bool release)
{
   int *data;
   unsigned i = 0;
   EINA_ITERATOR_FOREACH(itr, data)
     {
        if (*data != base[i])
          return EINA_FALSE;
        if (release)
          free(data);
        ++i;
     }

   if (i != len)
     return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Array *_iterator_int_eina_array_new()
{
   Eina_Array *arr = eina_array_new(32);
   for (unsigned i = 0; i < base_seq_int_size; ++i)
     {
        eina_array_push(arr, _new_int(base_seq_int[i]));
     }
   return arr;
}

Eina_Bool _iterator_int_test_array(Eina_Array *arr)
{
   if (eina_array_count(arr) != base_seq_int_size)
     return EINA_FALSE;

   for (unsigned i = 0; i < base_seq_int_size; ++i)
     {
        int *data = eina_array_data_get(arr, i);
        if (*data != base_seq_int[i])
          return EINA_FALSE;
        free(data);
     }

   eina_array_free(arr);

   return EINA_TRUE;
}

// <int> in

Eina_Bool _dummy_test_object_eina_iterator_int_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator *itr)
{
   Eina_Bool r = _iterator_int_equal(itr, base_seq_int, base_seq_int_size, EINA_FALSE);
   return r;
}

// <int> in own

static Eina_Iterator *_iterator_int_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_iterator_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator *itr)
{
   Eina_Bool r = _iterator_int_equal(itr, base_seq_int, base_seq_int_size, EINA_TRUE);
   _iterator_int_in_own_to_check = itr;
   return r;
}
Eina_Bool _dummy_test_object_check_eina_iterator_int_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   eina_iterator_free(_iterator_int_in_own_to_check);
   _iterator_int_in_own_to_check = NULL;
   return EINA_TRUE;
}

// <int> out

Eina_Iterator *_iterator_int_out_to_check = NULL;
Eina_Array *_iterator_int_out_array = NULL;

Eina_Bool _dummy_test_object_eina_iterator_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator **itr)
{
   if (!itr) return EINA_FALSE;

   _iterator_int_out_array = _iterator_int_eina_array_new();

   *itr = eina_array_iterator_new(_iterator_int_out_array);
   _iterator_int_out_to_check = *itr;

   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_iterator_int_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Iterator *itr = _iterator_int_out_to_check;
   if (!itr) return EINA_FALSE;
   _iterator_int_out_to_check = NULL;

   Eina_Array *arr = _iterator_int_out_array;
   if (!arr) return EINA_FALSE;
   _iterator_int_out_array = NULL;

   Eina_Bool r = _iterator_int_test_array(arr);
   if (!r) return r;

   eina_iterator_free(itr);

   return r;
}

// <int> out own

Eina_Bool _dummy_test_object_eina_iterator_int_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator **itr)
{
   if (!itr) return EINA_FALSE;

   Eina_Array *arr = _iterator_int_eina_array_new();

   *itr = eina_array_iterator_new(arr);

   return EINA_TRUE;
}

// <int> return

Eina_Iterator *_iterator_int_return_to_check = NULL;
Eina_Array *_iterator_int_return_array = NULL;

Eina_Iterator *_dummy_test_object_eina_iterator_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   _iterator_int_return_array = _iterator_int_eina_array_new();
   _iterator_int_return_to_check = eina_array_iterator_new(_iterator_int_return_array);
   return _iterator_int_return_to_check;
}
Eina_Bool _dummy_test_object_check_eina_iterator_int_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Iterator *itr = _iterator_int_return_to_check;
   if (!itr) return EINA_FALSE;
   _iterator_int_return_to_check = NULL;

   Eina_Array *arr = _iterator_int_return_array;
   if (!arr) return EINA_FALSE;
   _iterator_int_return_array = NULL;

   Eina_Bool r = _iterator_int_test_array(arr);
   if (!r) return r;

   eina_iterator_free(itr);

   return r;
}

// <int> return own

Eina_Iterator *_dummy_test_object_eina_iterator_int_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = _iterator_int_eina_array_new();
   return eina_array_iterator_new(arr);
}

// String //

Eina_Bool _iterator_str_equal(Eina_Iterator *itr, const char * const base[], unsigned int len, Eina_Bool release)
{
   const char *data;
   unsigned i = 0;
   EINA_ITERATOR_FOREACH(itr, data)
     {
        if (0 != strcmp(data, base[i]))
          return EINA_FALSE;
        if (release)
          free((void*)data);
        ++i;
     }

   if (i != len)
     return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Array *_iterator_str_eina_array_new()
{
   Eina_Array *arr = eina_array_new(32);
   for (unsigned i = 0; i < base_seq_str_size; ++i)
     {
        eina_array_push(arr, strdup(base_seq_str[i]));
     }
   return arr;
}

Eina_Bool _iterator_str_test_array(Eina_Array *arr)
{
   if (eina_array_count(arr) != base_seq_str_size)
     return EINA_FALSE;

   for (unsigned i = 0; i < base_seq_str_size; ++i)
     {
        const char *data = eina_array_data_get(arr, i);
        if (0 != strcmp(data, base_seq_str[i]))
          return EINA_FALSE;
        free((void*)data);
     }

   eina_array_free(arr);

   return EINA_TRUE;
}

// <str> in

Eina_Bool _dummy_test_object_eina_iterator_str_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator *itr)
{
   Eina_Bool r = _iterator_str_equal(itr, base_seq_str, base_seq_str_size, EINA_FALSE);
   return r;
}

// <str> in own

static Eina_Iterator *_iterator_str_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_iterator_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator *itr)
{
   Eina_Bool r = _iterator_str_equal(itr, base_seq_str, base_seq_str_size, EINA_TRUE);
   _iterator_str_in_own_to_check = itr;
   return r;
}
Eina_Bool _dummy_test_object_check_eina_iterator_str_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   eina_iterator_free(_iterator_str_in_own_to_check);
   _iterator_str_in_own_to_check = NULL;
   return EINA_TRUE;
}

// <str> out

Eina_Iterator *_iterator_str_out_to_check = NULL;
Eina_Array *_iterator_str_out_array = NULL;

Eina_Bool _dummy_test_object_eina_iterator_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator **itr)
{
   if (!itr) return EINA_FALSE;

   _iterator_str_out_array = _iterator_str_eina_array_new();

   *itr = eina_array_iterator_new(_iterator_str_out_array);
   _iterator_str_out_to_check = *itr;

   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_iterator_str_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Iterator *itr = _iterator_str_out_to_check;
   if (!itr) return EINA_FALSE;
   _iterator_str_out_to_check = NULL;

   Eina_Array *arr = _iterator_str_out_array;
   if (!arr) return EINA_FALSE;
   _iterator_str_out_array = NULL;

   Eina_Bool r = _iterator_str_test_array(arr);
   if (!r) return r;

   eina_iterator_free(itr);

   return r;
}

// <str> out own

Eina_Bool _dummy_test_object_eina_iterator_str_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator **itr)
{
   if (!itr) return EINA_FALSE;

   Eina_Array *arr = _iterator_str_eina_array_new();

   *itr = eina_array_iterator_new(arr);

   return EINA_TRUE;
}

// <str> return

Eina_Iterator *_iterator_str_return_to_check = NULL;
Eina_Array *_iterator_str_return_array = NULL;

Eina_Iterator *_dummy_test_object_eina_iterator_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   _iterator_str_return_array = _iterator_str_eina_array_new();
   _iterator_str_return_to_check = eina_array_iterator_new(_iterator_str_return_array);
   return _iterator_str_return_to_check;
}
Eina_Bool _dummy_test_object_check_eina_iterator_str_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Iterator *itr = _iterator_str_return_to_check;
   if (!itr) return EINA_FALSE;
   _iterator_str_return_to_check = NULL;

   Eina_Array *arr = _iterator_str_return_array;
   if (!arr) return EINA_FALSE;
   _iterator_str_return_array = NULL;

   Eina_Bool r = _iterator_str_test_array(arr);
   if (!r) return r;

   eina_iterator_free(itr);

   return r;
}

// <str> return own

Eina_Iterator *_dummy_test_object_eina_iterator_str_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = _iterator_str_eina_array_new();
   return eina_array_iterator_new(arr);
}

// Object //

Eina_Bool _iterator_obj_equal(Eina_Iterator *itr, const Dummy_Numberwrapper * const base[], unsigned int len, Eina_Bool release)
{
   Dummy_Numberwrapper *data;
   unsigned i = 0;
   EINA_ITERATOR_FOREACH(itr, data)
     {
        int a = dummy_numberwrapper_number_get(data);
        int b = dummy_numberwrapper_number_get(base[i]);
        if (a != b)
          return EINA_FALSE;
        if (release)
          efl_unref(data);
        ++i;
     }

   if (i != len)
     return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Array *_iterator_obj_eina_array_new()
{
   Eina_Array *arr = eina_array_new(32);
   for (unsigned i = 0; i < base_seq_obj_size; ++i)
     {
        eina_array_push(arr, _new_obj(dummy_numberwrapper_number_get(base_seq_obj[i])));
     }
   return arr;
}

Eina_Bool _iterator_obj_test_array(Eina_Array *arr)
{
   if (eina_array_count(arr) != base_seq_obj_size)
     return EINA_FALSE;

   for (unsigned i = 0; i < base_seq_obj_size; ++i)
     {
        Dummy_Numberwrapper *data = eina_array_data_get(arr, i);
        int a = dummy_numberwrapper_number_get(data);
        int b = dummy_numberwrapper_number_get(base_seq_obj[i]);
        if (a != b)
          return EINA_FALSE;
        efl_unref(data);
     }

   eina_array_free(arr);

   return EINA_TRUE;
}

// <obj> in

Eina_Bool _dummy_test_object_eina_iterator_obj_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator *itr)
{
   Eina_Bool r = _iterator_obj_equal(itr, base_seq_obj, base_seq_obj_size, EINA_FALSE);
   return r;
}

// <obj> in own

static Eina_Iterator *_iterator_obj_in_own_to_check = NULL;

Eina_Bool _dummy_test_object_eina_iterator_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator *itr)
{
   Eina_Bool r = _iterator_obj_equal(itr, base_seq_obj, base_seq_obj_size, EINA_TRUE);
   _iterator_obj_in_own_to_check = itr;
   return r;
}
Eina_Bool _dummy_test_object_check_eina_iterator_obj_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   eina_iterator_free(_iterator_obj_in_own_to_check);
   _iterator_obj_in_own_to_check = NULL;
   return EINA_TRUE;
}

// <obj> out

Eina_Iterator *_iterator_obj_out_to_check = NULL;
Eina_Array *_iterator_obj_out_array = NULL;

Eina_Bool _dummy_test_object_eina_iterator_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator **itr)
{
   if (!itr) return EINA_FALSE;

   _iterator_obj_out_array = _iterator_obj_eina_array_new();

   *itr = eina_array_iterator_new(_iterator_obj_out_array);
   _iterator_obj_out_to_check = *itr;

   return EINA_TRUE;
}
Eina_Bool _dummy_test_object_check_eina_iterator_obj_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Iterator *itr = _iterator_obj_out_to_check;
   if (!itr) return EINA_FALSE;
   _iterator_obj_out_to_check = NULL;

   Eina_Array *arr = _iterator_obj_out_array;
   if (!arr) return EINA_FALSE;
   _iterator_obj_out_array = NULL;

   Eina_Bool r = _iterator_obj_test_array(arr);
   if (!r) return r;

   eina_iterator_free(itr);

   return r;
}

// <obj> out own

Eina_Bool _dummy_test_object_eina_iterator_obj_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Iterator **itr)
{
   if (!itr) return EINA_FALSE;

   Eina_Array *arr = _iterator_obj_eina_array_new();

   *itr = eina_array_iterator_new(arr);

   return EINA_TRUE;
}

// <obj> return

Eina_Iterator *_iterator_obj_return_to_check = NULL;
Eina_Array *_iterator_obj_return_array = NULL;

Eina_Iterator *_dummy_test_object_eina_iterator_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   _iterator_obj_return_array = _iterator_obj_eina_array_new();
   _iterator_obj_return_to_check = eina_array_iterator_new(_iterator_obj_return_array);
   return _iterator_obj_return_to_check;
}
Eina_Bool _dummy_test_object_check_eina_iterator_obj_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Iterator *itr = _iterator_obj_return_to_check;
   if (!itr) return EINA_FALSE;
   _iterator_obj_return_to_check = NULL;

   Eina_Array *arr = _iterator_obj_return_array;
   if (!arr) return EINA_FALSE;
   _iterator_obj_return_array = NULL;

   Eina_Bool r = _iterator_obj_test_array(arr);
   if (!r) return r;

   eina_iterator_free(itr);

   return r;
}

// <obj> return own

Eina_Iterator *_dummy_test_object_eina_iterator_obj_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Eina_Array *arr = _iterator_obj_eina_array_new();
   return eina_array_iterator_new(arr);
}

//                                 //
// Callbacks and Function Pointers //
//                                 //

void _dummy_test_object_set_callback(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, void *cb_data, Dummy_SimpleCb cb, Eina_Free_Cb cb_free_cb)
{
   if (!pd)
     {
        EINA_LOG_ERR("Null private data");
        return;
     }

   if (pd->free_cb)
      pd->free_cb(pd->cb_data);

   pd->cb = cb;
   pd->cb_data = cb_data;
   pd->free_cb = cb_free_cb;
}

int _dummy_test_object_call_callback(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, int a)
{
   if (!pd->cb)
     {
       EINA_LOG_ERR("Trying to call with no callback set");
       return -1; // FIXME Maybe use Eina error when exceptions are supported?
     }

   return pd->cb(pd->cb_data, a);
}

// Global var used due to the current issue of calling methods from the GC thread
static Eina_Bool _free_called = EINA_FALSE;

EAPI Eina_Bool free_called_get() {
   return _free_called;
}

EAPI void free_called_set(Eina_Bool b) {
   _free_called = b;
}

static void _free_callback(void *data)
{
   Eo *obj = data;

   free_called_set(EINA_TRUE);
   efl_unref(obj);
}

int _wrapper_cb(EINA_UNUSED void *data, int a)
{
    return a * 3;
}

void _dummy_test_object_call_set_callback(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   dummy_test_object_set_callback(obj, efl_ref(obj), _wrapper_cb, _free_callback);
}

void _dummy_test_object_raises_eina_error(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   eina_error_set(EIO);
}

void _dummy_test_object_children_raise_error(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    // Native method shouldn't throw any error. Children must raise it.
}

void _dummy_test_object_call_children_raise_error(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    dummy_test_object_children_raise_error(obj);
}

void _dummy_test_object_error_ret_set(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Eina_Error error)
{
    pd->error_code = error;
}

Eina_Error _dummy_test_object_returns_error(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd)
{
   return pd->error_code;
}

//         //
// Structs //
//         //

// auxiliary functions

static
void struct_simple_with_values(Dummy_StructSimple *simple)
{
   simple->fbyte = -126;
   simple->fubyte = 254u;
   simple->fchar = '~';
   simple->fshort = -32766;
   simple->fushort = 65534u;
   simple->fint = -32766;
   simple->fuint = 65534u;
   simple->flong = -2147483646;
   simple->fulong = 4294967294u;
   simple->fllong = -9223372036854775806;
   simple->fullong = 18446744073709551614u;
   simple->fint8 = -126;
   simple->fuint8 = 254u;
   simple->fint16 = -32766;
   simple->fuint16 = 65534u;
   simple->fint32 = -2147483646;
   simple->fuint32 = 4294967294u;
   simple->fint64 = -9223372036854775806;
   simple->fuint64 = 18446744073709551614u;
   simple->fssize = -2147483646;
   simple->fsize = 4294967294u;
   simple->fintptr =  0xFE;
   simple->fptrdiff = -2147483646;
   simple->ffloat = -16777216.0;
   simple->fdouble = -9007199254740992.0;
   simple->fbool = EINA_TRUE;
   simple->fvoid_ptr = (void*) 0xFE;
   simple->fenum = DUMMY_SAMPLEENUM_V2;
   simple->fstring = "test/string";
   simple->fmstring = strdup("test/mstring");
   simple->fstringshare = eina_stringshare_add("test/stringshare");
}

static
Eina_Bool check_and_modify_struct_simple(Dummy_StructSimple *simple)
{
   Eina_Bool ret =
     EQUAL(simple->fbyte, -126)
     && EQUAL(simple->fubyte, 254u)
     && EQUAL(simple->fchar, '~')
     && EQUAL(simple->fshort, -32766)
     && EQUAL(simple->fushort, 65534u)
     && EQUAL(simple->fint, -32766)
     && EQUAL(simple->fuint, 65534u)
     && EQUAL(simple->flong, -2147483646)
     && EQUAL(simple->fulong, 4294967294u)
     && EQUAL(simple->fllong, -9223372036854775806)
     && EQUAL(simple->fullong, 18446744073709551614u)
     && EQUAL(simple->fint8, -126)
     && EQUAL(simple->fuint8, 254u)
     && EQUAL(simple->fint16, -32766)
     && EQUAL(simple->fuint16, 65534u)
     && EQUAL(simple->fint32, -2147483646)
     && EQUAL(simple->fuint32, 4294967294u)
     && EQUAL(simple->fint64, -9223372036854775806)
     && EQUAL(simple->fuint64, 18446744073709551614u)
     && EQUAL(simple->fssize, -2147483646)
     && EQUAL(simple->fsize, 4294967294u)
     && EQUAL(simple->fintptr, 0xFE)
     && EQUAL(simple->fptrdiff, -2147483646)
     && EQUAL(simple->ffloat, -16777216.0)
     && EQUAL(simple->fdouble, -9007199254740992.0)
     && EQUAL(simple->fbool, EINA_TRUE)
     && EQUAL(simple->fvoid_ptr, (void*) 0xFE)
     && EQUAL(simple->fenum, DUMMY_SAMPLEENUM_V2)
     && STR_EQUAL(simple->fstring, "test/string")
     && STR_EQUAL(simple->fmstring, "test/mstring")
     && STR_EQUAL(simple->fstringshare, "test/stringshare")
   ;

   if (!ret)
     return ret;

   simple->fmstring[4] = '-';
   return strcmp(simple->fmstring, "test-mstring") == 0;
}

static
void struct_complex_with_values(Dummy_StructComplex *complex)
{
   complex->farray = eina_array_new(4);
   eina_array_push(complex->farray, _new_int(0x0));
   eina_array_push(complex->farray, _new_int(0x2A));
   eina_array_push(complex->farray, _new_int(0x42));

   complex->finarray = eina_inarray_new(sizeof(int), 0);
   eina_inarray_push(complex->finarray, _int_ref(0x0));
   eina_inarray_push(complex->finarray, _int_ref(0x2A));
   eina_inarray_push(complex->finarray, _int_ref(0x42));

   complex->flist = eina_list_append(complex->flist, strdup("0x0"));
   complex->flist = eina_list_append(complex->flist, strdup("0x2A"));
   complex->flist = eina_list_append(complex->flist, strdup("0x42"));

   complex->finlist = eina_inlist_append(complex->finlist, _new_inlist_int(0x0));
   complex->finlist = eina_inlist_append(complex->finlist, _new_inlist_int(0x2A));
   complex->finlist = eina_inlist_append(complex->finlist, _new_inlist_int(0x42));

   complex->fhash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(complex->fhash, "aa", strdup("aaa"));
   eina_hash_add(complex->fhash, "bb", strdup("bbb"));
   eina_hash_add(complex->fhash, "cc", strdup("ccc"));

   complex->fiterator = eina_array_iterator_new(complex->farray);

   eina_value_setup(&complex->fany_value, EINA_VALUE_TYPE_DOUBLE);
   eina_value_set(&complex->fany_value, -9007199254740992.0);

   complex->fany_value_ptr = eina_value_new(EINA_VALUE_TYPE_STRING);
   eina_value_set(complex->fany_value_ptr, "abc");

   complex->fbinbuf = eina_binbuf_new();
   eina_binbuf_append_char(complex->fbinbuf, 126);

   complex->fslice.len = 1;
   complex->fslice.mem = malloc(1);
   memset((void*)complex->fslice.mem, 125, 1);

   complex->fobj = _new_obj(42);
}

static
Eina_Bool check_and_modify_struct_complex(Dummy_StructComplex *complex)
{
   if (!_array_int_equal(complex->farray, base_seq_int, base_seq_int_size))
     return EINA_FALSE;

   if (!_inarray_int_equal(complex->finarray, base_seq_int, base_seq_int_size))
     return EINA_FALSE;

   if (!_list_str_equal(complex->flist, base_seq_str, base_seq_str_size))
     return EINA_FALSE;

   if (!_inlist_int_equal(complex->finlist, base_seq_int, base_seq_int_size))
     return EINA_FALSE;

   if (!_hash_str_check(complex->fhash, "aa", "aaa")
       || !_hash_str_check(complex->fhash, "bb", "bbb")
       || !_hash_str_check(complex->fhash, "cc", "ccc"))
     return EINA_FALSE;

   if (!_iterator_int_equal(complex->fiterator, base_seq_int, base_seq_int_size, EINA_FALSE))
     return EINA_FALSE;

   double double_val = 0;
   if (!eina_value_get(&complex->fany_value, &double_val) || double_val != -9007199254740992.0)
     return EINA_FALSE;

   const char *str_val = NULL;
   if (!eina_value_get(complex->fany_value_ptr, &str_val) || strcmp(str_val, "abc") != 0)
     return EINA_FALSE;

   if (eina_binbuf_length_get(complex->fbinbuf) != 1 || eina_binbuf_string_get(complex->fbinbuf)[0] != 126)
     return EINA_FALSE;

   if (complex->fslice.len != 1 || *(char*)complex->fslice.mem != 125)
     return EINA_FALSE;

   if (complex->fobj == NULL || dummy_numberwrapper_number_get(complex->fobj) != 42)
     return EINA_FALSE;

   return EINA_TRUE;
}

// with simple types

EOLIAN
Eina_Bool _dummy_test_object_struct_simple_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple simple)
{
   return check_and_modify_struct_simple(&simple);
}

static void _reverse_string(char *str)
{
   int len = strlen(str);
   if (len > 0) {
       for (int i=0, k=len-1; i < len/2; i++, k--) {
           char tmp = str[k];
           str[k] = str[i];
           str[i] = tmp;
       }
   }
}

EOLIAN
Eina_Bool _dummy_test_object_struct_simple_ptr_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple *simple)
{
   simple->fint = -simple->fint;
   _reverse_string(simple->fmstring);
   return EINA_TRUE;
}

EOLIAN
Dummy_StructSimple _dummy_test_object_struct_simple_ptr_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple *simple)
{
   Dummy_StructSimple ret = *simple;
   free(simple);
   ret.fint = -ret.fint;
   _reverse_string(ret.fmstring);
   return ret;
}

EOLIAN
Eina_Bool _dummy_test_object_struct_simple_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple *simple)
{
   if (!simple)
     {
        EINA_LOG_ERR("Null struct pointer");
        return EINA_FALSE;
     }

   struct_simple_with_values(simple);

   return EINA_TRUE;
}

EOLIAN
Dummy_StructSimple _dummy_test_object_struct_simple_ptr_out(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Dummy_StructSimple **simple)
{
   struct_simple_with_values(&pd->stored_struct);
   *simple = &pd->stored_struct;
   return **simple;
}

EOLIAN
Dummy_StructSimple _dummy_test_object_struct_simple_ptr_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple **simple)
{
   *simple = malloc(sizeof(Dummy_StructSimple));
   struct_simple_with_values(*simple);
   (*simple)->fstring = "Ptr Out Own";
   return **simple;
}

EOLIAN
Dummy_StructSimple _dummy_test_object_struct_simple_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Dummy_StructSimple simple = {0,};
   struct_simple_with_values(&simple);
   return simple;
}

EOLIAN
Dummy_StructSimple *_dummy_test_object_struct_simple_ptr_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   struct_simple_with_values(&pd->stored_struct);
   pd->stored_struct.fstring = "Ret Ptr";
   return &pd->stored_struct;
}

EOLIAN
Dummy_StructSimple *_dummy_test_object_struct_simple_ptr_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Dummy_StructSimple *ret = malloc(sizeof(Dummy_StructSimple));
   struct_simple_with_values(ret);
   ret->fstring = "Ret Ptr Own";
   return ret;
}

EOLIAN
void _dummy_test_object_call_struct_simple_in(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple simple)
{
    dummy_test_object_struct_simple_in(obj, simple);
}

EOLIAN
void _dummy_test_object_call_struct_simple_ptr_in(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple *simple)
{
    dummy_test_object_struct_simple_ptr_in(obj, simple);
}

EOLIAN
void _dummy_test_object_call_struct_simple_ptr_in_own(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple *simple)
{
    dummy_test_object_struct_simple_ptr_in_own(obj, simple);
}

EOLIAN
void _dummy_test_object_call_struct_simple_out(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple *simple)
{
    dummy_test_object_struct_simple_out(obj, simple);
}

EOLIAN
void _dummy_test_object_call_struct_simple_ptr_out(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple **simple)
{
    dummy_test_object_struct_simple_ptr_out(obj, simple);
}

EOLIAN
void _dummy_test_object_call_struct_simple_ptr_out_own(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple **simple)
{
    dummy_test_object_struct_simple_ptr_out_own(obj, simple);
}

EOLIAN
Dummy_StructSimple _dummy_test_object_call_struct_simple_return(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    return dummy_test_object_struct_simple_return(obj);
}

EOLIAN
Dummy_StructSimple *_dummy_test_object_call_struct_simple_ptr_return(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    return dummy_test_object_struct_simple_ptr_return(obj);
}

EOLIAN
Dummy_StructSimple *_dummy_test_object_call_struct_simple_ptr_return_own(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
    return dummy_test_object_struct_simple_ptr_return_own(obj);
}

// with complex types

EOLIAN
Eina_Bool _dummy_test_object_struct_complex_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructComplex complex)
{
   return check_and_modify_struct_complex(&complex);
}

EOLIAN
Eina_Bool _dummy_test_object_struct_complex_ptr_in(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructComplex *complex)
{
   (void) complex;
   EINA_LOG_ERR("Not implemented!");
   return EINA_FALSE;
}

EOLIAN
Eina_Bool _dummy_test_object_struct_complex_ptr_in_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructComplex *complex)
{
   (void) complex;
   EINA_LOG_ERR("Not implemented!");
   return EINA_FALSE;
}

EOLIAN
Eina_Bool _dummy_test_object_struct_complex_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructComplex *complex)
{
   if (!complex)
     {
        EINA_LOG_ERR("Null struct pointer");
        return EINA_FALSE;
     }

   struct_complex_with_values(complex);

   return EINA_TRUE;
}

EOLIAN
Eina_Bool _dummy_test_object_struct_complex_ptr_out(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructComplex **complex)
{
   (void) complex;
   EINA_LOG_ERR("Not implemented!");
   return EINA_FALSE;
}

EOLIAN
Eina_Bool _dummy_test_object_struct_complex_ptr_out_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructComplex **complex)
{
   (void) complex;
   EINA_LOG_ERR("Not implemented!");
   return EINA_FALSE;
}

EOLIAN
Dummy_StructComplex _dummy_test_object_struct_complex_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   Dummy_StructComplex complex = {0,};
   struct_complex_with_values(&complex);
   return complex;
}

EOLIAN
Dummy_StructComplex* _dummy_test_object_struct_complex_ptr_return(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   EINA_LOG_ERR("Not implemented!");
   return NULL;
}

EOLIAN
Dummy_StructComplex* _dummy_test_object_struct_complex_ptr_return_own(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd)
{
   EINA_LOG_ERR("Not implemented!");
   return NULL;
}

//                   //
// Class constructor
//                   //
EOLIAN static void
_dummy_test_object_class_constructor(Efl_Class *klass)
{
   (void)klass;
   modified_seq_obj[0] = base_seq_obj[0] = _new_obj(0x0);
   modified_seq_obj[1] = base_seq_obj[1] = _new_obj(0x2A);
   modified_seq_obj[2] = base_seq_obj[2] = _new_obj(0x42);
   modified_seq_obj[3] = _new_obj(42);
   modified_seq_obj[4] = _new_obj(43);
   modified_seq_obj[5] = _new_obj(33);
}

EOLIAN static void
_dummy_test_object_class_destructor(Efl_Class *klass)
{
   (void)klass;
   for (unsigned i = 0; i < base_seq_obj_size; ++i)
     efl_unref(base_seq_obj[i]);
   for (unsigned i = 0; i < modified_seq_obj_size; ++i)
     efl_unref(modified_seq_obj[i]);
}


// ################## //
// Test.Numberwrapper //
// ################## //


void _dummy_numberwrapper_number_set(EINA_UNUSED Eo *obj, Dummy_Numberwrapper_Data *pd, int n)
{
   pd->number = n;
}

int _dummy_numberwrapper_number_get(EINA_UNUSED const Eo *obj, Dummy_Numberwrapper_Data *pd)
{
   return pd->number;
}

void _dummy_test_object_set_value_ptr(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Eina_Value *value)
{
    if (pd->stored_value) {
        eina_value_free(pd->stored_value);
        free(pd->stored_value);
    }

    pd->stored_value = eina_value_new(EINA_VALUE_TYPE_INT);

    eina_value_copy(value, pd->stored_value);
}

void _dummy_test_object_set_value_ptr_own(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Eina_Value *value)
{
    if (pd->stored_value) {
        eina_value_free(pd->stored_value);
        free(pd->stored_value);
    }

    pd->stored_value = value;
}

void _dummy_test_object_set_value(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Eina_Value value)
{
    if (pd->stored_value) {
        eina_value_free(pd->stored_value);
    } else {
        pd->stored_value = eina_value_new(EINA_VALUE_TYPE_INT);
    }
    eina_value_copy(&value, pd->stored_value);
}

void _dummy_test_object_call_set_value(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const Eina_Value v)
{
    dummy_test_object_set_value(obj, v);
}

Eina_Value *_dummy_test_object_get_value_ptr_own(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd)
{
    Eina_Value *val = pd->stored_value;
    pd->stored_value = NULL;
    return val;
}

Eina_Value *_dummy_test_object_get_value_ptr(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd)
{
    return pd->stored_value;
}

void _dummy_test_object_clear_value(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd)
{
    if (pd->stored_value) {
        eina_value_free(pd->stored_value);
        pd->stored_value = NULL;
    }
}

void _dummy_test_object_out_value_ptr(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Eina_Value **value)
{
    *value = pd->stored_value;
}

void _dummy_test_object_out_value_ptr_own(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Eina_Value **value)
{
    *value = pd->stored_value;
    pd->stored_value = NULL;
}

void _dummy_test_object_out_value(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, Eina_Value *value)
{
    *value = *pd->stored_value;
}

void _dummy_test_object_emit_event_with_string(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, const char *data)
{
    char *ptr = strdup(data);
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_STRING, ptr);
    free(ptr);
}

void _dummy_test_object_emit_event_with_bool(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Bool data)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_BOOL, (void *) (uintptr_t) data);
}

void _dummy_test_object_emit_event_with_int(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, int data)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_INT, (void *) (uintptr_t) data);
}

void _dummy_test_object_emit_event_with_uint(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, unsigned int data)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_UINT, (void *) (uintptr_t) data);
}

void _dummy_test_object_emit_event_with_obj(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eo *data)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_OBJ, data);
}

void _dummy_test_object_emit_event_with_error(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Error data)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_ERROR, &data);
}

void _dummy_test_object_emit_event_with_struct(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructSimple data)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_STRUCT, &data);
}

void _dummy_test_object_emit_event_with_struct_complex(Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_StructComplex data)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_STRUCT_COMPLEX, &data);
}

Efl_Object *_dummy_test_object_efl_part_part_get(EINA_UNUSED const Eo *obj, Dummy_Test_Object_Data *pd, const char *name)
{
    if (!strcmp(name, "part_one"))
      return pd->part_one;
    else if (!strcmp(name, "part_two"))
      return pd->part_two;
    else
      return NULL;
}

void _dummy_test_object_append_to_strbuf(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Strbuf *buf, const char *str)
{
    eina_strbuf_append(buf, str);
}

void _dummy_test_object_call_append_to_strbuf(Eo * obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Strbuf *buf, const char *str)
{
    dummy_test_object_append_to_strbuf(obj, buf, str);
}

void _dummy_test_object_call_format_cb(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Eina_Strbuf *buf, const Eina_Value value,
                               void *func_data, Dummy_FormatCb func, Eina_Free_Cb func_free_cb)
{
    func(func_data, buf, value);
    func_free_cb(func_data);
}

Dummy_MyInt _dummy_test_object_bypass_typedef(EINA_UNUSED Eo *obj, EINA_UNUSED Dummy_Test_Object_Data *pd, Dummy_MyInt data, Dummy_MyInt *receiver)
{
    *receiver = data;
    return data;
}

/* Class Properties */
static int _dummy_test_object_klass_prop = 0;

int _dummy_test_object_klass_prop_get(const Eo *klass, EINA_UNUSED void *pd)
{
    EINA_LOG_ERR("FAIL on GET");
   if (klass != dummy_test_object_class_get())
     {
        eina_error_set(EINVAL);
        return -1;
     }
   return _dummy_test_object_klass_prop;
}

void _dummy_test_object_klass_prop_set(Eo *klass, EINA_UNUSED void *pd, int value)
{
    EINA_LOG_ERR("FAIL on SET");
   if (klass != dummy_test_object_class_get())
     {
        eina_error_set(EINVAL);
     }
   _dummy_test_object_klass_prop = value;
}

static void _promise_cancelled(void *data, EINA_UNUSED const Eina_Promise *p)
{
    Dummy_Test_Object_Data *pd = data;
    pd->promise = NULL;
}

Eina_Future* _dummy_test_object_get_future(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd)
{
    if (pd->promise == NULL)
      {
         Eo *loop = efl_app_main_get(EFL_APP_CLASS);
         Eina_Future_Scheduler *scheduler = efl_loop_future_scheduler_get(loop);
         pd->promise = eina_promise_new(scheduler, _promise_cancelled, pd);
      }
    return eina_future_new(pd->promise);
}

void _dummy_test_object_fulfill_promise(Eo *obj EINA_UNUSED, Dummy_Test_Object_Data *pd, int data)
{
    if (pd->promise == NULL)
      {
         EINA_LOG_ERR("Can't fulfill an object without a valid promise.");
         return;
      }
    Eina_Value v;
    eina_value_setup(&v, EINA_VALUE_TYPE_INT);
    eina_value_set(&v, data);
    eina_promise_resolve(pd->promise, v);
}

void _dummy_test_object_reject_promise(Eo *obj EINA_UNUSED, Dummy_Test_Object_Data *pd, Eina_Error err)
{
    if (pd->promise == NULL)
      {
         EINA_LOG_ERR("Can't fulfill an object without a valid promise.");
         return;
      }

    eina_promise_reject(pd->promise, err);
}

Eina_Accessor *_dummy_test_object_clone_accessor(Eo *obj EINA_UNUSED, Dummy_Test_Object_Data *pd, Eina_Accessor *acc)
{
   if (pd->list_for_accessor)
     eina_list_free(pd->list_for_accessor);

   unsigned int i;
   int *data;
   EINA_ACCESSOR_FOREACH(acc, i, data)
     {
         pd->list_for_accessor = eina_list_append(pd->list_for_accessor, data);
     }

   return eina_list_accessor_new(pd->list_for_accessor);
}

void _dummy_test_object_dummy_test_iface_emit_test_conflicted(Eo *obj, Dummy_Test_Object_Data *pd EINA_UNUSED)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_IFACE_EVENT_CONFLICTED, NULL);
}

void _dummy_test_object_dummy_test_iface_emit_nonconflicted(Eo *obj, Dummy_Test_Object_Data *pd EINA_UNUSED)
{
    efl_event_callback_legacy_call(obj, DUMMY_TEST_IFACE_EVENT_NONCONFLICTED, NULL);
}

void _dummy_test_object_dummy_another_iface_emit_another_conflicted(Eo *obj, Dummy_Test_Object_Data *pd EINA_UNUSED)
{
    efl_event_callback_legacy_call(obj, DUMMY_ANOTHER_IFACE_EVENT_CONFLICTED, NULL);
}

void _dummy_test_object_setter_only_set(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, int value)
{
    pd->setter_only = value;
}

int _dummy_test_object_get_setter_only(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd)
{
    return pd->setter_only;
}

void _dummy_test_object_dummy_test_iface_iface_prop_set(EINA_UNUSED Eo *obj, Dummy_Test_Object_Data *pd, int value)
{
    pd->iface_prop = value;
}

int _dummy_test_object_dummy_test_iface_iface_prop_get(EINA_UNUSED const Eo *obj, Dummy_Test_Object_Data *pd)
{
    return pd->iface_prop;
}

/// Dummy.Child
EOLIAN static void
_dummy_child_class_constructor(Efl_Class *klass)
{
    (void)klass;
}

EOLIAN static void
_dummy_child_class_destructor(Efl_Class *klass)
{
    (void)klass;
}

// Inherit
int _dummy_inherit_helper_receive_dummy_and_call_int_out(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Dummy_Test_Object *x)
{
  int v = 8;
  dummy_test_object_int_out (x, 5, &v);
  return v;
}

const char* _dummy_inherit_helper_receive_dummy_and_call_in_stringshare(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Dummy_Test_Object *x)
{
  return dummy_inherit_iface_stringshare_test (x, eina_stringshare_add("hello world"));
}

#include "dummy_test_object.eo.c"
#include "dummy_numberwrapper.eo.c"
#include "dummy_child.eo.c"
#include "dummy_test_iface.eo.c"
#include "dummy_another_iface.eo.c"
#include "dummy_inherit_helper.eo.c"
#include "dummy_inherit_iface.eo.c"

