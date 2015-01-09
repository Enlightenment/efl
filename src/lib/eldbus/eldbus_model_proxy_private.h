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
   Efl_Model_Load load;
   Eldbus_Object *object;
   Eldbus_Proxy *proxy;
   Eina_Array *properties_array;
   Eina_Hash *properties_hash;
   Eina_List *children_list;
   Eina_Stringshare *name;
   Eina_List *pending_list;
   bool monitoring;
   const Eldbus_Introspection_Interface *interface;
   Eina_Value tmp_value;
};

#endif

