#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_list_private.h"
#include "efl_ui_list.eo.h"
#include "elm_interface_scrollable.h"
#include "elm_pan.eo.h"

#define MY_CLASS EFL_UI_LIST_CLASS
#define MY_CLASS_NAME "Efl.Ui.List"

static void _list_calc(Efl_Ui_List_Pan_Data *pd);

/* Custom Pan Class: overrides pan methods */
static const Eo_Op_Description custom_pan_op_desc[] = {
   EO_OP_CLASS_FUNC_OVERRIDE(evas_obj_smart_move, _efl_ui_list_pan_smart_move),
   EO_OP_CLASS_FUNC_OVERRIDE(evas_obj_smart_resize, _efl_ui_list_pan_smart_resize),
   EO_OP_CLASS_FUNC_OVERRIDE(evas_obj_smart_calculate, _efl_ui_list_pan_smart_calculate),
   EO_OP_CLASS_FUNC_OVERRIDE(elm_pan_pos_set, _efl_ui_list_pan_pos_set),
   EO_OP_CLASS_FUNC_OVERRIDE(elm_pan_pos_get, _efl_ui_list_pan_pos_get),
   EO_OP_CLASS_FUNC_OVERRIDE(elm_pan_pos_min_get, _efl_ui_list_pan_pos_min_get),
   EO_OP_CLASS_FUNC_OVERRIDE(elm_pan_pos_max_get, _efl_ui_list_pan_pos_max_get),
   EO_OP_CLASS_FUNC_OVERRIDE(elm_pan_content_size_get, _efl_ui_list_pan_content_size_get),
};

static const Eo_Class_Description custom_pan_class_desc = {
   EO_VERSION, "Efl.Ui.List.Pan", EO_CLASS_TYPE_REGULAR,
   EO_CLASS_DESCRIPTION_OPS(custom_pan_op_desc), NULL,
   sizeof(Efl_Ui_List_Pan_Data), NULL, NULL
};

EO_DEFINE_CLASS(_efl_ui_list_custom_pan_class_get, &custom_pan_class_desc,
                ELM_PAN_CLASS, NULL)

#define PAN_CLASS _efl_ui_list_custom_pan_class_get()


/* Custom Layout Class may needs
static const Eo_Op_Description item_laytout_op_desc[] = {
   EO_OP_CLASS_FUNC_OVERRIDE(evas_obj_smart_calculate, _efl_ui_list_item_smart_calculate),
};

static const Eo_Class_Description item_layout_class_desc = {
   EO_VERSION, "Efl.Ui.List.Item", EO_CLASS_TYPE_REGULAR,
   EO_CLASS_DESCRIPTION_OPS(item_layout_op_desc), NULL,
   sizeof(Efl_Ui_Layout_Data), NULL, NULL
};

EO_DEFINE_CLASS(_efl_ui_list_item_layout_class_get, &item_layout_class_desc,
                EFL_LAYOUT_CLASS, NULL)

#define ITEM_LAYOUT_CLASS _efl_ui_list_item_layout_class_get()
*/


static void
_list_size_calc(Efl_Ui_List_Data *pd)
{

}

static void
_list_changed(Efl_Ui_List_Data *pd)
{
   pd->changed = EINA_TRUE;
   if (pd->calc_job) ecore_job_del(pd->calc_job);
   pd->calc_job = ecore_job_add(_list_size_calc, pd);
}

static List_Item *
_list_item_new(Item_Block block, int index, Efl_Model *child)
{
   Efl_Ui_List_Data *pd = block->wd;
   List_Item *item = ELM_NEW(List_Item);
   item->index = index;
   item->block = block;
   item->my_model = child;
   block->count++;

   return item;
}

static void
_list_item_calc(List_Item *item)
{
   if (!item->layout || !item->block) return;
   Evas_Coord ow = 0, oh =0, mw = -1, mh = -1;
   Item_Block *block = item->block;
   Efl_Ui_List_Data *pd = block->wd;
   Eo *view;
   Elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   view = elm_layout_edje_get(item->layout);
   edje_object_size_min_restricted_calc(view, &mw, &mh, mw, mh);
   if ((pd->h_scroll == EINA_FALSE) && (mw > pd->w))
     mw = pd->w;

   oh = item->h;
   item->w = mw;
   item->h = mh;

   item->calculated = EINA_TRUE;

   if (block->w < item->w) block->w = item->w;
   block->h = block->h + item->h - ow;
   block->changed = EINA_TRUE;
   //_list_updated();
}

