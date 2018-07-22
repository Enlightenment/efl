#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Eet.h>

#include "eet_suite.h"
#include "eet_test_common.h"

typedef struct _Eet_Union_Test    Eet_Union_Test;
typedef struct _Eet_Variant_Test  Eet_Variant_Test;
typedef struct _Eet_Variant_Type  Eet_Variant_Type;
typedef struct _Eet_Inherit_Test1 Eet_Inherit_Test1;
typedef struct _Eet_Inherit_Test2 Eet_Inherit_Test2;
typedef struct _Eet_Inherit_Test3 Eet_Inherit_Test3;
typedef struct _Eet_St1           Eet_St1;
typedef struct _Eet_St2           Eet_St2;
typedef struct _Eet_St3           Eet_St3;
typedef struct _Eet_List          Eet_List;
typedef struct _Eet_Hash          Eet_Hash;

typedef enum _Eet_Union
{
   EET_UNKNOWN,
   EET_ST1,
   EET_ST2,
   EET_ST3
} Eet_Union;

struct
{
   Eet_Union   u;
   const char *name;
} eet_mapping[] = {
   { EET_ST1, "ST1" },
   { EET_ST2, "ST2" },
   { EET_ST3, "ST3" },
   { EET_UNKNOWN, NULL }
};

struct _Eet_St1
{
   double val1;
   int    stuff;
   char  *s1;
};

struct _Eet_St2
{
   Eina_Bool          b1;
   unsigned long long v1;
};

struct _Eet_St3
{
   int boby;
};

struct _Eet_Union_Test
{
   Eet_Union type;

   union {
      Eet_St1 st1;
      Eet_St2 st2;
      Eet_St3 st3;
   } u;
};

struct _Eet_Variant_Type
{
   const char *type;
   Eina_Bool   unknow : 1;
};

struct _Eet_Variant_Test
{
   Eet_Variant_Type t;

   void            *data;
   Eina_List       *data_list;
};

struct _Eet_Inherit_Test1
{
   Eet_Union type;
   Eet_St1   st1;
};
struct _Eet_Inherit_Test2
{
   Eet_Union type;
   Eet_St2   st2;
};
struct _Eet_Inherit_Test3
{
   Eet_Union type;
   Eet_St3   st3;
};

struct _Eet_List
{
   Eina_List *list;
};

struct _Eet_Hash
{
   Eina_Hash *hash;
};

static const char *
_eet_union_type_get(const void *data,
                    Eina_Bool  *unknow)
{
   const Eet_Union *u = data;
   int i;

   if (unknow)
     *unknow = EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (*u == eet_mapping[i].u)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_TRUE;

   return NULL;
} /* _eet_union_type_get */

static Eina_Bool
_eet_union_type_set(const char *type,
                    void       *data,
                    Eina_Bool   unknow)
{
   Eet_Union *u = data;
   int i;

   if (unknow)
     return EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(eet_mapping[i].name, type) == 0)
       {
          *u = eet_mapping[i].u;
          return EINA_TRUE;
       }

   return EINA_FALSE;
} /* _eet_union_type_set */

static const char *
_eet_variant_type_get(const void *data,
                      Eina_Bool  *unknow)
{
   const Eet_Variant_Type *type = data;
   int i;

   if (unknow)
     *unknow = type->unknow;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(type->type, eet_mapping[i].name) == 0)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_FALSE;

   return type->type;
} /* _eet_variant_type_get */

static Eina_Bool
_eet_variant_type_set(const char *type,
                      void       *data,
                      Eina_Bool   unknow)
{
   Eet_Variant_Type *vt = data;

   vt->type = type;
   vt->unknow = unknow;
   return EINA_TRUE;
} /* _eet_variant_type_set */

