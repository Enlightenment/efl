#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include <assert.h>

#include "elm_priv.h"
#include "efl_ui_list_segarray.h"

#undef DBG
#define DBG(...) do { \
    fprintf(stderr, __FILE__ ":" "%d %s ", __LINE__, __PRETTY_FUNCTION__); \
    fprintf(stderr,  __VA_ARGS__);                                     \
    fprintf(stderr, "\n"); fflush(stderr);                              \
  } while(0)

#define MY_CLASS EFL_UI_LIST_PRECISE_LAYOUTER_CLASS

typedef struct _Efl_Ui_List_Precise_Layouter_Data
{
   Eina_Bool initialized;
   Eina_Bool recalc;
   Eina_Bool resize;
   Eina_Size2D min;
   Efl_Model* model;
   Efl_Ui_List_Model *modeler;
   Efl_Future *count_future;
   Ecore_Job *calc_job;
   Efl_Ui_List_SegArray *segarray;
   int first;
   int count_total;
   unsigned int calc_progress;
} Efl_Ui_List_Precise_Layouter_Data;

typedef struct _Efl_Ui_List_Precise_Layouter_Node_Data
{
  Eina_Size2D min;
  Eina_Size2D size;
  Eina_Bool realized;
} Efl_Ui_List_Precise_Layouter_Node_Data;

typedef struct _Efl_Ui_List_Precise_Layouter_Callback_Data
{
  Efl_Ui_List_Precise_Layouter_Data* pd;
  Efl_Ui_List_SegArray_Node* node;
} Efl_Ui_List_Precise_Layouter_Callback_Data;


#include "efl_ui_list_precise_layouter.eo.h"


static void _efl_ui_list_relayout_layout_do(Efl_Ui_List_Precise_Layouter_Data *);
static void _initilize(Eo *, Efl_Ui_List_Precise_Layouter_Data*, Efl_Ui_List_Model*, Efl_Ui_List_SegArray*);
static void _finalize(Eo *, Efl_Ui_List_Precise_Layouter_Data*);
static void _node_realize(Efl_Ui_List_Precise_Layouter_Data*, Efl_Ui_List_SegArray_Node*);
static void _node_unrealize(Efl_Ui_List_Precise_Layouter_Data*, Efl_Ui_List_SegArray_Node*);

static void
_item_size_calc(Efl_Ui_List_Precise_Layouter_Data *pd, Efl_Ui_List_LayoutItem* item)
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
_item_min_calc(Efl_Ui_List_Precise_Layouter_Data *pd, Efl_Ui_List_LayoutItem* item
                , Eina_Size2D min, Efl_Ui_List_SegArray_Node *itemnode)
{
   Efl_Ui_List_Precise_Layouter_Node_Data *nodedata = itemnode->layout_data;
   Efl_Ui_List_LayoutItem *layout_item;
   int i, pad[4];

   efl_gfx_size_hint_margin_get(item->layout, &pad[0], &pad[1], &pad[2], &pad[3]);

   min.w += pad[0] + pad[1];
   min.h += pad[2] + pad[3];

   if (item->min.h == min.h && item->min.w == min.h)
     return;

   pd->min.h += min.h - item->min.h;
   nodedata->min.h += min.h - item->min.h;

   if (nodedata->min.w <= min.w)
     nodedata->min.w = min.w;
   else if (nodedata->min.w == item->min.w)
     for (i = 0; i != itemnode->length; ++i)
       {
          layout_item = (Efl_Ui_List_LayoutItem *)itemnode->pointers[i];
          if (nodedata->min.w < layout_item->min.w)
            nodedata->min.w = layout_item->min.w;

          if (item->min.w == layout_item->min.w)
            break;
       }

   if (pd->min.w <= min.w)
     pd->min.w = min.w;
   else if (pd->min.w == item->min.w)
     {
        Efl_Ui_List_SegArray_Node *node;
        Eina_Accessor *nodes = efl_ui_list_segarray_node_accessor_get(pd->segarray);
        pd->min.w = min.w;

        EINA_ACCESSOR_FOREACH(nodes, i, node)
          {
             Efl_Ui_List_Precise_Layouter_Node_Data *nodedata = node->layout_data;
             if (pd->min.w < nodedata->min.w)
               pd->min.w = nodedata->min.w;

             if (item->min.w == nodedata->min.w)
               break;
          }
       eina_accessor_free(nodes);
     }

   item->min.w = min.w;
   item->min.h = min.h;

   _item_size_calc(pd, item);
}

