#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include <assert.h>

#include "elm_priv.h"
#include "efl_ui_list_view_relayout.eo.h"
#include "efl_ui_list_view_seg_array.h"

#define MY_CLASS EFL_UI_LIST_VIEW_PRECISE_LAYOUTER_CLASS

typedef struct _Efl_Ui_List_View_Precise_Layouter_Data
{
   Efl_Model* model;
   Efl_Ui_List_View_Model *modeler;
   Ecore_Job *calc_job;
   Efl_Ui_List_View_Seg_Array *seg_array;

   Eina_Size2D min;

   unsigned int calc_progress;

   int count_total;

   Eina_Bool initialized : 1;
   Eina_Bool recalc : 1;
   Eina_Bool resize : 1;
} Efl_Ui_List_View_Precise_Layouter_Data;

typedef struct _Efl_Ui_List_View_Precise_Layouter_Node_Data
{
  Eina_Size2D min;
  Eina_Size2D size;

   Eina_Bool realized;
} Efl_Ui_List_View_Precise_Layouter_Node_Data;

typedef struct _Efl_Ui_List_View_Precise_Layouter_Callback_Data
{
  Efl_Ui_List_View_Precise_Layouter_Data *pd;
  Efl_Ui_List_View_Layout_Item *item;
} Efl_Ui_List_View_Precise_Layouter_Callback_Data;

#include "efl_ui_list_view_precise_layouter.eo.h"

static void _efl_ui_list_view_relayout_layout_do(Efl_Ui_List_View_Precise_Layouter_Data *);
static Eina_Bool _initilize(Eo *, Efl_Ui_List_View_Precise_Layouter_Data*, Efl_Ui_List_View_Model*, Efl_Ui_List_View_Seg_Array*);
static void _finalize(Eo *, Efl_Ui_List_View_Precise_Layouter_Data*);
static void _node_realize(Efl_Ui_List_View_Precise_Layouter_Data*, Efl_Ui_List_View_Seg_Array_Node*);
static void _node_unrealize(Efl_Ui_List_View_Precise_Layouter_Data*, Efl_Ui_List_View_Seg_Array_Node*);