static Eet_Data_Descriptor *
_eet_st1_dd(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *res;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_St1);
   res = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(res, Eet_St1, "val1", val1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(res, Eet_St1, "stuff", stuff, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(res, Eet_St1, "s1", s1, EET_T_STRING);

   return res;
} /* _eet_st1_dd */

static void
_eet_st1_set(Eet_St1 *st1,
             int      i)
{
   st1->val1 = EET_TEST_DOUBLE;
   st1->stuff = EET_TEST_INT + i;
   st1->s1 = EET_TEST_STRING;
} /* _eet_st1_set */

static void
_eet_st1_cmp(Eet_St1 *st1,
             int      i)
{
   double tmp;

   fail_if(!st1);

   tmp = st1->val1 - EET_TEST_DOUBLE;
   if (tmp < 0)
     tmp = -tmp;

   fail_if(tmp > 0.005);
   fail_if(st1->stuff != EET_TEST_INT + i);
   fail_if(strcmp(st1->s1, EET_TEST_STRING));
} /* _eet_st1_cmp */

static Eet_Data_Descriptor *
_eet_st2_dd(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *res;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_St2);
   res = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(res, Eet_St2, "b1", b1, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(res, Eet_St2, "v1", v1, EET_T_ULONG_LONG);

   return res;
} /* _eet_st2_dd */

static void
_eet_st2_set(Eet_St2 *st2,
             int      i)
{
   st2->b1 = EINA_TRUE;
   st2->v1 = EET_TEST_LONG_LONG + i;
} /* _eet_st2_set */

static void
_eet_st2_cmp(Eet_St2 *st2,
             int      i)
{
   fail_if(!st2->b1);
   fail_if(st2->v1 != EET_TEST_LONG_LONG + i);
} /* _eet_st2_cmp */

static Eet_Data_Descriptor *
_eet_st3_dd(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *res;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_St3);
   res = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(res, Eet_St3, "boby", boby, EET_T_INT);

   return res;
} /* _eet_st3_dd */

static void
_eet_st3_set(Eet_St3 *st3,
             int      i)
{
   st3->boby = EET_TEST_INT + i;
} /* _eet_st3_set */

static void
_eet_st3_cmp(Eet_St3 *st3,
             int      i)
{
   fail_if(st3->boby != EET_TEST_INT + i);
} /* _eet_st3_cmp */

EFL_START_TEST(eet_test_data_basic_type_encoding_decoding)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Basic_Type *result;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Basic_Type etbt;
   void *transfert;
   int size;

   eet_test_basic_set(&etbt, 0);

   eet_test_setup_eddc(&eddc);
   eddc.name = "Eet_Test_Basic_Type";
   eddc.size = sizeof(Eet_Test_Basic_Type);

   edd = eet_data_descriptor_stream_new(&eddc);
   fail_if(!edd);

   eet_build_basic_descriptor(edd);

   transfert = eet_data_descriptor_encode(edd, &etbt, &size);
   fail_if(!transfert || size <= 0);

   result = eet_data_descriptor_decode(edd, transfert, size);
   fail_if(!result);

   eet_test_basic_check(result, 0, EINA_FALSE);

   free(result->str);
   free(result);

   eet_data_descriptor_free(edd);
}
EFL_END_TEST

EFL_START_TEST(eet_test_data_type_encoding_decoding)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Ex_Type *result;
   void *transfert;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   int size;
   int test;

   eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, eet_test_ex_set(NULL, 2));
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY1, &i7);
   etbt.slist = eina_list_prepend(NULL, "test");
   etbt.shash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.shash, EET_TEST_KEY1, "test");
   memset(&etbt.charray, 0, sizeof(etbt.charray));
   etbt.charray[0] = "test";
   etbt.charray[5] = "plouf";

   eet_test_setup_eddc(&eddc);
   eddc.name = "Eet_Test_Ex_Type";
   eddc.size = sizeof(Eet_Test_Ex_Type);

   edd = eet_data_descriptor_file_new(&eddc);
   fail_if(!edd);

   eet_build_ex_descriptor(edd, EINA_FALSE);

   transfert = eet_data_descriptor_encode(edd, &etbt, &size);
   fail_if(!transfert || size <= 0);

   result = eet_data_descriptor_decode(edd, transfert, size);
   fail_if(!result);

   fail_if(eet_test_ex_check(result, 0, EINA_FALSE) != 0);
   fail_if(eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_FALSE) != 0);
   fail_if(eina_list_data_get(result->ilist) == NULL);
   fail_if(*((int *)eina_list_data_get(result->ilist)) != 42);
   fail_if(eina_list_data_get(result->slist) == NULL);
   fail_if(strcmp(eina_list_data_get(result->slist), "test") != 0);
   fail_if(eina_hash_find(result->shash, EET_TEST_KEY1) == NULL);
   fail_if(strcmp(eina_hash_find(result->shash, EET_TEST_KEY1), "test") != 0);
   fail_if(strcmp(result->charray[0], "test") != 0);
   fail_if(strcmp(result->charray[5], "plouf") != 0);

   test = 0;
   if (result->hash)
     eina_hash_foreach(result->hash, func, &test);

   fail_if(test != 0);
   if (result->ihash)
     eina_hash_foreach(result->ihash, func7, &test);

   fail_if(test != 0);
}
EFL_END_TEST

