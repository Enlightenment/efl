#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include <assert.h>

#define MY_CLASS EFL_UI_LIST_PRECISE_LAYOUTER_CLASS

typedef struct _Efl_Ui_List_Precise_Layouter_Data
{
   Eina_Bool initialized;
   Eina_Hash* size_information;
} Efl_Ui_List_Precise_Layouter_Data;

typedef struct _Efl_Ui_List_Precise_Layouter_Size
{
  Evas_Coord min_width, min_height;
  double weight_x, weight_y;
} Efl_Ui_List_Precise_Layouter_Size;

#include "efl_ui_list_precise_layouter.eo.h"

static void
_efl_ui_list_precise_layouter_efl_ui_list_relayout_layout_do
  (Eo *obj EINA_UNUSED, Efl_Ui_List_Precise_Layouter_Data *pd
   , Efl_Ui_List_Model *modeler, int first, int ccount, Eina_Accessor *items)
{
   Efl_Ui_List_LayoutItem* layout_item;
   Efl_Ui_List_Precise_Layouter_Size* size;
   int i;
   /****/
   Eina_Bool horiz = EINA_FALSE/*_horiz(pd->orient)*/, zeroweight = EINA_FALSE;
   Evas_Coord ow, oh, want, minw, minh;
   int boxx, boxy, boxw, boxh, length, /*pad, */extra = 0, rounding = 0;
   int boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   double cur_pos = 0, scale, box_align[2],  weight[2] = { 0, 0 };
   Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE };
   int count = 0;
   Eina_List *order = NULL;
   int pad[4];

   if(!pd->initialized)
     {
       efl_ui_list_model_load_range_set(modeler, 0, -1); // load all
       pd->size_information = eina_hash_pointer_new(&free);
       pd->initialized = EINA_TRUE;
     }

   // cache size of new items
   EINA_ACCESSOR_FOREACH(items, i, layout_item)
     {
        size = eina_hash_find(pd->size_information, &layout_item);
        if(!size)
        {
           size = malloc(sizeof(Efl_Ui_List_Precise_Layouter_Size));
           if(!layout_item->layout)
             efl_ui_list_model_realize(modeler, layout_item);
           if(!layout_item->layout)
             {
               // error
             }

           size = malloc(sizeof(Efl_Ui_List_Precise_Layouter_Size));
           edje_object_size_min_calc(layout_item->layout, &size->min_width, &size->min_height);
           efl_gfx_size_hint_margin_get(layout_item->layout, &pad[0], &pad[1], &pad[2], &pad[3]);
           efl_gfx_size_hint_weight_get(layout_item->layout, &size->weight_x, &size->weight_y);

           size->min_width += pad[0] + pad[1];
           size->min_height += pad[2] + pad[3];

           /* pd->weight.x += item->wx; */
           /* pd->weight.y += item->wy; */
        }
     }

   evas_object_geometry_get(modeler, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(modeler, &boxl, &boxr, &boxt, &boxb);

   scale = evas_object_scale_get(modeler);
/*    // Box align: used if "item has max size and fill" or "no item has a weight" */
/*    // Note: cells always expand on the orthogonal direction */
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

   count = 1;
/*    count = eina_inarray_count(&pd->items.array); */

   elm_interface_scrollable_content_viewport_geometry_get
              (modeler, NULL, NULL, &ow, &oh);
   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   int average_item_size = 10; //eina_inarray_count(&pd->items.array) ? (/*horz*/ EINA_FALSE ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT;
   if(!average_item_size)
     average_item_size = 10;//AVERAGE_SIZE_INIT;

   // total space & available space
   if (horiz)
     {
        int pad;
        length = boxw;
        want = 100;//pd->realized.w;
        //pad = pd->pad.scalable ? (pd->pad.h * scale) : pd->pad.h;
        pad = 1;

        // padding can not be squeezed (note: could make it an option)
        length -= pad * (count - 1);
        // available space. if <0 we overflow
        extra = length - want;

        minw = 100;//pd->realized.w + boxl + boxr + pad * (count - 1);
        minh = 100;//pd->realized.h + boxt + boxb;
        /* if (pd->item_count > count) */
        /*   minw = pd->item_count * average_item_size; */
     }
   else
     {
        int pad;
        length = boxh;
        want = 100;//pd->realized.h;
        pad = 1;//pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v;

        // padding can not be squeezed (note: could make it an option)
        length -= pad * (count - 1);
        // available space. if <0 we overflow
        extra = length - want;

        minw = 100;//pd->realized.w + boxl + boxr;
        minh = 100;//pd->realized.h + pad * (count - 1) + boxt + boxb;
        /* if (pd->item_count > count) */
        /*   minh = pd->item_count * average_item_size; */
     }

   /* if (pd->minh != minh || pd->minw != minw) */
   /*   { */
   /*      pd->minw = minw; */
   /*      pd->minh = minh; */

   /*      efl_event_callback_legacy_call(pd->pan.obj, ELM_PAN_EVENT_CHANGED, NULL); */
   /*   } */

   /* evas_object_size_hint_min_set(wd->resize_obj, pd->minw, pd->minh); */

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
        weight[!horiz] = count;
     }