static void
_count_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Precise_Layouter_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   pd->count_future = NULL;

   pd->count_total = *(int*)((Efl_Future_Event_Success*)event->info)->value;

   if (pd->modeler && (pd->count_total != efl_ui_list_segarray_count(pd->segarray)))
     {
        pd->recalc = EINA_TRUE;
        efl_ui_list_model_load_range_set(pd->modeler, 0, 0); // load all
     }
}

static void
_count_error(void * data, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Precise_Layouter_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   pd->count_future = NULL;
}

static void
_on_item_size_hint_change(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Efl_Ui_List_Precise_Layouter_Callback_Data *cb_data = data;
   Efl_Ui_List_SegArray_Node *node = cb_data->node;
   Efl_Ui_List_Precise_Layouter_Data *pd = cb_data->pd;
   Efl_Ui_List_LayoutItem *item;
   int i;

   Eina_Size2D min = efl_gfx_size_hint_combined_min_get(obj);

   for (i = 0; i != node->length; ++i)
     {
        item = (Efl_Ui_List_LayoutItem *)node->pointers[i];
        if (item->layout == obj)
          {
             Efl_Ui_List_Precise_Layouter_Node_Data *nodedata = node->layout_data;
             _item_min_calc(cb_data->pd, item, min, node);
             if (!nodedata->realized)
               {
                  //DBG("item size change unrealize");
                  free(evas_object_event_callback_del(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change));
                  efl_ui_list_model_unrealize(pd->modeler, item);
               }
             return;
          }
     }
}

static void
_on_modeler_resize(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Ui_List_Precise_Layouter_Data *pd = data;
   pd->resize = EINA_TRUE;
}

static void
_initilize(Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd, Efl_Ui_List_Model *modeler, Efl_Ui_List_SegArray *segarray)
{
   if(pd->initialized)
     return;

   pd->recalc = EINA_TRUE;
   pd->initialized = EINA_TRUE;

   pd->modeler = modeler;
   pd->segarray = segarray;

   evas_object_event_callback_add(modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);
   efl_ui_list_model_load_range_set(modeler, 0, 0); // load all

   pd->min.w = 0;
   pd->min.h = 0;
}

static void
_finalize(Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd)
{
   Efl_Ui_List_SegArray_Node* node;
   int i = 0;

   evas_object_event_callback_del_full(pd->modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);

   Eina_Accessor *nodes = efl_ui_list_segarray_node_accessor_get(pd->segarray);
   EINA_ACCESSOR_FOREACH(nodes, i, node)
     _node_unrealize(pd, node);
   eina_accessor_free(nodes);

   pd->min.w = 0;
   pd->min.h = 0;

   efl_ui_list_model_min_size_set(pd->modeler, pd->min);

   pd->segarray = NULL;
   pd->modeler = NULL;

   pd->initialized = EINA_FALSE;
}

static void
_node_realize(Efl_Ui_List_Precise_Layouter_Data *pd, Efl_Ui_List_SegArray_Node *node)
{
   Efl_Ui_List_LayoutItem* layout_item;
   Efl_Ui_List_Precise_Layouter_Callback_Data *cb_data;
   Efl_Ui_List_Precise_Layouter_Node_Data *nodedata = node->layout_data;
   int i;

   if (nodedata->realized)
     return;

   DBG("node_realize %d", node->first);
   nodedata->realized = EINA_TRUE;

   for (i = 0; i != node->length; ++i)
     {
       layout_item = (Efl_Ui_List_LayoutItem *)node->pointers[i];
       efl_ui_list_model_realize(pd->modeler, layout_item);

       if (layout_item->layout)
         {
            cb_data = calloc(1, sizeof(Efl_Ui_List_Precise_Layouter_Callback_Data));
            cb_data->pd = pd;
            cb_data->node = node;
            evas_object_event_callback_add(layout_item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, cb_data);
            _item_size_calc(pd, layout_item);
         }
     }
}

static void
_node_unrealize(Efl_Ui_List_Precise_Layouter_Data *pd, Efl_Ui_List_SegArray_Node *node)
{
   Efl_Ui_List_LayoutItem* layout_item;
   Efl_Ui_List_Precise_Layouter_Callback_Data *cb_data;
   Efl_Ui_List_Precise_Layouter_Node_Data *nodedata = node->layout_data;
   int i;

   if (!nodedata->realized)
     return;

   DBG("node_unrealize %d", node->first);
   nodedata->realized = EINA_FALSE;

   for (i = 0; i != node->length; ++i)
     {
       layout_item = (Efl_Ui_List_LayoutItem *)node->pointers[i];
       if (layout_item->layout)
         {
            cb_data = evas_object_event_callback_del(layout_item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change);
            free(cb_data);
         }
       efl_ui_list_model_unrealize(pd->modeler, layout_item);
     }
}

