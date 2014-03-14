#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_table.h"

EAPI Eo_Op ELM_OBJ_TABLE_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_TABLE_CLASS

#define MY_CLASS_NAME "Elm_Table"
#define MY_CLASS_NAME_LEGACY "elm_table"

static void
_elm_table_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_table_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

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
        items = evas_object_table_children_get
            (wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return;
     }

   int_ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);

   if (list_free) list_free((Eina_List *)items);

   if (ret) *ret = int_ret;
}

static void
_elm_table_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_table_smart_focus_direction(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   const Eina_List *items;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);

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
        items = evas_object_table_children_get
            (wd->resize_obj);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return;
     }

   int_ret = elm_widget_focus_list_direction_get
       (obj, base, items, list_data_get, degree, direction, weight);

   if (list_free)
     list_free((Eina_List *)items);

   if (ret) *ret = int_ret;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_mirrored_set(wd->resize_obj, rtl);
}

static void
_elm_table_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Eina_Bool super_ret;
   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&super_ret));
   if (super_ret == EINA_FALSE)
      return;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   if (ret) *ret = EINA_TRUE;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_size_hint_min_get
     (wd->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get
     (wd->resize_obj, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_on_size_hints_changed(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

static void
_elm_table_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(child, &int_ret));
   if (!int_ret) return;

   _sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_table_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object *table;

   elm_widget_sub_object_parent_add(obj);

   table = evas_object_table_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, table, EINA_TRUE);

   evas_object_event_callback_add
     (table, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_FALSE);

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

static void
_elm_table_smart_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Eina_List *l;
   Evas_Object *child;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_event_callback_del_full
     (wd->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_size_hints_changed, obj);

   /* let's make our table object the *last* to be processed, since it
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
elm_table_add(Evas_Object *parent)
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
elm_table_homogeneous_set(Evas_Object *obj,
                          Eina_Bool homogeneous)
{
   ELM_TABLE_CHECK(obj);
   eo_do(obj, elm_obj_table_homogeneous_set(homogeneous));
}

static void
_homogeneous_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool homogeneous = va_arg(*list, int);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_homogeneous_set
     (wd->resize_obj, homogeneous);
}

EAPI Eina_Bool
elm_table_homogeneous_get(const Evas_Object *obj)
{
   ELM_TABLE_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_table_homogeneous_get(&ret));
   return ret;
}

static void
_homogeneous_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   *ret = evas_object_table_homogeneous_get(wd->resize_obj);
}

EAPI void
elm_table_padding_set(Evas_Object *obj,
                      Evas_Coord horizontal,
                      Evas_Coord vertical)
{
   ELM_TABLE_CHECK(obj);
   eo_do(obj, elm_obj_table_padding_set(horizontal, vertical));
}

static void
_padding_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord horizontal = va_arg(*list, Evas_Coord);
   Evas_Coord vertical = va_arg(*list, Evas_Coord);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_padding_set
     (wd->resize_obj, horizontal, vertical);
}

EAPI void
elm_table_padding_get(const Evas_Object *obj,
                      Evas_Coord *horizontal,
                      Evas_Coord *vertical)
{
   ELM_TABLE_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_table_padding_get(horizontal, vertical));
}

static void
_padding_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord *horizontal = va_arg(*list, Evas_Coord *);
   Evas_Coord *vertical = va_arg(*list, Evas_Coord *);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_padding_get
     (wd->resize_obj, horizontal, vertical);
}

EAPI void
elm_table_pack(Evas_Object *obj,
               Evas_Object *subobj,
               int col,
               int row,
               int colspan,
               int rowspan)
{
   ELM_TABLE_CHECK(obj);
   eo_do(obj, elm_obj_table_pack(subobj, col, row, colspan, rowspan));
}

static void
_pack(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);
   int col = va_arg(*list, int);
   int row = va_arg(*list, int);
   int colspan = va_arg(*list, int);
   int rowspan = va_arg(*list, int);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (col < 0)
     {
        ERR("col < 0");
        return;
     }
   if (colspan < 1)
     {
        ERR("colspan < 1");
        return;
     }
   if ((0xffff - col) < colspan)
     {
        ERR("col + colspan > 0xffff");
        return;
     }
   if ((col + colspan) >= 0x7ffff)
     {
        WRN("col + colspan getting rather large (>32767)");
     }
   if (row < 0)
     {
        ERR("row < 0");
        return;
     }
   if (rowspan < 1)
     {
        ERR("rowspan < 1");
        return;
     }
   if ((0xffff - row) < rowspan)
     {
        ERR("row + rowspan > 0xffff");
        return;
     }
   if ((row + rowspan) >= 0x7ffff)
     {
        WRN("row + rowspan getting rather large (>32767)");
     }

   elm_widget_sub_object_add(obj, subobj);
   evas_object_table_pack(wd->resize_obj, subobj, col, row, colspan, rowspan);
}

EAPI void
elm_table_unpack(Evas_Object *obj,
                 Evas_Object *subobj)
{
   ELM_TABLE_CHECK(obj);
   eo_do(obj, elm_obj_table_unpack(subobj));
}

static void
_unpack(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_widget_sub_object_del(obj, subobj);
   evas_object_table_unpack(wd->resize_obj, subobj);
}

EAPI void
elm_table_pack_set(Evas_Object *subobj,
                   int col,
                   int row,
                   int colspan,
                   int rowspan)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);

   ELM_TABLE_CHECK(obj);
   eo_do(obj, elm_obj_table_pack_set(subobj, col, row, colspan, rowspan));
}

static void
_pack_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);
   int col = va_arg(*list, int);
   int row = va_arg(*list, int);
   int colspan = va_arg(*list, int);
   int rowspan = va_arg(*list, int);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_pack(wd->resize_obj, subobj, col, row, colspan, rowspan);
}

EAPI void
elm_table_pack_get(Evas_Object *subobj,
                   int *col,
                   int *row,
                   int *colspan,
                   int *rowspan)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);
   ELM_TABLE_CHECK(obj);
   eo_do(obj, elm_obj_table_pack_get(subobj, col, row, colspan, rowspan));
}

static void
_pack_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);
   int *col = va_arg(*list, int *);
   int *row = va_arg(*list, int *);
   int *colspan = va_arg(*list, int *);
   int *rowspan = va_arg(*list, int *);

   unsigned short icol, irow, icolspan, irowspan;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_pack_get
     (wd->resize_obj, subobj, &icol, &irow, &icolspan, &irowspan);
   if (col) *col = icol;
   if (row) *row = irow;
   if (colspan) *colspan = icolspan;
   if (rowspan) *rowspan = irowspan;
}

EAPI void
elm_table_clear(Evas_Object *obj,
                Eina_Bool clear)
{
   ELM_TABLE_CHECK(obj);
   eo_do(obj, elm_obj_table_clear(clear));
}

static void
_clear(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool clear = va_arg(*list, int);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_table_clear(wd->resize_obj, clear);
}

EAPI Evas_Object *
elm_table_child_get(const Evas_Object *obj, int col, int row)
{
   Evas_Object *ret;
   ELM_TABLE_CHECK(obj) NULL;

   eo_do((Eo *)obj, elm_obj_table_child_get(col, row, &ret));
   return ret;
}

static void
_child_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int col = va_arg(*list, int);
   int row = va_arg(*list, int);
   Evas_Object **ret = va_arg(*list, Evas_Object **);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (ret)
     *ret = evas_object_table_child_get(wd->resize_obj, col, row);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_table_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_table_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_table_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_table_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_table_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_table_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_table_smart_focus_direction),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_table_sub_object_del),

        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET), _homogeneous_set),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET), _homogeneous_get),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PADDING_SET), _padding_set),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PADDING_GET), _padding_get),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK), _pack),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_UNPACK), _unpack),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK_SET), _pack_set),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_PACK_GET), _pack_get),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_CLEAR), _clear),
        EO_OP_FUNC(ELM_OBJ_TABLE_ID(ELM_OBJ_TABLE_SUB_ID_CHILD_GET), _child_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET, "Set the homogeneous layout in the table."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET, "Get the current table homogeneous mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_PADDING_SET, "Set padding between cells."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_PADDING_GET, "Get padding between cells."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_PACK, "Add a subobject on the table with the coordinates passed."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_UNPACK, "Remove child from table."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_PACK_SET, "Set the packing location of an existing child of the table."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_PACK_GET, "Get the packing location of an existing child of the table."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_CLEAR, "Faster way to remove all child objects from a table object."),
     EO_OP_DESCRIPTION(ELM_OBJ_TABLE_SUB_ID_CHILD_GET, "Get child object of table at given coordinates."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_TABLE_BASE_ID, op_desc, ELM_OBJ_TABLE_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_table_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);
