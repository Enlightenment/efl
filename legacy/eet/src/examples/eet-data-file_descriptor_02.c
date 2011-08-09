/*
 * build: gcc -o eet_data_file_descriptor_02 eet-data-file_descriptor_02.c `pkg-config --cflags --libs eet eina`
 */

#include <Eina.h>
#include <Eet.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum _Example_Data_Type      Example_Data_Type;
typedef struct _Example_Variant_Type Example_Variant_Type;
typedef struct _Example_Variant      Example_Variant;
typedef struct _Example_Union        Example_Union;
typedef struct _Example_Struct1      Example_Struct1;
typedef struct _Example_Struct2      Example_Struct2;
typedef struct _Example_Struct3      Example_Struct3;
typedef struct _Example_Lists        Example_Lists;

enum _Example_Data_Type
{
   EET_UNKNOWN = 0,
   EET_STRUCT1,
   EET_STRUCT2,
   EET_STRUCT3
};

struct
{
   Example_Data_Type u;
   const char       *name;
} eet_mapping[] = {
   { EET_STRUCT1, "ST1" },
   { EET_STRUCT2, "ST2" },
   { EET_STRUCT3, "ST3" },
   { EET_UNKNOWN, NULL }
};

struct _Example_Struct1
{
   double      val1;
   int         stuff;
   const char *s1;
};

struct _Example_Struct2
{
   Eina_Bool          b1;
   unsigned long long v1;
};

struct _Example_Struct3
{
   int body;
};

struct _Example_Union
{
   Example_Data_Type type;

   union {
      Example_Struct1 st1;
      Example_Struct2 st2;
      Example_Struct3 st3;
   } u;
};

struct _Example_Variant_Type
{
   const char *type;
   Eina_Bool   unknow : 1;
};

struct _Example_Variant
{
   Example_Variant_Type t;

   void                *data; /* differently than the union type, we
                               * don't need to pre-allocate the memory
                               * for the field*/
};

struct _Example_Lists
{
   Eina_List *union_list;
   Eina_List *variant_list;
};

static void
_st1_set(Example_Struct1 *st1,
         double           v1,
         int              v2,
         const char      *v3)
{
   st1->val1 = v1;
   st1->stuff = v2;
   st1->s1 = v3;
} /* _st1_set */

static void
_st2_set(Example_Struct2   *st2,
         Eina_Bool          v1,
         unsigned long long v2)
{
   st2->b1 = v1;
   st2->v1 = v2;
} /* _st2_set */

static void
_st3_set(Example_Struct3 *st3,
         int              v1)
{
   st3->body = v1;
} /* _st3_set */

static const char * /* union type_get() */
_union_type_get(const void *data,
                Eina_Bool  *unknow)
{
   const Example_Data_Type *u = data;
   int i;

   if (unknow)
     *unknow = EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (*u == eet_mapping[i].u)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_TRUE;

   return NULL;
} /* _union_type_get */

static Eina_Bool
_union_type_set(const char *type,
                void       *data,
                Eina_Bool   unknow)
{
   Example_Data_Type *u = data;
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
} /* _union_type_set */

static const char *
_variant_type_get(const void *data,
                  Eina_Bool  *unknow)
{
   const Example_Variant_Type *type = data;
   int i;

   if (unknow)
     *unknow = type->unknow;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(type->type, eet_mapping[i].name) == 0)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_FALSE;

   return type->type;
} /* _variant_type_get */

static Eina_Bool
_variant_type_set(const char *type,
                  void       *data,
                  Eina_Bool   unknow)
{
   Example_Variant_Type *vt = data;

   vt->type = type;
   vt->unknow = unknow;
   return EINA_TRUE;
} /* _variant_type_set */

static Eet_Data_Descriptor *
_st1_dd(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *res;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Example_Struct1);
   res = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(
     res, Example_Struct1, "val1", val1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(
     res, Example_Struct1, "stuff", stuff, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(
     res, Example_Struct1, "s1", s1, EET_T_STRING);

   return res;
} /* _st1_dd */

static Eet_Data_Descriptor *
_st2_dd(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *res;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Example_Struct2);
   res = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(
     res, Example_Struct2, "b1", b1, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(
     res, Example_Struct2, "v1", v1, EET_T_ULONG_LONG);

   return res;
} /* _st2_dd */

static Eet_Data_Descriptor *
_st3_dd(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *res;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Example_Struct3);
   res = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(
     res, Example_Struct3, "body", body, EET_T_INT);

   return res;
} /* _st3_dd */