static void
_calc_range(Efl_Ui_List_Precise_Layouter_Data *pd)
{
   Efl_Ui_List_SegArray_Node *node;
   Evas_Coord ow, oh, scr_x, scr_y, ch;
   Efl_Ui_List_Precise_Layouter_Node_Data *nodedata;
   int i;

   elm_interface_scrollable_content_viewport_geometry_get
              (pd->modeler, NULL, NULL, &ow, &oh);
   elm_interface_scrollable_content_pos_get(pd->modeler, &scr_x, &scr_y);

   ch = 0;
   Eina_Accessor *nodes = efl_ui_list_segarray_node_accessor_get(pd->segarray);
   EINA_ACCESSOR_FOREACH(nodes, i, node)
     {
        nodedata = node->layout_data;
//        DBG("node %d h:%d ch:%d scr_y:%d oh:%d", node->first, nodedata->min.h, ch, scr_y, oh);
        if (!nodedata || !nodedata->min.h)
          continue;

        if ((ch >= scr_y || nodedata->min.h + ch >= scr_y) && (ch <= (scr_y + oh) || nodedata->min.h + ch <= scr_y + oh))
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
   Efl_Ui_List_Precise_Layouter_Data *pd;
   Efl_Ui_List_SegArray_Node *node;
   Efl_Ui_List_LayoutItem *layout_item;
   Eo *obj = data;
   Eina_Size2D min;
   int i;
   double start_time = ecore_time_get();

   EINA_SAFETY_ON_NULL_RETURN(data);
   pd = efl_data_scope_get(obj, MY_CLASS);
   if (EINA_UNLIKELY(!pd)) return;

   pd->recalc = EINA_FALSE;

   Eina_Accessor *nodes = efl_ui_list_segarray_node_accessor_get(pd->segarray);
   while (eina_accessor_data_get(nodes, pd->calc_progress, (void **)&node))
     {
        pd->calc_progress++;
        if (!node->layout_data)
          node->layout_data = calloc(1, sizeof(Efl_Ui_List_Precise_Layouter_Node_Data));

        for (i = 0; i != node->length; ++i)
          {
            layout_item = (Efl_Ui_List_LayoutItem *)node->pointers[i];
            EINA_SAFETY_ON_NULL_RETURN(layout_item);

            // cache size of new items
            if ((layout_item->min.w == 0) && (layout_item->min.h == 0))
              {
                if (!layout_item->layout)
                  {
                    //DBG("no layout, realizing");
                    efl_ui_list_model_realize(pd->modeler, layout_item);
                  }

                min = efl_gfx_size_hint_combined_min_get(layout_item->layout);
                if (min.w && min.h)
                  {
                    DBG("size was calculated w:%d h:%d", min.w, min.h);
                    _item_min_calc(pd, layout_item, min, node);
                    efl_ui_list_model_unrealize(pd->modeler, layout_item);
                  }
                else
                  {
                     Efl_Ui_List_Precise_Layouter_Callback_Data *cb_data = calloc(1, sizeof(Efl_Ui_List_Precise_Layouter_Callback_Data));
                     cb_data->pd = pd;
                     cb_data->node = node;
                     evas_object_event_callback_add(layout_item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, cb_data);
                  }
              }
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

   evas_object_smart_changed(pd->modeler);
}

EOLIAN static Efl_Object *
_efl_ui_list_precise_layouter_efl_object_constructor(Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->initialized = EINA_FALSE;
   pd->count_future = NULL;

   return obj;
}

EOLIAN static void
_efl_ui_list_precise_layouter_efl_ui_list_relayout_model_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd, Efl_Model *model)
{
   if (pd->model == model)
     return;

   pd->count_total = 0;
   if (pd->count_future)
     {
        efl_future_cancel(pd->count_future);
        pd->count_future = NULL;
     }

   if (pd->model)
     {
        _finalize(obj, pd);
        efl_unref(pd->model);
        pd->model = NULL;
     }

   if (model)
     {
        pd->model = model;
        efl_ref(pd->model);
        pd->count_future = efl_model_children_count_get(pd->model);
        efl_future_then(pd->count_future, &_count_then, &_count_error, NULL, pd);
     }
}

static void
_efl_ui_list_relayout_layout_do(Efl_Ui_List_Precise_Layouter_Data *pd)
{
   Evas_Coord ow, oh, scr_x, scr_y;
   double cur_pos = 0;
   Efl_Ui_List_LayoutItem* layout_item;
   Efl_Ui_List_SegArray_Node *items_node;
   int i, j = 0;

   _calc_range(pd);

   int boxx, boxy, boxw, boxh, extra = 0, rounding = 0;
   int boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   evas_object_geometry_get(pd->modeler, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(pd->modeler, &boxl, &boxr, &boxt, &boxb);

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

    //padding can not be squeezed (note: could make it an option)
//   int pad;
//   double scale;
//   scale = evas_object_scale_get(pd->modeler);
//   pad = 0;//pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v;
//   length -= pad * (pd->count_total - 1);

   // available space. if <0 we overflow
   extra = boxh - pd->min.h;
   if (extra < 0) extra = 0;

   efl_ui_list_model_min_size_set(pd->modeler, pd->min);

   elm_interface_scrollable_content_viewport_geometry_get(pd->modeler, NULL, NULL, &ow, &oh);
   elm_interface_scrollable_content_pos_get(pd->modeler, &scr_x, &scr_y);

   Eina_Accessor *nodes = efl_ui_list_segarray_node_accessor_get(pd->segarray);
   EINA_ACCESSOR_FOREACH(nodes, i, items_node)
     {
       Efl_Ui_List_Precise_Layouter_Node_Data *nodedata = items_node->layout_data;
       if (!nodedata)
         {
           DBG("no data in node!");
           continue;
         }

//       if(scr_y < cur_pos + nodedata->min.h + boxh
//          && scr_y + boxh + boxh > cur_pos) // start in this node
         if (nodedata->realized)
         {
            //DBG("cur_pos: %d\n", (int)cur_pos);

            for(j = 0; j != items_node->length && scr_y + boxh + boxh > cur_pos;++j)
              {
//                 DBG("cur_pos item by item: %d\n", (int)cur_pos);
                 layout_item = (Efl_Ui_List_LayoutItem *)items_node->pointers[j];
                 double x, y, w, h;
                 double weight_x, weight_y;

                 if(layout_item->min.w && layout_item->min.h)
                   {
//                      DBG("size information for item %d width %d height %d", j, layout_item->min.w, layout_item->min.h);
                      if(!layout_item->layout)
                        {
                           DBG("realizing showing item\n");
                           efl_ui_list_model_realize(pd->modeler, layout_item);
                           assert(layout_item->layout != NULL);
                        }

                      if (pd->resize)
                        _item_size_calc(pd, layout_item);

                      efl_gfx_size_hint_weight_get(layout_item->layout, &weight_x, &weight_y);
                      // extra rounding up (compensate cumulative error)
                      if ((i == (pd->count_total - 1)) && (cur_pos - floor(cur_pos) >= 0.5))
                        rounding = 1;

                      x = layout_item->pos.x;
                      y = layout_item->pos.y + cur_pos;
                      w = layout_item->size.w;
                      h = layout_item->size.h + rounding + weight_y * extra;
                      cur_pos += h;

                      if (w < pd->min.w) w = pd->min.w;
                      if (w > ow) w = ow;

                      //DBG("------- x=%0.f, y=%0.f, w=%0.f, h=%0.f --- ", x, y, w, h);
                      evas_object_geometry_set(layout_item->layout, (x + 0 - scr_x), (y + 0 - scr_y), w, h);
                   } /* if (size) end */
              }
         }
       else if (nodedata->realized) // unrealize
         {
            for(j = 0; j != items_node->length;++j)
              {
                 layout_item = (Efl_Ui_List_LayoutItem *)items_node->pointers[j];
                 if(layout_item->layout)
                   efl_ui_list_model_unrealize(pd->modeler, layout_item);
              }
            nodedata->realized = EINA_FALSE;
            cur_pos += nodedata->min.h;
         }
       else
         {
            cur_pos += nodedata->min.h;
         }
   } /* EINA ACCESSOR FOREACH END */
   eina_accessor_free(nodes);

   pd->resize = EINA_FALSE;
}

EOLIAN static void
_efl_ui_list_precise_layouter_efl_ui_list_relayout_layout_do
  (Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd
   , Efl_Ui_List_Model *modeler, int first, Efl_Ui_List_SegArray *segarray)
{
   _initilize(obj, pd, modeler, segarray);

   pd->first = first;

   if (pd->recalc)
     {
        // cache size of new items
        pd->calc_progress = 0;
        ecore_job_del(pd->calc_job);
        pd->calc_job = ecore_job_add(_calc_size_job, obj);
        return;
     }

   _efl_ui_list_relayout_layout_do(pd);
}

#include "efl_ui_list_precise_layouter.eo.c"