static void
_item_size_calc(Efl_Ui_List_View_Precise_Layouter_Data *pd, Efl_Ui_List_View_Layout_Item* item)
{
   int boxx, boxy, boxw, boxh, boxl, boxr, boxt, boxb, pad[4];
   double align[2];
   Eina_Bool fill[2];
   Eina_Size2D max;

   efl_gfx_size_hint_margin_get(item->layout, &pad[0], &pad[1], &pad[2], &pad[3]);
   evas_object_geometry_get(pd->modeler, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(pd->modeler, &boxl, &boxr, &boxt, &boxb);
   efl_gfx_size_hint_align_get(item->layout, &align[0], &align[1]);
   efl_gfx_size_hint_fill_get(item->layout, &fill[0], &fill[1]);
   max = efl_gfx_size_hint_max_get(item->layout);

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   if (EINA_DBL_EQ(align[0], -1))
     {
        align[0] = 0.5;
        fill[0] = EINA_TRUE;
     }
   else if (align[0] < 0)
     {
        align[0] = 0;
     }
   if (EINA_DBL_EQ(align[1], -1))
     {
        align[1] = 0.5;
        fill[1] = EINA_TRUE;
     }
   else if (align[1] < 0)
     {
        align[1] = 0;
     }

   if (align[0] > 1) align[0] = 1;
   if (align[1] > 1) align[1] = 1;

   if (max.w <= 0) max.w = INT_MAX;
   if (max.h <= 0) max.h = INT_MAX;
   if (max.w < item->min.w) max.w = item->min.w;
   if (max.h < item->min.h) max.h = item->min.h;

   // horizontally
   if (max.w < INT_MAX)
     {
        item->size.w = MIN(MAX(item->min.w - pad[0] - pad[1], max.w), boxw);
        item->pos.x = boxx + pad[0];
     }
   else if (fill[0])
     {
        // fill x
       item->size.w = boxw - pad[0] - pad[1];
       item->pos.x = boxx + pad[0];
     }
   else
     {
        item->size.w = item->min.w - pad[0] - pad[1];
        item->pos.x = boxx + ((boxw - item->size.w) * align[0]) + pad[0];
     }

   // vertically
   if (max.h < INT_MAX)
     {
        item->size.h = MIN(MAX(item->min.h - pad[2] - pad[3], max.h), boxh);
        item->pos.y = boxy + pad[2];
     }
   else if (fill[1])
     {
        // fill y
       item->size.h = item->min.h - pad[2] - pad[3];
       item->pos.y = boxy + pad[2];
     }
   else
     {
        item->size.h = item->min.h - pad[2] - pad[3];
        item->pos.y = boxy + ((item->min.h - item->size.h) * align[1]) + pad[2];
     }
}

static Eina_Size2D
_item_min_calc(Efl_Ui_List_View_Precise_Layouter_Data *pd, Efl_Ui_List_View_Layout_Item* item)
{
   Efl_Ui_List_View_Seg_Array_Node *itemnode = item->tree_node;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata;
   Efl_Ui_List_View_Layout_Item *layout_item;
   int i, pad[4];

   Eina_Size2D min = efl_gfx_size_hint_combined_min_get(item->layout);

   efl_gfx_size_hint_margin_get(item->layout, &pad[0], &pad[1], &pad[2], &pad[3]);
   min.w += pad[0] + pad[1];
   min.h += pad[2] + pad[3];

   if (item->min.h == min.h && item->min.w == min.w)
     return min;

   EINA_SAFETY_ON_NULL_RETURN_VAL(itemnode, min);
   EINA_SAFETY_ON_NULL_RETURN_VAL(itemnode->layout_data, min);
   nodedata = itemnode->layout_data;

   pd->min.h += min.h - item->min.h;
   nodedata->min.h += min.h - item->min.h;

   if (nodedata->min.w <= min.w)
     nodedata->min.w = min.w;
   else if (nodedata->min.w == item->min.w)
     {
       nodedata->min.w = 0;
       for (i = 0; i != itemnode->length; ++i)
         {
            layout_item = (Efl_Ui_List_View_Layout_Item *)itemnode->pointers[i];
            if (nodedata->min.w < layout_item->min.w)
              nodedata->min.w = layout_item->min.w;

            if (item->min.w == layout_item->min.w)
              break;
         }
     }

   if (pd->min.w <= min.w)
     pd->min.w = min.w;
   else if (pd->min.w == item->min.w)
     {
        Efl_Ui_List_View_Seg_Array_Node *node2;
        Eina_Accessor *nodes = efl_ui_list_view_seg_array_node_accessor_get(pd->seg_array);
        pd->min.w = min.w;

        EINA_ACCESSOR_FOREACH(nodes, i, node2)
          {
             Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata2 = node2->layout_data;
             if (pd->min.w < nodedata2->min.w)
               pd->min.w = nodedata2->min.w;

             if (item->min.w == nodedata2->min.w)
               break;
          }
       eina_accessor_free(nodes);
     }

   item->min.w = min.w;
   item->min.h = min.h;
   return item->min;
}

static void
_on_item_size_hint_change(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Ui_List_View_Precise_Layouter_Callback_Data *cb_data = data;
   Efl_Ui_List_View_Precise_Layouter_Data *pd = cb_data->pd;
   Efl_Ui_List_View_Layout_Item *item = cb_data->item;;
   Efl_Ui_List_View_Seg_Array_Node *node = item->tree_node;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata = node->layout_data;

   _item_min_calc(pd, item);
   if (!nodedata->realized)
     {
        free(evas_object_event_callback_del(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change));
        efl_ui_list_view_model_unrealize(pd->modeler, item);
     }
}

static void
_on_modeler_resize(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Ui_List_View_Precise_Layouter_Data *pd = data;
   pd->resize = EINA_TRUE;
}

typedef struct _Request Request;
struct _Request
{
   Efl_Ui_List_View_Precise_Layouter_Data *pd;
   unsigned int index;
};

static Eina_Value
_children_get(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Request *r = data;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     goto on_error;

   efl_ui_list_view_seg_array_insert_value(r->pd->seg_array, r->index, v);
   r->pd->recalc = EINA_TRUE;
   evas_object_smart_changed(r->pd->modeler);

 on_error:
   free(r);
   return v;
}

static void
_child_added_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   Efl_Ui_List_View_Precise_Layouter_Data *pd = data;
   Eina_Future *f;
   Request *r;

   r = calloc(1, sizeof (Request));
   if (!r) return;

   r->index = evt->index;
   r->pd = pd;

   f = efl_model_children_slice_get(pd->model, evt->index, 1);
   f = eina_future_then(f, _children_get, r, NULL);
}