/* string that represents the entry in the eet file. you might like to
 * have different profiles or so in the same file, this is possible
 * with different strings
 */
static const char CACHE_FILE_ENTRY[] = "cache";

/* keep the descriptor static global, so it can be shared by different
 * functions (load/save) of this and only this file.
 */
static Eet_Data_Descriptor *_lists_descriptor;
static Eet_Data_Descriptor *_struct_1_descriptor;
static Eet_Data_Descriptor *_struct_2_descriptor;
static Eet_Data_Descriptor *_struct_3_descriptor;
static Eet_Data_Descriptor *_union_descriptor;
static Eet_Data_Descriptor *_variant_descriptor;
static Eet_Data_Descriptor *_union_unified_descriptor;
static Eet_Data_Descriptor *_variant_unified_descriptor;

/* keep file handle alive, so mmap()ed strings are all alive as
 * well */
static Eet_File *_cache_file = NULL;
static Eet_Dictionary *_cache_dict = NULL;

static void /* declaring types */
_data_descriptors_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Example_Lists);
   _lists_descriptor = eet_data_descriptor_file_new(&eddc);

   _struct_1_descriptor = _st1_dd();
   _struct_2_descriptor = _st2_dd();
   _struct_3_descriptor = _st3_dd();

   /* for union */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Example_Union);
   _union_descriptor = eet_data_descriptor_file_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _union_type_get;
   eddc.func.type_set = _union_type_set;
   _union_unified_descriptor = eet_data_descriptor_file_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(
     _union_unified_descriptor, "ST1", _struct_1_descriptor);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(
     _union_unified_descriptor, "ST2", _struct_2_descriptor);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(
     _union_unified_descriptor, "ST3", _struct_3_descriptor);

   EET_DATA_DESCRIPTOR_ADD_UNION(
     _union_descriptor, Example_Union, "u", u, type,
     _union_unified_descriptor);

   EET_DATA_DESCRIPTOR_ADD_LIST(
     _lists_descriptor, Example_Lists, "union_list", union_list,
     _union_descriptor);

   /* for variant */
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Example_Variant);
   _variant_descriptor = eet_data_descriptor_file_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _variant_type_get;
   eddc.func.type_set = _variant_type_set;
   _variant_unified_descriptor = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(
     _variant_unified_descriptor, "ST1", _struct_1_descriptor);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(
     _variant_unified_descriptor, "ST2", _struct_2_descriptor);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(
     _variant_unified_descriptor, "ST3", _struct_3_descriptor);

   EET_DATA_DESCRIPTOR_ADD_VARIANT(
     _variant_descriptor, Example_Variant, "data", data, t,
     _variant_unified_descriptor);

   EET_DATA_DESCRIPTOR_ADD_LIST(
     _lists_descriptor, Example_Lists, "variant_list", variant_list,
     _variant_descriptor);
} /* _data_descriptors_init */

static void
_data_descriptors_shutdown(void)
{
   eet_data_descriptor_free(_lists_descriptor);
   eet_data_descriptor_free(_struct_1_descriptor);
   eet_data_descriptor_free(_struct_2_descriptor);
   eet_data_descriptor_free(_struct_3_descriptor);
   eet_data_descriptor_free(_union_descriptor);
   eet_data_descriptor_free(_variant_descriptor);
   eet_data_descriptor_free(_union_unified_descriptor);
   eet_data_descriptor_free(_variant_unified_descriptor);
} /* _data_descriptors_shutdown */

/* need to check if the pointer came from mmap()ed area in
 * eet_dictionary or it was allocated with eina_stringshare_add()
 */
static void
_string_free(const char *str)
{
   if (!str)
     return;

   if ((_cache_dict) && (eet_dictionary_string_check(_cache_dict, str)))
     return;

   eina_stringshare_del(str);
} /* _string_free */

static Example_Union *
_union_1_new(const char *v1,
             const char *v2,
             const char *v3)
{
   Example_Union *un = calloc(1, sizeof(Example_Union));
   if (!un)
     {
        fprintf(
          stderr, "ERROR: could not allocate an Example_Union struct.\n");
        return NULL;
     }

   un->type = EET_STRUCT1;
   _st1_set(&(un->u.st1), atof(v1), atoi(v2), eina_stringshare_add(v3));

   return un;
}

