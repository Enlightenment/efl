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
   Eina_Size2D min;
   Efl_Model* model;
   Efl_Ui_List_Model *modeler;
   Efl_Future *count_future;
   Ecore_Job *calc_job;
   Eina_Accessor *nodes;
   int first;
   unsigned int count;
   unsigned int count_total;
   unsigned int calc_progress;
} Efl_Ui_List_Precise_Layouter_Data;

typedef struct _Efl_Ui_List_Precise_Layouter_Node_Data
{
  Eina_Size2D min;
  Eina_Bool realized;
} Efl_Ui_List_Precise_Layouter_Node_Data;

typedef struct _Efl_Ui_List_Precise_Layouter_Callback_Data
{
  Efl_Ui_List_Precise_Layouter_Data* pd;
  Efl_Ui_List_SegArray_Node* node;
} Efl_Ui_List_Precise_Layouter_Callback_Data;


#include "efl_ui_list_precise_layouter.eo.h"


static void _efl_ui_list_relayout_layout_do(Efl_Ui_List_Precise_Layouter_Data *);
static void _initilize(Eo *, Efl_Ui_List_Precise_Layouter_Data*, Efl_Ui_List_Model*);
static void _finalize(Eo *, Efl_Ui_List_Precise_Layouter_Data*);

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
        pd->min.w = min.w;
        Efl_Ui_List_SegArray_Node *node;
        EINA_ACCESSOR_FOREACH(pd->nodes, i, node)
          {
             Efl_Ui_List_Precise_Layouter_Node_Data *nodedata = node->layout_data;
             if (pd->min.w < nodedata->min.w)
               pd->min.w = nodedata->min.w;

             if (item->min.w == nodedata->min.w)
               break;
          }
     }

   item->min.w = min.w;
   item->min.h = min.h;
}

static void
_count_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Precise_Layouter_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   pd->count_future = NULL;

   pd->count_total = *(int*)((Efl_Future_Event_Success*)event->info)->value;
}

static void
_count_error(void * data, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Precise_Layouter_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   pd->count_future = NULL;
}
/*
static void
_slice_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Precise_Layouter_Data *pd = data;

   if (pd->slice_acc)
     eina_accessor_free(pd->slice_acc);

   pd->slice_acc = (Eina_Accessor*)((Efl_Future_Event_Success*)event->info)->value;

   pd->slice_future = NULL;
}

static void
_slice_error(void * data, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Precise_Layouter_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   pd->slice_future = NULL;
}
*/

static void
_on_item_size_hint_change(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Efl_Ui_List_Precise_Layouter_Callback_Data *cb_data = data;
   Efl_Ui_List_SegArray_Node *node = cb_data->node;
   Efl_Ui_List_LayoutItem *item;
   int i;

   Eina_Size2D min = efl_gfx_size_hint_combined_min_get(obj);

   for (i = 0; i != node->length; ++i)
     {
        item = (Efl_Ui_List_LayoutItem *)node->pointers[i];
        if (item->layout == obj)
          {
             _item_min_calc(cb_data->pd, item, min, node);
             return;
          }
     }
}

static void
_on_modeler_resize(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
//   Efl_Ui_List_Precise_Layouter_Data *pd = data;
//   pd->recalc = EINA_TRUE;
}

static void
_initilize(Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd, Efl_Ui_List_Model *modeler)
{
   if(pd->initialized)
     return;

   pd->recalc = EINA_TRUE;
   pd->initialized = EINA_TRUE;
   evas_object_event_callback_add(modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);
   efl_ui_list_model_load_range_set(modeler, 0, 0); // load all
/*
   if (pd->slice_future)
     efl_future_cancel(pd->slice_future);

   pd->slice_future = efl_model_children_slice_get(pd->model, 0, 0);
   efl_future_then(pd->slice_future, &_slice_then, &_slice_error, NULL, pd);
*/
   pd->min.w = 0;
   pd->min.h = 0;
}

