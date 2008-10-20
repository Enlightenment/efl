#include <string.h>
#include <stdio.h>

#include <Eina.h>

#include "eet_suite.h"

static char*
_eet_str_direct_alloc(const char *str)
{
   return (char*) str;
}

static void
_eet_str_direct_free(const char *str)
{
   /* FIXME: Use attribute unused */
   (void) str;
}

/* Internal wrapper for eina_hash */
Eina_Hash*
eet_eina_hash_add(Eina_Hash *hash, const char *key, const void *data)
{
   if (!hash) hash = eina_hash_string_superfast_new(NULL);
   if (!hash) return NULL;

   eina_hash_add(hash, key, data);
   return hash;
}

void
eet_test_setup_eddc(Eet_Data_Descriptor_Class *eddc)
{
   eddc->version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc->func.mem_alloc = NULL;
   eddc->func.mem_free = NULL;
   eddc->func.str_alloc = NULL;
   eddc->func.str_free = NULL;
   eddc->func.list_next = (void*) eina_list_next;
   eddc->func.list_append = (void*) eina_list_append;
   eddc->func.list_data = (void*) eina_list_data_get;
   eddc->func.list_free = (void*) eina_list_free;
   eddc->func.hash_foreach = (void*) eina_hash_foreach;
   eddc->func.hash_add = (void*) eet_eina_hash_add;
   eddc->func.hash_free = (void*) eina_hash_free;
   eddc->func.str_direct_alloc = (void*) _eet_str_direct_alloc;
   eddc->func.str_direct_free = (void*) _eet_str_direct_free;
}