static void
_child_removed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   Efl_Ui_List_View_Precise_Layouter_Data *pd = data;
   Efl_Ui_List_View_Layout_Item *layout_item, *litem;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata;
   Efl_Ui_List_View_Seg_Array_Node *itemnode;
   int i;

   litem = efl_ui_list_view_seg_array_remove(pd->seg_array, evt->index);
   if (!litem) return;

   itemnode = litem->tree_node;
   nodedata = itemnode->layout_data;

   free(evas_object_event_callback_del(litem->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change));

   pd->min.h -= litem->min.h;
   nodedata->min.h -= litem->min.h;

   if (nodedata->min.w == litem->min.w)
     nodedata->min.w = 0;

   for (i = 0; i != itemnode->length; ++i)
     {
        layout_item = (Efl_Ui_List_View_Layout_Item *)itemnode->pointers[i];
        if (nodedata->min.w < layout_item->min.w)
          nodedata->min.w = layout_item->min.w;

        if (litem->min.w == layout_item->min.w)
          {
             nodedata->min.w = layout_item->min.w;
             break;
          }
     }

   if (pd->min.w == litem->min.w)
     {
        Efl_Ui_List_View_Seg_Array_Node *node2;
        Eina_Accessor *nodes = efl_ui_list_view_seg_array_node_accessor_get(pd->seg_array);
        pd->min.w = 0;

        EINA_ACCESSOR_FOREACH(nodes, i, node2)
          {
             Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata2 = node2->layout_data;
             if (pd->min.w < nodedata2->min.w)
               pd->min.w = nodedata2->min.w;

             if (litem->min.w == nodedata2->min.w)
               break;
          }
        eina_accessor_free(nodes);
     }
   efl_ui_list_view_model_unrealize(pd->modeler, litem);

   free(litem);
   pd->recalc = EINA_TRUE;
   evas_object_smart_changed(pd->modeler);
}

static void
_child_count_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_List_View_Precise_Layouter_Data *pd = data;
   pd->count_total = efl_model_children_count_get(pd->model);
   if (pd->count_total)
     efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _child_count_changed_cb, pd);
}

static Eina_Bool
_initilize(Eo *obj EINA_UNUSED, Efl_Ui_List_View_Precise_Layouter_Data *pd, Efl_Ui_List_View_Model *modeler, Efl_Ui_List_View_Seg_Array *seg_array)
{
   if(pd->initialized)
     return EINA_TRUE;

   efl_replace(&pd->modeler, modeler);

   if(!pd->model || !pd->modeler)
     return EINA_FALSE;

   pd->recalc = EINA_TRUE;
   pd->initialized = EINA_TRUE;

   pd->seg_array = seg_array;

   efl_ui_list_view_model_load_range_set(pd->modeler, 0, pd->count_total); // load all
   efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, pd);
   efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, pd);

   evas_object_event_callback_add(modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);
   pd->min.w = 0;
   pd->min.h = 0;

   return EINA_TRUE;
}

