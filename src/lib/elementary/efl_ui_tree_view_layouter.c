#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include <assert.h>

#include "elm_priv.h"
#include "efl_ui_tree_view_seg_array.h"

#define MY_CLASS EFL_UI_TREE_VIEW_LAYOUTER_CLASS

typedef struct _Efl_Ui_Tree_View_Layouter_Data
{
   Efl_Ui_Tree_View_Seg_Array *segarray;
   Efl_Ui_Tree_View_Model *modeler;
   Ecore_Job *calc_job;
   Efl_Model* model;
   Eina_Size2D min;
   int count_total;
   int count_segarray;

   Eina_Bool initialized : 1;
   Eina_Bool recalc : 1;
   Eina_Bool resize : 1;
} Efl_Ui_Tree_View_Layouter_Data;

typedef struct _Efl_Ui_Tree_View_Layouter_Node_Data
{
   Eina_Size2D min;
   Eina_Size2D size;
   Eina_Bool   realized : 1;
   Eina_Bool   expanded : 1;
} Efl_Ui_Tree_View_Layouter_Node_Data;

typedef struct _Efl_Ui_Tree_View_Layouter_Callback_Data
{
  Efl_Ui_Tree_View_Layouter_Data *pd;
  Efl_Ui_Tree_View_Item *item;
} Efl_Ui_Tree_View_Layouter_Callback_Data;

typedef struct _Request Request;
struct _Request
{
   Efl_Ui_Tree_View_Layouter_Data *pd;
   unsigned int index;
};

typedef struct _Draw_Data
{
   Eina_Rect vgmt;
   Eina_Position2D spos;
   int extra;
   int rounding;
   double cur_pos;
} Draw_Data;

#include "efl_ui_tree_view_layouter.eo.h"

static void
_item_size_calc(Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Item* item)
{
   int boxx, boxy, boxw, boxh, boxl, boxr, boxt, boxb, pad[4];
   double align[2];
   Eina_Size2D max;

   efl_gfx_size_hint_margin_get(item->layout, &pad[0], &pad[1], &pad[2], &pad[3]);
   evas_object_geometry_get(pd->modeler, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(pd->modeler, &boxl, &boxr, &boxt, &boxb);
   efl_gfx_size_hint_align_get(item->layout, &align[0], &align[1]);
   max = efl_gfx_size_hint_max_get(item->layout);

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   if (align[0] < 0) align[0] = -1;
   if (align[1] < 0) align[1] = -1;
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
   else if (align[0] < 0)
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
   else if (align[1] < 0)
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

static void
_node_size_fix(Efl_Ui_Tree_View_Item* item, int sizediff)
{
   Efl_Ui_Tree_View_Layouter_Node_Data *nodedata;
   Efl_Ui_Tree_View_Seg_Array_Node *node;
   Efl_Ui_Tree_View_Item *parent;

   if (!item || !item->parent)
     return;

   parent = item->parent;
   node = parent->tree_node;
   if (node && node->layout_data)
     {
        nodedata = node->layout_data;
        nodedata->min.h += sizediff;
     }

   parent->children_min.h += sizediff;
   if (parent->children_min.h < 0) parent->children_min.h = 0;

   _node_size_fix(parent, sizediff);
}

static void
_item_min_calc(Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Item* item
                , Eina_Size2D min, Efl_Ui_Tree_View_Seg_Array_Node *itemnode
                , Efl_Ui_Tree_View_Seg_Array *segarray)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);
   EINA_SAFETY_ON_NULL_RETURN(item);
   EINA_SAFETY_ON_NULL_RETURN(itemnode);
   Efl_Ui_Tree_View_Layouter_Node_Data *nodedata = itemnode->layout_data;
   Efl_Ui_Tree_View_Item *litem;
   int i, pad[4];

   efl_gfx_size_hint_margin_get(item->layout, &pad[0], &pad[1], &pad[2], &pad[3]);
   min.w += pad[0] + pad[1];
   min.h += pad[2] + pad[3];

   if (item->min.h == min.h && item->min.w == min.w)
     return;

   pd->min.h += min.h - item->min.h;
   nodedata->min.h += min.h - item->min.h;

   if (nodedata->min.w <= min.w)
     nodedata->min.w = min.w;
   else if (nodedata->min.w == item->min.w)
     {
       nodedata->min.w = 0;
       for (i = 0; i != itemnode->length; ++i)
         {
            litem = (Efl_Ui_Tree_View_Item *)itemnode->pointers[i];
            if (nodedata->min.w < litem->min.w)
              nodedata->min.w = litem->min.w;
            if (item->min.w == litem->min.w)
              break;
         }
     }

   if (pd->min.w <= min.w)
     pd->min.w = min.w;
   else if (pd->min.w == item->min.w)
     {
        Efl_Ui_Tree_View_Seg_Array_Node *node2;
        Eina_Accessor *nodes = efl_ui_tree_view_seg_array_node_accessor_get(segarray);
        pd->min.w = min.w;

        EINA_ACCESSOR_FOREACH(nodes, i, node2)
          {
             Efl_Ui_Tree_View_Layouter_Node_Data *nodedata2 = node2->layout_data;
             if (pd->min.w < nodedata2->min.w)
               pd->min.w = nodedata2->min.w;

             if (item->min.w == nodedata2->min.w)
               break;
          }
       eina_accessor_free(nodes);
     }

   _node_size_fix(item, (min.h - item->min.h));

   item->min.w = min.w;
   item->min.h = min.h;
   _item_size_calc(pd, item);
}