/**EOLIAN @protected??**/
static void
_list_item_unrealize(Eo *obj, Efl_Ui_List_Data *pd, List_Item *item, Eina_Bool calc)
{
   /* Caching items or */
   evas_object_del(item->layout);
   item->realized = EINA_FALSE;
}

static void
_list_item_resized_cb(void *data, Eo_Event *event EINA_UNUSED)
{
   List_Item *item = data;
   Efl_Ui_List_Data *pd;
   if (!item->layout || !item->block) return;
   pd = item->block->wd;

   _list_item_calc(item);
   if (item->calc_once)
     _list_item_unrealize(pd->obj, item, EINA_TRUE);
}

static Eo *
_list_item_layout_create(Eo *obj, List_Item *item, Eina_Stringshare *style)
{
   //Eo *layout = eo_add(ELM_LAYOUT_CLASS, obj, elm_object_style_set(style););
   Eo *layout = eo_add(ELM_LAYOUT_CLASS, obj);
   elm_object_style_set(style);
   efl_gfx_position_set(layout, -9999, -9999);
   return layout;
}

/**EOLIAN @protected??**/
static void
_list_item_realize(Eo *obj, Efl_Ui_List_Data *pd, List_Item *item, int index, Efl_Model *child_model, Eina_Bool calc)
{
   if (it->realized || !child_model) return;

   Eina_Stringshare *style = efl_ui_list_item_style_get(obj, child_model, index);
   item->layout = _layout_create(obj, item, style);
   //
   // state update - select, focus, disable
   // mouse_event add

   pd->event->layout = layout;
   pd->event->child = child_model;
   pd->event->index = index;
   eo_event_callback_call(obj, EFL_UI_LIST_EVENT_REALIZED, pd->event);
   eo_event_callback_add(item->layout, EVAS_OBJECT_EVENT_RESIZE, _list_item_resized_cb, item);
}

static void
_list_item_free(List_Item *item)
{
   Efl_Ui_List_Data *pd = item->block->wd;
   if (item->realized) _list_item_unrealize(pd->obj, pd, item, EINA_FALSE);

   if (item->queued)
     {
        pd->queue = eina_list_remove(pd->queue, item);
        item->queued = EINA_FALSE;
     }
   else if (item->prequeued)
     {
        pd->prequeue = eina_list_remove(pd->prequeue, item);
        item->prequeued = EINA_FALSE;
     }
   if (!pd->queue && !pd->prequeue)
     {
        ecore_idle_enterer_del(pd->queue_idle_enterer);
        pd->queue_idle_enterer = NULL;
     }

   item->block->items = eina_list_remove(item->block->items, item);
   /*
    * free all item index and states needs;
    */
   free(item);

}

static Item_Block
_list_block_new(Eo* obj, Efl_Ui_List_Data *pd)
{
   Item_Block *block = ELM_NEW(Item_Block);
   block->wd = pd;

   return block;
}

static void
_list_block_del(Item_Block *block)
{
   Efl_Ui_List_Data *pd = block->wd;
   List_Item *item;
   EINA_LIST_FREE(block->items, item)
     {
        _list_item_free(item);
     }
   pd->item_blocks = eina_list_remove(pd->item_blocks, block);
   free(block);
}

static void
_list_clear(Eo* obj, Efl_Ui_List_Data *pd)
{
   Eina_List *l;
   Item_Block *block;
   EINA_LIST_FOREACH(pd->block, l, block)
     {
        _list_block_del(pd, block);
     }
}

