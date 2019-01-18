#include "efl_ui_box_private.h"

#define MY_CLASS EFL_UI_BOX_CLASS
#define MY_CLASS_NAME "Efl.Ui.Box"

/* COPIED FROM ELM_BOX
 * - removed transition stuff (TODO: add back - needs clean API first)
 */

static const char SIG_CHILD_ADDED[] = "child,added";
static const char SIG_CHILD_REMOVED[] = "child,removed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

static void
_child_added_cb_proxy(void *data, const Efl_Event *event)
{
   Evas_Object *box = data;
   Evas_Object_Box_Option *opt = event->info;

   efl_event_callback_legacy_call(box, EFL_CONTAINER_EVENT_CONTENT_ADDED, opt->obj);
}

static void
_child_removed_cb_proxy(void *data, const Efl_Event *event)
{
   Evas_Object *box = data;
   Evas_Object *child = event->info;

   efl_event_callback_legacy_call(box, EFL_CONTAINER_EVENT_CONTENT_REMOVED, child);
}

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Box_Data *sd)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!efl_alive_get(obj)) return;
   if (sd->delete_me)
     return;

   evas_object_size_hint_combined_min_get(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_max_get(wd->resize_obj, &maxw, &maxh);
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
_on_size_hints_changed(void *data, Evas *e EINA_UNUSED,
                       Evas_Object *resizeobj, void *event_info EINA_UNUSED)
{
   Efl_Ui_Box *obj = data;
   Efl_Ui_Box_Data *pd = efl_data_scope_get(obj, EFL_UI_BOX_CLASS);

   if (obj == resizeobj)
     efl_pack_layout_request(obj);
   else
      _sizing_eval(data, pd);
}

static void
_evas_box_custom_layout(Evas_Object *evas_box EINA_UNUSED,
                        Evas_Object_Box_Data *bd EINA_UNUSED, void *data)
{
   Efl_Ui_Box *obj = data;

   efl_pack_layout_update(obj);
}

EOLIAN static void
_efl_ui_box_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Object_Box_Data *bd;

   bd = evas_object_smart_data_get(wd->resize_obj);
   _efl_ui_box_custom_layout(obj, bd);
   efl_event_callback_legacy_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

EOLIAN static void
_efl_ui_box_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Box_Data *pd)
{
   if (pd->recalc) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_smart_need_recalculate_set(wd->resize_obj, EINA_TRUE);
   pd->recalc = EINA_TRUE;
   evas_object_smart_calculate(wd->resize_obj);
   pd->recalc = EINA_FALSE;
}