static void
_on_item_size_hint_change(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Efl_Ui_Tree_View_Layouter_Callback_Data *cb_data = data;
   EINA_SAFETY_ON_NULL_RETURN(cb_data);
   Efl_Ui_Tree_View_Layouter_Data *pd = cb_data->pd;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   Efl_Ui_Tree_View_Item *item = cb_data->item;;
   EINA_SAFETY_ON_NULL_RETURN(item);
   Efl_Ui_Tree_View_Seg_Array_Node *node = item->tree_node;
   EINA_SAFETY_ON_NULL_RETURN(node);
   Efl_Ui_Tree_View_Layouter_Node_Data *nodedata = node->layout_data;
   EINA_SAFETY_ON_NULL_RETURN(nodedata);

   Eina_Size2D min = efl_gfx_size_hint_combined_min_get(obj);
   if (item->parent)
     {
        Efl_Ui_Tree_View_Item *parent = item->parent;
        _item_min_calc(pd, item, min, node, parent->segarray);
     }
   else
     _item_min_calc(pd, item, min, node, pd->segarray);

   if (!nodedata->realized)
     free(evas_object_event_callback_del(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change));
}

static Eina_Value
_children_get(void *data, Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Model *children;
   unsigned int i, len;
   Request *r = data;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     goto on_error;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_OBJECT)
     {
        children = eina_value_object_get(&v);
        efl_ui_tree_view_seg_array_insert(r->pd->segarray, NULL, r->index, children);
     }
   else if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ARRAY)
     {
        EINA_VALUE_ARRAY_FOREACH(&v, len, i, children)
          {
             unsigned int idx = r->index + i;
             efl_ui_tree_view_seg_array_insert(r->pd->segarray, NULL, idx, children);
          }
     }

   r->pd->recalc = EINA_TRUE;
   evas_object_smart_changed(r->pd->modeler);

 on_error:
   free(r);
   return v;
}

static void
_on_modeler_resize(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Ui_Tree_View_Layouter_Data *pd = data;
   pd->resize = EINA_TRUE;
}

static void
_child_added_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   Efl_Ui_Tree_View_Layouter_Data *pd = data;
   Eina_Future *f;
   Request *r;

   r = calloc(1, sizeof (Request));
   if (!r) return;

   r->index = evt->index;
   r->pd = pd;

   f = efl_model_children_slice_get(pd->model, evt->index, 1);
   f = eina_future_then(f, _children_get, r);
}

static void
_node_realize(Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Seg_Array_Node *node)
{
   Efl_Ui_Tree_View_Item* item;
   Efl_Ui_Tree_View_Layouter_Node_Data *nodedata = node->layout_data;
   int i;

   EINA_SAFETY_ON_NULL_RETURN(nodedata);
   if (nodedata->realized)
     return;

   nodedata->realized = EINA_TRUE;

   for (i = 0; i != node->length; ++i)
     {
        item = (Efl_Ui_Tree_View_Item *)node->pointers[i];
        efl_ui_tree_view_model_realize(pd->modeler, item);
     }
}

