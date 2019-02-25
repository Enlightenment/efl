#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_pan.h"

#define MY_CLASS EFL_UI_PAN_CLASS
#define MY_CLASS_NAME "Efl_Ui_Pan"

#define EFL_UI_PAN_DATA_GET(o, sd) \
  Efl_Ui_Pan_Data *sd = efl_data_scope_safe_get(o, MY_CLASS)

#define EFL_UI_PAN_DATA_GET_OR_RETURN(o, ptr, ...)                      \
  EFL_UI_PAN_DATA_GET(o, ptr);                                     \
  if (EINA_UNLIKELY(!ptr))                                         \
    {                                                              \
      ERR("No widget data for object %p (%s)",                     \
          o, evas_object_type_get(o));                             \
      return __VA_ARGS__;                                                      \
    }

EOLIAN static void
_efl_ui_pan_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Pan_Data *psd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);

   psd->x = pos.x;
   psd->y = pos.y;

   evas_object_smart_changed(obj);
}

EOLIAN static void
_efl_ui_pan_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Pan_Data *psd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);

   psd->w = sz.w;
   psd->h = sz.h;

   evas_object_smart_changed(obj);
   efl_event_callback_call(obj, EFL_UI_PAN_EVENT_VIEWPORT_CHANGED, NULL);
}

EOLIAN static void
_efl_ui_pan_efl_gfx_entity_visible_set(Eo *obj, Efl_Ui_Pan_Data *psd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);
   if (psd->content) efl_gfx_entity_visible_set(psd->content, vis);
}

EOLIAN static void
_efl_ui_pan_pan_position_set(Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *psd, Eina_Position2D pos)
{
   if ((pos.x == psd->px) && (pos.y == psd->py)) return;
   psd->px = pos.x;
   psd->py = pos.y;

   evas_object_smart_changed(obj);
   efl_event_callback_call(obj, EFL_UI_PAN_EVENT_POSITION_CHANGED, NULL);
}

EOLIAN static Eina_Position2D
_efl_ui_pan_pan_position_get(const Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *psd)
{
   return EINA_POSITION2D(psd->px, psd->py);
}

EOLIAN static Eina_Position2D
_efl_ui_pan_pan_position_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *psd)
{
   Eina_Position2D pos = { 0, 0};
   if (psd->w < psd->content_w) pos.x = psd->content_w - psd->w;
   if (psd->h < psd->content_h) pos.y = psd->content_h - psd->h;

   return pos;
}

EOLIAN static Eina_Position2D
_efl_ui_pan_pan_position_min_get(const Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *_pd EINA_UNUSED)
{
   return EINA_POSITION2D(0 ,0);
}

EOLIAN static Eina_Size2D
_efl_ui_pan_content_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *psd)
{
   return EINA_SIZE2D(psd->content_w, psd->content_h);
}

EOLIAN static Eo *
_efl_ui_pan_efl_object_constructor(Eo *obj, Efl_Ui_Pan_Data *_pd EINA_UNUSED)
{
   efl_canvas_group_clipped_set(obj, EINA_TRUE);
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static void
_efl_ui_pan_efl_object_destructor(Eo *obj, Efl_Ui_Pan_Data *sd EINA_UNUSED)
{
   efl_content_set(obj, NULL);
   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_efl_ui_pan_content_del_cb(void *data,
                        Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Evas_Object *pobj = data;
   EFL_UI_PAN_DATA_GET_OR_RETURN(pobj, psd);

   psd->content = NULL;
   psd->content_w = psd->content_h = psd->px = psd->py = 0;
   efl_event_callback_call(pobj, EFL_UI_PAN_EVENT_CONTENT_CHANGED, NULL);
}

static void
_efl_ui_pan_content_resize_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Evas_Object *pobj = data;
   EFL_UI_PAN_DATA_GET_OR_RETURN(pobj, psd);

   Eina_Size2D sz = efl_gfx_entity_size_get(psd->content);
   if ((sz.w != psd->content_w) || (sz.h != psd->content_h))
     {
        psd->content_w = sz.w;
        psd->content_h = sz.h;
        evas_object_smart_changed(pobj);
     }
   efl_event_callback_call(pobj, EFL_UI_PAN_EVENT_CONTENT_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_ui_pan_efl_content_content_set(Evas_Object *obj, Efl_Ui_Pan_Data *psd, Evas_Object *content)
{
   Eina_Size2D sz;

   if (content == psd->content) return EINA_TRUE;
   if (psd->content)
     {
        efl_content_unset(obj);
     }
   if (!content) goto end;

   psd->content = content;
   efl_canvas_group_member_add(obj, content);
   sz = efl_gfx_entity_size_get(psd->content);
   psd->content_w = sz.w;
   psd->content_h = sz.h;
   evas_object_event_callback_add
     (content, EVAS_CALLBACK_DEL, _efl_ui_pan_content_del_cb, obj);
   evas_object_event_callback_add
     (content, EVAS_CALLBACK_RESIZE, _efl_ui_pan_content_resize_cb, obj);

   if (evas_object_visible_get(obj))
     evas_object_show(psd->content);
   else
     evas_object_hide(psd->content);

   evas_object_smart_changed(obj);

end:
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, content);
   efl_event_callback_call(obj, EFL_UI_PAN_EVENT_CONTENT_CHANGED, NULL);
   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_pan_efl_content_content_get(const Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *pd)
{
   return pd->content;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_pan_efl_content_content_unset(Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *pd)
{
   Efl_Gfx_Stack *old_content = pd->content;

   efl_canvas_group_member_del(obj, pd->content);
   evas_object_event_callback_del_full
     (pd->content, EVAS_CALLBACK_DEL, _efl_ui_pan_content_del_cb, obj);
   evas_object_event_callback_del_full
     (pd->content, EVAS_CALLBACK_RESIZE, _efl_ui_pan_content_resize_cb,
     obj);
   pd->content = NULL;
   pd->content_w = pd->content_h = pd->px = pd->py = 0;
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, NULL);
   efl_event_callback_call(obj, EFL_UI_PAN_EVENT_CONTENT_CHANGED, NULL);

   return old_content;
}

EOLIAN static void
_efl_ui_pan_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Efl_Ui_Pan_Data *psd)
{
   efl_gfx_entity_position_set(psd->content, EINA_POSITION2D(psd->x - psd->px, psd->y - psd->py));
}
#include "efl_ui_pan.eo.c"