static Example_Union *
_union_2_new(const char *v1,
             const char *v2)
{
   Example_Union *un = calloc(1, sizeof(Example_Union));
   if (!un)
     {
        fprintf(
          stderr, "ERROR: could not allocate an Example_Union struct.\n");
        return NULL;
     }

   un->type = EET_STRUCT2;
   _st2_set(&(un->u.st2), atoi(v1), atoi(v2));

   return un;
}

static Example_Union *
_union_3_new(const char *v1)
{
   Example_Union *un = calloc(1, sizeof(Example_Union));
   if (!un)
     {
        fprintf(
          stderr, "ERROR: could not allocate an Example_Union struct.\n");
        return NULL;
     }

   un->type = EET_STRUCT3;
   _st3_set(&(un->u.st3), atoi(v1));

   return un;
}

static Example_Variant *
_variant_1_new(const char *v1,
               const char *v2,
               const char *v3)
{
   Example_Struct1 *st1;
   Example_Variant *va = calloc(1, sizeof(Example_Variant));
   if (!va)
     {
        fprintf(
          stderr, "ERROR: could not allocate an Example_Variant struct.\n");
        return NULL;
     }

   va = calloc(1, sizeof (Example_Variant));
   va->t.type = eet_mapping[0].name;
   st1 = calloc(1, sizeof (Example_Struct1));
   _st1_set(st1, atof(v1), atoi(v2), eina_stringshare_add(v3));
   va->data = st1;

   return va;
}

static Example_Variant *
_variant_2_new(const char *v1,
               const char *v2)
{
   printf("varinant 2 new\n");

   Example_Struct2 *st2;
   Example_Variant *va = calloc(1, sizeof(Example_Variant));
   if (!va)
     {
        fprintf(
          stderr, "ERROR: could not allocate an Example_Variant struct.\n");
        return NULL;
     }

   va = calloc(1, sizeof (Example_Variant));

   va->t.type = eet_mapping[1].name;

   printf("type gets %s\n", va->t.type);

   st2 = calloc(1, sizeof (Example_Struct2));
   _st2_set(st2, atoi(v1), atoi(v2));
   va->data = st2;

   return va;
}

static Example_Variant *
_variant_3_new(const char *v1)
{
   Example_Struct3 *st3;
   Example_Variant *va = calloc(1, sizeof(Example_Variant));
   if (!va)
     {
        fprintf(
          stderr, "ERROR: could not allocate an Example_Variant struct.\n");
        return NULL;
     }

   va = calloc(1, sizeof (Example_Variant));
   va->t.type = eet_mapping[2].name;
   st3 = calloc(1, sizeof (Example_Struct3));
   _st3_set(st3, atoi(v1));
   va->data = st3;

   return va;
}

static Example_Lists *
_data_new(void)
{
   Example_Lists *example_lists = calloc(1, sizeof(Example_Lists));
   if (!example_lists)
     {
        fprintf(stderr, "ERROR: could not allocate a Example_Lists struct.\n");
        return NULL;
     }

   return example_lists;
} /* _data_new */

static void
_union_free(Example_Union *un)
{
   if (un->type == EET_STRUCT1)
     {
        Example_Struct1 *st1 = &(un->u.st1);
        _string_free(st1->s1);
     }

   free(un);
}

static void
_variant_free(Example_Variant *va)
{
   if (!strcmp(va->t.type, eet_mapping[0].name))
     {
        Example_Struct1 *st1 = va->data;
        _string_free(st1->s1);
     }

   free(va->data);
   free(va);
}

static void
_data_free(Example_Lists *cache)
{
   Example_Union *un;
   Example_Variant *va;

   EINA_LIST_FREE(cache->union_list, un)
     _union_free(un);

   EINA_LIST_FREE(cache->variant_list, va)
     _variant_free(va);

   free(cache);
} /* _data_free */

static Example_Lists *
_data_load(const char *filename)
{
   Example_Lists *data;
   Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
   if (!ef)
     {
        fprintf(stderr, "ERROR: could not open '%s' for read\n", filename);
        return NULL;
     }

   data = eet_data_read(ef, _lists_descriptor, CACHE_FILE_ENTRY);
   if (!data)
     {
        eet_close(ef);
        return NULL;
     }

   if (_cache_file)
     eet_close(_cache_file);

   _cache_file = ef;
   _cache_dict = eet_dictionary_get(ef);

   return data;
} /* _data_load */

