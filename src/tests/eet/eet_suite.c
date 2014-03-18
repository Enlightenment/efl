#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <Eina.h>

#include <check.h>

#include "eet_suite.h"

static char _key_pem[PATH_MAX] = "";
static char _cert_pem[PATH_MAX] = "";
static char _key_enc[PATH_MAX] = "";
static char _key_enc_pem[PATH_MAX] = "";
static char _key_enc_none_pem[PATH_MAX] = "";

START_TEST(eet_test_init)
{
   int ret;

   ret = eet_init();
   fail_if(ret != 1);

   ret = eet_shutdown();
   fail_if(ret != 0);
}
END_TEST

typedef struct _Eet_Test_Basic_Type Eet_Test_Basic_Type;
struct _Eet_Test_Basic_Type
{
   char                 c;
   short                s;
   int                  i;
   long long            l;
   char                *str;
   char                *istr;
   float                f1;
   float                f2;
   double               d;
   unsigned char        uc;
   unsigned short       us;
   unsigned int         ui;
   unsigned long long   ul;
   Eina_Value          *vp;
   Eet_Test_Basic_Type *empty;
   Eet_Test_Basic_Type *with;
};

#define EET_TEST_CHAR       0x42
#define EET_TEST_SHORT      0x4224
#define EET_TEST_INT        0x42211224
#define EET_TEST_LONG_LONG  0x84CB42211224BC48
#define EET_TEST_STRING     "my little test with escape \\\""
#define EET_TEST_KEY1       "key1"
#define EET_TEST_KEY2       "key2"
#define EET_TEST_FLOAT      123.45689
#define EET_TEST_FLOAT2     1.0
#define EET_TEST_FLOAT3     0.25
#define EET_TEST_FLOAT4     0.0001234
#define EET_TEST_DOUBLE     123456789.9876543210
#define EET_TEST_DOUBLE2    1.0
#define EET_TEST_DOUBLE3    0.25
#define EET_TEST_FILE_KEY1  "keys/data/1"
#define EET_TEST_FILE_KEY2  "keys/data/2"
#define EET_TEST_FILE_IMAGE "keys/images/"

typedef struct _Eet_Test_Image Eet_Test_Image;
struct _Eet_Test_Image
{
   unsigned int w;
   unsigned int h;
   int          alpha;
   unsigned int color[64];
};

static const Eet_Test_Image test_noalpha = {
   8, 8, 0,
   {
      0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00,
      0x000000AA, 0x00110000,
      0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA,
      0x00110000, 0x00AA0000,
      0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000,
      0x00AA0000, 0x0000AA00,
      0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000,
      0x0000AA00, 0x000000AA,
      0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00,
      0x000000AA, 0x00110000,
      0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA,
      0x00110000, 0x00AA0000,
      0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000,
      0x00AA0000, 0x0000AA00,
      0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000,
      0x0000AA00, 0x000000AA
   }
};

static const Eet_Test_Image test_alpha = {
   8, 8, 1,
   {
      0x0FAA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000, 0x0000AA00,
      0x000000AA, 0x0F110000,
      0x0000AA00, 0x0F0000AA, 0x00110000, 0x00AA0000, 0x0000AA00, 0x000000AA,
      0x0F110000, 0x00AA0000,
      0x000000AA, 0x00110000, 0x0FAA0000, 0x0000AA00, 0x000000AA, 0x0F110000,
      0x00AA0000, 0x0000AA00,
      0x00110000, 0x00AA0000, 0x0000AA00, 0x0F0000AA, 0x0F110000, 0x00AA0000,
      0x0000AA00, 0x000000AA,
      0x00AA0000, 0x0000AA00, 0x000000AA, 0x0F110000, 0x0FAA0000, 0x0000AA00,
      0x000000AA, 0x00110000,
      0x0000AA00, 0x000000AA, 0x0F110000, 0x00AA0000, 0x0000AA00, 0x0F0000AA,
      0x00110000, 0x00AA0000,
      0x000000AA, 0x0F110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000,
      0x0FAA0000, 0x0000AA00,
      0x0F110000, 0x00AA0000, 0x0000AA00, 0x000000AA, 0x00110000, 0x00AA0000,
      0x0000AA00, 0x0F0000AA
   }
};

static void
_eet_test_basic_set(Eet_Test_Basic_Type *res,
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
} /* _eet_test_basic_set */

static void
_eet_test_basic_check(Eet_Test_Basic_Type *result,
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
} /* _eet_test_basic_check */

static void
_eet_build_basic_descriptor(Eet_Data_Descriptor *edd)
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
} /* _eet_build_basic_descriptor */

START_TEST(eet_test_basic_data_type_encoding_decoding)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Basic_Type *result;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Basic_Type etbt;
   void *transfert;
   int size;

   eet_init();

   _eet_test_basic_set(&etbt, 0);

   eet_test_setup_eddc(&eddc);
   eddc.name = "Eet_Test_Basic_Type";
   eddc.size = sizeof(Eet_Test_Basic_Type);

   edd = eet_data_descriptor_stream_new(&eddc);
   fail_if(!edd);

   _eet_build_basic_descriptor(edd);

   transfert = eet_data_descriptor_encode(edd, &etbt, &size);
   fail_if(!transfert || size <= 0);

   result = eet_data_descriptor_decode(edd, transfert, size);
   fail_if(!result);

   _eet_test_basic_check(result, 0, EINA_FALSE);

   free(result->str);
   free(result);

   eet_data_descriptor_free(edd);

   eet_shutdown();
}
END_TEST

