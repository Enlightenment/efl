#include "edje_private.h"
#include "edje_part_helper.h"

#define MY_CLASS EFL_CANVAS_LAYOUT_EXTERNAL_CLASS

PROXY_IMPLEMENTATION(external, EXTERNAL)
#undef PROXY_IMPLEMENTATION

EOLIAN static Eina_Bool
_efl_canvas_layout_external_external_param_set(Eo *obj, void *_pd EINA_UNUSED,
                                               const char *name, const Eina_Value *value)
{
   Edje_External_Param_Type type;
   Edje_External_Param param = {};
   const Eina_Value_Type *vtype;

   PROXY_DATA_GET(obj, pd);
   type = _edje_object_part_external_param_type_get(pd->ed, pd->part, name);

   param.name = name;
   param.type = type;
   vtype = eina_value_type_get(value);

   switch (type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        if (vtype == EINA_VALUE_TYPE_INT)
          eina_value_get(value, &param.i);
        else
          {
             Eina_Value v2;

             eina_value_setup(&v2, EINA_VALUE_TYPE_INT);
             eina_value_convert(value, &v2);
             eina_value_get(&v2, &param.i);
             eina_value_flush(&v2);
          }
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        if (vtype == EINA_VALUE_TYPE_DOUBLE)
          eina_value_get(value, &param.d);
        else
          {
             Eina_Value v2;

             eina_value_setup(&v2, EINA_VALUE_TYPE_DOUBLE);
             eina_value_convert(value, &v2);
             eina_value_get(&v2, &param.d);
             eina_value_flush(&v2);
          }
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
        // Should we use eina_value_to_string() here and not complain?
        if ((vtype != EINA_VALUE_TYPE_STRING) && (vtype != EINA_VALUE_TYPE_STRINGSHARE))
          {
             ERR("External parameter '%s' of object '%p' expects a string, "
                 "got a '%s'.", name, pd->ed->obj, eina_value_type_name_get(vtype));
             RETURN_VAL(EINA_FALSE);
          }
        eina_value_get(value, &param.s);
        break;

      default:
        ERR("Unsupported type for parameter '%s' of object '%p'", name, pd->ed->obj);
        RETURN_VAL(EINA_FALSE);
     }

   RETURN_VAL(_edje_object_part_external_param_set(pd->ed, pd->part, &param));
}

EOLIAN static Eina_Value *
_efl_canvas_layout_external_external_param_get(Eo *obj, void *_pd EINA_UNUSED,
                                               const char *name)
{
   Edje_External_Param param = {};
   Eina_Value *v;

   PROXY_DATA_GET(obj, pd);

   param.name = name;
   param.type = _edje_object_part_external_param_type_get(pd->ed, pd->part, name);
   if (param.type == EDJE_EXTERNAL_PARAM_TYPE_MAX)
     RETURN_VAL(NULL);
   if (!_edje_object_part_external_param_get(pd->ed, pd->part, &param))
     RETURN_VAL(NULL);

   switch (param.type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        v = eina_value_new(EINA_VALUE_TYPE_INT);
        eina_value_set(v, param.i);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        v = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
        eina_value_set(v, param.d);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
        v = eina_value_new(EINA_VALUE_TYPE_STRING);
        eina_value_set(v, param.s);
        break;

      default:
        ERR("Unsupported type for parameter '%s' of object '%p'", name, pd->ed->obj);
        RETURN_VAL(NULL);
     }

   RETURN_VAL(v);
}

EOLIAN static Edje_External_Param_Type
_efl_canvas_layout_external_external_param_type_get(Eo *obj, void *_pd EINA_UNUSED,
                                                    const char *name)
{
   PROXY_DATA_GET(obj, pd);
   RETURN_VAL(_edje_object_part_external_param_type_get(pd->ed, pd->part, name));
}

#include "efl_canvas_layout_external.eo.c"