static Eina_Bool
_data_save(const Example_Lists *cache,
           const char          *filename)
{
   char tmp[PATH_MAX];
   Eet_File *ef;
   Eina_Bool ret;
   unsigned int i, len;
   struct stat st;

   len = eina_strlcpy(tmp, filename, sizeof(tmp));
   if (len + 12 >= (int)sizeof(tmp))
     {
        fprintf(stderr, "ERROR: file name is too big: %s\n", filename);
        return EINA_FALSE;
     }

   i = 0;
   do
     {
        snprintf(tmp + len, 12, ".%u", i);
        i++;
     }
   while (stat(tmp, &st) == 0);

   ef = eet_open(tmp, EET_FILE_MODE_WRITE);
   if (!ef)
     {
        fprintf(stderr, "ERROR: could not open '%s' for write\n", tmp);
        return EINA_FALSE;
     }

   ret = eet_data_write
       (ef, _lists_descriptor, CACHE_FILE_ENTRY, cache, EINA_TRUE);

   eet_close(ef);

   if (ret)
     {
        unlink(filename);
        rename(tmp, filename);
     }

   return ret;
} /* _data_save */

static void
_print_union(const Example_Union *un)
{
   printf("\t  |   type: %s'\n", eet_mapping[un->type - 1].name);

   switch (un->type)
     {
      case EET_STRUCT1:
        printf("\t\t  val1: %f\n", un->u.st1.val1);
        printf("\t\t  stuff: %d\n", un->u.st1.stuff);
        printf("\t\t  s1: %s\n", un->u.st1.s1);
        break;

      case EET_STRUCT2:
        printf("\t\t  val1: %i\n", un->u.st2.b1);
        printf("\t\t  stuff: %lli\n", un->u.st2.v1);
        break;

      case EET_STRUCT3:
        printf("\t\t  val1: %i\n", un->u.st3.body);
        break;

      default:
        return;
     }
}

static void
_print_variant(const Example_Variant *va)
{
   printf("\t  |   type: %s'\n", va->t.type);

   switch (va->t.type[2])
     {
      case '1':
      {
         Example_Struct1 *st1 = va->data;

         printf("\t\t  val1: %f\n", st1->val1);
         printf("\t\t  stuff: %d\n", st1->stuff);
         printf("\t\t  s1: %s\n", st1->s1);
      }
      break;

      case '2':
      {
         Example_Struct2 *st2 = va->data;

         printf("\t\t  val1: %i\n", st2->b1);
         printf("\t\t  stuff: %lli\n", st2->v1);
      }
      break;

      case '3':
      {
         Example_Struct3 *st3 = va->data;

         printf("\t\t  val1: %i\n", st3->body);
      }
      break;

      default:
        return;
     }
}

