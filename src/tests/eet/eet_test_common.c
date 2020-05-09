#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include <Eina.h>
#include <Eet.h>

#include <check.h>

#include "eet_test_common.h"

int i42 = 42;
int i7 = 7;
Eina_Bool _dump_call = EINA_FALSE;

void
append_string(void       *data,
              const char *str)
{
   char **string = data;
   int length;

   if (!data)
     return;

   length = *string ? strlen(*string) : 0;
   *string = realloc(*string, strlen(str) + length + 1);

   fail_unless(*string); // Fail test case if realloc fails.

   memcpy((*string) + length, str, strlen(str) + 1);
}

void
append_strbuf_string(void *data, const char *string)
{
   Eina_Strbuf *strbuf = data;
   eina_strbuf_append(strbuf, string);
}

Eina_Bool
func(EINA_UNUSED const Eina_Hash *hash,
     const void                 *key,
     void                       *data,
     void                       *fdata)
{
   int *res = fdata;

   if (strcmp(key, EET_TEST_KEY1) != 0
       && strcmp(key, EET_TEST_KEY2) != 0)
     *res = 1;

   if (eet_test_ex_check(data, 2, _dump_call))
     *res = 1;

   return EINA_TRUE;
} /* func */

Eina_Bool
func7(EINA_UNUSED const Eina_Hash *hash,
      EINA_UNUSED const void      *key,
      void                       *data,
      void                       *fdata)
{
   int *res = fdata;
   int *val;

   val = data;
   if (!val)
     *res = 1;
   else if (*val != 7)
     *res = 1;

   return EINA_TRUE;
} /* func7 */

static char *
_eet_str_direct_alloc(const char *str)
{
   return (char *)str;
} /* _eet_str_direct_alloc */

static void
_eet_str_direct_free(const char *str)
{
   /* FIXME: Use attribute unused */
    (void)str;
} /* _eet_str_direct_free */

static void
_eet_eina_hash_foreach(void             *hash,
                       Eina_Hash_Foreach cb,
                       void             *fdata)
{
   if (hash)
     eina_hash_foreach(hash, cb, fdata);
} /* _eet_eina_hash_foreach */

/* Internal wrapper for eina_hash */
static Eina_Hash *
_eet_eina_hash_add(Eina_Hash  *hash,
                   const char *key,
                   const void *data)
{
   if (!hash)
     hash = eina_hash_string_superfast_new(NULL);

   if (!hash)
     return NULL;

   eina_hash_add(hash, key, data);
   return hash;
} /* _eet_eina_hash_add */

static void
_eet_eina_hash_free(Eina_Hash *hash)
{
   if (hash)
     eina_hash_free(hash);
} /* _eet_eina_hash_free */

void
eet_test_setup_eddc(Eet_Data_Descriptor_Class *eddc)
{
   eddc->version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc->func.mem_alloc = NULL;
   eddc->func.mem_free = NULL;
   eddc->func.str_alloc = NULL;
   eddc->func.str_free = NULL;
   eddc->func.list_next = (void *)eina_list_next;
   eddc->func.list_append = (void *)eina_list_append;
   eddc->func.list_data = (void *)eina_list_data_get;
   eddc->func.list_free = (void *)eina_list_free;
   eddc->func.hash_foreach = (void *)_eet_eina_hash_foreach;
   eddc->func.hash_add = (void *)_eet_eina_hash_add;
   eddc->func.hash_free = (void *)_eet_eina_hash_free;
   eddc->func.str_direct_alloc = (void *)_eet_str_direct_alloc;
   eddc->func.str_direct_free = (void *)_eet_str_direct_free;
   eddc->func.array_alloc = NULL;
   eddc->func.array_free = NULL;
}

void
eet_build_basic_descriptor(Eet_Data_Descriptor *edd)
{
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "c",
                                 c,
                                 EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "s",
                                 s,
                                 EET_T_SHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "i",
                                 i,
                                 EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "l",
                                 l,
                                 EET_T_LONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "str",
                                 str,
                                 EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "istr",
                                 istr,
                                 EET_T_INLINED_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "f1",
                                 f1,
                                 EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "f2",
                                 f2,
                                 EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "d",
                                 d,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "uc",
                                 uc,
                                 EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "us",
                                 us,
                                 EET_T_USHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "ui",
                                 ui,
                                 EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "ul",
                                 ul,
                                 EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Basic_Type,
                                 "vp",
                                 vp,
                                 EET_T_VALUE);

   EET_DATA_DESCRIPTOR_ADD_SUB(edd, Eet_Test_Basic_Type, "empty", empty, edd);
   EET_DATA_DESCRIPTOR_ADD_SUB(edd, Eet_Test_Basic_Type, "with", with, edd);
}

