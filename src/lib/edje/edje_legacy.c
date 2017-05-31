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
   return _edje_object_part_external_param_set(ed, part, param);
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

/* Efl.Ui.Base APIs */
EAPI void
edje_object_mirrored_set(Edje_Object *obj, Eina_Bool rtl)
{
   efl_ui_mirrored_set(obj, rtl);
}

EAPI Eina_Bool edje_object_mirrored_get(const Edje_Object *obj)
{
   return efl_ui_mirrored_get(obj);
}

EAPI void edje_object_language_set(Edje_Object *obj, const char *language)
{
   efl_ui_language_set(obj, language);
}

EAPI const char *edje_object_language_get(const Edje_Object *obj)
{
   return efl_ui_language_get(obj);
}

EAPI Eina_Bool edje_object_scale_set(Edje_Object *obj, double scale)
{
   return efl_ui_scale_set(obj, scale);
}

EAPI double edje_object_scale_get(const Edje_Object *obj)
{
   return efl_ui_scale_get(obj);
}

/* Legacy part drag APIs */
EAPI Edje_Drag_Dir
edje_object_part_drag_dir_get(const Evas_Object *obj, const char *part)
{
   return efl_ui_drag_dir_get(efl_part(obj, part));
}

EAPI Eina_Bool
edje_object_part_drag_value_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_value_set(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_value_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   return efl_ui_drag_value_get(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_size_set(Evas_Object *obj, const char *part, double dw, double dh)
{
   return efl_ui_drag_size_set(efl_part(obj, part), dw, dh);
}

EAPI Eina_Bool
edje_object_part_drag_size_get(const Evas_Object *obj, const char *part, double *dw, double *dh)
{
   return efl_ui_drag_size_get(efl_part(obj, part), dw, dh);
}

EAPI Eina_Bool
edje_object_part_drag_step_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_step_set(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_step_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   return efl_ui_drag_step_get(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_page_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_page_set(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_page_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   return efl_ui_drag_page_get(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_step_move(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_page_move(efl_part(obj, part), dx, dy);
}