static Eina_Bool
_item_process(Efl_Ui_List_Data *pd, List_Item *item, unsigned index)
{
   Evas_Coord ow, oh;
   if (!item->my_model) return EINA_FALSE;
   if (pd->homogeneous)
     {
        Item_Size *size;
        if (pd->size_caches)
          {
             Eina_Stringshare *style = efl_ui_list_item_style_get(pd->obj, item->my_model, index);
             size = eina_hash_find(sd->size_caches, style);
             if (size)
               {
                  oh = item->h;
                  item->w = size->w;
                  item->h = size->h;
                  item->calculated = EINA_TRUE;
                  item->queued = EINA_FALSE;
                  item->block->h = item->block->h + item->h - oh;
                  item->block->changed;
                  //_list_updated(pd);
                  return EINA_TRUE;
               }
          }
     }
   item->calc_once = EINA_TRUE;
   item->queued = EINA_FALSE;
   _list_item_realize(pd->obj, item, item->my_model, index, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_queue_idle_enterer(void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(data);
   Efl_Ui_List_Data *pd = data;
   List_Item *item;
   Eina_Bool wakeup;
   double t0, t1;
   int n;

   t0 = ecore_time_get();
   for (n = 0; ((pd->queue) || (pd->prequeue)) && (n < (pd->block_size * 2)); n++)
     {
        if (pd->queue)
          {
             item = eina_list_data_get(pd->queue);
             if (!_item_process(pd, item)) break;
             pd->queue = eina_list_remove_list(pd->queue, sd->queue);
          }
        t1 = ecore_time_get();
        if ((t - t0) > (ecore_animator_frametime_get())) break;

        if (pd->prequeue)
          {
             item = eina_list_data_get(pd->prequeue);
             if (!_item_process(pd, item)) break;
             pd->prequeue = eina_list_remove_list(pd->prequeue, sd->queue);
          }
        t1 = ecore_time_get();
        /* same as eina_inlist_count > 1 */
        if ((t - t0) > (ecore_animator_frametime_get())) break;
     }

   if (wakeup)
     {
        // wake up mainloop
        if (pd->calc_promise) efl_job_cancel(pd->calc_promise, pd->calc_promise);
        pd->calc_promise = efl_loop_job(ds->calc_promise, pd);
        eina_promise_then(pd->calc_promise, _list_calc, NULL, pd);
     }

   return ECORE_CALLBACK_RENEW;
   return ECORE_CALLBACK_CANCEL;
}


static void
_list_children_promise_then(Efl_Ui_List_Data *pd, Eina_Accessor **accessor)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);
   EINA_SAFETY_ON_NULL_RETURN(pd->model);
   Item_Block *block;
   int index = 0;
   Eo *child;

   block = _list_block_new(pd);
   pd->item_blocks = eina_list_append(pd->item_blocks, block);

   EINA_ACCESSOR_FOREACH(*accessor, i, child)
     {
        if (block->count == pd->block_size)
          {
             block = _list_block_new(pd);
             pd->item_blocks = eina_list_append(pd->item_blocks, block);
          }
        item = _list_item_new(block, block->count, child);
        b->items = eina_list_append(block->items, item);

        if (pd->show_item =< i)
          {
             pd->queue = eina_list_append(pd->queue, item);
             item->queued = EINA_TRUE;
          }
        else
          {
             pd->prequeue = eina_list_prepend(pd->prequeue, item);
             item->prequeued = EINA_TRUE;
          }
     }

   ecore_idle_enterer_del(pd->queue_idle_enterer);
   pd->queue_idle_enterer = ecore_idle_enterer_add(_queue_idle_enterer, pd);
}

static void
_list_count_promise_then(Efl_Ui_List_Data *pd, int count)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);
   EINA_SAFETY_ON_NULL_RETURN(pd->model);
   if (pd->block_size =< 0) return;
   pd->count = count;

   efl_model_children_slice_get(pd->model, 0, 0, &pd->child_promise);
   eina_promise_then(pd->child_promise, &_list_children_promise_then, NULL, pd);
   while (count < index)
     {
        int size = ((pd->block_size > (count - index))?
                    (count - index) : pd->block_size);
        itb = _list_block_new(obj, pd, size);
        pd->item_blocks = eina_list_append(pd->item_blocks, itb);
        index += size;
     }

   pd->count_promise = NULL;
}

static void
_list_model_unset(Eo* obj, Efl_Ui_List_Data *pd)
{
   if (!pd->model) return;

   _list_clear(obj, pd);

   if (pd->count_promise)
     {
        eina_promise_cancel(pd->count_promise);
        pd->count_promise = NULL;
     }

   pd->model = NULL;
}

static void
_list_calc(Efl_Ui_List_Data *pd)
{

   evas_object_smart_changed();
}

EOLIAN void
_efl_ui_list_homogeneous_set(Eo *obj, Efl_Ui_List_Data *pd, Eina_Bool homogeneous)
{
   pd->homogeneous = homogeneous;
}

EOLIAN Eina_Bool
_efl_ui_list_homogeneous_get(Eo *obj, Efl_Ui_List_Data *pd)
{
   return pd->homogeneous;
}

EOLIAN void
_efl_ui_list_item_style_set(Eo *obj, Efl_Ui_List_Data *pd, const char* style)
{
   pd->style = eina_stringshare_add(style);
}

EOLIAN Eina_Stringshare *
_efl_ui_list_item_style_get(Eo *obj, Efl_Ui_List_Data *pd, Efl_Model *model, int index)
{
   if (pd->style)
     return pd->style;
   else
     return NULL;
}

EOLIAN void
_efl_ui_list_scroll_set(Eo *obj, Efl_Ui_List_Data *pd, Eina_Bool h_scroll, Eina_Bool v_scroll)
{
   pd->h_scroll = h_scroll;
   pd->v_scroll = v_scroll;
}

