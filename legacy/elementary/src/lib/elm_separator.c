#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_separator.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_SEPARATOR_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_SEPARATOR_CLASS

#define MY_CLASS_NAME "Elm_Separator"
#define MY_CLASS_NAME_LEGACY "elm_separator"

static void
_elm_separator_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Separator_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (sd->horizontal)
     eina_stringshare_replace(&ld->group, "horizontal");
   else
     eina_stringshare_replace(&ld->group, "vertical");

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_separator_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

static void
_elm_separator_smart_add(Eo *obj, void *_pd EINA_UNUSED,
                         va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   if (!elm_layout_theme_set
       (obj, "separator", "vertical", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_separator_add(Evas_Object *parent)
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
         evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

EAPI void
elm_separator_horizontal_set(Evas_Object *obj,
                             Eina_Bool horizontal)
{
   ELM_SEPARATOR_CHECK(obj);
   eo_do(obj, elm_obj_separator_horizontal_set(horizontal));
}

static void
_horizontal_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool horizontal = va_arg(*list, int);
   Elm_Separator_Smart_Data *sd = _pd;

   horizontal = !!horizontal;
   if (sd->horizontal == horizontal) return;

   sd->horizontal = horizontal;

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

EAPI Eina_Bool
elm_separator_horizontal_get(const Evas_Object *obj)
{
   ELM_SEPARATOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret;
   eo_do((Eo *) obj, elm_obj_separator_horizontal_get(&ret));
   return ret;
}

static void
_horizontal_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Separator_Smart_Data *sd = _pd;
   *ret = sd->horizontal;
}

static void
_elm_separator_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_separator_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_separator_smart_add),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_separator_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_separator_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_separator_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_separator_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_SEPARATOR_ID(ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_SET), _horizontal_set),
        EO_OP_FUNC(ELM_OBJ_SEPARATOR_ID(ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_GET), _horizontal_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_SET, "Set the horizontal mode of a separator object."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_GET, "Get the horizontal mode of a separator object."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_SEPARATOR_BASE_ID, op_desc, ELM_OBJ_SEPARATOR_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Separator_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_separator_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
