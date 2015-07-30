#ifndef _EJSON_MODEL_PRIVATE_H
#define _EJSON_MODEL_PRIVATE_H

#include "ejson_private.h"

#include <json.h>

#include <stdbool.h>

typedef struct _Ejson_Model_Data Ejson_Model_Data;

/**
 * ejson_model
 */
struct _Ejson_Model_Data
{
   Eo                *obj;
   Efl_Model_Load     load;
   json_object       *json;
   Eina_Array        *properties_array;
   Eina_Hash         *properties_hash;
   Eina_List         *children_list;
   Eina_Hash         *children_hash;
   Ejson_Model_Type   type;
   Eina_Array        *properties_calc;
   bool               json_property_valid;
   char              *stream;
};

#endif

