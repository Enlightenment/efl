#define EFL_GFX_SIZE_HINT_PROTECTED

#include "efl_ui_box_private.h"

// FIXME: Aspect support is not implemented
// FIXME: handle RTL? just invert the horizontal order?

typedef struct _Item_Calc Item_Calc;

struct _Item_Calc
{
   Evas_Object *obj;
   double weight[2];
   double align[2];
   Eina_Bool fill[2];
   Eina_Size2D max, want, aspect;
   int pad[4];
   Efl_Gfx_Size_Hint_Aspect aspect_type;
   int id;
};

void
_efl_ui_box_custom_layout(Efl_Ui_Box *ui_box, Evas_Object_Box_Data *bd)
{
   Efl_Ui_Box_Data *pd = efl_data_scope_get(ui_box, EFL_UI_BOX_CLASS);
   Evas_Object_Box_Option *opt;
   Evas_Object *o;
   Eina_List *li;
   int wantw = 0, wanth = 0; // requested size
   int boxx, boxy, boxw, boxh;
   Item_Calc *items, *item;
   Eina_Bool horiz = efl_ui_dir_is_horizontal(pd->dir, EINA_FALSE);
   Eina_Bool zeroweight = EINA_FALSE;
   int id = 0, count, boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   int length, want, pad, extra = 0, rounding = 0;
   double cur_pos = 0, weight[2] = { 0, 0 }, scale;
   double box_align[2];
   Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE };

   evas_object_geometry_get(ui_box, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(ui_box, &boxl, &boxr, &boxt, &boxb);
   scale = evas_object_scale_get(ui_box);

   // Box align: used if "item has max size and fill" or "no item has a weight"
   // Note: cells always expand on the orthogonal direction
   box_align[0] = pd->align.h;
   box_align[1] = pd->align.v;
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

   count = eina_list_count(bd->children);
   if (!count)
     {
        evas_object_size_hint_min_set(ui_box, 0, 0);
        return;
     }

   items = alloca(count * sizeof(*items));
#ifdef DEBUG
   memset(items, 0, count * sizeof(*items));
#endif

   // scan all items, get their properties, calculate total weight & min size
   EINA_LIST_FOREACH(bd->children, li, opt)
     {
        item = &items[id];
        o = item->obj = opt->obj;

        efl_gfx_size_hint_weight_get(o, &item->weight[0], &item->weight[1]);
        efl_gfx_size_hint_align_get(o, &item->align[0], &item->align[1]);
        efl_gfx_size_hint_margin_get(o, &item->pad[0], &item->pad[1], &item->pad[2], &item->pad[3]);
        efl_gfx_size_hint_fill_get(o, &item->fill[0], &item->fill[1]);
        item->max = efl_gfx_size_hint_max_get(o);
        item->want = efl_gfx_size_hint_combined_min_get(o);
        efl_gfx_size_hint_aspect_get(o, &item->aspect_type, &item->aspect);

        if (item->weight[0] < 0) item->weight[0] = 0;
        if (item->weight[1] < 0) item->weight[1] = 0;

        if (EINA_DBL_EQ(item->align[0], -1))
          {
             item->align[0] = 0.5;
             item->fill[0] = EINA_TRUE;
          }
        else if (item->align[0] < 0)
          {
             item->align[0] = 0;
          }
        if (EINA_DBL_EQ(item->align[1], -1))
          {
             item->align[1] = 0.5;
             item->fill[1] = EINA_TRUE;
          }
        else if (item->align[1] < 0)
          {
             item->align[1] = 0;
          }
        if (item->align[0] > 1) item->align[0] = 1;
        if (item->align[1] > 1) item->align[1] = 1;

        if (item->want.w < 0) item->want.w = 0;
        if (item->want.h < 0) item->want.h = 0;

        if (item->max.w < 0) item->max.w = INT_MAX;
        if (item->max.h < 0) item->max.h = INT_MAX;

        if (item->aspect.w <= 0 || item->aspect.h <= 0)
          {
             if (item->aspect_type >= EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
               ERR("Invalid aspect parameter for obj: %p", item->obj);
             item->aspect.w = item->aspect.h = 0;
             item->aspect_type = EFL_GFX_SIZE_HINT_ASPECT_NONE;
          }

        if (item->aspect_type >= EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
          {
             double w, h;

             w = item->want.w;
             h = w * item->aspect.h / item->aspect.w;
             if (h < item->want.h)
               {
                  h = item->want.h;
                  w = h * item->aspect.w / item->aspect.h;
               }

             if (horiz)
               {
                  if (item->fill[1] && (h < boxh))
                    {
                       double w1, h1;
                       h1 = item->max.h > 0 ? MIN(boxh, item->max.h) : boxh;
                       h1 = MAX(h, h1);
                       w1 = h1 * item->aspect.w / item->aspect.h;
                       w = item->max.w > 0 ? MIN(w1, item->max.w) : w1;
                    }
               }
             else
               {
                  if (item->fill[0] && (w < boxw))
                    {
                       double w1, h1;
                       w1 = item->max.w > 0 ? MIN(boxw, item->max.w) : boxw;
                       w1 = MAX(w, w1);
                       h1 = w1 * item->aspect.h / item->aspect.w;
                       h = item->max.h > 0 ? MIN(h1, item->max.h) : h1;
                    }
               }
             item->want.w = w;
             item->want.h = h;
          }
        if (item->max.w < item->want.w) item->max.w = item->want.w;
        if (item->max.h < item->want.h) item->max.h = item->want.h;

        item->want.w += item->pad[0] + item->pad[1];
        item->want.h += item->pad[2] + item->pad[3];

        weight[0] += item->weight[0];
        weight[1] += item->weight[1];
        if (horiz)
          {
             wantw += item->want.w;
             if (item->want.h > wanth)
               wanth = item->want.h;
          }
        else
          {
             wanth += item->want.h;
             if (item->want.w > wantw)
               wantw = item->want.w;
          }

        item->id = id++;
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
        want = wantw;
        pad = pd->pad.scalable ? (pd->pad.h * scale) : pd->pad.h;
     }
   else
     {
        length = boxh;
        want = wanth;
        pad = pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v;
     }

   // padding can not be squeezed (note: could make it an option)
   length -= pad * (count - 1);

   // available space. if <0 we overflow
   extra = length - want;

   if (horiz)
     {
        evas_object_size_hint_min_set(ui_box,
                                      wantw + boxl + boxr + pad * (count - 1),
                                      wanth + boxt + boxb);
     }
   else
     {
        evas_object_size_hint_min_set(ui_box,
                                      wantw + boxl + boxr,
                                      wanth + pad * (count - 1) + boxt + boxb);
     }

   if (extra < 0) extra = 0;

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

   for (id = 0; id < count; id++)
     {
        double cx, cy, cw, ch, x, y, w, h;
        item = &items[id];

        // extra rounding up (compensate cumulative error)
        if ((item->id == (count - 1)) && (cur_pos - floor(cur_pos) >= 0.5))
          rounding = 1;

        if (horiz)
          {
             cx = boxx + cur_pos;
             cy = boxy;
             cw = item->want.w + rounding + (zeroweight ? 1.0 : item->weight[0]) * extra / weight[0];
             ch = boxh;
             cur_pos += cw + pad;
          }
        else
          {
             cx = boxx;
             cy = boxy + cur_pos;
             cw = boxw;
             ch = item->want.h + rounding + (zeroweight ? 1.0 : item->weight[1]) * extra / weight[1];
             cur_pos += ch + pad;
          }

        if (item->aspect_type >= EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
          {
             if (horiz)
               {
                  w = item->want.w;
                  h = item->want.h;
                  if (weight[0] > 0)
                    w = item->want.w + extra * item->weight[0] / weight[0];
                  h = w * item->aspect.h / item->aspect.w;

                  if (item->aspect_type == EFL_GFX_SIZE_HINT_ASPECT_BOTH ||
                      item->aspect_type == EFL_GFX_SIZE_HINT_ASPECT_VERTICAL)
                    {
                       if (h > boxh)
                         {
                            h = boxh;
                            w = h * item->aspect.w / item->aspect.h;
                         }
                    }
               }
             else
               {
                  w = item->want.w;
                  h = item->want.h;
                  if (weight[1] > 0)
                    h = item->want.h + extra * item->weight[1] / weight[1];
                  w = h * item->aspect.w / item->aspect.h;

                  if (item->aspect_type == EFL_GFX_SIZE_HINT_ASPECT_BOTH ||
                      item->aspect_type == EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
                    {
                       if (w > boxw)
                         {
                            w = boxw;
                            h = w * item->aspect.h / item->aspect.w;
                         }
                    }
               }

             if ((item->max.w != INT_MAX) && (item->max.h != INT_MAX))
               {
                  double mar, ar;
                  mar = item->max.w / (double)item->max.h;
                  ar = item->aspect.w / (double)item->aspect.h;
                  if (ar < mar)
                    {
                       if (h > item->max.h)
                         {
                            h = item->max.h;
                            w = h * item->aspect.w / item->aspect.h;
                         }
                    }
                  else
                    {
                       if (w > item->max.w)
                         {
                            w = item->max.w;
                            h = w * item->aspect.h / item->aspect.w;
                         }
                    }
               }
             else if (item->max.w != INT_MAX)
               {
                  w = MIN(w, MAX(item->want.w, item->max.w));
                  h = w * item->aspect.h / item->aspect.w;
               }
             else
               {
                  h = MIN(h, MAX(item->want.h, item->max.h));
                  w = h * item->aspect.w / item->aspect.h;
               }
             w = w - item->pad[0] - item->pad[1];
             h = h - item->pad[2] - item->pad[3];

             if (item->fill[0])
               x = cx + (cw - w) * 0.5 + item->pad[0];
             else
               x = cx + (cw - w) * item->align[0] + item->pad[0];

             if (item->fill[1])
               y = cy + (ch - h) * 0.5 + item->pad[2];
             else
               y = cy + (ch - h) * item->align[1] + item->pad[2];
          }
        else
          {
             // horizontally
             if (item->max.w < INT_MAX)
               {
                  w = MIN(MAX(item->want.w - item->pad[0] - item->pad[1], item->max.w), cw);
                  x = cx + ((cw - w) * item->align[0]) + item->pad[0];
               }
             else if (item->fill[0])
               {
                  // fill x
                  w = cw - item->pad[0] - item->pad[1];
                  x = cx + item->pad[0];
               }
             else
               {
                  if (horiz && item->weight[0] > 0)
                    w = cw - item->pad[0] - item->pad[1];
                  else
                    w = item->want.w - item->pad[0] - item->pad[1];
                  x = cx + ((cw - w) * item->align[0]) + item->pad[0];
               }

             // vertically
             if (item->max.h < INT_MAX)
               {
                  h = MIN(MAX(item->want.h - item->pad[2] - item->pad[3], item->max.h), ch);
                  y = cy + ((ch - h) * item->align[1]) + item->pad[2];
               }
             else if (item->fill[1])
               {
                  // fill y
                  h = ch - item->pad[2] - item->pad[3];
                  y = cy + item->pad[2];
               }
             else
               {
                  if (!horiz && item->weight[1] > 0)
                    h = ch - item->pad[2] - item->pad[3];
                  else
                    h = item->want.h - item->pad[2] - item->pad[3];
                  y = cy + ((ch - h) * item->align[1]) + item->pad[2];
               }
          }

        //DBG("[%2d/%2d] cell: %.0f,%.0f %.0fx%.0f item: %.0f,%.0f %.0fx%.0f",
        //    id, count, cx, cy, cw, ch, x, y, w, h);
        evas_object_geometry_set(item->obj, x, y, w, h);
     }
}
