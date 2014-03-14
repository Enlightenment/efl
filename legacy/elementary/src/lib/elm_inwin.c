#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_inwin.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_WIN_INWIN_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_WIN_INWIN_CLASS

#define MY_CLASS_NAME "Elm_Inwin"
#define MY_CLASS_NAME_LEGACY "elm_inwin"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {NULL, NULL}
};

static void
_elm_inwin_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object *content;
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   content = elm_layout_content_get(obj, NULL);

   if (!content) return;

   evas_object_size_hint_min_get(content, &minw, &minh);
   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_elm_inwin_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_inwin_smart_focus_next(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   /* attempt to follow focus cycle into sub-object */
   if (content)
     {
        elm_widget_focus_next_get(content, dir, next);
        if (*next)
          {
             if (ret) *ret = EINA_TRUE;
             return;
          }
     }

   *next = (Evas_Object *)obj;
}

static void
_elm_inwin_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);

   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (!elm_layout_theme_set(obj, "win", "inwin", elm_object_style_get(obj)))
     CRI("Failed to set layout!");
}

static void
_elm_inwin_smart_parent_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   elm_win_resize_object_add(parent, obj);

   elm_layout_sizing_eval(obj);
}

static void
_elm_inwin_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

EAPI Evas_Object *
elm_win_inwin_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object *parent;

   eo_do(obj, eo_parent_get(&parent));

   if (parent && !eo_isa(parent, ELM_OBJ_WIN_CLASS))
     {
        eo_error_set(obj);  /* *has* to have a parent window */
        return;
     }

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

EAPI void
elm_win_inwin_activate(Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj);
   eo_do(obj, elm_obj_win_inwin_activate());
}

static void
_elm_inwin_smart_activate(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_disabled_get(obj)) return;

   evas_object_raise(obj);
   evas_object_show(obj);
   edje_object_signal_emit
     (wd->resize_obj, "elm,action,show", "elm");
   elm_object_focus_set(obj, EINA_TRUE);
}

EAPI void
elm_win_inwin_content_set(Evas_Object *obj,
                          Evas_Object *content)
{
   ELM_INWIN_CHECK(obj);
   eo_do(obj, elm_obj_container_content_set(NULL, content, NULL));
}

EAPI Evas_Object *
elm_win_inwin_content_get(const Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *)obj, elm_obj_container_content_get(NULL, &ret));
   return ret;
}

EAPI Evas_Object *
elm_win_inwin_content_unset(Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_container_content_unset(NULL, &ret));
   return ret;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_inwin_smart_add),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_inwin_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_inwin_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_PARENT_SET), _elm_inwin_smart_parent_set),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_inwin_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_inwin_smart_content_aliases_get),

        EO_OP_FUNC(ELM_OBJ_WIN_INWIN_ID(ELM_OBJ_WIN_INWIN_SUB_ID_ACTIVATE), _elm_inwin_smart_activate),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_INWIN_SUB_ID_ACTIVATE, "Activates an inwin object, ensuring its visibility."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_WIN_INWIN_BASE_ID, op_desc, ELM_OBJ_WIN_INWIN_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_win_inwin_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
