#ifndef _ELDBUS_MODEL_OBJECT_PRIVATE_H
#define _ELDBUS_MODEL_OBJECT_PRIVATE_H

#include "Eldbus_Model.h"

#include <stdbool.h>

/**
 * eldbus_model_object
 */
typedef struct _Eldbus_Model_Object_Data Eldbus_Model_Object_Data;
struct _Eldbus_Model_Object_Data
{
   Eo *obj;
   Eina_Bool is_listed : 1;
   Eldbus_Connection *connection;
   Eina_List *object_list;
   Eina_Array *properties_array;
   Eina_List *children_list;
   Eina_List *children_promises;
   Eina_List *count_promises;
   Eldbus_Connection_Type type;
   Eina_Stringshare *address;
   bool private;
   Eina_Stringshare *bus;
   Eina_Stringshare *path;
   char *unique_name;
   Eina_List *pending_list;
   Eldbus_Introspection_Node *introspection;
};

#endif

