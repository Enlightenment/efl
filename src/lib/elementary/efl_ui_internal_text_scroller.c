#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_SCROLLBAR_BETA

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "efl_ui_widget_scroller.h"

#include "efl_ui_internal_text_scroller.h"

#define EFL_UI_SCROLLER_DATA_GET(o, sd) \
  Efl_Ui_Scroller_Data * sd = efl_data_scope_safe_get(o, EFL_UI_SCROLLER_CLASS)

#define MY_CLASS EFL_UI_INTERNAL_TEXT_SCROLLER_CLASS
#define MY_CLASS_PFX efl_ui_internal_text_scroller

#define MY_CLASS_NAME "Efl.Ui.Internal_Text_Scroller"

typedef struct _Efl_Ui_Internal_Text_Scroller_Data
{
   Efl_Canvas_Text *text_obj;
   Efl_Ui_Table *text_table;
   Eo *smanager;

   Efl_Ui_Text_Scroller_Mode mode;

   Eina_Bool  match_content_w: 1;
   Eina_Bool  match_content_h: 1;
} Efl_Ui_Internal_Text_Scroller_Data;

#define EFL_UI_INTERNAL_TEXT_SCROLLER_DATA_GET(o, sd) \
  Efl_Ui_Internal_Text_Scroller_Data * sd = efl_data_scope_safe_get(o, EFL_UI_INTERNAL_TEXT_SCROLLER_CLASS)

#define EFL_UI_INTERNAL_TEXT_SCROLLER_DATA_GET_OR_RETURN(o, ptr, ...) \
  EFL_UI_INTERNAL_TEXT_SCROLLER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return __VA_ARGS__;                                    \
    }

EOLIAN static Eo *
_efl_ui_internal_text_scroller_efl_object_constructor(Eo *obj,
                                        Efl_Ui_Internal_Text_Scroller_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   //EFL_UI_SCROLLER_DATA_GET(obj, psd);
   efl_ui_scrollbar_bar_mode_set(obj,
         EFL_UI_SCROLLBAR_MODE_OFF, EFL_UI_SCROLLBAR_MODE_OFF);

   return obj;
}

EOLIAN static void
_efl_ui_internal_text_scroller_elm_layout_sizing_eval(Eo *obj,
      Efl_Ui_Internal_Text_Scroller_Data *sd)
{
   Eina_Size2D size = {-1, -1};
   Eina_Rect view = EINA_RECT(0, 0, 0, 0);
   Evas_Coord vmw = 0, vmh = 0;

   EFL_UI_SCROLLER_DATA_GET(obj, psd);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);


   if (psd->smanager)
     {
        view = efl_ui_scrollable_viewport_geometry_get(psd->smanager);
     }

   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   if (sd->text_obj)
     {
        Eina_Size2D fsz = EINA_SIZE2D(0, 0);
        Eina_Size2D sz = EINA_SIZE2D(0, 0);

        sz = efl_gfx_entity_size_get(sd->text_table);
        efl_event_freeze(sd->text_table);
        efl_event_freeze(sd->text_obj);
        efl_gfx_entity_size_set(sd->text_table, view.size);
        efl_gfx_entity_size_set(sd->text_obj, view.size);
        efl_canvas_text_size_formatted_get(sd->text_obj, &fsz.w, &fsz.h);
        efl_gfx_entity_size_set(sd->text_table, sz);
        efl_gfx_entity_size_set(sd->text_obj, sz);
        efl_event_thaw(sd->text_obj);
        efl_event_thaw(sd->text_table);


        if (sd->mode == EFL_UI_TEXT_SCROLLER_MODE_SINGLELINE)
          {
             size.h = vmh + fsz.h;
             if (fsz.w < view.w)
               {
                  fsz.w = view.w;
               }
          }
        else
          {
             if (fsz.h < view.h)
                fsz.h = view.h;

             if (fsz.w < view.w)
                fsz.w = view.w;
          }

        // FIXME: should be restricted_min?
        efl_gfx_entity_size_set(sd->text_table, fsz);
        efl_gfx_size_hint_min_set(obj, size);
        efl_gfx_size_hint_max_set(obj, EINA_SIZE2D(-1, size.h));
     }
}

EOLIAN static Eo *
_efl_ui_internal_text_scroller_efl_object_finalize(Eo *obj,
                                     Efl_Ui_Internal_Text_Scroller_Data *sd EINA_UNUSED)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));
   efl_ui_scrollbar_bar_mode_set(obj,
         EFL_UI_SCROLLBAR_MODE_OFF, EFL_UI_SCROLLBAR_MODE_OFF);
   efl_content_set(obj, sd->text_table);

   return obj;
}

EOLIAN static void
_efl_ui_internal_text_scroller_efl_object_destructor(Eo *obj,
                                       Efl_Ui_Internal_Text_Scroller_Data *sd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_internal_text_scroller_initialize(Eo *obj,
                                       Efl_Ui_Internal_Text_Scroller_Data *sd,
                                       Efl_Canvas_Text *text_obj,
                                       Efl_Ui_Table *text_table)
{
   if (efl_finalized_get(obj))
     {
        ERR("Can only be called on construction");
        return;
     }

   sd->text_obj = text_obj;
   sd->text_table = text_table;
}

EOLIAN static void
_efl_ui_internal_text_scroller_scroller_mode_set(Eo *obj,
                                       Efl_Ui_Internal_Text_Scroller_Data *sd,
                                       Efl_Ui_Text_Scroller_Mode mode)
{
   EFL_UI_SCROLLER_DATA_GET(obj, psd);
   sd->mode = mode;
   if (mode == EFL_UI_TEXT_SCROLLER_MODE_MULTILINE)
     {
        efl_ui_scrollbar_bar_mode_set(psd->smanager,
              EFL_UI_SCROLLBAR_MODE_AUTO, EFL_UI_SCROLLBAR_MODE_AUTO);
     }
   else // default (single-line)
     {
        efl_ui_scrollbar_bar_mode_set(psd->smanager,
              EFL_UI_SCROLLBAR_MODE_OFF, EFL_UI_SCROLLBAR_MODE_OFF);
     }
}

EOLIAN static Eo *
_efl_ui_internal_text_scroller_viewport_clip_get(const Eo *obj,
      Efl_Ui_Internal_Text_Scroller_Data *sd EINA_UNUSED)
{
   EFL_UI_SCROLLER_DATA_GET(obj, psd);
   return evas_object_clip_get(psd->pan_obj);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_INTERNAL_TEXT_SCROLLER_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_internal_text_scroller)

#include "efl_ui_internal_text_scroller.eo.c"
