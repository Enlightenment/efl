#ifndef EFL_MODEL_PRIVATE_H__
# define EFL_MODEL_PRIVATE_H__

typedef struct _Child_Property_Data Child_Property_Data;
struct _Child_Property_Data
{
   const Eina_Value_Type            *property_type;
   Eina_Array                       *property_values;
};

typedef struct _Efl_Model_Container_Data Efl_Model_Container_Data;
struct _Efl_Model_Container_Data
{
   Eo                               *obj;
   Eina_Hash                        *property_data;
   Eina_Array                       *defined_properties;
   Eina_List                        *children;
};


typedef struct _Efl_Model_Container_Item_Data Efl_Model_Container_Item_Data;
struct _Efl_Model_Container_Item_Data
{
   Efl_Model_Container_Data         *parent_data;
   unsigned int                      index;
};

void *_value_copy_alloc(void *v, const Eina_Value_Type *type);

void _value_free(void *v, const Eina_Value_Type *type);

#endif
