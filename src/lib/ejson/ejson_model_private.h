#ifndef _EJSON_MODEL_PRIVATE_H
#define _EJSON_MODEL_PRIVATE_H

#include "ejson_private.h"

#include <Eo.h>
#include <Efl.h>

#include <stdbool.h>

#include "Ejson.h"


typedef struct _Ejson_Model_Data Ejson_Model_Data;

/**
 * ejson_model
 */
struct _Ejson_Model_Data
{
   Eo *obj;
   Eina_Array *properties_array;
   Eina_List *children_list;
   Ejson_Model_Type type;
   bool json_property_valid;
   const char *name;
   Eina_Value *value;
};

#endif