EOLIAN static void
_efl_ui_box_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED)
{
   Evas *e = evas_object_evas_get(obj);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   elm_widget_resize_object_set(obj, evas_object_box_add(e));
   evas_object_box_layout_set(wd->resize_obj, _evas_box_custom_layout, obj, NULL);

   evas_object_event_callback_add(wd->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, obj);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   efl_event_callback_add(wd->resize_obj, EVAS_BOX_EVENT_CHILD_ADDED, _child_added_cb_proxy, obj);
   efl_event_callback_add(wd->resize_obj, EVAS_BOX_EVENT_CHILD_REMOVED, _child_removed_cb_proxy, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

EOLIAN static void
_efl_ui_box_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Box_Data *sd)
{
   Eina_List *l;
   Evas_Object *child;

   sd->delete_me = EINA_TRUE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_event_callback_del_full
         (wd->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
          _on_size_hints_changed, obj);

   /* let's make our box object the *last* to be processed, since it
    * may (smart) parent other sub objects here */
   EINA_LIST_FOREACH (wd->subobjs, l, child)
     {
        if (child == wd->resize_obj)
          {
             wd->subobjs =
                   eina_list_demote_list(wd->subobjs, l);
             break;
          }
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_box_efl_object_constructor(Eo *obj, Efl_Ui_Box_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_SKIPPED);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   pd->align.h = 0.5;
   pd->align.v = 0.5;

   return obj;
}

/* CLEAN API BELOW */

EOLIAN static int
_efl_ui_box_efl_container_content_count(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED)
{
   Evas_Object_Box_Data *bd;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);

   bd = evas_object_smart_data_get(wd->resize_obj);
   return bd ? eina_list_count(bd->children) : 0;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_pack_clear(Eo *obj, Efl_Ui_Box_Data *pd)
{
   Eina_Bool ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ret = evas_object_box_remove_all(wd->resize_obj, EINA_TRUE);
   _sizing_eval(obj, pd);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_unpack_all(Eo *obj, Efl_Ui_Box_Data *pd)
{
   Evas_Object_Box_Data *bd;
   Evas_Object_Box_Option *opt;
   Eina_List *l;
   Eina_Bool ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   /* set this to block _sizing_eval() calls */
   pd->delete_me = EINA_TRUE;
   bd = evas_object_smart_data_get(wd->resize_obj);
   EINA_LIST_FOREACH(bd->children, l, opt)
     _elm_widget_sub_object_redirect_to_top(obj, opt->obj);
   pd->delete_me = EINA_FALSE;

   ret = evas_object_box_remove_all(wd->resize_obj, EINA_FALSE);
   _sizing_eval(obj, pd);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_unpack(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eina_Bool ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (evas_object_box_remove(wd->resize_obj, subobj))
     {
        ret = _elm_widget_sub_object_redirect_to_top(obj, subobj);
        _sizing_eval(obj, pd);
     }

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_pack(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_end(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   Eina_Bool ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ret  = elm_widget_sub_object_add(obj, subobj);
   ret &= (evas_object_box_append(wd->resize_obj, subobj) != NULL);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   Eina_Bool ret;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ret  = elm_widget_sub_object_add(obj, subobj);
   ret &= (evas_object_box_prepend(wd->resize_obj, subobj) != NULL);

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!elm_widget_sub_object_add(obj, subobj))
     return EINA_FALSE;

   if (!evas_object_box_insert_before(wd->resize_obj, subobj, existing))
     {
        elm_widget_sub_object_del(obj, subobj);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (!elm_widget_sub_object_add(obj, subobj))
     return EINA_FALSE;

   if (!evas_object_box_insert_after(wd->resize_obj, subobj, existing))
     {
        elm_widget_sub_object_del(obj, subobj);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED,
                                        Efl_Gfx_Entity *subobj, int index)
{
   if (!index)
     return efl_pack_begin(obj, subobj);
   else if (index == -1)
     return efl_pack_end(obj, subobj);
   else
     {
        Evas_Object_Box_Data *bd;
        int cnt;

        ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

        bd = evas_object_smart_data_get(wd->resize_obj);
        cnt = eina_list_count(bd ? bd->children : NULL);
        if (!cnt)
          index = 0;
        else
          {
             index %= cnt;
             if (index < 0) index += cnt;
          }
        return (evas_object_box_insert_at(wd->resize_obj, subobj, index) != NULL);
     }
}

static inline Efl_Gfx_Entity *
_box_item(Evas_Object_Box_Option *opt)
{
   return opt ? opt->obj : NULL;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_box_efl_pack_linear_pack_content_get(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED,
                                           int index)
{
   Evas_Object_Box_Data *bd;
   int cnt;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   bd = evas_object_smart_data_get(wd->resize_obj);
   if (!bd || !bd->children) return NULL;

   if (!index)
     return _box_item(eina_list_data_get(bd->children));
   else if (index == -1)
     return _box_item(eina_list_last_data_get(bd->children));

   cnt = eina_list_count(bd->children);
   if (!cnt) return NULL;

   if (index >= (cnt - 1))
     return _box_item(eina_list_last_data_get(bd->children));
   else if (index <= (-cnt))
     return _box_item(eina_list_data_get(bd->children));

   // this should loop only once
   while (index < 0)
     index += cnt;

   return _box_item(eina_list_nth(bd->children, index));
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_box_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED,
                                              int index)
{
   Efl_Gfx_Entity *content;

   content = efl_pack_content_get(obj, index);
   if (!content) return NULL;

   if (!efl_pack_unpack(obj, content))
     return NULL;

   return content;
}

EOLIAN static int
_efl_ui_box_efl_pack_linear_pack_index_get(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED,
                                           const Efl_Gfx_Entity *subobj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, -1);
   Evas_Object_Box_Data *bd;
   Evas_Object_Box_Option *opt;
   Eina_List *l;
   int k = 0;

   if (evas_object_smart_parent_get(subobj) != wd->resize_obj)
     goto end;

   bd = evas_object_smart_data_get(wd->resize_obj);
   EINA_LIST_FOREACH(bd->children, l, opt)
     {
        if (opt->obj == subobj)
          return k;
        k++;
     }

end:
   ERR("object %p (%s) is not a child of %p (%s)",
       subobj, efl_class_name_get(subobj), obj, efl_class_name_get(obj));
   return -1;
}

EOLIAN static void
_efl_ui_box_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED)
{
   evas_object_smart_need_recalculate_set(obj, EINA_TRUE);
}

static Eina_Bool
_box_item_iterator_next(Box_Item_Iterator *it, void **data)
{
   Efl_Gfx_Entity *sub;

   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_box_item_iterator_get_container(Box_Item_Iterator *it)
{
   return it->object;
}

static void
_box_item_iterator_free(Box_Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eina_list_free(it->list);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_ui_box_efl_container_content_iterate(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED)
{
   Box_Item_Iterator *it;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = evas_object_box_children_get(wd->resize_obj);
   it->real_iterator = eina_list_iterator_new(it->list);
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_box_item_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_box_item_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_box_item_iterator_free);
   it->object = obj;

   return &it->iterator;
}

EOLIAN static void
_efl_ui_box_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Ui_Dir dir)
{
   switch (dir)
     {
      case EFL_UI_DIR_RTL:
        // FIXME: Should be inverted!
      case EFL_UI_DIR_HORIZONTAL:
      case EFL_UI_DIR_LTR:
        pd->dir = EFL_UI_DIR_HORIZONTAL;
        break;

      case EFL_UI_DIR_UP:
        // FIXME: Should be inverted!
      case EFL_UI_DIR_DOWN:
      case EFL_UI_DIR_VERTICAL:
      case EFL_UI_DIR_DEFAULT:
      default:
        pd->dir = EFL_UI_DIR_VERTICAL;
        break;
     }

   efl_pack_layout_request(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_box_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_box_efl_pack_pack_padding_set(Eo *obj, Efl_Ui_Box_Data *pd, double h, double v, Eina_Bool scalable)
{

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (h < 0) h = 0;
   if (v < 0) v = 0;
   pd->pad.h = h;
   pd->pad.v = v;
   pd->pad.scalable = !!scalable;
   if (pd->pad.scalable)
     {
        double scale = elm_object_scale_get(obj);
        evas_object_box_padding_set(wd->resize_obj, h * scale, v * scale);
     }
   else
      evas_object_box_padding_set(wd->resize_obj, h, v);
}

EOLIAN static void
_efl_ui_box_efl_pack_pack_padding_get(const Eo *obj, Efl_Ui_Box_Data *pd, double *h, double *v, Eina_Bool *scalable)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (scalable) *scalable = pd->pad.scalable;
   if (h) *h = pd->pad.h;
   if (v) *v = pd->pad.v;
}

EOLIAN static void
_efl_ui_box_efl_pack_pack_align_set(Eo *obj, Efl_Ui_Box_Data *pd, double h, double v)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (h < 0) h = -1;
   if (v < 0) v = -1;
   if (h > 1) h = 1;
   if (v > 1) v = 1;
   pd->align.h = h;
   pd->align.v = v;

   evas_object_box_align_set(wd->resize_obj, h, v);
   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_box_efl_pack_pack_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd, double *h, double *v)
{
   if (h) *h = pd->align.h;
   if (v) *v = pd->align.v;
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_BOX_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_box)

#include "efl_ui_box.eo.c"