typedef struct _Eet_Test_Ex_Type Eet_Test_Ex_Type;
struct _Eet_Test_Ex_Type
{
   char                 c;
   short                s;
   int                  i;
   unsigned long long   l;
   char                *str;
   char                *istr;
   float                f1;
   float                f2;
   float                f3;
   float                f4;
   double               d1;
   double               d2;
   double               d3;
   double               d4;
   Eina_List           *list;
   Eina_Hash           *hash;
   Eina_List           *ilist;
   Eina_List           *slist;
   Eina_Hash           *ihash;
   Eina_Hash           *shash;
   Eet_Test_Basic_Type  sarray1[10];
   unsigned int         sarray2[5];
   unsigned int         varray1_count;
   unsigned int        *varray1;
   unsigned int         varray2_count;
   Eet_Test_Basic_Type *varray2;
   unsigned char        uc;
   unsigned short       us;
   unsigned int         ui;
   unsigned long long   ul;
   char                *charray[10];
};

static int i42 = 42;
static int i7 = 7;

static void
_eet_build_ex_descriptor(Eet_Data_Descriptor *edd, Eina_Bool stream)
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

   _eet_build_basic_descriptor(eddb);

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
} /* _eet_build_ex_descriptor */

static Eet_Test_Ex_Type *
_eet_test_ex_set(Eet_Test_Ex_Type *res,
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
        _eet_test_basic_set(res->sarray1 + i, i);
        _eet_test_basic_set(res->varray2 + i, i);
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
} /* _eet_test_ex_set */

static int
_eet_test_ex_check(Eet_Test_Ex_Type *stuff,
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

   if (stuff->f2 != -(EET_TEST_FLOAT2 + offset))
     return 1;

   if (stuff->d2 != -(EET_TEST_DOUBLE2 + offset))
     return 1;

   if (stuff->f3 != EET_TEST_FLOAT3 + offset)
     return 1;

   if (stuff->d3 != EET_TEST_DOUBLE3 + offset)
     return 1;

   if (stuff->f4 != EET_TEST_FLOAT2 + offset)
     return 1;

   if (stuff->d4 != EET_TEST_DOUBLE2 + offset)
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
        _eet_test_basic_check(stuff->sarray1 + i, i, dumper);
        _eet_test_basic_check(stuff->varray2 + i, i, dumper);
     }

   return 0;
} /* _eet_test_ex_check */

static Eina_Bool _dump_call = EINA_FALSE;

static Eina_Bool
func(EINA_UNUSED const Eina_Hash *hash,
     const void                 *key,
     void                       *data,
     void                       *fdata)
{
   int *res = fdata;

   if (strcmp(key, EET_TEST_KEY1) != 0
       && strcmp(key, EET_TEST_KEY2) != 0)
     *res = 1;

   if (_eet_test_ex_check(data, 2, _dump_call))
     *res = 1;

   return EINA_TRUE;
} /* func */

static Eina_Bool
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

   if (*val != 7)
     *res = 1;

   return EINA_TRUE;
} /* func7 */

START_TEST(eet_test_data_type_encoding_decoding)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Ex_Type *result;
   void *transfert;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   int size;
   int test;

   eet_init();

   _eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, _eet_test_ex_set(NULL, 2));
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

   _eet_build_ex_descriptor(edd, EINA_FALSE);

   transfert = eet_data_descriptor_encode(edd, &etbt, &size);
   fail_if(!transfert || size <= 0);

   result = eet_data_descriptor_decode(edd, transfert, size);
   fail_if(!result);

   fail_if(_eet_test_ex_check(result, 0, EINA_FALSE) != 0);
   fail_if(_eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_FALSE) != 0);
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

   eet_shutdown();
}
END_TEST

static void
append_string(void       *data,
              const char *str)
{
   char **string = data;
   int length;

   if (!data)
     return;

   length = *string ? strlen(*string) : 0;
   *string = realloc(*string, strlen(str) + length + 1);

   memcpy((*string) + length, str, strlen(str) + 1);
} /* append_string */

START_TEST(eet_test_data_type_dump_undump)
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

   eet_init();

   _eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, _eet_test_ex_set(NULL, 2));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY2, _eet_test_ex_set(NULL, 2));
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

   _eet_build_ex_descriptor(edd, EINA_FALSE);

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

   fail_if(_eet_test_ex_check(result, 0, EINA_TRUE) != 0);
   fail_if(_eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_TRUE) != 0);
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

   eet_shutdown();
}
END_TEST

static void
append_strbuf_string(void *data, const char *string)
{
   Eina_Strbuf *strbuf = data;
   eina_strbuf_append(strbuf, string);
}