void
_node_unrealize(Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Seg_Array_Node *node, Eina_Bool is_free)
{
   Efl_Ui_Tree_View_Seg_Array_Node *s_node;
   Efl_Ui_Tree_View_Layouter_Node_Data *nodedata;
   Efl_Ui_Tree_View_Item* item;
   int i, j;

   EINA_SAFETY_ON_NULL_RETURN(node);
   EINA_SAFETY_ON_NULL_RETURN(node->layout_data);

   nodedata = node->layout_data;
   if (!nodedata->realized)
     return;

   nodedata->realized = EINA_FALSE;
   for (i = 0; i != node->length; ++i)
     {
        item = (Efl_Ui_Tree_View_Item *)node->pointers[i];
        if (item->expanded && item->segarray)
          {
             Eina_Accessor *nodes = efl_ui_tree_view_seg_array_node_accessor_get(item->segarray);
             EINA_ACCESSOR_FOREACH(nodes, j, s_node)
               {
                 _node_unrealize(pd, s_node, is_free);
               }
             eina_accessor_free(nodes);
          }

        if (item->layout)
          {
             free(evas_object_event_callback_del(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change));
             efl_ui_tree_view_model_unrealize(pd->modeler, item);
          }
     }

   if (is_free)
     {
        free(node->layout_data);
        node->layout_data = NULL;
     }
}

static void
_child_count_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Tree_View_Layouter_Data *pd = data;
   pd->count_total = efl_model_children_count_get(pd->model);
   if (pd->count_total)
     efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _child_count_changed_cb, pd);
}

static Eina_Bool
_initilize(Eo *obj EINA_UNUSED, Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Model *modeler)
{
   if (pd->initialized)
     return EINA_TRUE;

   efl_replace(&pd->modeler, modeler);

   if (!pd->model || !pd->modeler)
     return EINA_FALSE;

   pd->recalc = EINA_TRUE;
   pd->initialized = EINA_TRUE;

   efl_ui_tree_view_model_load_range_set(pd->modeler, 0, pd->count_total); // load all
   efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, pd);

   evas_object_event_callback_add(pd->modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);
   pd->min.w = 0;
   pd->min.h = 0;

   return EINA_TRUE;
}

static void
_finalize(Eo *obj EINA_UNUSED, Efl_Ui_Tree_View_Layouter_Data *pd)
{
   Efl_Ui_Tree_View_Seg_Array_Node* node;
   int i;

   if (pd->segarray)
     {
        Eina_Accessor *nodes = efl_ui_tree_view_seg_array_node_accessor_get(pd->segarray);
        EINA_ACCESSOR_FOREACH(nodes, i, node)
          {
            _node_unrealize(pd, node, EINA_TRUE);
          }
        eina_accessor_free(nodes);
     }

   if (pd->model)
     {
        efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, pd);
        pd->count_total = 0;
     }

   if (pd->modeler)
     {
        evas_object_event_callback_del_full(pd->modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);
        efl_ui_tree_view_model_min_size_set(pd->modeler, pd->min);
     }

   pd->min.w = 0;
   pd->min.h = 0;
   efl_replace(&pd->modeler, NULL);
   efl_replace(&pd->segarray, NULL);
   pd->initialized = EINA_FALSE;
   pd->recalc = EINA_TRUE;
}

static void
_calc_range(Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Seg_Array *segarray, Evas_Coord cur_pos)
{
   Efl_Ui_Tree_View_Layouter_Node_Data *nodedata;
   Efl_Ui_Tree_View_Seg_Array_Node *node;
   Eina_Position2D spos;
   Evas_Coord ny;
   Eina_Rect vgmt;
   int i;

   vgmt = efl_ui_scrollable_viewport_geometry_get(pd->modeler);
   spos = efl_ui_scrollable_content_pos_get(pd->modeler);
   ny = spos.y - (vgmt.h / 2);
   if (ny < 0) spos.y = 0;
   else spos.y = ny;
   vgmt.h *= 2;

   Eina_Accessor *nodes = efl_ui_tree_view_seg_array_node_accessor_get(segarray);
   EINA_ACCESSOR_FOREACH(nodes, i, node)
     {
        nodedata = node->layout_data;
        if (!nodedata || !nodedata->min.h)
          continue;

        if ((cur_pos > spos.y || nodedata->min.h + cur_pos > spos.y) && (cur_pos < (spos.y + vgmt.h) || nodedata->min.h + cur_pos < spos.y + vgmt.h))
          _node_realize(pd, node);
        else
          _node_unrealize(pd, node, EINA_FALSE);

        cur_pos += nodedata->min.h;
     }
   eina_accessor_free(nodes);
}

