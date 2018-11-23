#ifndef _ELDBUS_MODEL_ARGUMENTS_PRIVATE_H
#define _ELDBUS_MODEL_ARGUMENTS_PRIVATE_H

#include "Eldbus_Model.h"

#include <stdbool.h>

typedef struct _Eldbus_Model_Arguments_Data Eldbus_Model_Arguments_Data;

/**
 * eldbus_model_arguments
 */
struct _Eldbus_Model_Arguments_Data
{
   Eo *obj;
   Eldbus_Proxy *proxy;
   Eina_Hash *properties;
   Eina_Stringshare *name;
   Eina_List *pending_list;
   const Eina_List *arguments;
   Eina_Value tmp_value;
};

Eina_Bool eldbus_model_arguments_process_arguments(Eldbus_Model_Arguments_Data *, const Eldbus_Message *, Eldbus_Pending *);

#define ARGUMENT_FORMAT "arg%u"

#endif