START_TEST(eet_test_data_type_escape_dump_undump)
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

   eet_init();

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

   eet_shutdown();
}
END_TEST
START_TEST(eet_file_simple_write)
{
   const char *buffer = "Here is a string of data to save !";
   Eet_File *ef;
   char *test;
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   int size;

   eet_init();

   fail_if(!(file = tmpnam(file)));

   fail_if(eet_mode_get(NULL) != EET_FILE_MODE_INVALID);

   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_write(ef, "keys/tests", buffer, strlen(buffer) + 1, 1));
   fail_if(!eet_alias(ef, "keys/alias", "keys/tests", 0));
   fail_if(!eet_alias(ef, "keys/alias2", "keys/alias", 1));

   fail_if(eet_mode_get(ef) != EET_FILE_MODE_WRITE);

   fail_if(eet_list(ef, "*", &size) != NULL);
   fail_if(eet_num_entries(ef) != -1);

   eet_close(ef);

   /* Test read of simple file */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read(ef, "keys/tests", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   test = eet_read(ef, "keys/alias2", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(eet_read_direct(ef, "key/alias2", &size));

   fail_if(eet_mode_get(ef) != EET_FILE_MODE_READ);
   fail_if(eet_num_entries(ef) != 3);

   eet_close(ef);

   /* Test eet cache system */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read(ef, "keys/tests", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   eet_close(ef);

   fail_if(unlink(file) != 0);

   eet_shutdown();
} /* START_TEST */

END_TEST
START_TEST(eet_file_data_test)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Ex_Type *result;
   Eet_Dictionary *ed;
   Eet_File *ef;
   char **list;
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   int size;
   int test;

   eet_init();

   _eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, _eet_test_ex_set(NULL, 2));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY2, _eet_test_ex_set(NULL, 2));
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

   _eet_build_ex_descriptor(edd, EINA_FALSE);

   fail_if(!(file = tmpnam(file)));

   /* Insert an error in etbt. */
   etbt.i = 0;

   /* Save the encoded data in a file. */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   fail_if(eet_mode_get(ef) != EET_FILE_MODE_READ_WRITE);

   /* Test string space. */
   ed = eet_dictionary_get(ef);

   fail_if(!eet_dictionary_string_check(ed, result->str));
   fail_if(eet_dictionary_string_check(ed, result->istr));

   eet_close(ef);

   /* Attempt to replace etbt by the correct one. */
   etbt.i = EET_TEST_INT;

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   /* Test the resulting data. */
   fail_if(_eet_test_ex_check(result, 0, EINA_FALSE) != 0);

   eet_close(ef);

   /* Read back the data. */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY2, &etbt, 0));

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   /* Test string space. */
   ed = eet_dictionary_get(ef);
   fail_if(!ed);

   fail_if(!eet_dictionary_string_check(ed, result->str));
   fail_if(eet_dictionary_string_check(ed, result->istr));

   /* Test the resulting data. */
   fail_if(_eet_test_ex_check(result, 0, EINA_FALSE) != 0);
   fail_if(_eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_FALSE) != 0);
   fail_if(eina_list_data_get(result->ilist) == NULL);
   fail_if(*((int *)eina_list_data_get(result->ilist)) != 42);
   fail_if(eina_list_data_get(result->slist) == NULL);
   fail_if(strcmp(eina_list_data_get(result->slist), "test") != 0);
   fail_if(eina_hash_find(result->shash, EET_TEST_KEY1) == NULL);
   fail_if(strcmp(eina_hash_find(result->shash, EET_TEST_KEY1), "test") != 0);
   fail_if(strcmp(result->charray[0], "test") != 0);

   test = 0;
   if (result->hash)
     eina_hash_foreach(result->hash, func, &test);

   fail_if(test != 0);
   if (result->ihash)
     eina_hash_foreach(result->ihash, func7, &test);

   fail_if(test != 0);

   list = eet_list(ef, "keys/*", &size);
   fail_if(eet_num_entries(ef) != 2);
   fail_if(size != 2);
   fail_if(!(strcmp(list[0],
                    EET_TEST_FILE_KEY1) == 0 &&
             strcmp(list[1], EET_TEST_FILE_KEY2) == 0)
           && !(strcmp(list[0],
                       EET_TEST_FILE_KEY2) == 0 &&
                strcmp(list[1], EET_TEST_FILE_KEY1) == 0));
   free(list);

   fail_if(eet_delete(ef, NULL) != 0);
   fail_if(eet_delete(NULL, EET_TEST_FILE_KEY1) != 0);
   fail_if(eet_delete(ef, EET_TEST_FILE_KEY1) == 0);

   list = eet_list(ef, "keys/*", &size);
   fail_if(size != 1);
   fail_if(eet_num_entries(ef) != 1);

   /* Test some more wrong case */
   fail_if(eet_data_read(ef, edd, "plop") != NULL);
   fail_if(eet_data_read(ef, edd, EET_TEST_FILE_KEY1) != NULL);

   /* Reinsert and reread data */
   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));
   fail_if(eet_data_read(ef, edd, EET_TEST_FILE_KEY1) == NULL);
   fail_if(eet_read_direct(ef, EET_TEST_FILE_KEY1, &size) == NULL);

   eet_close(ef);

   fail_if(unlink(file) != 0);

   eet_shutdown();
} /* START_TEST */