EFL_START_TEST(eet_test_data_type_dump_undump)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Ex_Type *result;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   char *transfert1;
   char *transfert2;
   char *string1;
   char *string2;
   int size1;
   int size2;
   int test;

   eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, eet_test_ex_set(NULL, 1));
   etbt.list = eina_list_prepend(etbt.list, eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, eet_test_ex_set(NULL, 2));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY2, eet_test_ex_set(NULL, 2));
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY1, &i7);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY2, &i7);
   etbt.slist = eina_list_prepend(NULL, "test");
   etbt.shash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.shash, EET_TEST_KEY1, "test");
   memset(&etbt.charray, 0, sizeof(etbt.charray));
   etbt.charray[0] = "test";

   eet_test_setup_eddc(&eddc);
   eddc.name = "Eet_Test_Ex_Type";
   eddc.size = sizeof(Eet_Test_Ex_Type);

   edd = eet_data_descriptor_file_new(&eddc);
   fail_if(!edd);

   eet_build_ex_descriptor(edd, EINA_FALSE);

   transfert1 = eet_data_descriptor_encode(edd, &etbt, &size1);
   fail_if(!transfert1 || size1 <= 0);

   string1 = NULL;
   eet_data_text_dump(transfert1, size1, append_string, &string1);
   fail_if(!string1);

   transfert2 = eet_data_text_undump(string1, string1 ? strlen(
                                       string1) : 0, &size2);
   fail_if(!transfert2 && size2 <= 0);

   string2 = NULL;
   eet_data_text_dump(transfert2, size2, append_string, &string2);
   fail_if(!string2);

   fail_if(strlen(string2) != strlen(string1));

   result = eet_data_descriptor_decode(edd, transfert2, size2);
   fail_if(!result);

   fail_if(eet_test_ex_check(result, 0, EINA_TRUE) != 0);
   fail_if(eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_TRUE) != 0);
   fail_if(eina_list_data_get(result->ilist) == NULL);
   fail_if(*((int *)eina_list_data_get(result->ilist)) != 42);
   fail_if(eina_list_data_get(result->slist) == NULL);
   fail_if(strcmp(eina_list_data_get(result->slist), "test") != 0);
   fail_if(eina_hash_find(result->shash, EET_TEST_KEY1) == NULL);
   fail_if(strcmp(eina_hash_find(result->shash, EET_TEST_KEY1), "test") != 0);
   fail_if(strcmp(result->charray[0], "test") != 0);

   test = 0;
   _dump_call = EINA_TRUE;
   if (result->hash)
     eina_hash_foreach(result->hash, func, &test);
   _dump_call = EINA_FALSE;

   fail_if(test != 0);
   if (result->ihash)
     eina_hash_foreach(result->ihash, func7, &test);

   fail_if(test != 0);

}
EFL_END_TEST

EFL_START_TEST(eet_test_data_type_escape_dump_undump)
{
   void *blob;
   int blob_len;
   int ret = 0;
   const char *outputstr;
   Eina_Strbuf *strbuf;
   const char inputstr[] = ""
     "group \"\\\\My\\\"Group\\\\\" struct {\n"
     "    value \"\\\\My\\\\BackSlash\\\\\" string: \"\\\\\";\n"
     "    value \"\\\\My\\\\DoubleQuote\\\\\" string: \"\\\"\";\n"
     "    value \"\\\\My\\\\NewLine\\\\\" string: \"\\n\";\n"
     "}\n";

   blob = eet_data_text_undump(inputstr, strlen(inputstr), &blob_len);
   fail_if(!blob);

   strbuf = eina_strbuf_new();
   ret = eet_data_text_dump(blob, blob_len, append_strbuf_string, strbuf);
   ck_assert_int_eq(ret, 1);

   outputstr = eina_strbuf_string_get(strbuf);
   fail_if(!outputstr);
   ck_assert_str_eq(inputstr, outputstr);

   eina_strbuf_free(strbuf);
   free(blob);

}
EFL_END_TEST

