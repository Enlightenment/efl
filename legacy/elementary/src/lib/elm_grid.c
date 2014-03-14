#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_grid.h"

EAPI Eo_Op ELM_OBJ_GRID_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_GRID_CLASS
#define MY_CLASS_NAME "Elm_Grid"
#define MY_CLASS_NAME_LEGACY "elm_grid"

static void
_elm_grid_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_grid_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next =  va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = evas_object_grid_children_get(wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return;
     }

   int_ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);

   if (list_free) list_free((Eina_List *)items);

   if (ret) *ret = int_ret;
}

static void
_elm_grid_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_grid_smart_focus_direction(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

   Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = evas_object_grid_children_get(wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return;
     }

   int_ret = elm_widget_focus_list_direction_get(obj, base, items, list_data_get,
                                             degree, direction, weight);

   if (list_free) list_free((Eina_List *)items);

   if (ret) *ret = int_ret;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_mirrored_set(wd->resize_obj, rtl);
}

static void
_elm_grid_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_grid_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Object *grid;

   elm_widget_sub_object_parent_add(obj);

   grid = evas_object_grid_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, grid, EINA_TRUE);
   evas_object_grid_size_set(wd->resize_obj, 100, 100);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_FALSE);

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

static void
_elm_grid_smart_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   /* let's make our grid object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH(wd->subobjs, l, child)
     {
        if (child == wd->resize_obj)
          {
             wd->subobjs =
               eina_list_demote_list(wd->subobjs, l);
             break;
          }
     }

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_grid_add(Evas_Object *parent)
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
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

EAPI void
elm_grid_size_set(Evas_Object *obj,
                  Evas_Coord w,
                  Evas_Coord h)
{
   ELM_GRID_CHECK(obj);
   eo_do(obj, elm_obj_grid_size_set(w, h));
}

static void
_size_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_size_set(wd->resize_obj, w, h);
}

EAPI void
elm_grid_size_get(const Evas_Object *obj,
                  Evas_Coord *w,
                  Evas_Coord *h)
{
   ELM_GRID_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_grid_size_get(w, h));
}

static void
_size_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_size_get(wd->resize_obj, w, h);
}

EAPI void
elm_grid_pack(Evas_Object *obj,
              Evas_Object *subobj,
              Evas_Coord x,
              Evas_Coord y,
              Evas_Coord w,
              Evas_Coord h)
{
   ELM_GRID_CHECK(obj);
   eo_do(obj, elm_obj_grid_pack(subobj, x, y, w, h));
}

static void
_pack(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_add(obj, subobj);
   evas_object_grid_pack(wd->resize_obj, subobj, x, y, w, h);
}

EAPI void
elm_grid_unpack(Evas_Object *obj,
                Evas_Object *subobj)
{
   ELM_GRID_CHECK(obj);
   eo_do(obj, elm_obj_grid_unpack(subobj));
}

static void
_unpack(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_del(obj, subobj);
   evas_object_grid_unpack(wd->resize_obj, subobj);
}

EAPI void
elm_grid_clear(Evas_Object *obj,
               Eina_Bool clear)
{
   ELM_GRID_CHECK(obj);
   eo_do(obj, elm_obj_grid_clear(clear));
}

static void
_clear(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool clear = va_arg(*list, int);

   Eina_List *chld;
   Evas_Object *o;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!clear)
     {
        chld = evas_object_grid_children_get(wd->resize_obj);
        EINA_LIST_FREE(chld, o)
          elm_widget_sub_object_del(obj, o);
     }

   evas_object_grid_clear(wd->resize_obj, clear);
}

EAPI void
elm_grid_pack_set(Evas_Object *subobj,
                  Evas_Coord x,
                  Evas_Coord y,
                  Evas_Coord w,
                  Evas_Coord h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);

   ELM_GRID_CHECK(obj);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_pack(wd->resize_obj, subobj, x, y, w, h);
}

EAPI void
elm_grid_pack_get(Evas_Object *subobj,
                  int *x,
                  int *y,
                  int *w,
                  int *h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);

   ELM_GRID_CHECK(obj);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_grid_pack_get
     (wd->resize_obj, subobj, x, y, w, h);
}

EAPI Eina_List *
elm_grid_children_get(const Evas_Object *obj)
{
   ELM_GRID_CHECK(obj) NULL;
   Eina_List *ret = NULL;
   eo_do((Eo *) obj, elm_obj_grid_children_get(&ret));
   return ret;
}

static void
_children_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_List **ret = va_arg(*list, Eina_List **);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   *ret = evas_object_grid_children_get(wd->resize_obj);
   return;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_grid_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_grid_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_grid_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_grid_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT),  _elm_grid_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_grid_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_grid_smart_focus_direction),

        EO_OP_FUNC(ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_SIZE_SET), _size_set),
        EO_OP_FUNC(ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_SIZE_GET), _size_get),
        EO_OP_FUNC(ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_PACK), _pack),
        EO_OP_FUNC(ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_UNPACK), _unpack),
        EO_OP_FUNC(ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_CLEAR), _clear),
        EO_OP_FUNC(ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_CHILDREN_GET), _children_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_GRID_SUB_ID_SIZE_SET, "Set the virtual size of the grid."),
     EO_OP_DESCRIPTION(ELM_OBJ_GRID_SUB_ID_SIZE_GET, "Get the virtual size of the grid."),
     EO_OP_DESCRIPTION(ELM_OBJ_GRID_SUB_ID_PACK, "Pack child at given position and size."),
     EO_OP_DESCRIPTION(ELM_OBJ_GRID_SUB_ID_UNPACK, "Unpack a child from a grid object."),
     EO_OP_DESCRIPTION(ELM_OBJ_GRID_SUB_ID_CLEAR, "Faster way to remove all child objects from a grid object."),
     EO_OP_DESCRIPTION(ELM_OBJ_GRID_SUB_ID_CHILDREN_GET, "Get the list of the children for the grid."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_GRID_BASE_ID, op_desc, ELM_OBJ_GRID_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_grid_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);