static void
_finalize(Eo *obj EINA_UNUSED, Efl_Ui_List_View_Precise_Layouter_Data *pd)
{
   Efl_Ui_List_View_Seg_Array_Node* node;
   int i = 0;

   if (pd->model)
     {
         efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, pd);
         efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, pd);
         pd->count_total = 0;
     }

   if (pd->seg_array)
     {
        Eina_Accessor *nodes = efl_ui_list_view_seg_array_node_accessor_get(pd->seg_array);
        EINA_ACCESSOR_FOREACH(nodes, i, node)
          {
             _node_unrealize(pd, node);
             free(node->layout_data);
          }

        eina_accessor_free(nodes);
     }

   pd->min.w = 0;
   pd->min.h = 0;

   if (pd->modeler)
     {
        evas_object_event_callback_del_full(pd->modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);
        efl_ui_list_view_model_min_size_set(pd->modeler, pd->min);
     }

   pd->seg_array = NULL;
   efl_replace(&pd->modeler, NULL);

   pd->initialized = EINA_FALSE;
   pd->recalc = EINA_TRUE;
}

static void
_node_realize(Efl_Ui_List_View_Precise_Layouter_Data *pd, Efl_Ui_List_View_Seg_Array_Node *node)
{
   Efl_Ui_List_View_Layout_Item* layout_item;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata = node->layout_data;
   int i;

   EINA_SAFETY_ON_NULL_RETURN(nodedata);
   if (nodedata->realized)
     return;

   nodedata->realized = EINA_TRUE;

   for (i = 0; i != node->length; ++i)
     {
       layout_item = (Efl_Ui_List_View_Layout_Item *)node->pointers[i];
       efl_ui_list_view_model_realize(pd->modeler, layout_item);
     }
}

static void
_node_unrealize(Efl_Ui_List_View_Precise_Layouter_Data *pd, Efl_Ui_List_View_Seg_Array_Node *node)
{
   Efl_Ui_List_View_Layout_Item* layout_item;
   Efl_Ui_List_View_Precise_Layouter_Callback_Data *cb_data;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata = node->layout_data;
   int i;

   EINA_SAFETY_ON_NULL_RETURN(nodedata);
   if (!nodedata->realized)
     return;

   nodedata->realized = EINA_FALSE;

   for (i = 0; i != node->length; ++i)
     {
       layout_item = (Efl_Ui_List_View_Layout_Item *)node->pointers[i];
       if (layout_item->layout)
         {
            cb_data = evas_object_event_callback_del(layout_item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change);
            free(cb_data);
            efl_ui_list_view_model_unrealize(pd->modeler, layout_item);
         }
     }
}

static void
_calc_range(Efl_Ui_List_View_Precise_Layouter_Data *pd)
{
   Efl_Ui_List_View_Seg_Array_Node *node;
   Evas_Coord ch, ny;
   Eina_Rect vgmt;
   Eina_Position2D spos;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata;
   int i;

   vgmt = efl_ui_scrollable_viewport_geometry_get(pd->modeler);
   spos = efl_ui_scrollable_content_pos_get(pd->modeler);

   ny = spos.y - (vgmt.h / 2);
   if (ny < 0) spos.y = 0;
   else spos.y = ny;
   vgmt.h *= 2;

   ch = 0;
   Eina_Accessor *nodes = efl_ui_list_view_seg_array_node_accessor_get(pd->seg_array);
   EINA_ACCESSOR_FOREACH(nodes, i, node)
     {
        nodedata = node->layout_data;
        if (!nodedata || !nodedata->min.h)
          continue;

        if ((ch > spos.y || nodedata->min.h + ch > spos.y) && (ch < (spos.y + vgmt.h) || nodedata->min.h + ch < spos.y + vgmt.h))
          _node_realize(pd, node);
        else
          _node_unrealize(pd, node);

        ch += nodedata->min.h;
     }
   eina_accessor_free(nodes);
}

