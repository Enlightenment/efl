#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_frame.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_OBJ_FRAME_CLASS

#define MY_CLASS_NAME "Elm_Frame"
#define MY_CLASS_NAME_LEGACY "elm_frame"

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static void
_sizing_eval(Evas_Object *obj,
             Elm_Frame_Data *sd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord cminw = -1, cminh = -1;

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_get(obj, &cminw, &cminh);
   if ((minw == cminw) && (minh == cminh)) return;

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

EOLIAN static Eina_Bool
_elm_frame_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Frame_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_frame_elm_widget_focus_next(Eo *obj EINA_UNUSED, Elm_Frame_Data *_pd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next)
{
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   if (!content) return EINA_FALSE;

   else
     {
        /* attempt to follow focus cycle into sub-object */
        return elm_widget_focus_next_get(content, dir, next);
     }
}

EOLIAN static Eina_Bool
_elm_frame_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Frame_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_frame_elm_widget_focus_direction(Eo *obj EINA_UNUSED, Elm_Frame_Data *_pd EINA_UNUSED, const Evas_Object *base, double degree, Evas_Object **direction, double *weight)
{
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   if (!content) return EINA_FALSE;

   else
     {
        /* Try to cycle focus on content */
        return elm_widget_focus_direction_get
           (content, base, degree, direction, weight);
     }
}

static void
_recalc(void *data,
        Evas_Object *obj EINA_UNUSED,
        void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_on_recalc_done(void *data,
                Evas_Object *obj EINA_UNUSED,
                const char *sig EINA_UNUSED,
                const char *src EINA_UNUSED)
{
   ELM_FRAME_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   evas_object_smart_callback_del
     (wd->resize_obj, "recalc", _recalc);
   sd->anim = EINA_FALSE;

   elm_layout_sizing_eval(data);
}

static void
_on_frame_clicked(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *sig EINA_UNUSED,
                  const char *src EINA_UNUSED)
{
   ELM_FRAME_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   if (sd->anim) return;

   if (sd->collapsible)
     {
        evas_object_smart_callback_add
          (wd->resize_obj, "recalc", _recalc, data);
        elm_layout_signal_emit(data, "elm,action,toggle", "elm");
        sd->collapsed++;
        sd->anim = EINA_TRUE;
     }
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

/* using deferred sizing evaluation, just like the parent */
EOLIAN static void
_elm_frame_evas_smart_calculate(Eo *obj, Elm_Frame_Data *sd)
{
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (ld->needs_size_calc)
     {
        /* calling OWN sizing evaluate code here */
        _sizing_eval(obj, sd);
        ld->needs_size_calc = EINA_FALSE;
     }
}

EOLIAN static void
_elm_frame_evas_smart_add(Eo *obj, Elm_Frame_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,anim,done", "elm",
     _on_recalc_done, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,click", "elm",
     _on_frame_clicked, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   if (!elm_layout_theme_set(obj, "frame", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_sizing_eval(obj);
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_frame_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Frame_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_frame_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Elm_Frame_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EAPI Evas_Object *
elm_frame_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_frame_constructor(Eo *obj, Elm_Frame_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EOLIAN static void
_elm_frame_autocollapse_set(Eo *obj EINA_UNUSED, Elm_Frame_Data *sd, Eina_Bool autocollapse)
{

   sd->collapsible = !!autocollapse;
}

EOLIAN static Eina_Bool
_elm_frame_autocollapse_get(Eo *obj EINA_UNUSED, Elm_Frame_Data *sd)
{
   return sd->collapsible;
}

EOLIAN static void
_elm_frame_collapse_set(Eo *obj, Elm_Frame_Data *sd, Eina_Bool collapse)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   collapse = !!collapse;
   if (sd->collapsed == collapse) return;

   elm_layout_signal_emit(obj, "elm,action,switch", "elm");
   edje_object_message_signal_process(wd->resize_obj);
   sd->collapsed = !!collapse;
   sd->anim = EINA_FALSE;

   _sizing_eval(obj, sd);
}

EOLIAN static void
_elm_frame_collapse_go(Eo *obj, Elm_Frame_Data *sd, Eina_Bool collapse)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   collapse = !!collapse;
   if (sd->collapsed == collapse) return;

   elm_layout_signal_emit(obj, "elm,action,toggle", "elm");
   evas_object_smart_callback_add
     (wd->resize_obj, "recalc", _recalc, obj);
   sd->collapsed = collapse;
   sd->anim = EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_frame_collapse_get(Eo *obj EINA_UNUSED, Elm_Frame_Data *sd)
{
   return sd->collapsed;
}

EOLIAN static void
_elm_frame_class_constructor(Eo_Class *klass)
{
      evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_frame.eo.c"
