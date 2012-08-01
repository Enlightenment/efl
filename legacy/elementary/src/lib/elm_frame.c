#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_frame.h"

EAPI const char ELM_FRAME_SMART_NAME[] = "elm_frame";

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
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

EVAS_SMART_SUBCLASS_NEW
  (ELM_FRAME_SMART_NAME, _elm_frame, Elm_Frame_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_sizing_eval(Evas_Object *obj,
             Elm_Frame_Smart_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord cminw = -1, cminh = -1;

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_get(obj, &cminw, &cminh);
   if ((minw == cminw) && (minh == cminh)) return;

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_elm_frame_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   if (!content) return EINA_FALSE;

   /* attempt to follow focus cycle into sub-object */
   return elm_widget_focus_next_get(content, dir, next);
}

static Eina_Bool
_elm_frame_smart_focus_direction(const Evas_Object *obj,
                                 const Evas_Object *base,
                                 double degree,
                                 Evas_Object **direction,
                                 double *weight)
{
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   if (!content) return EINA_FALSE;

   /* Try to cycle focus on content */
   return elm_widget_focus_direction_get
            (content, base, degree, direction, weight);
}

static void
_recalc(void *data,
        Evas_Object *obj __UNUSED__,
        void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_on_recalc_done(void *data,
                Evas_Object *obj __UNUSED__,
                const char *sig __UNUSED__,
                const char *src __UNUSED__)
{
   ELM_FRAME_DATA_GET(data, sd);

   evas_object_smart_callback_del
     (ELM_WIDGET_DATA(sd)->resize_obj, "recalc", _recalc);
   sd->anim = EINA_FALSE;

   elm_layout_sizing_eval(data);
}

static void
_on_frame_clicked(void *data,
                  Evas_Object *obj __UNUSED__,
                  const char *sig __UNUSED__,
                  const char *src __UNUSED__)
{
   ELM_FRAME_DATA_GET(data, sd);

   if (sd->anim) return;

   if (sd->collapsible)
     {
        evas_object_smart_callback_add
          (ELM_WIDGET_DATA(sd)->resize_obj, "recalc", _recalc, data);
        elm_layout_signal_emit(data, "elm,action,toggle", "elm");
        sd->collapsed++;
        sd->anim = EINA_TRUE;
     }
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

/* using deferred sizing evaluation, just like the parent */
static void
_elm_frame_smart_calculate(Evas_Object *obj)
{
   ELM_FRAME_DATA_GET(obj, sd);

   if (ELM_LAYOUT_DATA(sd)->needs_size_calc)
     {
        /* calling OWN sizing evaluate code here */
        _sizing_eval(obj, sd);
        ELM_LAYOUT_DATA(sd)->needs_size_calc = EINA_FALSE;
     }
}

static void
_elm_frame_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Frame_Smart_Data);

   ELM_WIDGET_CLASS(_elm_frame_parent_sc)->base.add(obj);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,anim,done", "elm",
     _on_recalc_done, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,click", "elm",
     _on_frame_clicked, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   elm_layout_theme_set(obj, "frame", "base", elm_widget_style_get(obj));

   elm_layout_sizing_eval(obj);
}

static void
_elm_frame_smart_set_user(Elm_Frame_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_frame_smart_add;
   ELM_WIDGET_CLASS(sc)->base.calculate = _elm_frame_smart_calculate;

   ELM_WIDGET_CLASS(sc)->focus_next = _elm_frame_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_frame_smart_focus_direction;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;
}

EAPI const Elm_Frame_Smart_Class *
elm_frame_smart_class_get(void)
{
   static Elm_Frame_Smart_Class _sc =
     ELM_FRAME_SMART_CLASS_INIT_NAME_VERSION(ELM_FRAME_SMART_NAME);
   static const Elm_Frame_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_frame_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_frame_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_frame_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_frame_autocollapse_set(Evas_Object *obj,
                           Eina_Bool autocollapse)
{
   ELM_FRAME_CHECK(obj);
   ELM_FRAME_DATA_GET_OR_RETURN(obj, sd);

   sd->collapsible = !!autocollapse;
}

EAPI Eina_Bool
elm_frame_autocollapse_get(const Evas_Object *obj)
{
   ELM_FRAME_CHECK(obj) EINA_FALSE;
   ELM_FRAME_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->collapsible;
}

EAPI void
elm_frame_collapse_set(Evas_Object *obj,
                       Eina_Bool collapse)
{
   ELM_FRAME_CHECK(obj);
   ELM_FRAME_DATA_GET_OR_RETURN(obj, sd);

   collapse = !!collapse;
   if (sd->collapsed == collapse) return;

   elm_layout_signal_emit(obj, "elm,action,switch", "elm");
   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
   sd->collapsed = !!collapse;
   sd->anim = EINA_FALSE;

   _sizing_eval(obj, sd);
}

EAPI void
elm_frame_collapse_go(Evas_Object *obj,
                      Eina_Bool collapse)
{
   ELM_FRAME_CHECK(obj);
   ELM_FRAME_DATA_GET_OR_RETURN(obj, sd);

   collapse = !!collapse;
   if (sd->collapsed == collapse) return;

   elm_layout_signal_emit(obj, "elm,action,toggle", "elm");
   evas_object_smart_callback_add
     (ELM_WIDGET_DATA(sd)->resize_obj, "recalc", _recalc, obj);
   sd->collapsed = collapse;
   sd->anim = EINA_TRUE;
}

EAPI Eina_Bool
elm_frame_collapse_get(const Evas_Object *obj)
{
   ELM_FRAME_CHECK(obj) EINA_FALSE;
   ELM_FRAME_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->collapsed;
}