static void
_calc_size_job(void *data)
{
   Efl_Ui_List_View_Precise_Layouter_Data *pd;
   Efl_Ui_List_View_Seg_Array_Node *node;
   Efl_Ui_List_View_Layout_Item *layout_item;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata;
   Eo *obj = data;
   int i;
   double start_time = ecore_time_get();

   EINA_SAFETY_ON_NULL_RETURN(data);
   pd = efl_data_scope_get(obj, MY_CLASS);
   if (EINA_UNLIKELY(!pd)) return;

   Eina_Accessor *nodes = efl_ui_list_view_seg_array_node_accessor_get(pd->seg_array);
   while (eina_accessor_data_get(nodes, pd->calc_progress, (void **)&node))
     {
        pd->calc_progress++;
        if (!node->layout_data)
          node->layout_data = calloc(1, sizeof(Efl_Ui_List_View_Precise_Layouter_Node_Data));

        nodedata = node->layout_data;
        if (pd->calc_progress == 1)
          nodedata->realized = EINA_TRUE;

        for (i = 0; i != node->length; ++i)
          {
            layout_item = (Efl_Ui_List_View_Layout_Item *)node->pointers[i];
            EINA_SAFETY_ON_NULL_RETURN(layout_item);

            // cache size of new items
            if ((layout_item->min.w == 0) && (layout_item->min.h == 0) && (!layout_item->layout))
              efl_ui_list_view_model_realize(pd->modeler, layout_item);
          }

        if ( (ecore_time_get() - start_time ) > 0.01 )
          {
            ecore_job_del(pd->calc_job);
            pd->calc_job = ecore_job_add(_calc_size_job, obj);
            eina_accessor_free(nodes);
            return;
          }
     }
   eina_accessor_free(nodes);
   pd->calc_progress = 0;
   pd->calc_job = NULL;
   pd->recalc = EINA_FALSE;
}

EOLIAN static Efl_Object *
_efl_ui_list_view_precise_layouter_efl_object_constructor(Eo *obj, Efl_Ui_List_View_Precise_Layouter_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->initialized = EINA_FALSE;

   return obj;
}

EOLIAN static void
_efl_ui_list_view_precise_layouter_efl_ui_list_view_relayout_content_created(Eo *obj EINA_UNUSED, Efl_Ui_List_View_Precise_Layouter_Data *pd, Efl_Ui_List_View_Layout_Item *item)
{
   Efl_Ui_List_View_Precise_Layouter_Callback_Data *cb_data;
   EINA_SAFETY_ON_NULL_RETURN(item);
   EINA_SAFETY_ON_NULL_RETURN(item->layout);
   Efl_Ui_List_View_Seg_Array_Node *node = item->tree_node;
   Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata = node->layout_data;

   Eina_Size2D min = _item_min_calc(pd, item);

   if (min.w && min.h && !nodedata->realized)
     {
        efl_ui_list_view_model_unrealize(pd->modeler, item);
        return;
     }

   cb_data = calloc(1, sizeof(Efl_Ui_List_View_Precise_Layouter_Callback_Data));
   if (!cb_data) return;
   cb_data->pd = pd;
   cb_data->item = item;
   evas_object_event_callback_add(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, cb_data);

   _item_size_calc(pd, item);
}

EOLIAN static Eina_List *
_efl_ui_list_view_precise_layouter_efl_ui_list_view_relayout_elements_get(const Eo *obj EINA_UNUSED, Efl_Ui_List_View_Precise_Layouter_Data *pd)
{
   Eina_List *elements_order = NULL;
   Efl_Ui_List_View_Layout_Item* layout_item;
   Efl_Ui_List_View_Seg_Array_Node *items_node;
   int i, j = 0;

   Eina_Accessor *nodes = efl_ui_list_view_seg_array_node_accessor_get(pd->seg_array);
   EINA_ACCESSOR_FOREACH(nodes, i, items_node)
     {
        Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata = items_node->layout_data;
        if (!nodedata || !nodedata->realized)
            continue;

        for (j = 0; j != items_node->length;++j)
          {
             layout_item = (Efl_Ui_List_View_Layout_Item *)items_node->pointers[j];
             if (layout_item->layout)
               elements_order = eina_list_append(elements_order, layout_item->layout);
          }
     }

   eina_accessor_free(nodes);
   return elements_order;
}