static void
_calc_size_segarray(Efl_Ui_Tree_View_Layouter_Data* pd, Efl_Ui_Tree_View_Seg_Array *segarray)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);
   EINA_SAFETY_ON_NULL_RETURN(segarray);
   Efl_Ui_Tree_View_Seg_Array_Node *node;
   Efl_Ui_Tree_View_Item *item;
   int i, j = 0, count;

   Eina_Accessor *nodes = efl_ui_tree_view_seg_array_node_accessor_get(segarray);
   while (eina_accessor_data_get(nodes, j, (void **)&node))
     {
        ++j;
        if (!node->layout_data)
          node->layout_data = calloc(1, sizeof(Efl_Ui_Tree_View_Layouter_Node_Data));

        if (j == 1)
          {
             Efl_Ui_Tree_View_Layouter_Node_Data *nodedata = node->layout_data;
             nodedata->realized = EINA_TRUE;
          }

        for (i = 0; i != node->length; ++i)
          {
             item = (Efl_Ui_Tree_View_Item *)node->pointers[i];
             EINA_SAFETY_ON_NULL_RETURN(item);

             // cache size of new items
             if ((item->min.w == 0) && (item->min.h == 0) && !item->layout)
               efl_ui_tree_view_model_realize(pd->modeler, item);

             count = efl_model_children_count_get(item->model);
             if (count && !item->expanded)
               {
                  efl_ui_tree_view_model_expand(pd->modeler, item, 0, count);
               }
             else if (item->expanded && item->segarray)
               {
                  _calc_size_segarray (pd, item->segarray);
               }
          }
     }
   eina_accessor_free(nodes);
}

static void
_calc_size_job(void *data)
{
   Efl_Ui_Tree_View_Layouter_Data *pd;
   Eo *obj = data;
   /* double start_time = ecore_time_get(); */

   EINA_SAFETY_ON_NULL_RETURN(data);
   pd = efl_data_scope_get(obj, MY_CLASS);
   if (EINA_UNLIKELY(!pd)) return;

   _calc_size_segarray(pd, pd->segarray);

   pd->calc_job = NULL;
   pd->recalc = EINA_FALSE;

   evas_object_smart_changed(pd->modeler);
}

EOLIAN static Efl_Object *
_efl_ui_tree_view_layouter_efl_object_constructor(Eo *obj, Efl_Ui_Tree_View_Layouter_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->initialized = EINA_FALSE;

   return obj;
}

EOLIAN static void
_efl_ui_tree_view_layouter_efl_ui_tree_view_relayout_content_created(Eo *obj EINA_UNUSED, Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Item *item)
{
   Efl_Ui_Tree_View_Layouter_Callback_Data *cb_data;
   EINA_SAFETY_ON_NULL_RETURN(item);
   EINA_SAFETY_ON_NULL_RETURN(item->layout);
   Efl_Ui_Tree_View_Seg_Array_Node *node = item->tree_node;
   Efl_Ui_Tree_View_Layouter_Node_Data *nodedata = node->layout_data;

   Eina_Size2D min = efl_gfx_size_hint_combined_min_get(item->layout);
   _item_min_calc(pd, item, min, node, pd->segarray);

   if (min.w && min.h && !nodedata->realized)
     {
        efl_ui_tree_view_model_unrealize(pd->modeler, item);
        return;
     }

   cb_data = calloc(1, sizeof(Efl_Ui_Tree_View_Layouter_Callback_Data));
   cb_data->pd = pd;
   cb_data->item = item;
   evas_object_event_callback_add(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, cb_data);

   _item_size_calc(pd, item);
}

EOLIAN static Eina_List *
_efl_ui_tree_view_layouter_efl_ui_tree_view_relayout_elements_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tree_View_Layouter_Data *pd)
{
   Eina_List *elements_order = NULL;
   Efl_Ui_Tree_View_Item* item;
   Efl_Ui_Tree_View_Seg_Array_Node *items_node;
   int i, j;

   Eina_Accessor *nodes = efl_ui_tree_view_seg_array_node_accessor_get(pd->segarray);
   EINA_ACCESSOR_FOREACH(nodes, i, items_node)
     {
        Efl_Ui_Tree_View_Layouter_Node_Data *nodedata = items_node->layout_data;
        if (!nodedata || !nodedata->realized)
          continue;

        for (j = 0; j != items_node->length; ++j)
          {
             item = (Efl_Ui_Tree_View_Item *)items_node->pointers[j];
             if (item->layout)
               elements_order = eina_list_append(elements_order, item->layout);
          }
     }
   eina_accessor_free(nodes);
   return elements_order;
}

EOLIAN static void
_efl_ui_tree_view_layouter_efl_ui_tree_view_relayout_model_set(Eo *obj EINA_UNUSED, Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Model *model)
{
   _finalize(obj, pd);

   efl_replace(&pd->model, model);
   if (pd->model)
     {
        pd->count_total = efl_model_children_count_get(pd->model);
        if (pd->count_total && pd->modeler)
          efl_ui_tree_view_model_load_range_set(pd->modeler, 0, pd->count_total); // load all
        else
          efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _child_count_changed_cb, pd);
     }
}

