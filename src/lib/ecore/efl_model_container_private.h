#ifndef EFL_MODEL_CONTAINER_PRIVATE_H__
#define EFL_MODEL_CONTAINER_PRIVATE_H__

typedef struct _Child_Property_Data Child_Property_Data;
struct _Child_Property_Data
{
   const Eina_Value_Type            *type;
   Eina_Array                       *values;
};

typedef struct _Efl_Model_Container_Data Efl_Model_Container_Data;
struct _Efl_Model_Container_Data
{
   Eo                               *obj;

   Eina_Hash                        *properties;

   Eina_List                        *childrens;
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