void
eet_test_basic_check(Eet_Test_Basic_Type *result,
                     int                  i,
                     Eina_Bool            dumper)
{
   int test = -1;
   float tmp;

   fail_if(result->c != EET_TEST_CHAR);
   fail_if(result->s != EET_TEST_SHORT);
   fail_if(result->i != EET_TEST_INT + i);
   fail_if(result->l != (long long)EET_TEST_LONG_LONG);
   fail_if(strcmp(result->str, EET_TEST_STRING) != 0);
   fail_if(strcmp(result->istr, EET_TEST_STRING) != 0);
   fail_if(result->uc != EET_TEST_CHAR);
   fail_if(result->us != EET_TEST_SHORT);
   fail_if(result->ui != EET_TEST_INT);
   fail_if(result->ul != EET_TEST_LONG_LONG);
   if (!dumper)
     {
        fail_if(result->vp == NULL);
        eina_value_get(result->vp, &test);
        fail_if(test != EET_TEST_INT + i);
     }
   else
     {
        fail_if(result->vp != NULL);
     }

   tmp = (result->f1 + EET_TEST_FLOAT);
   if (tmp < 0)
     tmp = -tmp;

   fail_if(tmp > 0.005);

   tmp = (result->f2 - EET_TEST_FLOAT4);
   if (tmp < 0)
     tmp = -tmp;

   fail_if(tmp > 0.005);

   tmp = (result->d + EET_TEST_DOUBLE);
   if (tmp < 0)
     tmp = -tmp;

   fail_if(tmp > 0.00005);

   fail_if(result->empty != NULL);
   if (i == 0)
     {
        Eet_Test_Basic_Type *tmp2;

        tmp2 = result->with;
        fail_if(tmp2 == NULL);

        fail_if(tmp2->c != EET_TEST_CHAR);
        fail_if(tmp2->s != EET_TEST_SHORT);
        fail_if(tmp2->i != EET_TEST_INT + i + 1);
        fail_if(tmp2->l != (long long)EET_TEST_LONG_LONG);
        fail_if(strcmp(tmp2->str, EET_TEST_STRING) != 0);
        fail_if(strcmp(tmp2->istr, EET_TEST_STRING) != 0);
        fail_if(tmp2->uc != EET_TEST_CHAR);
        fail_if(tmp2->us != EET_TEST_SHORT);
        fail_if(tmp2->ui != EET_TEST_INT);
        fail_if(tmp2->ul != EET_TEST_LONG_LONG);
        fail_if(tmp2->vp != NULL);
     }
   else
     fail_if(result->with != NULL);
}

