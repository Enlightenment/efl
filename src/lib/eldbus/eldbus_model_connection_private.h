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

   Eldbus_Connection *connection;
   Eldbus_Pending *pending;
   Eldbus_Connection_Type type;

   Eina_List *childrens;
   Eina_List *requests;

   Eina_Stringshare *address;
   char *unique_name;

   bool private;

   Eina_Bool is_listed : 1;
};

#endif