EOLIAN static void
_efl_ui_list_view_precise_layouter_efl_ui_list_view_relayout_model_set(Eo *obj, Efl_Ui_List_View_Precise_Layouter_Data *pd, Efl_Model *model)
{
   _finalize(obj, pd);

   efl_replace(&pd->model, model);

   if (pd->model)
     {
        pd->count_total = efl_model_children_count_get(pd->model);
        if (pd->count_total && pd->modeler)
          efl_ui_list_view_model_load_range_set(pd->modeler, 0, pd->count_total); // load all
        else
          efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _child_count_changed_cb, pd);
     }
}

static void
_efl_ui_list_view_relayout_layout_do(Efl_Ui_List_View_Precise_Layouter_Data *pd)
{
   Eina_Rect vgmt;
   Eina_Position2D spos;
   double cur_pos = 0;
   Efl_Ui_List_View_Layout_Item* layout_item;
   Efl_Ui_List_View_Seg_Array_Node *items_node;
   int i, j = 0;
   int boxx, boxy, boxw, boxh, extra = 0, rounding = 0;
   int boxl = 0, boxr = 0, boxt = 0, boxb = 0;

   _calc_range(pd);

   evas_object_geometry_get(pd->modeler, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(pd->modeler, &boxl, &boxr, &boxt, &boxb);

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   // available space. if <0 we overflow
   extra = boxh - pd->min.h;
   if (extra < 0) extra = 0;

   efl_ui_list_view_model_min_size_set(pd->modeler, pd->min);

   vgmt = efl_ui_scrollable_viewport_geometry_get(pd->modeler);
   spos = efl_ui_scrollable_content_pos_get(pd->modeler);

   Eina_Accessor *nodes = efl_ui_list_view_seg_array_node_accessor_get(pd->seg_array);
   EINA_ACCESSOR_FOREACH(nodes, i, items_node)
     {
        Efl_Ui_List_View_Precise_Layouter_Node_Data *nodedata = items_node->layout_data;
        if (!items_node->layout_data)
          continue;

        if (!nodedata->realized)
          {
             cur_pos += nodedata->min.h;
             continue;
          }

        for (j = 0; j != items_node->length;++j)
          {
             layout_item = (Efl_Ui_List_View_Layout_Item *)items_node->pointers[j];
             double x, y, w, h;
             double weight_x, weight_y;
             if (!(layout_item->min.w && layout_item->min.h))
               continue;

             // extra rounding up (compensate cumulative error)
             if ((i == (pd->count_total - 1)) && (cur_pos - floor(cur_pos) >= 0.5))
               rounding = 1;

             if (layout_item->layout)
               {
                  if (pd->resize)
                    _item_size_calc(pd, layout_item);

                  efl_gfx_size_hint_weight_get(layout_item->layout, &weight_x, &weight_y);
               }
             else
               {
                  cur_pos += layout_item->size.h;
                  continue;
               }

             x = layout_item->pos.x;
             y = layout_item->pos.y + cur_pos;
             w = layout_item->size.w;
             h = layout_item->size.h + rounding + weight_y * extra;
             cur_pos += h;

             if (w < pd->min.w) w = pd->min.w;
             if (w > vgmt.w) w = vgmt.w;

             evas_object_geometry_set(layout_item->layout, (x + 0 - spos.x), (y + 0 - spos.y), w, h);
          }
     }
   eina_accessor_free(nodes);

   pd->resize = EINA_FALSE;
}

EOLIAN static void
_efl_ui_list_view_precise_layouter_efl_ui_list_view_relayout_layout_do
  (Eo *obj EINA_UNUSED, Efl_Ui_List_View_Precise_Layouter_Data *pd
   , Efl_Ui_List_View_Model *modeler, int first EINA_UNUSED, Efl_Ui_List_View_Seg_Array *seg_array)
{
   if (!_initilize(obj, pd, modeler, seg_array) || !pd->seg_array)
     return;

   if (!pd->calc_job && pd->recalc && efl_ui_list_view_seg_array_count(seg_array) > 0)
     {
        // cache size of new items
        pd->calc_progress = 0;
        pd->calc_job = ecore_job_add(_calc_size_job, obj);
        return;
     }

   _efl_ui_list_view_relayout_layout_do(pd);
}

#include "efl_ui_list_view_precise_layouter.eo.c"