int
main(int   argc,
     char *argv[])
{
   Example_Lists *data_lists;
   int ret = 0;

   if (argc < 3)
     {
        fprintf(stderr,
                "Usage:\n\t%s <input> <output> [action action-params]\n\n"
                "where actions and their parameters are:\n"
                "\tunion <type> [fields]\n"
                "\tvariant <type> [fields]\n"
                "\n",
                argv[0]);
        return -1;
     }

   eina_init();
   eet_init();
   _data_descriptors_init();

   data_lists = _data_load(argv[1]);
   if (!data_lists)
     {
        printf("Creating new data lists.\n");
        data_lists = _data_new();
        if (!data_lists)
          {
             ret = -2;
             goto end;
          }
     }

   if (argc > 3)
     {
        if (strcmp(argv[3], "union") == 0)
          {
             if (argc > 4)
               {
                  int type = atoi(argv[4]);
                  Example_Union *un;

                  if (type < EET_STRUCT1 || type > EET_STRUCT3)
                    {
                       fprintf(stderr,
                               "ERROR: invalid type parameter (%s).\n",
                               argv[4]);
                       goto cont;
                    }

                  switch (type)
                    {
                     case 1:
                       if (argc != 8)
                         {
                            fprintf(
                              stderr, "ERROR: wrong number of parameters"
                                      " (%d).\n", argc);
                            goto cont;
                         }

                       un = _union_1_new(
                           argv[5], argv[6], argv[7]);
                       if (!un)
                         {
                            fprintf(
                              stderr, "ERROR: could not create the "
                                      "requested union.\n");
                            goto cont;
                         }
                       data_lists->union_list =
                         eina_list_append(data_lists->union_list, un);
                       break;

                     case 2:
                       if (argc != 7)
                         {
                            fprintf(
                              stderr, "ERROR: wrong number of parameters"
                                      " (%d).\n", argc);
                            goto cont;
                         }

                       un = _union_2_new(argv[5], argv[6]);
                       if (!un)
                         {
                            fprintf(
                              stderr, "ERROR: could not create the "
                                      "requested union.\n");
                            goto cont;
                         }
                       data_lists->union_list =
                         eina_list_append(data_lists->union_list, un);
                       break;

                     case 3:
                       if (argc != 6)
                         {
                            fprintf(
                              stderr, "ERROR: wrong number of parameters"
                                      " (%d).\n", argc);
                            goto cont;
                         }

                       un = _union_3_new(argv[5]);
                       if (!un)
                         {
                            fprintf(
                              stderr, "ERROR: could not create the "
                                      "requested union.\n");
                            goto cont;
                         }
                       data_lists->union_list =
                         eina_list_append(data_lists->union_list, un);
                       break;

                     default:
                       fprintf(
                         stderr, "ERROR: bad type of of struct passed\n");
                       goto cont;
                    }
               }
             else
               fprintf(stderr,
                       "ERROR: wrong number of parameters (%d).\n",
                       argc);
          }
        else if (strcmp(argv[3], "variant") == 0)
          {
             if (argc > 4)
               {
                  int type = atoi(argv[4]);
                  Example_Variant *va;

                  if (type < EET_STRUCT1 || type > EET_STRUCT3)
                    {
                       fprintf(stderr,
                               "ERROR: invalid type parameter (%s).\n",
                               argv[4]);
                       goto cont;
                    }

                  switch (type)
                    {
                     case 1:
                       if (argc != 8)
                         {
                            fprintf(
                              stderr, "ERROR: wrong number of parameters"
                                      " (%d).\n", argc);
                            goto cont;
                         }

                       va = _variant_1_new(
                           argv[5], argv[6], argv[7]);
                       if (!va)
                         {
                            fprintf(
                              stderr, "ERROR: could not create the "
                                      "requested variant.\n");
                            goto cont;
                         }
                       data_lists->variant_list =
                         eina_list_append(data_lists->variant_list, va);
                       break;

                     case 2:
                       if (argc != 7)
                         {
                            fprintf(
                              stderr, "ERROR: wrong number of parameters"
                                      " (%d).\n", argc);
                            goto cont;
                         }

                       va = _variant_2_new(argv[5], argv[6]);
                       if (!va)
                         {
                            fprintf(
                              stderr, "ERROR: could not create the "
                                      "requested variant.\n");
                            goto cont;
                         }
                       data_lists->variant_list =
                         eina_list_append(data_lists->variant_list, va);
                       break;

                     case 3:
                       if (argc != 6)
                         {
                            fprintf(
                              stderr, "ERROR: wrong number of parameters"
                                      " (%d).\n", argc);
                            goto cont;
                         }

                       va = _variant_3_new(argv[5]);
                       if (!va)
                         {
                            fprintf(
                              stderr, "ERROR: could not create the "
                                      "requested variant.\n");
                            goto cont;
                         }
                       data_lists->variant_list =
                         eina_list_append(data_lists->variant_list, va);
                       break;

                     default:
                       fprintf(
                         stderr, "ERROR: bad type of of struct passed\n");
                       goto cont;
                    }
               }
             else
               fprintf(stderr,
                       "ERROR: wrong number of parameters (%d).\n",
                       argc);
          }
        else
          fprintf(stderr, "ERROR: unknown action '%s'\n", argv[3]);
     }

cont:
   printf("Cached data:\n");

   printf("\tstats: unions=%u, variants=%u\n",
          eina_list_count(data_lists->union_list),
          eina_list_count(data_lists->variant_list));

   if (eina_list_count(data_lists->union_list))
     {
        const Eina_List *l;
        const Example_Union *un;
        printf("\t  * union list:\n");

        EINA_LIST_FOREACH(data_lists->union_list, l, un)
          {
             _print_union(un);
          }
     }

   if (eina_list_count(data_lists->variant_list))
     {
        const Eina_List *l;
        const Example_Variant *un;
        printf("\t  * variant list:\n");

        EINA_LIST_FOREACH(data_lists->variant_list, l, un)
          {
             _print_variant(un);
          }
     }

   printf("\n");

   if (!_data_save(data_lists, argv[2]))
     ret = -3;

   _data_free(data_lists);

end:
   if (_cache_file)
     eet_close(_cache_file);
   _data_descriptors_shutdown();
   eet_shutdown();
   eina_shutdown();

   return ret;
} /* main */