/*    cur_pos += average_item_size * pd->realized.start; */
/*    // scan all items, get their properties, calculate total weight & min size */
   // cache size of new items
   EINA_ACCESSOR_FOREACH(items, i, layout_item)
     {
/*    EINA_INARRAY_FOREACH(&pd->items.array, it) */
/*      { */
/*         litem = *it; */
        double cx, cy, cw, ch, x, y, w, h;
        double align[2];
        int item_pad[4];
        Eina_Size2D max;
        int pad = 1;
        
        size = eina_hash_find(pd->size_information, &layout_item);

        assert(layout_item->layout != NULL);
        efl_gfx_size_hint_align_get(layout_item->layout, &align[0], &align[1]);
        max = efl_gfx_size_hint_max_get(layout_item->layout);
        efl_gfx_size_hint_margin_get(layout_item->layout, &item_pad[0], &item_pad[1], &item_pad[2], &item_pad[3]);

        if (align[0] < 0) align[0] = -1;
        if (align[1] < 0) align[1] = -1;
        if (align[0] > 1) align[0] = 1;
        if (align[1] > 1) align[1] = 1;

        if (max.w <= 0) max.w = INT_MAX;
        if (max.h <= 0) max.h = INT_MAX;
        if (max.w < size->min_width) max.w = size->min_width;
        if (max.h < size->min_height) max.h = size->min_height;

        /* // extra rounding up (compensate cumulative error) */
        /* if ((id == (count - 1)) && (cur_pos - floor(cur_pos) >= 0.5)) */
        /*   rounding = 1; */

        if (horiz)
          {
             cx = boxx + cur_pos;
             cy = boxy;
             cw = size->min_width + rounding + (zeroweight ? 1.0 : size->weight_x) * extra / weight[0];
             ch = boxh;
             cur_pos += cw + pad;
          }
        else
          {
             cx = boxx;
             cy = boxy + cur_pos;
             cw = boxw;
             ch = size->min_height + rounding + (zeroweight ? 1.0 : size->weight_y) * extra / weight[1];
             cur_pos += ch + pad;
          }

        // horizontally
        if (max.w < INT_MAX)
          {
             w = MIN(MAX(size->min_width - item_pad[0] - item_pad[1], max.w), cw);
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
             w = size->min_width - item_pad[0] - item_pad[1];
             x = cx + ((cw - w) * align[0]) + item_pad[0];
          }

        // vertically
        if (max.h < INT_MAX)
          {
             h = MIN(MAX(size->min_height - item_pad[2] - item_pad[3], max.h), ch);
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
             h = size->min_height - item_pad[2] - item_pad[3];
             y = cy + ((ch - h) * align[1]) + item_pad[2];
          }

        /* if (horiz) */
        /*   { */
        /*      if (h < pd->minh) h = pd->minh; */
        /*      if (h > oh) h = oh; */
        /*   } */
        /* else */
        /*   { */
        /*      if (w < pd->minw) w = pd->minw; */
        /*      if (w > ow) w = ow; */
        /*   } */

        evas_object_geometry_set(layout_item->layout, (x + 0 - /*pd->pan.x*/0), (y + 0 - /*pd->pan.y*/0), w, h);

        /* layout_item->x = x; */
        /* layout_item->y = y; */
/*         order = eina_list_append(order, litem->layout); */

/* //        fprintf(stderr, "x: %.2f y: %.2f w: %.2f h: %.2f old x: %.2f old y: %.2f old w: %.2f old h: %.2f\n" */
/* //                , (x + 0 - pd->pan.x), (y + 0 - pd->pan.y), (float)w, (float)h */
/* //               , (float)litem->x, (float)litem->y, (float)litem->w, (float)litem->h); fflush(stderr); */
/* //        printf("obj=%d currpos=%.2f moved to X=%.2f, Y=%.2f average_item_size %d\n", litem->index, cur_pos, x, y */
/* //               , eina_inarray_count(&pd->items.array) ? (/\*horz*\/ EINA_FALSE ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT); */
     }
  
}

#include "efl_ui_list_precise_layouter.eo.c"
