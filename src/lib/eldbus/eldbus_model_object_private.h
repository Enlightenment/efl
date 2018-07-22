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

   Eina_List *objects;
   Eina_List *childrens;
   Eina_List *requests;
   Eina_List *pendings;

   Eina_Stringshare *bus;
   Eina_Stringshare *path;

   Eldbus_Introspection_Node *introspection;

   Eina_Bool is_listed : 1;
};

#endif