EOLIAN void
_efl_ui_list_eo_base_destructor(Eo *obj, Efl_Ui_List_Data *pd)
{
   eo_destructor(eo_super(obj, MY_CLASS));
}
EOLIAN static void
_efl_ui_list_pan_pos_set(Eo *obj, Efl_Ui_List_Pan_Data *pd, Evas_Coord x, Evas_Coord y)
{
   Efl_Ui_List_Data *wd = pd->wd;

   if ((x == wd->pan_x) && (y == wd->pan_y)) return;
   wd->pan_x = x;
   wd->pan_y = y;
}

EOLIAN static void
_efl_ui_list_pan_pos_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *pd, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = pd->wd->pan_x;
   if (y) *y = pd->wd->pan_y;
}

EOLIAN static void
_efl_ui_list_pan_pos_max_get(Eo *obj, Efl_Ui_List_Pan_Data *pd, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = pd->wd->min_w - ow;
   if (ow < 0) ow = 0;
   oh = pd->wd->min_h - oh;
   if (oh < 0) oh = 0;

   if (x) *x = ow;
   if (y) *y = oh;
}

EOLIAN static void
_efl_ui_list_pan_pos_min_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *_pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

EOLIAN static void
_efl_ui_list_pan_content_size_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *pd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = pd->wd->min_w;
   if (h) *h = pd->wd->min_h;
}

EOLIAN static void
_efl_ui_list_pan_smart_move(Eo *obj, Efl_Ui_List_Pan_Data *pd, Evas_Coord _gen_param2 EINA_UNUSED, Evas_Coord _gen_param3 EINA_UNUSED)
{
   pd->wd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(obj);
   //ELM_SAFE_FREE(psd->wsd->calc_job, ecore_job_del);
}

static void
_list_pan_smart_resize_job(void *data)
{
   EFL_UI_LIST_PAN_DATA_GET(data, pd);

   elm_layout_sizing_eval(pd->wobj);
   psd->resize_job = NULL;
}

EOLIAN static void
_efl_ui_list_pan_smart_resize(Eo *obj, Efl_Ui_List_Pan_Data *pd, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord ow, oh;

   Efl_Ui_List_Data *wd = pd->wd;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if ((wd->mode == EFL_UI_LIST_COMPRESS) && (ow != w))
     {
        /* fix me later */
        ecore_job_del(pd->resize_job);
        pd->resize_job =
          ecore_job_add(_list_pan_smart_resize_job, obj);
     }
   wd->pan_changed = EINA_TRUE;
   evas_object_smart_changed(obj);
   //ecore_job_del(sd->calc_job);
   // if the width changed we may have to resize content if scrollbar went
   // away or appeared to queue a job to deal with it. it should settle in
   // the end to a steady-state
   //if (ow != w)
   //  wd->calc_job = ecore_job_add(_calc_job, pd->wobj);
   //else
   //  wd->calc_job = NULL;
}

// ****
EOLIAN static void
_efl_ui_list_pan_smart_calculate(Eo *obj, Efl_Ui_List_Pan_Data *pd)
{
   Evas_Coord ox, oy, ow, oh, cvx, cvy, cvw, cvh;
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0;
   Eina_List *l;

   Efl_Ui_List_Data *wd = pd->wd;

   evas_event_freeze(evas_object_evas_get(obj));
     {
        evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
        evas_output_viewport_get(evas_object_evas_get(obj), &cvx, &cvy, &cvw, &cvh);

        // Item Realize on Pan Position
        //
        //
        //
        //

        elm_interface_scrollable_content_pos_get(wd->obj, &vx, &vy);
        elm_interface_scrollable_content_viewport_geometry_get
           (wd->obj, NULL, NULL, &vw, &vh);

     }
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   eo_event_callback_call
         (pd->wobj, ELM_INTERFACE_SCROLLABLE_EVENT_CHANGED, NULL);
}
// ****

static void
_evas_viewport_resize_cb(void *d, Evas *e EINA_UNUSED, void *ei EINA_UNUSED)
{
   Efl_Ui_List_Data *pd = d;
   evas_object_smart_changed(pd->pan_obj);
}