END_TEST
START_TEST(eet_file_data_dump_test)
{
   Eet_Data_Descriptor *edd;
   Eet_Test_Ex_Type *result;
   Eet_Data_Descriptor_Class eddc;
   Eet_Test_Ex_Type etbt;
   Eet_File *ef;
   char *string1;
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   int test;

   eet_init();

   _eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, _eet_test_ex_set(NULL, 2));
   eina_hash_add(etbt.hash, EET_TEST_KEY2, _eet_test_ex_set(NULL, 2));
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY1, &i7);
   eina_hash_add(etbt.ihash, EET_TEST_KEY2, &i7);
   etbt.slist = eina_list_prepend(NULL, "test");
   etbt.shash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.shash, EET_TEST_KEY1, "test");
   memset(&etbt.charray, 0, sizeof(etbt.charray));
   etbt.charray[0] = "test";

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc),
                                           "Eet_Test_Ex_Type",
                                           sizeof(Eet_Test_Ex_Type));

   edd = eet_data_descriptor_file_new(&eddc);
   fail_if(!edd);

   _eet_build_ex_descriptor(edd, EINA_FALSE);

   fail_if(!(file = tmpnam(file)));

   /* Save the encoded data in a file. */
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd, EET_TEST_FILE_KEY1, &etbt, 0));

   eet_close(ef);

   /* Use dump/undump in the middle */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   string1 = NULL;
   fail_if(eet_data_dump(ef, EET_TEST_FILE_KEY1, append_string, &string1) != 1);
   fail_if(eet_delete(ef, EET_TEST_FILE_KEY1) == 0);
   fail_if(!eet_data_undump(ef, EET_TEST_FILE_KEY1, string1, strlen(string1), 1));

   eet_close(ef);

   /* Test the correctness of the reinsertion. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   result = eet_data_read(ef, edd, EET_TEST_FILE_KEY1);
   fail_if(!result);

   eet_close(ef);

   /* Test the resulting data. */
   fail_if(_eet_test_ex_check(result, 0, EINA_TRUE) != 0);
   fail_if(_eet_test_ex_check(eina_list_data_get(result->list), 1, EINA_TRUE) != 0);
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

   fail_if(unlink(file) != 0);

   eet_shutdown();
} /* START_TEST */

END_TEST
START_TEST(eet_image)
{
   Eet_File *ef;
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   unsigned int *data;
   int compress;
   int quality;
   int result;
   Eet_Image_Encoding lossy;
   int alpha;
   unsigned int w;
   unsigned int h;

   eet_init();

   fail_if(!(file = tmpnam(file)));

   /* Save the encoded data in a file. */
   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "0",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 100,
                                 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "1",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 5,
                                 100,
                                 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "2",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 9,
                                 100,
                                 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "3",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 100,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "4",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 60,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "5",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 10,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef,
                                 EET_TEST_FILE_IMAGE "6",
                                 test_noalpha.color,
                                 test_noalpha.w,
                                 test_noalpha.h,
                                 test_noalpha.alpha,
                                 0,
                                 0,
                                 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef, EET_TEST_FILE_IMAGE "7", test_alpha.color,
                                 test_alpha.w, test_alpha.h, test_alpha.alpha,
                                 9, 100, 0);
   fail_if(result == 0);

   result = eet_data_image_write(ef, EET_TEST_FILE_IMAGE "8", test_alpha.color,
                                 test_alpha.w, test_alpha.h, test_alpha.alpha,
                                 0, 80, 1);
   fail_if(result == 0);

   result = eet_data_image_write(ef, EET_TEST_FILE_IMAGE "9", test_alpha.color,
                                 test_alpha.w, test_alpha.h, test_alpha.alpha,
                                 0, 100, 1);
   fail_if(result == 0);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "2",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "2",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);

   eet_close(ef);

   /* Test read of image */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "0",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 0);
   fail_if(lossy != 0);

   data = malloc(w * h * 4);
   fail_if(data == NULL);
   result = eet_data_image_read_to_surface(ef,
                                           EET_TEST_FILE_IMAGE "0",
                                           4,
                                           4,
                                           data,
                                           2,
                                           2,
                                           w * 4,
                                           &alpha,
                                           &compress,
                                           &quality,
                                           &lossy);
   fail_if(result != 1);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 0);
   fail_if(quality != 100);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[4 + 4 * w]);
   free(data);

   data = malloc(w * h * 4);
   fail_if(data == NULL);
   result = eet_data_image_read_to_surface(ef,
                                           EET_TEST_FILE_IMAGE "0",
                                           0,
                                           0,
                                           data,
                                           w,
                                           h,
                                           w * 4,
                                           &alpha,
                                           &compress,
                                           &quality,
                                           &lossy);
   fail_if(result != 1);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 0);
   fail_if(quality != 100);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "1",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 5);
   fail_if(quality != 100);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "2",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);
   fail_if(data[0] != test_noalpha.color[0]);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "3",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(lossy != 1);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "5",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(lossy != 1);
   free(data);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "6",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_noalpha.w);
   fail_if(h != test_noalpha.h);
   fail_if(alpha != test_noalpha.alpha);
   fail_if(lossy != 1);
   free(data);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "7",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "7",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(compress != 9);
   fail_if(lossy != 0);
   fail_if(data[0] != test_alpha.color[0]);
   free(data);

   result = eet_data_image_header_read(ef,
                                       EET_TEST_FILE_IMAGE "9",
                                       &w,
                                       &h,
                                       &alpha,
                                       &compress,
                                       &quality,
                                       &lossy);
   fail_if(result == 0);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(lossy != 1);

   data = eet_data_image_read(ef,
                              EET_TEST_FILE_IMAGE "9",
                              &w,
                              &h,
                              &alpha,
                              &compress,
                              &quality,
                              &lossy);
   fail_if(data == NULL);
   fail_if(w != test_alpha.w);
   fail_if(h != test_alpha.h);
   fail_if(alpha != test_alpha.alpha);
   fail_if(lossy != 1);
   free(data);

   eet_close(ef);

   fail_if(unlink(file) != 0);

   eet_shutdown();
} /* START_TEST */

