#ifndef EET_TEST_COMMON_H
#define EET_TEST_COMMON_H

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

typedef struct _Eet_5FP Eet_5FP;
struct _Eet_5FP
{
   Eina_F32p32 fp32;
   Eina_F16p16 fp16;
   Eina_F8p24  fp8;
   Eina_F32p32 f1;
   Eina_F32p32 f0;
};

typedef struct _Eet_5DBL Eet_5DBL;
struct _Eet_5DBL
{
   double fp32;
   double fp16;
   float  fp8;
   double f1;
   double f0;
};

extern char argv0[PATH_MAX];
extern int i42;
extern int i7;
extern Eina_Bool _dump_call;

void append_string(void       *data,
                   const char *str);

void append_strbuf_string(void *data, const char *string);

Eina_Bool func(EINA_UNUSED const Eina_Hash *hash,
               const void                 *key,
               void                       *data,
               void                       *fdata);

Eina_Bool func7(EINA_UNUSED const Eina_Hash *hash,
                EINA_UNUSED const void      *key,
                void                       *data,
                void                       *fdata);

void eet_test_setup_eddc(Eet_Data_Descriptor_Class *eddc);

void eet_build_basic_descriptor(Eet_Data_Descriptor *edd);

void eet_test_basic_check(Eet_Test_Basic_Type *result,
                          int                  i,
                          Eina_Bool            dumper);

void eet_build_ex_descriptor(Eet_Data_Descriptor *edd, Eina_Bool stream);

Eet_Test_Ex_Type *eet_test_ex_set(Eet_Test_Ex_Type *res,
                                  int               offset);

int eet_test_ex_check(Eet_Test_Ex_Type *stuff,
                      int               offset,
                      Eina_Bool         dumper);

void eet_test_basic_set(Eet_Test_Basic_Type *res,
                        int                  i);

#endif
