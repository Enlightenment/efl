#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct _Ecore_Con_Mempool Ecore_Con_Mempool;
struct _Ecore_Con_Mempool
{
   const char *name;
   Eina_Mempool *mp;
   size_t size;
};

#define GENERIC_ALLOC_FREE(TYPE, Type)                                  \
  Ecore_Con_Mempool Type##_mp = { #TYPE,  NULL, sizeof (TYPE) };        \
                                                                        \
  TYPE *                                                                \
  Type##_alloc(void)                                                    \
  {                                                                     \
     return eina_mempool_malloc(Type##_mp.mp, sizeof (TYPE));           \
  }                                                                     \
                                                                        \
  void                                                                  \
  Type##_free(TYPE *e)                                                  \
  {                                                                     \
     eina_mempool_free(Type##_mp.mp, e);                                \
  }

GENERIC_ALLOC_FREE(Ecore_Con_Event_Client_Add, ecore_con_event_client_add);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Client_Del, ecore_con_event_client_del);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Client_Write, ecore_con_event_client_write);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Client_Data, ecore_con_event_client_data);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Server_Error, ecore_con_event_server_error);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Client_Error, ecore_con_event_client_error);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Server_Add, ecore_con_event_server_add);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Server_Del, ecore_con_event_server_del);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Server_Write, ecore_con_event_server_write);
GENERIC_ALLOC_FREE(Ecore_Con_Event_Server_Data, ecore_con_event_server_data);

static Ecore_Con_Mempool *mempool_array[] = {
  &ecore_con_event_client_add_mp,
  &ecore_con_event_client_del_mp,
  &ecore_con_event_client_write_mp,
  &ecore_con_event_client_data_mp,
  &ecore_con_event_server_error_mp,
  &ecore_con_event_client_error_mp,
  &ecore_con_event_server_add_mp,
  &ecore_con_event_server_del_mp,
  &ecore_con_event_server_write_mp,
  &ecore_con_event_server_data_mp
};

void
ecore_con_mempool_init(void)
{
   const char *choice;
   unsigned int i;

   choice = getenv("EINA_MEMPOOL");
   if (!choice || !choice[0])
     choice = "chained_mempool";

   for (i = 0; i < sizeof (mempool_array) / sizeof (mempool_array[0]); ++i)
     {
     retry:
        mempool_array[i]->mp = eina_mempool_add(choice, mempool_array[i]->name, NULL, mempool_array[i]->size, 64);
        if (!mempool_array[i]->mp)
          {
             if (strcmp(choice, "pass_through") != 0)
               {
                  ERR("Falling back to pass through ! Previously tried '%s' mempool.", choice);
                  choice = "pass_through";
                  goto retry;
               }
             else
               {
                  ERR("Impossible to allocate mempool '%s' !", choice);
                  return ;
               }
          }
     }
}

void
ecore_con_mempool_shutdown(void)
{
   unsigned int i;

   for (i = 0; i < sizeof (mempool_array) / sizeof (mempool_array[0]); ++i)
     {
        eina_mempool_del(mempool_array[i]->mp);
        mempool_array[i]->mp = NULL;
     }
}