END_TEST

#define IM0 0x00112233
#define IM1 0x44556677
#define IM2 0x8899aabb
#define IM3 0xccddeeff

START_TEST(eet_small_image)
{
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   unsigned int image[4];
   unsigned int *data;
   Eet_File *ef;
   unsigned int w;
   unsigned int h;
   int alpha;
   int compression;
   int quality;
   Eet_Image_Encoding lossy;
   int result;

   image[0] = IM0;
   image[1] = IM1;
   image[2] = IM2;
   image[3] = IM3;

   eet_init();

   fail_if(!(file = tmpnam(file)));

   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   result = eet_data_image_write(ef, "/images/test", image, 2, 2, 1, 9, 100, 0);
   fail_if(result == 0);

   eet_close(ef);

   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   data = (unsigned int *)eet_data_image_read(ef,
                                              "/images/test",
                                              &w,
                                              &h,
                                              &alpha,
                                              &compression,
                                              &quality,
                                              &lossy);
   fail_if(data == NULL);

   eet_close(ef);

   fail_if(unlink(file) != 0);

   fail_if(data[0] != IM0);
   fail_if(data[1] != IM1);
   fail_if(data[2] != IM2);
   fail_if(data[3] != IM3);

   free(data);

   eet_shutdown();
} /* START_TEST */

END_TEST
START_TEST(eet_identity_simple)
{
   const char *buffer = "Here is a string of data to save !";
   const void *tmp;
   Eet_File *ef;
   Eet_Key *k;
   FILE *noread;
   char *test;
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   int size;
   int fd;

   eet_init();

   fail_if(!(file = tmpnam(file)));
   fail_if(!(noread = fopen("/dev/null", "w")));

   /* Sign an eet file. */
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_write(ef, "keys/tests", buffer, strlen(buffer) + 1, 0));

   k = eet_identity_open(_cert_pem, _key_pem, NULL);
   fail_if(!k);

   fail_if(eet_identity_set(ef, k) != EET_ERROR_NONE);
   eet_identity_print(k, noread);

   eet_close(ef);

   /* Open a signed file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read(ef, "keys/tests", &size);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   tmp = eet_identity_x509(ef, &size);
   fail_if(tmp == NULL);

   eet_identity_certificate_print(tmp, size, noread);

   eet_close(ef);

   /* As we are changing file contain in less than 1s, this could get unnoticed
      by eet cache system. */
   eet_clearcache();

   /* Corrupting the file. */
   fd = open(file, O_WRONLY);
   fail_if(fd < 0);

   fail_if(lseek(fd, 200, SEEK_SET) != 200);
   fail_if(write(fd, "42", 2) != 2);
   fail_if(lseek(fd, 50, SEEK_SET) != 50);
   fail_if(write(fd, "42", 2) != 2);
   fail_if(lseek(fd, 88, SEEK_SET) != 88);
   fail_if(write(fd, "42", 2) != 2);

   close(fd);

   /* Attempt to open a modified file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(ef);

   fail_if(unlink(file) != 0);

   eet_shutdown();
} /* START_TEST */

END_TEST
START_TEST(eet_identity_open_simple)
{
   Eet_Key *k = NULL;

   eet_init();

   k = eet_identity_open(_cert_pem, _key_pem, NULL);
   fail_if(!k);

   if (k)
     eet_identity_close(k);

   eet_shutdown();
} /* START_TEST */

END_TEST
START_TEST(eet_identity_open_pkcs8)
{
   Eet_Key *k = NULL;

   eet_init();

   k = eet_identity_open(_cert_pem, _key_enc_none_pem, NULL);
   fail_if(!k);

   if (k)
     eet_identity_close(k);

   eet_shutdown();
} /* START_TEST */

END_TEST

static int
pass_get(char            *pass,
         int              size,
         EINA_UNUSED int   rwflags,
         EINA_UNUSED void *u)
{
   memset(pass, 0, size);

   if ((int)strlen("password") > size)
     return 0;

   snprintf(pass, size, "%s", "password");
   return strlen(pass);
} /* pass_get */

static int
badpass_get(char            *pass,
            int              size,
            EINA_UNUSED int   rwflags,
            EINA_UNUSED void *u)
{
   memset(pass, 0, size);

   if ((int)strlen("bad password") > size)
     return 0;

   snprintf(pass, size, "%s", "bad password");
   return strlen(pass);
} /* badpass_get */

