#ifndef __EO_DEFINITIONS_H__
#define __EO_DEFINITIONS_H__

#include <Eina.h>
#include <Eolian.h>

/* RET */

typedef struct _eo_ret_def
{
   const char *type;
   const char *comment;
   const char *dflt_ret_val;
   Eina_Bool warn_unused:1;
   Eina_Bool own:1;
} Eo_Ret_Def;

/* PARAM */

typedef enum _param_way
{
   PARAM_IN,
   PARAM_OUT,
   PARAM_INOUT,
   PARAM_WAY_LAST
} Param_Way;

typedef struct _eo_param_def
{
   Param_Way way;
   const char *type;
   const char *name;
   const char *comment;
   Eina_Bool nonull:1;
   Eina_Bool own:1;
} Eo_Param_Def;

/* ACCESSOR */

typedef enum _eo_accessor_type
{
   SETTER,
   GETTER,
   ACCESSOR_TYPE_LAST
} Eo_Accessor_Type;

typedef struct _eo_accessor_param
{
   const char *name;
   const char *attrs;
} Eo_Accessor_Param;

typedef struct _eo_accessor_def
{
   Eo_Accessor_Type type;
   Eo_Ret_Def *ret;
   const char *comment;
   const char* legacy;
   Eina_List *params; /* List of Eo_Accessor_Param */
} Eo_Accessor_Def;

/* PROPERTY */

typedef struct _eo_property_def
{
   const char *name;
   Eina_List *keys;
   Eina_List *values;
   Eina_List *accessors;
} Eo_Property_Def;

/* METHOD */

typedef enum _eo_method_type {
     METH_REGULAR,
     METH_CONSTRUCTOR,
     METH_DESTRUCTOR,
     METH_TYPE_LAST
} Eo_Method_Type;

typedef struct _eo_method_def
{
   Eo_Ret_Def *ret;
   Eo_Method_Type type;
   const char *name;
   const char *comment;
   Eina_List *params;
   const char* legacy;
   Eina_Bool obj_const;
} Eo_Method_Def;

/* SIGNAL */

typedef struct _eo_event_def
{
   const char *name;
   const char *comment;
} Eo_Event_Def;

/* IMPLEMENT */

typedef struct _eo_implement_legacy_param_def
{
   const char *eo_name;
   const char *legacy_name;
   const char *comment;
} Eo_Implement_Legacy_Param_Def;

typedef struct _eo_implement_legacy_def
{
   const char *function_name;
   Eina_List *params; /* List of Eo_Implement_Legacy_Param_Def * */
   const char *ret_type;
   const char *ret_value;
} Eo_Implement_Legacy_Def;

typedef struct _eo_implement_def
{
   const char *meth_name;
   Eo_Implement_Legacy_Def *legacy;
} Eo_Implement_Def;

/* CLASS */

typedef struct _eo_class_def
{
   const char *name;
   Eolian_Class_Type type;
   const char *comment;
   const char *legacy_prefix;
   const char *eo_prefix;
   const char *data_type;
   Eina_List *inherits;
   Eina_List *implements;
   Eina_List *events;
   Eina_List *constructors;
   Eina_List *destructors;
   Eina_List *properties;
   Eina_List *methods;
} Eo_Class_Def;

void eo_definitions_class_def_free(Eo_Class_Def *kls);

#endif /* __EO_DEFINITIONS_H__ */
