#ifndef _ELDBUS_MODEL_METHOD_PRIVATE_H
#define _ELDBUS_MODEL_METHOD_PRIVATE_H

#include "Eldbus_Model.h"

typedef struct _Eldbus_Model_Method_Data Eldbus_Model_Method_Data;

/**
 * eldbus_model_method
 */
struct _Eldbus_Model_Method_Data
{
   Eo *obj;

   const Eldbus_Introspection_Method *method;

   Eldbus_Proxy *proxy;
};

#endif
