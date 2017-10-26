#ifndef _ELDBUS_MODEL_PROXY_PRIVATE_H
#define _ELDBUS_MODEL_PROXY_PRIVATE_H

#include "Eldbus_Model.h"

#include <stdbool.h>

typedef struct _Eldbus_Model_Proxy_Data Eldbus_Model_Proxy_Data;

/**
 * eldbus_model_proxy
 */
struct _Eldbus_Model_Proxy_Data
{
   Eo *obj;

   const Eldbus_Introspection_Interface *interface;

   Eldbus_Object *object;
   Eldbus_Proxy *proxy;
   Eina_Hash *properties;
   Eina_List *childrens;
   Eina_List *pendings;
   Eina_List *promises;

   Eina_Stringshare *name;

   Eina_Bool monitoring : 1;
   Eina_Bool is_listed : 1;
   Eina_Bool is_loaded : 1;
};

#endif
