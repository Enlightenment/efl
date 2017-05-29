/* Legacy API implementations based on internal EO calls */

#include "edje_private.h"

EAPI Eina_Bool
edje_object_part_geometry_get(const Edje_Object *obj, const char *part, int *x, int *y, int *w, int *h)
{
   Edje_Real_Part *rp;
   Edje *ed;

   // Similar to geometry_get(efl_part(obj, part), x, y, w, h) but the bool
   // return value matters here.

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        return EINA_FALSE;
     }
   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
   return EINA_TRUE;
}

EAPI const char *
edje_object_part_state_get(const Edje_Object *obj, const char * part, double *val_ret)
{
   const char *str = "";
   efl_canvas_layout_internal_state_get(efl_part(obj, part), &str, val_ret);
   return str;
}

EAPI void
edje_object_message_signal_process(Edje_Object *obj)
{
   edje_obj_message_signal_process(obj, EINA_FALSE);
}

/* since 1.20 */
EAPI void
edje_object_message_signal_recursive_process(Edje_Object *obj)
{
   edje_obj_message_signal_process(obj, EINA_TRUE);
}

EAPI Eina_Bool
edje_object_part_external_param_set(Eo *obj, const char *part, const Edje_External_Param *param)
{
   Edje *ed = _edje_fetch(obj);

   if (!ed || !param || !part) return EINA_FALSE;

#if 0
   /* validate EO API - disabled for performance */
   Eina_Value *v;
   Eina_Bool ok;

   switch (param->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        v = eina_value_new(EINA_VALUE_TYPE_INT);
        eina_value_set(v, param->i);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        v = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
        eina_value_set(v, param->d);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
        v = eina_value_new(EINA_VALUE_TYPE_STRING);
        eina_value_set(v, param->s);
        break;

      default: return EINA_FALSE;
     }

   ok = efl_canvas_layout_external_param_set(efl_part(obj, part), param->name, v);
   eina_value_free(v);

   return ok;

#else
   return _edje_object_part_external_param_set(ed, part, param);
#endif
}

EAPI Eina_Bool
edje_object_part_external_param_get(const Eo *obj, const char *part, Edje_External_Param *param)
{
   Edje *ed = _edje_fetch(obj);
   return _edje_object_part_external_param_get(ed, part, param);
}

EAPI Edje_External_Param_Type
edje_object_part_external_param_type_get(const Eo *obj, const char *part, const char *param)
{
   Edje *ed = _edje_fetch(obj);
   return _edje_object_part_external_param_type_get(ed, part, param);
}

EAPI Evas_Object *
edje_object_part_external_object_get(const Edje_Object *obj, const char *part)
{
   return efl_content_get(efl_part(obj, part));
}

/* Legacy only. Shall we deprecate this API? */
EAPI Evas_Object *
edje_object_part_external_content_get(const Edje_Object *obj, const char *part, const char *content)
{
   Edje *ed = _edje_fetch(obj);
   return _edje_object_part_external_content_get(ed, part, content);
}