EFL_START_TEST(eet_test_data_fp)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd_5FP;
   Eet_Data_Descriptor *edd_5DBL;
   Eet_5FP origin;
   Eet_5DBL *convert;
   Eet_5FP *build;
   void *blob;
   int size;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_5FP);
   edd_5FP = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp32", fp32, EET_T_F32P32);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp16", fp16, EET_T_F16P16);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp8", fp8, EET_T_F8P24);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "f1", f1, EET_T_F32P32);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "f0", f0, EET_T_F32P32);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "Eet_5FP", sizeof (Eet_5DBL));
   edd_5DBL = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "fp32", fp32, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "fp16", fp16, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "fp8", fp8, EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "f1", f1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5DBL, Eet_5DBL, "f0", f0, EET_T_DOUBLE);

   origin.fp32 = eina_f32p32_double_from(1.125);
   origin.fp16 = eina_f16p16_int_from(2000);
   origin.fp8 = eina_f8p24_int_from(125);
   origin.f1 = eina_f32p32_int_from(1);
   origin.f0 = 0;

   blob = eet_data_descriptor_encode(edd_5FP, &origin, &size);
   fail_if(!blob || size <= 0);

   build = eet_data_descriptor_decode(edd_5FP, blob, size);
   fail_if(!build);

   convert = eet_data_descriptor_decode(edd_5DBL, blob, size);
   fail_if(!convert);

   fail_if(build->fp32 != eina_f32p32_double_from(1.125));
   fail_if(build->fp16 != eina_f16p16_int_from(2000));
   fail_if(build->fp8 != eina_f8p24_int_from(125));
   fail_if(build->f1 != eina_f32p32_int_from(1));
   fail_if(build->f0 != 0);

   fail_if(convert->fp32 != 1.125);
   fail_if(convert->fp16 != 2000);
   fail_if(convert->fp8 != 125);
   fail_if(convert->f1 != 1);
   fail_if(convert->f0 != 0);

}
EFL_END_TEST

EFL_START_TEST(eet_test_data_union)
{
   Eet_Union_Test *eut;
   Eet_List *l;
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *unified;
   Eet_Data_Descriptor *m;
   void *blob;
   int size;
   int i;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_Union_Test);
   edd = eet_data_descriptor_stream_new(&eddc);

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_Union_Test);
   m = eet_data_descriptor_stream_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _eet_union_type_get;
   eddc.func.type_set = _eet_union_type_set;
   unified = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, "ST1", _eet_st1_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, "ST2", _eet_st2_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, "ST3", _eet_st3_dd());

   EET_DATA_DESCRIPTOR_ADD_UNION(edd, Eet_Union_Test, "u", u, type, unified);

   EET_DATA_DESCRIPTOR_ADD_LIST(m, Eet_List, "list", list, edd);

   l = calloc(1, sizeof (Eet_List));

#define EUT_NEW(Type_Index)                 \
  eut = calloc(1, sizeof (Eet_Union_Test)); \
  eut->type = EET_ST ## Type_Index;         \
  _eet_st ## Type_Index ## _set(&(eut->u.st ## Type_Index), i);

   for (i = 0; i < 3; ++i)
     {
        EUT_NEW(1);
        l->list = eina_list_append(l->list, eut);

        EUT_NEW(2);
        l->list = eina_list_append(l->list, eut);

        EUT_NEW(3);
        l->list = eina_list_append(l->list, eut);
     }

   blob = eet_data_descriptor_encode(m, l, &size);
   fail_if(!blob || size <= 0);

   l = eet_data_descriptor_decode(m, blob, size);
   fail_if(!l);

   fail_if(eina_list_count(l->list) != 9);

#define EUT_CMP(Type_Index)                             \
  eut = eina_list_nth(l->list, i * 3 + Type_Index - 1); \
  fail_if(eut->type != EET_ST ## Type_Index);           \
  _eet_st ## Type_Index ## _cmp(&(eut->u.st ## Type_Index), i);

   for (i = 0; i < 3; ++i)
     {
        EUT_CMP(1);
        EUT_CMP(2);
        EUT_CMP(3);
     }
}
EFL_END_TEST

EFL_START_TEST(eet_test_data_variant)
{
   Eet_Variant_Test *evt;
   Eet_List *l;
   Eet_St1 *st1;
   Eet_St2 *st2;
   Eet_St3 *st3;
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *unified;
   Eet_Data_Descriptor *m;
   void *blob;
   int size;
   int i;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_Variant_Test);
   edd = eet_data_descriptor_stream_new(&eddc);

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_Variant_Test);
   m = eet_data_descriptor_stream_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _eet_variant_type_get;
   eddc.func.type_set = _eet_variant_type_set;
   unified = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, "ST1", _eet_st1_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, "ST2", _eet_st2_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, "ST3", _eet_st3_dd());

   EET_DATA_DESCRIPTOR_ADD_VARIANT(edd,
                                   Eet_Variant_Test,
                                   "data",
                                   data,
                                   t,
                                   unified);

   unified = eet_data_descriptor_stream_new(&eddc);
   eet_data_descriptor_element_add(unified, "ST1",
                                   EET_T_UNKNOW, EET_G_LIST,
                                   0, 0, NULL, _eet_st1_dd());
   eet_data_descriptor_element_add(unified, "ST2",
                                   EET_T_UNKNOW, EET_G_LIST,
                                   0, 0, NULL, _eet_st2_dd());

   EET_DATA_DESCRIPTOR_ADD_VARIANT(edd, Eet_Variant_Test,
                                   "data_list", data_list, t, unified);

   EET_DATA_DESCRIPTOR_ADD_LIST(m, Eet_List, "list", list, edd);

   l = calloc(1, sizeof (Eet_List));