static void
_finalize(Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd)
{
   Efl_Ui_List_Precise_Layouter_Callback_Data *cb_data;
   Efl_Ui_List_LayoutItem *layout_item;
   Eina_Iterator *item_iterator;
   void *data;

   evas_object_event_callback_del_full(pd->modeler, EVAS_CALLBACK_RESIZE, _on_modeler_resize, pd);
/*
   item_iterator = eina_hash_iterator_key_new(pd->size_information);
   while(eina_iterator_next(item_iterator, &data))
     {
        layout_item = *(Efl_Ui_List_LayoutItem **)data;
        cb_data = evas_object_event_callback_del(layout_item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change);
        efl_ui_list_model_unrealize(pd->modeler, layout_item);
        free(cb_data);
     }
   eina_iterator_free(item_iterator);
   eina_hash_free_buckets(pd->size_information);
*/
   pd->modeler = NULL;
   pd->min.w = 0;
   pd->min.h = 0;

   pd->initialized = EINA_FALSE;
}

/*
static void
_calc_range(, Efl_Ui_List_Precise_Layouter_Data *pd, )
{
   Efl_Ui_List_Precise_Layouter_Size* size;
   Efl_Ui_List_Precise_Layouter_Data *pd;
   Efl_Ui_List_LayoutItem* layout_item;
   Eo *obj = data;
   Evas_Coord ow, oh, scr_x, scr_y;

   double start_time = ecore_time_get();

   elm_interface_scrollable_content_viewport_geometry_get
              (pd->modeler, NULL, NULL, &ow, &oh);

   elm_interface_scrollable_content_pos_get(pd->modeler, &scr_x, &scr_y);
}
*/
static void
_calc_size_job(void *data)
{
   Efl_Ui_List_Precise_Layouter_Data *pd;
   Efl_Ui_List_SegArray_Node *node;
   Efl_Ui_List_LayoutItem *layout_item;
   Eo *obj = data;
   int i;
   double start_time = ecore_time_get();

   DBG(" >>>>> CALC JOB <<<<<<< \n");
   EINA_SAFETY_ON_NULL_RETURN(data);
   pd = efl_data_scope_get(obj, MY_CLASS);
   if (EINA_UNLIKELY(!pd)) return;

   pd->recalc = EINA_FALSE;

   while (eina_accessor_data_get(pd->nodes, pd->calc_progress, (void **)&node))
     {
        pd->calc_progress++;
        DBG("node first %d", node->first);
        if (!node->layout_data)
          node->layout_data = calloc(1, sizeof(Efl_Ui_List_Precise_Layouter_Node_Data));

        for (i = 0; i != node->length; ++i)
          {
            layout_item = (Efl_Ui_List_LayoutItem *)node->pointers[i];
            DBG("layout_do first %d count %d", pd->first, pd->count);
            EINA_SAFETY_ON_NULL_RETURN(layout_item);

            // cache size of new items
            if ((layout_item->min.w == 0) && (layout_item->min.h == 0))
              {
                Eina_Bool realized = EINA_FALSE;
                if (!layout_item->layout)
                  {
                    DBG("no layout, realizing");
                    efl_ui_list_model_realize(pd->modeler, layout_item);
                  }
                else
                  {
                    realized = EINA_TRUE;
                    DBG("already realized");
                    /* if(!layout_item->layout) */
                    /*   { */
                    /*     // error */
                    /*     continue; */
                    /*   } */
                  }

                Eina_Size2D min = efl_gfx_size_hint_combined_min_get(layout_item->layout);
                if (min.w && min.h)
                  {
                    //DBG("size was calculated");
                    _item_min_calc(pd, layout_item, min, node);
                  }

//                if (realized)
                  {
                     Efl_Ui_List_Precise_Layouter_Callback_Data *cb_data = calloc(1, sizeof(Efl_Ui_List_Precise_Layouter_Callback_Data));
                     cb_data->pd = pd;
                     cb_data->node = node;
                     evas_object_event_callback_add(layout_item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, cb_data);
                  }
//                else
//                  efl_ui_list_model_unrealize(pd->modeler, layout_item);
              }
          }
        if ( (ecore_time_get() - start_time ) > 0.01 )
          {
            DBG(" <><><> RECALC JOB SPLIT <><><> ");
            ecore_job_del(pd->calc_job);
            pd->calc_job = ecore_job_add(_calc_size_job, obj);
            _efl_ui_list_relayout_layout_do(pd);
            return;
         }
     }
   pd->calc_progress = 0;
   pd->calc_job = NULL;

   _efl_ui_list_relayout_layout_do(pd);
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
        //_initilize(obj, pd, pd->modeler);
     }
}