START_TEST(eet_identity_open_pkcs8_enc)
{
   Eet_Key *k = NULL;

   eet_init();

   k = eet_identity_open(_cert_pem, _key_enc_pem, NULL);
   fail_if(k);

   if (k)
     eet_identity_close(k);

   k = eet_identity_open(_cert_pem, _key_enc_pem, &badpass_get);
   fail_if(k);

   if (k)
     eet_identity_close(k);

   k = eet_identity_open(_cert_pem, _key_enc_pem, &pass_get);
   fail_if(!k);

   if (k)
     eet_identity_close(k);

   eet_shutdown();
}
END_TEST
START_TEST(eet_cipher_decipher_simple)
{
   const char *buffer = "Here is a string of data to save !";
   const char *key = "This is a crypto key";
   const char *key_bad = "This is another crypto key";
   Eet_File *ef;
   char *test;
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   int size;

   eet_init();

   fail_if(!(file = tmpnam(file)));

   /* Crypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);

   fail_if(!eet_write_cipher(ef, "keys/tests", buffer, strlen(buffer) + 1, 0,
                             key));

   eet_close(ef);

   /* Decrypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read_cipher(ef, "keys/tests", &size, key);
   fail_if(!test);
   fail_if(size != (int)strlen(buffer) + 1);

   fail_if(memcmp(test, buffer, strlen(buffer) + 1) != 0);

   eet_close(ef);

   /* Decrypt an eet file. */
   ef = eet_open(file, EET_FILE_MODE_READ);
   fail_if(!ef);

   test = eet_read_cipher(ef, "keys/tests", &size, key_bad);

   if (size == (int)strlen(buffer) + 1)
     fail_if(memcmp(test, buffer, strlen(buffer) + 1) == 0);

   eet_close(ef);

   fail_if(unlink(file) != 0);

   eet_shutdown();
} /* START_TEST */

END_TEST

static Eina_Bool open_worker_stop;
static Eina_Condition open_worker_cond;
static Eina_Lock open_worker_mutex;

static void *
open_close_worker(void *path, Eina_Thread tid EINA_UNUSED)
{
   static Eina_Bool first = EINA_TRUE;

   while (!open_worker_stop)
     {
        Eet_File *ef = eet_open((char const *)path, EET_FILE_MODE_READ);
        if (ef == NULL)
          return "eet_open() failed";
        else
          {
             Eet_Error err_code = eet_close(ef);
             if (err_code != EET_ERROR_NONE)
               return "eet_close() failed";
          }

        if (first)
          {
             eina_lock_take(&open_worker_mutex);
             eina_condition_broadcast(&open_worker_cond);
             eina_lock_release(&open_worker_mutex);
             first = EINA_FALSE;
          }
     }

   return NULL;
}

START_TEST(eet_cache_concurrency)
{
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   const char *buffer = "test data";
   Eet_File *ef;
   void *thread_ret;
   unsigned int n;
   Eina_Thread thread;
   Eina_Bool r;

   eet_init();
   eina_threads_init();

   eina_lock_new(&open_worker_mutex);
   eina_condition_new(&open_worker_cond, &open_worker_mutex);

   /* create a file to test with */
   fail_if(!(file = tmpnam(file)));
   ef = eet_open(file, EET_FILE_MODE_WRITE);
   fail_if(!ef);
   fail_if(!eet_write(ef, "keys/tests", buffer, strlen(buffer) + 1, 0));

   eina_lock_take(&open_worker_mutex);
   /* start a thread that repeatedly opens and closes a file */
   open_worker_stop = 0;
   r = eina_thread_create(&thread, EINA_THREAD_NORMAL, -1, open_close_worker, file);
   fail_unless(r);

   eina_condition_wait(&open_worker_cond);
   eina_lock_release(&open_worker_mutex);

   /* clear the cache repeatedly in this thread */
   for (n = 0; n < 20000; ++n)
     {
        eet_clearcache();
     }

   /* join the other thread, and fail if it returned an error message */
   open_worker_stop = 1;
   thread_ret = eina_thread_join(thread);
   fail_unless(thread_ret == NULL, (char const *)thread_ret);

   eet_close(ef);

   fail_if(unlink(file) != 0);

   eina_threads_shutdown();
   eet_shutdown();
}
END_TEST

typedef struct _Eet_Connection_Data Eet_Connection_Data;
struct _Eet_Connection_Data
{
   Eet_Connection      *conn;
   Eet_Data_Descriptor *edd;
   Eina_Bool            test;
};

