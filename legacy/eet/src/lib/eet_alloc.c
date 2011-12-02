#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "Eet.h"
#include "Eet_private.h"

typedef struct _Eet_Mempool Eet_Mempool;
struct _Eet_Mempool
{
   const char *name;
   Eina_Mempool *mp;
   size_t size;
};

#define GENERIC_ALLOC_FREE(TYPE, Type)                                  \
  Eet_Mempool Type##_mp = { #TYPE,  NULL, sizeof (TYPE) };              \
                                                                        \
  TYPE *                                                                \
  Type##_malloc(unsigned int num)                                       \
  {                                                                     \
     return eina_mempool_malloc(Type##_mp.mp, num * sizeof (TYPE));     \
  }                                                                     \
  TYPE *                                                                \
  Type##_calloc(unsigned int num)                                       \
  {                                                                     \
     return eina_mempool_calloc(Type##_mp.mp, num * sizeof (TYPE));     \
  }                                                                     \
  void                                                                  \
  Type##_mp_free(TYPE *e)                                               \
  {                                                                     \
     eina_mempool_free(Type##_mp.mp, e);                                \
  }

GENERIC_ALLOC_FREE(Eet_File_Directory, eet_file_directory);
GENERIC_ALLOC_FREE(Eet_File_Node, eet_file_node);
GENERIC_ALLOC_FREE(Eet_File_Header, eet_file_header);
GENERIC_ALLOC_FREE(Eet_Dictionary, eet_dictionary);
GENERIC_ALLOC_FREE(Eet_File, eet_file);
GENERIC_ALLOC_FREE(Eet_String, eet_string);

static Eet_Mempool *mempool_array[] = {
  &eet_file_directory_mp,
  &eet_file_node_mp,
  &eet_file_header_mp,
  &eet_dictionary_mp,
  &eet_file_mp,
  &eet_string_mp
};

Eina_Bool
eet_mempool_init(void)
{
   const char *choice;
   unsigned int i;

   choice = getenv("EINA_MEMPOOL");
   if ((!choice) || (!choice[0]))
     choice = "chained_mempool";

   for (i = 0; i < sizeof (mempool_array) / sizeof (mempool_array[0]); ++i)
     {
     retry:
        mempool_array[i]->mp = eina_mempool_add(choice, mempool_array[i]->name, NULL, mempool_array[i]->size, 64);
        if (!mempool_array[i]->mp)
          {
             if (!strcmp(choice, "pass_through"))
               {
                  ERR("Falling back to pass through ! Previously tried '%s' mempool.", choice);
                  choice = "pass_through";
                  goto retry;
               }
             else
               {
                  ERR("Impossible to allocate mempool '%s' !", choice);
                  return EINA_FALSE;
               }
          }
     }
   return EINA_TRUE;
}

void
eet_mempool_shutdown(void)
{
   unsigned int i;

   for (i = 0; i < sizeof (mempool_array) / sizeof (mempool_array[0]); ++i)
     {
        eina_mempool_del(mempool_array[i]->mp);
        mempool_array[i]->mp = NULL;
     }
}