EOLIAN void
_efl_ui_list_evas_object_smart_add(Eo *obj, Efl_Ui_List_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_List_Pan_Data *pan_data;
   Evas_Coord minw, minh;
   int i;

   evas_obj_smart_add(eo_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->obj = obj;
   pd->size_caches = eina_hash_pointer_new(_size_cache_free);
   pd->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(pd->hit_rect, obj);
   elm_widget_sub_object_add(obj, pd->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(pd->hit_rect, 0, 0, 0, 0);
   evas_object_show(pd->hit_rect);
   evas_object_repeat_events_set(pd->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_on_show_region_hook_set(obj, _show_region_hook, NULL);

   // efl_ui_pack_layout
   if (!elm_layout_theme_set
       (obj, "list", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_interface_scrollable_objects_set(obj, wd->resize_obj, pd->hit_rect);

   pd->v_scroll = EINA_TRUE;
   pd->h_scroll = EINA_FALSE;
   pd->block_size = MAX_ITEMS_PER_BLOCK;

   // Creating custom pan object
   pd->pan = eo_add(PAN_CLASS, evas_object_evas_get(obj));
   pan_data = eo_data_scope_get(pd->pan_obj, PAN_CLASS);
   eo_data_ref(obj, NULL);
   pan_data->wobj = obj;
   pan_data->wsd = pd;

   elm_interface_scrollable_extern_pan_set(obj, pd->pan);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   elm_layout_sizing_eval(obj);

   evas_event_callback_add(evas_object_evas_get(obj),
                           EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE,
                           _evas_viewport_resize_cb, pd);
}

EOLIAN
void _efl_ui_list_evas_object_smart_del(Eo *obj, Efl_Ui_List_Data *pd)
{
   _list_clear(obj);
   evas_event_callback_del_full(evas_object_evas_get(obj),
                                EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE,
                                _evas_viewport_resize_cb, pd);
   ELM_SAFE_FREE(pd->pan, evas_object_del);
   ecore_idle_enterer_del(pd->queue_idle_enterer);

   evas_obj_smart_del(eo_super(obj, MY_CLASS));
}

EOLIAN
void _efl_ui_list_evas_object_smart_member_add(Eo *obj, Efl_Ui_List_Data *pd, Evas_Object *member)
{
   evas_obj_smart_member_add(eo_super(obj, MY_CLASS), member);
   if (pd->hit_rect)
     evas_object_raise(pd->hit_rect);
}


EOLIAN
void _efl_ui_list_evas_object_smart_move(Eo *obj, Efl_Ui_List_Data *pd, Evas_Coord x, Evas_Coord y)
{
   evas_obj_smart_move(eo_super(obj, MY_CLASS), x, y);
   evas_object_move(pd->hit_rect, x, y);
}

EOLIAN
void _efl_ui_list_evas_object_smart_resize(Eo *obj, Efl_Ui_List_Data *pd, Evas_Coord w, Evas_Coord h)
{
   evas_obj_smart_resize(eo_super(obj, MY_CLASS), w, h);
   evas_object_resize(pd->hit_rect, w, h);
   /*
   if (((pd->queue) || (pd->prequeue)) &&
       (!pd->queue_idle_enterer) && (w > 0))
     _requeue_idle_enterer(pd);
   */
}

/*
 * No theme apply exist in Efl.Ui.Object
EOLIAN
Eina_Bool _efl_ui_list_elm_widget_theme_apply(Eo *obj, Efl_Ui_List_Data *pd)
{
   Eina_Bool int_ret = EINA_FALSE;
   Eina_List *l;
   Evas *e = evas_object_evas_get(obj);
   Efl_Ui_List_Item *it;

   int_ret = elm_obj_widget_theme_apply(eo_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   evas_event_freeze(e);
   _item_cache_zero(pd);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   eina_hash_free_buckets(pd->size_caches);
   pd->min_w = pd->min_h = 0;

   elm_layout_sizing_eval(obj);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));

   return EINA_TRUE;
}
*/


EOLIAN
Eina_Bool _efl_ui_list_elm_widget_sub_object_del(Eo *obj, Efl_Ui_List_Data *pd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   /* XXX: hack -- also skipping sizing recalculation on
    * sub-object-del. list's crazy code paths (like groups and
    * such) seem to issue a whole lot of deletions and Evas bitches
    * about too many recalculations */
   pd->on_sub_del = EINA_TRUE;
   int_ret = elm_obj_widget_sub_object_del(eo_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   pd->on_sub_del = EINA_FALSE;
   return EINA_TRUE;
}

EOLIAN
void _efl_ui_list_efl_ui_view_model_set(Eo *obj, Efl_Ui_List_Data *pd, Efl_Model *model)
{
   if (pd->model)
     _list_model_unset(obj, pd);
   pd->model = model;

   efl_model_children_count_get(model, &pd->count_promise);
   eina_promise_then(
      pd->count_promise, (Eina_Promise_Cb)&_list_count_promise_then, NULL, pd);
}

EOLIAN
Efl_Model * _efl_ui_list_efl_ui_view_model_get(Eo *obj, Efl_Ui_List_Data *pd)
{
   return pd->model;
}