#define EVT_NEW(Type_Index)                                    \
  evt = calloc(1, sizeof (Eet_Variant_Test));                  \
  evt->t.type = eet_mapping[Type_Index - 1].name;              \
  st ## Type_Index = calloc(1, sizeof (Eet_St ## Type_Index)); \
  _eet_st ## Type_Index ## _set(st ## Type_Index, i);          \
  evt->data = st ## Type_Index;

   for (i = 0; i < 3; ++i)
     {
        EVT_NEW(1);
        l->list = eina_list_append(l->list, evt);

        st1 = calloc(1, sizeof (Eet_St1));
        _eet_st1_set(st1, i);
        evt->data_list = eina_list_append(evt->data_list, st1);

        EVT_NEW(2);
        l->list = eina_list_append(l->list, evt);

        EVT_NEW(3);
        l->list = eina_list_append(l->list, evt);
     }

   blob = eet_data_descriptor_encode(m, l, &size);
   fail_if(!blob || size <= 0);

   l = eet_data_descriptor_decode(m, blob, size);
   fail_if(!l);

   fail_if(eina_list_count(l->list) != 9);

#define EVT_CMP(Type_Index)                                            \
  evt = eina_list_nth(l->list, i * 3 + Type_Index - 1);                \
  fail_if(strcmp(evt->t.type, eet_mapping[Type_Index - 1].name) != 0); \
  _eet_st ## Type_Index ## _cmp(evt->data, i);

   for (i = 0; i < 3; ++i)
     {
        EVT_CMP(1);

        fail_if(!evt->data_list);
        fail_if(eina_list_count(evt->data_list) != 1);

        st1 = eina_list_data_get(evt->data_list);
        _eet_st1_cmp(st1, i);

        EVT_CMP(2);
        EVT_CMP(3);
     }

} /* EFL_START_TEST */
EFL_END_TEST

EFL_START_TEST(eet_test_data_hash_value)
{
   Eet_Hash *h;
   Eina_Value *val;
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd;
   void *blob;
   int size;
   int i;
   double d;
   char *s;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_Hash);
   edd = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_HASH_VALUE(edd, Eet_Hash, "hash", hash);

   h = calloc(1, sizeof(Eet_Hash));
   h->hash = eina_hash_string_small_new((Eina_Free_Cb)eina_value_free);

   val = eina_value_new(EINA_VALUE_TYPE_INT);
   eina_value_set(val, EET_TEST_INT);
   eina_hash_direct_add(h->hash, "val/int", val);

   val = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
   eina_value_set(val, EET_TEST_DOUBLE);
   eina_hash_direct_add(h->hash, "val/double", val);

   val = eina_value_new(EINA_VALUE_TYPE_STRING);
   eina_value_set(val, EET_TEST_STRING);
   eina_hash_direct_add(h->hash, "val/string", val);

   blob = eet_data_descriptor_encode(edd, h, &size);
   fail_if((!blob) || (size <= 0));

   h = eet_data_descriptor_decode(edd, blob, size);
   fail_if(!h);

   val = (Eina_Value *)eina_hash_find(h->hash, "val/int");
   eina_value_get(val, &i);
   fail_if((!val) || (i != EET_TEST_INT));

   val = (Eina_Value *)eina_hash_find(h->hash, "val/double");
   eina_value_get(val, &d);
   fail_if((!val) || (d != EET_TEST_DOUBLE));

   val = (Eina_Value *)eina_hash_find(h->hash, "val/string");
   eina_value_get(val, &s);
   fail_if((!val) || strcmp(s, EET_TEST_STRING));
} /* EFL_START_TEST */
EFL_END_TEST

void eet_test_data(TCase *tc)
{
   tcase_add_test(tc, eet_test_data_basic_type_encoding_decoding);
   tcase_add_test(tc, eet_test_data_type_encoding_decoding);
   tcase_add_test(tc, eet_test_data_type_dump_undump);
   tcase_add_test(tc, eet_test_data_type_escape_dump_undump);
   tcase_add_test(tc, eet_test_data_fp);
   tcase_add_test(tc, eet_test_data_union);
   tcase_add_test(tc, eet_test_data_variant);
   tcase_add_test(tc, eet_test_data_hash_value);
}