static Eina_Bool
_eet_connection_read(const void *eet_data,
                     size_t      size,
                     void       *user_data)
{
   Eet_Connection_Data *dt = user_data;
   Eet_Test_Ex_Type *result;
   Eet_Node *node;
   int test;

   result = eet_data_descriptor_decode(dt->edd, eet_data, size);
   node = eet_data_node_decode_cipher(eet_data, NULL, size);

   /* Test the resulting data. */
   fail_if(!node);
   fail_if(_eet_test_ex_check(result, 0, _dump_call) != 0);
   fail_if(_eet_test_ex_check(eina_list_data_get(result->list), 1, _dump_call) != 0);
   fail_if(eina_list_data_get(result->ilist) == NULL);
   fail_if(*((int *)eina_list_data_get(result->ilist)) != 42);
   fail_if(eina_list_data_get(result->slist) == NULL);
   fail_if(strcmp(eina_list_data_get(result->slist), "test") != 0);
   fail_if(eina_hash_find(result->shash, EET_TEST_KEY1) == NULL);
   fail_if(strcmp(eina_hash_find(result->shash, EET_TEST_KEY1), "test") != 0);
   fail_if(strcmp(result->charray[0], "test") != 0);

   test = 0;
   if (result->hash)
     eina_hash_foreach(result->hash, func, &test);

   fail_if(test != 0);
   if (result->ihash)
     eina_hash_foreach(result->ihash, func7, &test);

   fail_if(test != 0);

   if (!dt->test)
     {
        dt->test = EINA_TRUE;
        _dump_call = EINA_TRUE;
        fail_if(!eet_connection_node_send(dt->conn, node, NULL));
        _dump_call = EINA_FALSE;
     }

   return EINA_TRUE;
} /* _eet_connection_read */

static Eina_Bool
_eet_connection_write(const void *data,
                      size_t      size,
                      void       *user_data)
{
   Eet_Connection_Data *dt = user_data;
   int still;

   if (!dt->test)
     {
        int step = size / 3;

        eet_connection_received(dt->conn, data, step);
        eet_connection_received(dt->conn, (char *)data + step, step);
        size -= 2 * step;
        still = eet_connection_received(dt->conn, (char *)data + 2 * step, size);
     }
   else
     still = eet_connection_received(dt->conn, data, size);

   fail_if(still);

   return EINA_TRUE;
} /* _eet_connection_write */

START_TEST(eet_connection_check)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   Eet_Connection_Data ecd;
   Eet_Test_Ex_Type etbt;
   Eina_Bool on_going;

   eet_init();

   _eet_test_ex_set(&etbt, 0);
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.list = eina_list_prepend(etbt.list, _eet_test_ex_set(NULL, 1));
   etbt.hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.hash, EET_TEST_KEY1, _eet_test_ex_set(NULL, 2));
   eina_hash_add(etbt.hash, EET_TEST_KEY2, _eet_test_ex_set(NULL, 2));
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ilist = eina_list_prepend(etbt.ilist, &i42);
   etbt.ihash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.ihash, EET_TEST_KEY1, &i7);
   eina_hash_add(etbt.ihash, EET_TEST_KEY2, &i7);
   etbt.slist = eina_list_prepend(NULL, "test");
   etbt.shash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(etbt.shash, EET_TEST_KEY1, "test");
   memset(&etbt.charray, 0, sizeof(etbt.charray));
   etbt.charray[0] = "test";

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                             "Eet_Test_Ex_Type",
                                             sizeof(Eet_Test_Ex_Type));

   edd = eet_data_descriptor_stream_new(&eddc);
   fail_if(!edd);

   _eet_build_ex_descriptor(edd, EINA_TRUE);

   /* Init context. */
   ecd.test = EINA_FALSE;
   ecd.edd = edd;

   /* Create a connection. */
   ecd.conn = eet_connection_new(_eet_connection_read, _eet_connection_write, &ecd);
   fail_if(!ecd.conn);

   /* Test the connection. */
   fail_if(!eet_connection_send(ecd.conn, edd, &etbt, NULL));

   fail_if(!ecd.test);

   fail_if(!eet_connection_close(ecd.conn, &on_going));

   fail_if(on_going);

   eet_shutdown();
}
END_TEST

struct _Eet_5FP
{
   Eina_F32p32 fp32;
   Eina_F16p16 fp16;
   Eina_F8p24  fp8;
   Eina_F32p32 f1;
   Eina_F32p32 f0;
};
typedef struct _Eet_5FP Eet_5FP;

struct _Eet_5DBL
{
   double fp32;
   double fp16;
   float  fp8;
   double f1;
   double f0;
};
typedef struct _Eet_5DBL Eet_5DBL;

START_TEST(eet_fp)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd_5FP;
   Eet_Data_Descriptor *edd_5DBL;
   Eet_5FP origin;
   Eet_5DBL *convert;
   Eet_5FP *build;
   void *blob;
   int size;

   eet_init();

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

   eet_shutdown();
}
END_TEST
START_TEST(eet_file_fp)
{
   char *file = strdup("/tmp/eet_suite_testXXXXXX");
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd_5FP;
   Eet_Data_Descriptor *edd_5DBL;
   Eet_File *ef;
   Eet_5FP origin;
   Eet_5DBL *convert;
   Eet_5FP *build;

   eet_init();

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Eet_5FP);
   edd_5FP = eet_data_descriptor_file_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp32", fp32, EET_T_F32P32);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp16", fp16, EET_T_F16P16);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "fp8", fp8, EET_T_F8P24);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "f1", f1, EET_T_F32P32);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_5FP, Eet_5FP, "f0", f0, EET_T_F32P32);

   eet_eina_file_data_descriptor_class_set(&eddc, sizeof (eddc), "Eet_5FP", sizeof (Eet_5DBL));
   edd_5DBL = eet_data_descriptor_file_new(&eddc);

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

   fail_if(!(file = tmpnam(file)));

   ef = eet_open(file, EET_FILE_MODE_READ_WRITE);
   fail_if(!ef);

   fail_if(!eet_data_write(ef, edd_5FP, EET_TEST_FILE_KEY1, &origin, 1));

   build = eet_data_read(ef, edd_5FP, EET_TEST_FILE_KEY1);
   fail_if(!build);

   convert = eet_data_read(ef, edd_5DBL, EET_TEST_FILE_KEY1);
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

   eet_close(ef);

   fail_if(unlink(file) != 0);

   eet_shutdown();
} /* START_TEST */

