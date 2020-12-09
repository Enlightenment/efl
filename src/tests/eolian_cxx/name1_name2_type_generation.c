#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <check.h>

struct _Type_Generation_Data
{
};
typedef struct _Type_Generation_Data Type_Generation_Data;

#define MY_CLASS TYPE1_TYPE2_TYPE_GENERATION_CLASS

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include "name1_name2_type_generation.eo.h"

void _name1_name2_type_generation_invoidptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, void *v)
{
  ck_assert(v == NULL);
}

void _name1_name2_type_generation_inint(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, int v EINA_UNUSED)
{
  ck_assert(v == 42);
}

void * _name1_name2_type_generation_returnvoidptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  static int i = 42;
  return &i;
}

void _name1_name2_type_generation_instring(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, const char *v EINA_UNUSED)
{
  ck_assert_str_eq(v, "foobar");
}

void _name1_name2_type_generation_instringptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, const char * *v EINA_UNUSED)
{
  ck_assert_str_eq(*v, "foobar");
}

void _name1_name2_type_generation_instringshare(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, const char *v EINA_UNUSED)
{
  ck_assert_str_eq(v, "foobar");
  ck_assert(eina_stringshare_add(v) == v);
  eina_stringshare_del(v);
}

void _name1_name2_type_generation_instringshareown(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, const char * v EINA_UNUSED)
{
  ck_assert_str_eq(v, "foobar");
  eina_stringshare_del(v);
}

void _name1_name2_type_generation_instringptrown(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, const char * *v)
{
  ck_assert_str_eq(*v, "foobar");
  free((void*)*v);
}

void _name1_name2_type_generation_instringown(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, char *v EINA_UNUSED)
{
  ck_assert_str_eq(v, "foobar");
  free((void*)v);
}

void _name1_name2_type_generation_instringownptrown(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, const char **v EINA_UNUSED)
{
  ck_assert_str_eq(*v, "foobar");
  free(v);
}

void _name1_name2_type_generation_instringownptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, const char * *v EINA_UNUSED)
{
  ck_assert_str_eq(*v, "foobar");
  free(v);
}

int _name1_name2_type_generation_returnint(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  return 42;
}

const char * _name1_name2_type_generation_returnstring(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  return "foobar";
}

const char * * _name1_name2_type_generation_returnstringptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  static const char* foobar = "foobar";
  return &foobar;
}

Eina_Stringshare* _name1_name2_type_generation_returnstringshare(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  return eina_stringshare_add("foobar");
}

Eina_Stringshare* _name1_name2_type_generation_returnstringshareown(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  return eina_stringshare_add("foobar");
}

char * _name1_name2_type_generation_returnstringown(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  const char foobar[] = "foobar";
  char* p = malloc(sizeof(foobar));
  memcpy(p, foobar, sizeof(foobar));
  return p;
}

const char * * _name1_name2_type_generation_returnstringownptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED)
{
  const char foobar[] = "foobar";
  char** p1 = malloc(sizeof(const char*));
  char* p = malloc(sizeof(foobar));
  memcpy(p, foobar, sizeof(foobar));

  *p1 = p;
  return (const char**)p1;
}

void _name1_name2_type_generation_outvoidptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, void **v EINA_UNUSED)
{
}

void _name1_name2_type_generation_outint(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, int *v EINA_UNUSED)
{
}

void _name1_name2_type_generation_outstringshare(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Eina_Stringshare** v)
{
  *v = eina_stringshare_add("foobar");
}

void _name1_name2_type_generation_outstringshareown(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Eina_Stringshare** v)
{
  *v = eina_stringshare_add("foobar");
}

void _name1_name2_type_generation_inclassname(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Name1_Name2_Type_Generation *v EINA_UNUSED)
{
}

void _name1_name2_type_generation_outclassname(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Name1_Name2_Type_Generation **v EINA_UNUSED)
{
}

void _name1_name2_type_generation_inoutclassname(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Name1_Name2_Type_Generation **v EINA_UNUSED)
{
}

void _name1_name2_type_generation_optionalinvoidptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, void *v EINA_UNUSED)
{
}

void _name1_name2_type_generation_optionalinint(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, int v EINA_UNUSED)
{
}

void _name1_name2_type_generation_optionaloutvoidptr(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, void **v EINA_UNUSED)
{
}

void _name1_name2_type_generation_optionaloutint(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, int *v EINA_UNUSED)
{
  if(v)
    *v = 42;
}

void _name1_name2_type_generation_optionalinclassname(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Name1_Name2_Type_Generation *v EINA_UNUSED)
{
}

void _name1_name2_type_generation_optionaloutclassname(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Name1_Name2_Type_Generation **v EINA_UNUSED)
{
}

void _name1_name2_type_generation_optionalinoutclassname(Eo *obj EINA_UNUSED, Type_Generation_Data *pd EINA_UNUSED, Name1_Name2_Type_Generation **v EINA_UNUSED)
{
}

#include "name1_name2_type_generation.eo.c"