static void
_node_draw(Efl_Ui_Tree_View_Layouter_Data *pd, Efl_Ui_Tree_View_Seg_Array *segarray, Draw_Data *drawdata)
{
   Efl_Ui_Tree_View_Seg_Array_Node *items_node;
   Efl_Ui_Tree_View_Item* item;
   int i, j;

   _calc_range(pd, segarray, drawdata->cur_pos);

   Eina_Accessor *nodes = efl_ui_tree_view_seg_array_node_accessor_get(segarray);
   EINA_SAFETY_ON_NULL_RETURN(nodes);
   EINA_ACCESSOR_FOREACH(nodes, i, items_node)
     {
        Efl_Ui_Tree_View_Layouter_Node_Data *nodedata = items_node->layout_data;
        if (!nodedata) continue;

        if (nodedata->realized)
          for (j = 0; j != items_node->length;++j)
            {
               double x, y, w, h;
               double weight_x, weight_y;
               item = (Efl_Ui_Tree_View_Item *)items_node->pointers[j];
               EINA_SAFETY_ON_NULL_RETURN(item);

               if (item->layout)
                 {
                    if (pd->resize)
                      _item_size_calc(pd, item);

                    efl_gfx_size_hint_weight_get(item->layout, &weight_x, &weight_y);
                    // extra rounding up (compensate cumulative error)
                    if ((i == (pd->count_total - 1)) && (drawdata->cur_pos - floor(drawdata->cur_pos) >= 0.5))
                      drawdata->rounding = 1;

                    x = item->pos.x;
                    y = item->pos.y + drawdata->cur_pos;
                    w = item->size.w;
                    h = item->size.h + drawdata->rounding + weight_y * drawdata->extra;
                    drawdata->cur_pos += h;

                    if (w < pd->min.w) w = pd->min.w;
                    if (w > drawdata->vgmt.w) w = drawdata->vgmt.w;

                    evas_object_geometry_set(item->layout, (x + 0 - drawdata->spos.x), (y + 0 - drawdata->spos.y), w, h);
                 }
               else
                 {
                    drawdata->cur_pos += item->size.h;
                 }

               if (item->expanded && item->segarray)
                 _node_draw(pd, item->segarray, drawdata);
            }
        else
          drawdata->cur_pos += nodedata->min.h;
     }
   eina_accessor_free(nodes);
}

static void
_efl_ui_tree_view_relayout_layout_do(Efl_Ui_Tree_View_Layouter_Data *pd)
{
   int boxx, boxy, boxw, boxh;
   int boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   Draw_Data drawdata;
   memset(&drawdata, 0, sizeof(drawdata));

   evas_object_geometry_get(pd->modeler, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(pd->modeler, &boxl, &boxr, &boxt, &boxb);

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   drawdata.extra = boxh - pd->min.h;
   if (drawdata.extra < 0) drawdata.extra = 0;

   efl_ui_tree_view_model_min_size_set(pd->modeler, pd->min);
   drawdata.vgmt = efl_ui_scrollable_viewport_geometry_get(pd->modeler);
   drawdata.spos = efl_ui_scrollable_content_pos_get(pd->modeler);

   _node_draw(pd, pd->segarray, &drawdata);
   pd->resize = EINA_FALSE;
}

EOLIAN static void
_efl_ui_tree_view_layouter_efl_ui_tree_view_relayout_layout_do
  (Eo *obj EINA_UNUSED, Efl_Ui_Tree_View_Layouter_Data *pd
   , Efl_Ui_Tree_View_Model *modeler, int first EINA_UNUSED, Efl_Ui_Tree_View_Seg_Array *segarray)
{
   int count;

   if (!_initilize(obj, pd, modeler))
     return;

   efl_replace(&pd->segarray, segarray);
   if (!pd->segarray)
     return;

   count = efl_ui_tree_view_seg_array_count(pd->segarray);

   if ((pd->recalc && count > 0) || pd->count_segarray != count)
     {
        // cache size of new items
        pd->count_segarray = count;
        ecore_job_del(pd->calc_job);
        pd->calc_job = ecore_job_add(_calc_size_job, obj);
        return;
     }
   _efl_ui_tree_view_relayout_layout_do(pd);
}

#include "efl_ui_tree_view_layouter.eo.c"
