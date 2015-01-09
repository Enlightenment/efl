#ifndef _ELDBUS_MODEL_CONNECTION_PRIVATE_H
#define _ELDBUS_MODEL_CONNECTION_PRIVATE_H

#include "Eldbus_Model.h"

#include <stdbool.h>

typedef struct _Eldbus_Model_Connection_Data Eldbus_Model_Connection_Data;

/**
 * eldbus_model_connection
 */
struct _Eldbus_Model_Connection_Data
{
   Eo *obj;
   Efl_Model_Load load;
   Eldbus_Connection *connection;
   Eina_Array *properties_array;
   Eina_List *children_list;
   Eldbus_Connection_Type type;
   Eina_Stringshare *address;
   bool private;
   Eina_Value *unique_name;
   Eina_List *pending_list;
};

#endif