END_TEST

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

START_TEST(eet_test_union)
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

   eina_init();
   eet_init();

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

   eet_shutdown();
   eina_shutdown();
}
END_TEST
START_TEST(eet_test_variant)
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

   eina_init();
   eet_init();

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

   eet_shutdown();
   eina_shutdown();
} /* START_TEST */

END_TEST

Suite *
eet_suite(void)
{
   Suite *s;
   TCase *tc;

   s = suite_create("Eet");

   tc = tcase_create("Eet_Init");
   tcase_add_test(tc, eet_test_init);
   suite_add_tcase(s, tc);

   tc = tcase_create("Eet Data Encoding/Decoding");
   tcase_add_test(tc, eet_test_basic_data_type_encoding_decoding);
   tcase_add_test(tc, eet_test_data_type_encoding_decoding);
   tcase_add_test(tc, eet_test_data_type_dump_undump);
   tcase_add_test(tc, eet_test_data_type_escape_dump_undump);
   tcase_add_test(tc, eet_fp);
   tcase_add_test(tc, eet_test_union);
   tcase_add_test(tc, eet_test_variant);
   suite_add_tcase(s, tc);

   tc = tcase_create("Eet File");
   tcase_add_test(tc, eet_file_simple_write);
   tcase_add_test(tc, eet_file_data_test);
   tcase_add_test(tc, eet_file_data_dump_test);
   tcase_add_test(tc, eet_file_fp);
   suite_add_tcase(s, tc);

   tc = tcase_create("Eet Image");
   tcase_add_test(tc, eet_image);
   tcase_add_test(tc, eet_small_image);
   suite_add_tcase(s, tc);

#ifdef HAVE_SIGNATURE
   tc = tcase_create("Eet Identity");
   tcase_add_test(tc, eet_identity_simple);
   tcase_add_test(tc, eet_identity_open_simple);
   tcase_add_test(tc, eet_identity_open_pkcs8);
   tcase_add_test(tc, eet_identity_open_pkcs8_enc);
   suite_add_tcase(s, tc);
#endif /* ifdef HAVE_SIGNATURE */

#ifdef HAVE_CIPHER
   tc = tcase_create("Eet Cipher");
   tcase_add_test(tc, eet_cipher_decipher_simple);
   suite_add_tcase(s, tc);
#endif /* ifdef HAVE_CIPHER */

   tc = tcase_create("Eet Cache");
   tcase_add_test(tc, eet_cache_concurrency);
   suite_add_tcase(s, tc);

   tc = tcase_create("Eet Connection");
   tcase_add_test(tc, eet_connection_check);
   suite_add_tcase(s, tc);

   return s;
} /* eet_suite */

static const char *_cert_dir_find(const char *argv0)
{
   static char base[PATH_MAX] = "";
   char path[PATH_MAX];
   struct stat st;

   eina_strlcpy(base, TESTS_SRC_DIR, sizeof(base));
   eina_str_join(path, sizeof(path), '/', base, "key.pem");
   if (stat(path, &st) == 0)
     return base;

   if (base[0] != '/')
     {
        snprintf(base, sizeof(base), "%s/%s", TESTS_WD, TESTS_SRC_DIR);
        eina_str_join(path, sizeof(path), '/', base, "key.pem");
        if (stat(path, &st) == 0)
          return base;
     }

   eina_strlcpy(base, argv0, sizeof(base));
   do
     {
        char *p = strrchr(base, '/');
        if (!p)
          {
             base[0] = '\0';
             break;
          }
        *p = '\0';
        eina_str_join(path, sizeof(path), '/', base, "key.pem");
     }
   while (stat(path, &st) != 0);

   return base;
}

int
main(int argc EINA_UNUSED, char *argv[])
{
   Suite *s;
   SRunner *sr;
   int failed_count;
   const char *base = _cert_dir_find(argv[0]);

   putenv("EFL_RUN_IN_TREE=1");

   eina_str_join(_key_pem, sizeof(_key_pem), '/', base, "key.pem");
   eina_str_join(_cert_pem, sizeof(_cert_pem), '/', base,"cert.pem");
   eina_str_join(_key_enc, sizeof(_key_enc), '/', base, "key.enc");
   eina_str_join(_key_enc_pem, sizeof(_key_enc_pem), '/', base, "key_enc.pem");
   eina_str_join(_key_enc_none_pem, sizeof(_key_enc_none_pem), '/',
                 base, "key_enc_none.pem");

   s = eet_suite();
   sr = srunner_create(s);
   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
} /* main */