static void
_efl_ui_list_relayout_layout_do(Efl_Ui_List_Precise_Layouter_Data *pd)
{
   Eina_Bool horiz = EINA_FALSE/*_horiz(pd->orient)*/, zeroweight = EINA_FALSE;
   Evas_Coord ow, oh, want, scr_x, scr_y;
   int boxx, boxy, boxw, boxh, length, pad, extra = 0, rounding = 0;
   int boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   double cur_pos = 0, scale, box_align[2],  weight[2] = { 0, 0 };
   Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE };
   Efl_Ui_List_LayoutItem* layout_item;
   Efl_Ui_List_SegArray_Node *items_node;
   unsigned int i;
   int j = 0;

   evas_object_geometry_get(pd->modeler, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(pd->modeler, &boxl, &boxr, &boxt, &boxb);

   scale = evas_object_scale_get(pd->modeler);
   // Box align: used if "item has max size and fill" or "no item has a weight"
   // Note: cells always expand on the orthogonal direction
   box_align[0] = 0;/*pd->align.h;*/
   box_align[1] = 0;/*pd->align.v;*/
   if (box_align[0] < 0)
     {
        box_fill[0] = EINA_TRUE;
        box_align[0] = 0.5;
     }
   if (box_align[1] < 0)
     {
        box_fill[1] = EINA_TRUE;
        box_align[1] = 0.5;
     }

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   // total space & available space
   if (horiz)
     {
        length = boxw;
        want = pd->min.w;
        pad = 0;//pd->pad.scalable ? (pd->pad.h * scale) : pd->pad.h;
     }
   else
     {
        length = boxh;
        want = pd->min.h;
        pad = 0;//pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v;
     }

   // padding can not be squeezed (note: could make it an option)
   length -= pad * (pd->count_total - 1);
   // available space. if <0 we overflow
   extra = length - want;

   /* Evas_Coord minw = pd->min.w + boxl + boxr; */
   /* Evas_Coord minh = pd->min.h + boxt + boxb; */

   efl_ui_list_model_min_size_set(pd->modeler, pd->min);
   if (extra < 0) extra = 0;

   weight[0] = 1;//pd->weight.x;
   weight[1] = 1;//pd->weight.y;
   if (EINA_DBL_EQ(weight[!horiz], 0))
     {
        if (box_fill[!horiz])
          {
             // box is filled, set all weights to be equal
             zeroweight = EINA_TRUE;
          }
        else
          {
             // move bounding box according to box align
             cur_pos = extra * box_align[!horiz];
          }
        weight[!horiz] = pd->count;
     }

   elm_interface_scrollable_content_viewport_geometry_get
              (pd->modeler, NULL, NULL, &ow, &oh);

   elm_interface_scrollable_content_pos_get(pd->modeler, &scr_x, &scr_y);
   // scan all items, get their properties, calculate total weight & min size
   // cache size of new items
   EINA_ACCESSOR_FOREACH(pd->nodes, i, items_node)
     {
   for(j = 0; j != items_node->length;++j)
     {
        layout_item = (Efl_Ui_List_LayoutItem *)items_node->pointers[j];
        double cx, cy, cw, ch, x, y, w, h;
        double weight_x, weight_y;
        double align[2];
        int item_pad[4];
        Eina_Size2D max;
        int pad = 0;

        if(layout_item->min.w && layout_item->min.h)
          {
//        DBG("size information for item %d width %d height %d", i, size->min.w, size->min.h);

        assert(layout_item->layout != NULL);
        efl_gfx_size_hint_weight_get(layout_item->layout, &weight_x, &weight_y);
        efl_gfx_size_hint_align_get(layout_item->layout, &align[0], &align[1]);
        max = efl_gfx_size_hint_max_get(layout_item->layout);
        efl_gfx_size_hint_margin_get(layout_item->layout, &item_pad[0], &item_pad[1], &item_pad[2], &item_pad[3]);

        if (align[0] < 0) align[0] = -1;
        if (align[1] < 0) align[1] = -1;
        if (align[0] > 1) align[0] = 1;
        if (align[1] > 1) align[1] = 1;

        if (max.w <= 0) max.w = INT_MAX;
        if (max.h <= 0) max.h = INT_MAX;
        if (max.w < layout_item->min.w) max.w = layout_item->min.w;
        if (max.h < layout_item->min.h) max.h = layout_item->min.h;

        // extra rounding up (compensate cumulative error)
        if ((i == (pd->count - 1)) && (cur_pos - floor(cur_pos) >= 0.5))
           rounding = 1;

        if (horiz)
          {
             cx = boxx + cur_pos;
             cy = boxy;
             cw = layout_item->min.w + rounding + (zeroweight ? 1.0 : weight_x) * extra / weight[0];
             ch = boxh;
             cur_pos += cw + pad;
          }
        else
          {
             cx = boxx;
             cy = boxy + cur_pos;
             cw = boxw;
             ch = layout_item->min.h + rounding + (zeroweight ? 1.0 : weight_y) * extra / weight[1];
             cur_pos += ch + pad;
          }

        // horizontally
        if (max.w < INT_MAX)
          {
             w = MIN(MAX(layout_item->min.w - item_pad[0] - item_pad[1], max.w), cw);
             if (align[0] < 0)
               {
                  // bad case: fill+max are not good together
                  x = cx + ((cw - w) * box_align[0]) + item_pad[0];
               }
             else
               x = cx + ((cw - w) * align[0]) + item_pad[0];
          }
        else if (align[0] < 0)
          {
             // fill x
             w = cw - item_pad[0] - item_pad[1];
             x = cx + item_pad[0];
          }
        else
          {
             w = layout_item->min.w - item_pad[0] - item_pad[1];
             x = cx + ((cw - w) * align[0]) + item_pad[0];
          }

        // vertically
        if (max.h < INT_MAX)
          {
             h = MIN(MAX(layout_item->min.h - item_pad[2] - item_pad[3], max.h), ch);
             if (align[1] < 0)
               {
                  // bad case: fill+max are not good together
                  y = cy + ((ch - h) * box_align[1]) + item_pad[2];
               }
             else
               y = cy + ((ch - h) * align[1]) + item_pad[2];
          }
        else if (align[1] < 0)
          {
             // fill y
             h = ch - item_pad[2] - item_pad[3];
             y = cy + item_pad[2];
          }
        else
          {
             h = layout_item->min.h - item_pad[2] - item_pad[3];
             y = cy + ((ch - h) * align[1]) + item_pad[2];
          }

        if (horiz)
          {
             if (h < pd->min.h) h = pd->min.h;
             if (h > oh) h = oh;
          }
        else
          {
             if (w < pd->min.w) w = pd->min.w;
             if (w > ow) w = ow;
          }

//        DBG("------- size_h:%d -- h:%.2f --- ", size->min.h, h);
        evas_object_geometry_set(layout_item->layout, (x + 0 - scr_x), (y + 0 - scr_y), w, h);

        /* layout_item->x = x; */
        /* layout_item->y = y; */
        } /* if (size) end */
        if (pd->calc_progress && i > pd->calc_progress)
          return;
      }
   } /* EINA ACCESSOR FOREACH END */

   if (!pd->calc_progress)
     {
       eina_accessor_free(pd->nodes);
       pd->nodes = NULL;
     }
}

EOLIAN static void
_efl_ui_list_precise_layouter_efl_ui_list_relayout_layout_do
  (Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd
   , Efl_Ui_List_Model *modeler, int first, int count, Eina_Accessor *nodes)
{
   unsigned int pop;
   EINA_SAFETY_ON_NULL_RETURN(nodes);

   _initilize(obj, pd, modeler);

   pd->modeler = modeler;
   pd->first = first;
   pd->count = count;

   if (pd->nodes)
      eina_accessor_free(pd->nodes);
   pd->nodes = nodes;

   if (pd->recalc) // || pd->count_total > pop)
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