void
eet_build_ex_descriptor(Eet_Data_Descriptor *edd, Eina_Bool stream)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   Eet_Data_Descriptor *eddb;

   if (stream)
     {
        eet_eina_stream_data_descriptor_class_set(&eddc,
                                                  sizeof (Eet_Data_Descriptor_Class),
                                                  "Eet_Test_Basic_Type",
                                                  sizeof(Eet_Test_Basic_Type));
        eddb = eet_data_descriptor_stream_new(&eddc);
     }
   else
     {
        eet_eina_file_data_descriptor_class_set(&eddc,
                                                sizeof (Eet_Data_Descriptor_Class),
                                                "Eet_Test_Basic_Type",
                                                sizeof(Eet_Test_Basic_Type));
        eddb = eet_data_descriptor_file_new(&eddc);
     }
   fail_if(!eddb);

   eet_build_basic_descriptor(eddb);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "c",
                                 c,
                                 EET_T_CHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "s",
                                 s,
                                 EET_T_SHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Eet_Test_Ex_Type, "i", i, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "l",
                                 l,
                                 EET_T_LONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "str",
                                 str,
                                 EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "istr",
                                 istr,
                                 EET_T_INLINED_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "f1",
                                 f1,
                                 EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "f2",
                                 f2,
                                 EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "f3",
                                 f3,
                                 EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "f4",
                                 f4,
                                 EET_T_FLOAT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "d1",
                                 d1,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "d2",
                                 d2,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "d3",
                                 d3,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "d4",
                                 d4,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "uc",
                                 uc,
                                 EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "us",
                                 us,
                                 EET_T_USHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "ui",
                                 ui,
                                 EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd,
                                 Eet_Test_Ex_Type,
                                 "ul",
                                 ul,
                                 EET_T_ULONG_LONG);
   EET_DATA_DESCRIPTOR_ADD_ARRAY(edd,
                                 Eet_Test_Ex_Type,
                                 "sarray1",
                                 sarray1,
                                 eddb);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(edd,
                                     Eet_Test_Ex_Type,
                                     "varray2",
                                     varray2,
                                     eddb);
   eet_data_descriptor_element_add(edd,
                                   "varray1",
                                   EET_T_INT,
                                   EET_G_VAR_ARRAY,
                                   (char *)(&(etbt.varray1)) - (char *)(&(etbt)),
                                   (char *)(&(etbt.varray1_count)) -
                                   (char *)(&(etbt)),
                                   /* 0,  */ NULL,
                                   NULL);
   eet_data_descriptor_element_add(edd, "sarray2", EET_T_INT, EET_G_ARRAY,
                                   (char *)(&(etbt.sarray2)) - (char *)(&(etbt)),
                                   /* 0,  */ sizeof(etbt.sarray2) /
                                   sizeof(etbt.sarray2[0]), NULL, NULL);
   eet_data_descriptor_element_add(edd, "charray", EET_T_STRING, EET_G_ARRAY,
                                   (char *)(&(etbt.charray)) - (char *)(&(etbt)),
                                   /* 0,  */ sizeof(etbt.charray) /
                                   sizeof(etbt.charray[0]), NULL, NULL);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd, Eet_Test_Ex_Type, "list", list, edd);
   EET_DATA_DESCRIPTOR_ADD_HASH(edd, Eet_Test_Ex_Type, "hash", hash, edd);
   eet_data_descriptor_element_add(edd, "ilist", EET_T_INT, EET_G_LIST,
                                   (char *)(&(etbt.ilist)) - (char *)(&(etbt)),
                                   0, /* 0,  */ NULL, NULL);
   eet_data_descriptor_element_add(edd, "ihash", EET_T_INT, EET_G_HASH,
                                   (char *)(&(etbt.ihash)) - (char *)(&(etbt)),
                                   0, /* 0,  */ NULL, NULL);
   eet_data_descriptor_element_add(edd, "slist", EET_T_STRING, EET_G_LIST,
                                   (char *)(&(etbt.slist)) - (char *)(&(etbt)),
                                   0, /* 0,  */ NULL, NULL);
   eet_data_descriptor_element_add(edd, "shash", EET_T_STRING, EET_G_HASH,
                                   (char *)(&(etbt.shash)) - (char *)(&(etbt)),
                                   0, /* 0,  */ NULL, NULL);
}

Eet_Test_Ex_Type *
eet_test_ex_set(Eet_Test_Ex_Type *res,
                int               offset)
{
   unsigned int i;

   if (!res)
     res = malloc(sizeof(Eet_Test_Ex_Type));

   if (!res)
     return NULL;

   res->c = EET_TEST_CHAR + offset;
   res->s = EET_TEST_SHORT + offset;
   res->i = EET_TEST_INT + offset;
   res->l = EET_TEST_LONG_LONG + offset;
   res->str = EET_TEST_STRING;
   res->istr = EET_TEST_STRING;
   res->f1 = EET_TEST_FLOAT + offset;
   res->f2 = -(EET_TEST_FLOAT2 + offset);
   res->f3 = EET_TEST_FLOAT3 + offset;
   res->f4 = EET_TEST_FLOAT2 + offset;
   res->d1 = EET_TEST_DOUBLE + offset;
   res->d2 = -(EET_TEST_DOUBLE2 + offset);
   res->d3 = EET_TEST_DOUBLE3 + offset;
   res->d4 = EET_TEST_DOUBLE2 + offset;
   res->list = NULL;
   res->hash = NULL;
   res->ilist = NULL;
   res->ihash = NULL;
   res->slist = NULL;
   res->shash = NULL;
   for (i = 0; i < sizeof(res->charray) / sizeof(res->charray[0]); ++i)
     res->charray[i] = NULL;

   res->varray2 = malloc(sizeof (Eet_Test_Basic_Type) * 10);
   res->varray1 = malloc(sizeof (int) * 5);
   fail_if(!res->varray1 || !res->varray2);
   for (i = 0; i < 10; ++i)
     {
        eet_test_basic_set(res->sarray1 + i, i);
        eet_test_basic_set(res->varray2 + i, i);
     }
   res->varray2_count = 10;
   for (i = 0; i < 5; ++i)
     {
        res->sarray2[i] = i * 42 + 1;
        res->varray1[i] = i * 42 + 1;
     }
   res->varray1_count = 5;

   res->uc = EET_TEST_CHAR + offset;
   res->us = EET_TEST_SHORT + offset;
   res->ui = EET_TEST_INT + offset;
   res->ul = EET_TEST_LONG_LONG + offset;

   return res;
}

