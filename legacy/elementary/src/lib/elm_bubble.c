#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_bubble.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_BUBBLE_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_BUBBLE_CLASS

#define MY_CLASS_NAME "Elm_Bubble"
#define MY_CLASS_NAME_LEGACY "elm_bubble"

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_CLICKED, ""},
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {"icon", "elm.swallow.icon"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {"info", "elm.info"},
   {NULL, NULL}
};

static const char *corner_string[] =
{
   "top_left",
   "top_right",
   "bottom_left",
   "bottom_right"
};

static void
_elm_bubble_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

/* overriding layout's focus_next() in order to just cycle through the
 * content's tree */
static void
_elm_bubble_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *content;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   if ((elm_widget_can_focus_get(obj)) &&
       (!wd->focused))
     {
        // ACCESS
        *next = (Evas_Object *)obj;
        if (ret) *ret = EINA_TRUE;
        return;
     }
   else
     {
        content = elm_layout_content_get(obj, NULL);
        if (!content) return;

        /* attempt to follow focus cycle into sub-object */
        int_ret = elm_widget_focus_next_get(content, dir, next);
        if (ret) *ret = int_ret;
     }
}

static void
_elm_bubble_smart_focus_direction(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *content;

   Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   content = elm_layout_content_get(obj, NULL);

   if (!content) return;

   /* Try Focus cycle in subitem */
   int_ret = elm_widget_focus_direction_get
            (content, base, degree, direction, weight);
   if (ret) *ret = int_ret;
}

static void
_elm_bubble_smart_text_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_layout_text_set(part, label, &int_ret));
   if (!int_ret) return;

   if (part && (!strcmp(part, "info") || !strcmp(part, "elm.info")))
     {
        if (label)
          elm_layout_signal_emit(obj, "elm,state,info,visible", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,info,hidden", "elm");
     }

   elm_layout_sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();
   Evas_Object *content;
   const char *default_txt = NULL;
   const char *content_txt = NULL;
   const char *info_txt = NULL;

   default_txt = elm_widget_access_info_get(obj);
   if (!default_txt) default_txt = elm_layout_text_get(obj, NULL);
   if (default_txt) eina_strbuf_append(buf, default_txt);

   content = elm_layout_content_get(obj, NULL);
   if (content) content_txt = elm_layout_text_get(content, NULL);
   if (content_txt)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, content_txt);
        else
          eina_strbuf_append_printf(buf, ", %s", content_txt);
     }


   info_txt = edje_object_part_text_get(elm_layout_edje_get(obj), "elm.info");
   if (info_txt)
     {
        if (!eina_strbuf_length_get(buf))
          eina_strbuf_append(buf, info_txt);
        else
          eina_strbuf_append_printf(buf, ", %s", info_txt);
     }

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_elm_bubble_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Bubble_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->pos = ELM_BUBBLE_POS_TOP_LEFT; //default

   elm_widget_can_focus_set(obj, EINA_FALSE);

   evas_object_event_callback_add
     (wd->resize_obj, EVAS_CALLBACK_MOUSE_UP,
     _on_mouse_up, obj);

   // ACCESS
   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Bubble"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);

   if (!elm_layout_theme_set(obj, "bubble", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_sizing_eval(obj);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_bubble_smart_access(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool is_access = va_arg(*list, int);
   ELM_BUBBLE_CHECK(obj);

   if (is_access)
     elm_widget_can_focus_set(obj, EINA_TRUE);
   else
     elm_widget_can_focus_set(obj, EINA_FALSE);
}

EAPI Evas_Object *
elm_bubble_add(Evas_Object *parent)
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
elm_bubble_pos_set(Evas_Object *obj,
                   Elm_Bubble_Pos pos)
{
   ELM_BUBBLE_CHECK(obj);
   eo_do(obj, elm_obj_bubble_pos_set(pos));
}

static void
_pos_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Bubble_Pos pos = va_arg(*list, Elm_Bubble_Pos);
   Elm_Bubble_Smart_Data *sd = _pd;
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (pos < ELM_BUBBLE_POS_TOP_LEFT || pos > ELM_BUBBLE_POS_BOTTOM_RIGHT)
     return;

   sd->pos = pos;

   eina_stringshare_replace
     (&ld->group, corner_string[sd->pos]);

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

EAPI Elm_Bubble_Pos
elm_bubble_pos_get(const Evas_Object *obj)
{
   ELM_BUBBLE_CHECK(obj) ELM_BUBBLE_POS_INVALID;
   Elm_Bubble_Pos ret = ELM_BUBBLE_POS_INVALID;
   eo_do((Eo *) obj, elm_obj_bubble_pos_get(&ret));
   return ret;
}

static void
_pos_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Bubble_Pos *ret = va_arg(*list, Elm_Bubble_Pos *);
   Elm_Bubble_Smart_Data *sd = _pd;
   *ret = sd->pos;
}

static void
_elm_bubble_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_bubble_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_bubble_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _text_aliases;
}

static void
_elm_bubble_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_bubble_smart_add),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_bubble_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_bubble_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_bubble_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_bubble_smart_focus_direction),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACCESS), _elm_bubble_smart_access),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), _elm_bubble_smart_text_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_bubble_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_bubble_smart_text_aliases_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_bubble_smart_content_aliases_get),

        EO_OP_FUNC(ELM_OBJ_BUBBLE_ID(ELM_OBJ_BUBBLE_SUB_ID_POS_SET), _pos_set),
        EO_OP_FUNC(ELM_OBJ_BUBBLE_ID(ELM_OBJ_BUBBLE_SUB_ID_POS_GET), _pos_get),

        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_BUBBLE_SUB_ID_POS_SET, "Set the corner of the bubble."),
     EO_OP_DESCRIPTION(ELM_OBJ_BUBBLE_SUB_ID_POS_GET, "Get the corner of the bubble."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_BUBBLE_BASE_ID, op_desc, ELM_OBJ_BUBBLE_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Bubble_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_bubble_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
