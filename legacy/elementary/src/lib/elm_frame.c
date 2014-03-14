#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_frame.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_FRAME_BASE_ID = EO_NOOP;

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
             Elm_Frame_Smart_Data *sd EINA_UNUSED)
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

static void
_elm_frame_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_frame_smart_focus_next(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret;
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   if (!content) int_ret = EINA_FALSE;

   else
     {
        /* attempt to follow focus cycle into sub-object */
        int_ret = elm_widget_focus_next_get(content, dir, next);
     }
   if (ret) *ret = int_ret;
}

static void
_elm_frame_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_frame_smart_focus_direction(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Evas_Object *base = va_arg(*list, const Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret;
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   if (!content) int_ret = EINA_FALSE;

   else
     {
        /* Try to cycle focus on content */
        int_ret = elm_widget_focus_direction_get
           (content, base, degree, direction, weight);
     }
   if (ret) *ret = int_ret;
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
static void
_elm_frame_smart_calculate(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Frame_Smart_Data *sd = _pd;
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (ld->needs_size_calc)
     {
        /* calling OWN sizing evaluate code here */
        _sizing_eval(obj, sd);
        ld->needs_size_calc = EINA_FALSE;
     }
}

static void
_elm_frame_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
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

static void
_elm_frame_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

static void
_elm_frame_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _text_aliases;
}

EAPI Evas_Object *
elm_frame_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_frame_autocollapse_set(Evas_Object *obj,
                           Eina_Bool autocollapse)
{
   ELM_FRAME_CHECK(obj);
   eo_do(obj, elm_obj_frame_autocollapse_set(autocollapse));
}

static void
_autocollapse_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool autocollapse = va_arg(*list, int);
   Elm_Frame_Smart_Data *sd = _pd;

   sd->collapsible = !!autocollapse;
}

EAPI Eina_Bool
elm_frame_autocollapse_get(const Evas_Object *obj)
{
   ELM_FRAME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_frame_autocollapse_get(&ret));
   return ret;
}

static void
_autocollapse_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Frame_Smart_Data *sd = _pd;

   *ret = sd->collapsible;
}

EAPI void
elm_frame_collapse_set(Evas_Object *obj,
                       Eina_Bool collapse)
{
   ELM_FRAME_CHECK(obj);
   eo_do(obj, elm_obj_frame_collapse_set(collapse));
}

static void
_collapse_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool collapse = va_arg(*list, int);
   Elm_Frame_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   collapse = !!collapse;
   if (sd->collapsed == collapse) return;

   elm_layout_signal_emit(obj, "elm,action,switch", "elm");
   edje_object_message_signal_process(wd->resize_obj);
   sd->collapsed = !!collapse;
   sd->anim = EINA_FALSE;

   _sizing_eval(obj, sd);
}

EAPI void
elm_frame_collapse_go(Evas_Object *obj,
                      Eina_Bool collapse)
{
   ELM_FRAME_CHECK(obj);
   eo_do(obj, elm_obj_frame_collapse_go(collapse));
}

static void
_collapse_go(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool collapse = va_arg(*list, int);
   Elm_Frame_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   collapse = !!collapse;
   if (sd->collapsed == collapse) return;

   elm_layout_signal_emit(obj, "elm,action,toggle", "elm");
   evas_object_smart_callback_add
     (wd->resize_obj, "recalc", _recalc, obj);
   sd->collapsed = collapse;
   sd->anim = EINA_TRUE;
}

EAPI Eina_Bool
elm_frame_collapse_get(const Evas_Object *obj)
{
   ELM_FRAME_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_frame_collapse_get(&ret));
   return ret;
}

static void
_collapse_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Frame_Smart_Data *sd = _pd;

   *ret = sd->collapsed;
}

static void
_class_constructor(Eo_Class *klass)
{
      const Eo_Op_Func_Description func_desc[] = {
           EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_frame_smart_add),
           EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE), _elm_frame_smart_calculate),

           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_frame_smart_focus_next_manager_is),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_frame_smart_focus_next),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_frame_smart_focus_direction_manager_is),
           EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_frame_smart_focus_direction),

           EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_frame_smart_content_aliases_get),
           EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_frame_smart_text_aliases_get),

           EO_OP_FUNC(ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_SET), _autocollapse_set),
           EO_OP_FUNC(ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_GET), _autocollapse_get),
           EO_OP_FUNC(ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_SET), _collapse_set),
           EO_OP_FUNC(ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GO), _collapse_go),
           EO_OP_FUNC(ELM_OBJ_FRAME_ID(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GET), _collapse_get),
           EO_OP_FUNC_SENTINEL
      };
      eo_class_funcs_set(klass, func_desc);

      evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_SET, "Toggle autocollapsing of a frame."),
     EO_OP_DESCRIPTION(ELM_OBJ_FRAME_SUB_ID_AUTOCOLLAPSE_GET, "Determine autocollapsing of a frame."),
     EO_OP_DESCRIPTION(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_SET, "Manually collapse a frame without animations."),
     EO_OP_DESCRIPTION(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GO, "Manually collapse a frame with animations."),
     EO_OP_DESCRIPTION(ELM_OBJ_FRAME_SUB_ID_COLLAPSE_GET, "Determine the collapse state of a frame."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_FRAME_BASE_ID, op_desc, ELM_OBJ_FRAME_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Frame_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_frame_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, EVAS_SMART_CLICKABLE_INTERFACE, NULL);