int
eet_test_ex_check(Eet_Test_Ex_Type *stuff,
                  int               offset,
                  Eina_Bool         dumper)
{
   double tmp;
   unsigned int i;

   if (!stuff)
     return 1;

   if (stuff->c != EET_TEST_CHAR + offset)
     return 1;

   if (stuff->s != EET_TEST_SHORT + offset)
     return 1;

   if (stuff->i != EET_TEST_INT + offset)
     return 1;

   if (stuff->l != EET_TEST_LONG_LONG + offset)
     return 1;

   if (strcmp(stuff->str, EET_TEST_STRING) != 0)
     return 1;

   if (strcmp(stuff->istr, EET_TEST_STRING) != 0)
     return 1;

   tmp = stuff->f1 - (EET_TEST_FLOAT + offset);
   if (tmp < 0)
     tmp = -tmp;

   if (tmp > 0.005)
     return 1;

   tmp = stuff->d1 - (EET_TEST_DOUBLE + offset);
   if (tmp < 0)
     tmp = -tmp;

   if (tmp > 0.00005)
     return 1;

   if (!EINA_FLT_EQ(stuff->f2, -(EET_TEST_FLOAT2 + offset)))
     return 1;

   if (!EINA_DBL_EQ(stuff->d2, -(EET_TEST_DOUBLE2 + offset)))
     return 1;

   if (!EINA_FLT_EQ(stuff->f3, EET_TEST_FLOAT3 + offset))
     return 1;

   if (!EINA_DBL_EQ(stuff->d3, EET_TEST_DOUBLE3 + offset))
     return 1;

   if (!EINA_FLT_EQ(stuff->f4, EET_TEST_FLOAT2 + offset))
     return 1;

   if (!EINA_DBL_EQ(stuff->d4, EET_TEST_DOUBLE2 + offset))
     return 1;

   if (stuff->uc != EET_TEST_CHAR + offset)
     return 1;

   if (stuff->us != EET_TEST_SHORT + offset)
     return 1;

   if (stuff->ui != (unsigned int)EET_TEST_INT + offset)
     return 1;

   if (stuff->ul != EET_TEST_LONG_LONG + offset)
     return 1;

   if (stuff->varray1_count != 5)
     return 1;

   if (stuff->varray2_count != 10)
     return 1;

   for (i = 0; i < 5; ++i)
     if (stuff->sarray2[i] != i * 42 + 1 && stuff->varray1[i] != i * 42 + 1)
       return 1;

   for (i = 0; i < 10; ++i)
     {
        eet_test_basic_check(stuff->sarray1 + i, i, dumper);
        eet_test_basic_check(stuff->varray2 + i, i, dumper);
     }

   return 0;
}

void
eet_test_basic_set(Eet_Test_Basic_Type *res,
                   int                  i)
{
   res->c = EET_TEST_CHAR;
   res->s = EET_TEST_SHORT;
   res->i = EET_TEST_INT + i;
   res->l = EET_TEST_LONG_LONG;
   res->str = EET_TEST_STRING;
   res->istr = EET_TEST_STRING;
   res->f1 = -EET_TEST_FLOAT;
   res->d = -EET_TEST_DOUBLE;
   res->f2 = EET_TEST_FLOAT4;
   res->uc = EET_TEST_CHAR;
   res->us = EET_TEST_SHORT;
   res->ui = EET_TEST_INT;
   res->ul = EET_TEST_LONG_LONG;
   res->empty = NULL;
   res->with = NULL;
   res->vp = eina_value_new(EINA_VALUE_TYPE_INT);
   eina_value_set(res->vp, EET_TEST_INT + i);

   if (i == 0)
     {
        Eet_Test_Basic_Type *tmp;

        tmp = malloc(sizeof (Eet_Test_Basic_Type));
        fail_if(!tmp);

        res->with = tmp;
        tmp->c = EET_TEST_CHAR;
        tmp->s = EET_TEST_SHORT;
        tmp->i = EET_TEST_INT + i + 1;
        tmp->l = EET_TEST_LONG_LONG;
        tmp->str = EET_TEST_STRING;
        tmp->istr = EET_TEST_STRING;
        tmp->f1 = -EET_TEST_FLOAT;
        tmp->d = -EET_TEST_DOUBLE;
        tmp->f2 = EET_TEST_FLOAT4;
        tmp->uc = EET_TEST_CHAR;
        tmp->us = EET_TEST_SHORT;
        tmp->ui = EET_TEST_INT;
        tmp->ul = EET_TEST_LONG_LONG;
        tmp->empty = NULL;
        tmp->with = NULL;
        tmp->vp = NULL;
     }
}
