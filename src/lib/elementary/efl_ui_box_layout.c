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
   int max[2];
   int pad[4];
   int want[2];
   Efl_Gfx_Size_Hint_Aspect aspect;
   int aw, ah;
   int min[2];
   int id;
};

#define D(format, args...) WRN("item: %d (%s): " format, id, elm_object_text_get((items[id]).obj), ##args)
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
   Eina_Bool horiz = _horiz(pd->orient), zeroweight = EINA_FALSE;
   int id = 0, count, boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   int length, want, pad, extra = 0, rounding = 0;
   double cur_pos = 0, weight[2] = { 0, 0 }, scale;
   double box_align[2];
   Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE };

   evas_object_geometry_get(ui_box, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(ui_box, &boxl, &boxr, &boxt, &boxb);
   scale = evas_object_scale_get(ui_box);
   WRN("box geo: %d %d %d %d ------------------------------", boxx, boxy, boxw, boxh);

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
        efl_gfx_size_hint_max_get(o, &item->max[0], &item->max[1]);
        efl_gfx_size_hint_combined_min_get(o, &item->want[0], &item->want[1]);
        efl_gfx_size_hint_aspect_get(o, &item->aspect, &item->aw, &item->ah);
        efl_gfx_size_hint_min_get(o, &item->min[0], &item->min[1]);
        D("weight: %1.f %.1f, align: %.1f %.1f, min: %d %d, max: %d %d, aspect: (%d) %d %d", item->weight[0], item->weight[1], item->align[0], item->align[1], item->min[0], item->min[1], item->max[0], item->max[1], item->aspect, item->aw, item->ah);
        if (item->aw == 0 || item->ah == 0)
          {
             item->aw = item->ah = 0;
             item->aspect = EFL_GFX_SIZE_HINT_ASPECT_NONE;
          }

        //aspect ratio support
        //lam sao de tinh chi tinh min + van support aspect ratio???
        if (item->aspect >= EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
          {
             int w1, h1;
             if (horiz)
               {
                  if (item->min[0] > 0)
                    {
                       w1 = item->min[0];
                       h1 = w1 * item->ah / item->aw;
                       if (h1 < item->min[1])
                         {
                            h1 = item->min[1];
                            w1 = h1 * item->aw / item->ah;
                         }
                       D("min: %d %d, w h: %d %d", item->min[0], item->min[1], w1, h1);
                    }
                  else if (item->min[1] > 0)
                    {
                       h1 = item->min[1];
                       w1 = h1 * item->aw / item->ah;
                       if (w1 < item->min[0])
                         {
                            w1 = item->min[0];
                            h1 = w1 * item->ah / item->aw;
                         }
                       D("min: %d %d, w h: %d %d", item->min[0], item->min[1], w1, h1);
                    }
                  else
                    {
                       //no min: keep combined min + aspect
                       if (item->aw < item->ah)
                         {
                            w1 = item->want[0];
                            h1 = w1 * item->ah / item->aw;
                            D("want: %d %d, w h: %d %d", item->want[0], item->want[1], w1, h1);
                         }
                       else
                         {
                            h1 = item->want[1];
                            w1 = h1 * item->aw / item->ah;
                            D("want: %d %d, w h: %d %d", item->want[0], item->want[1], w1, h1);
                         }
                    }
                  if (w1 > 0 || h1 > 0)
                    {
                       item->want[0] = w1;
                       item->want[1] = h1;
                    }
               }
             else
               {
                  //FIXME: add more handling as horiz case
                  w1 = item->min[0];
                  h1 = w1 * item->ah / item->aw;
                  if (h1 < item->min[1])
                    {
                       h1 = item->min[1];
                       w1 = h1 * item->aw / item->ah;
                    }
                  if (w1 > 0 || h1 > 0)
                    {
                       item->want[0] = w1;
                       item->want[1] = h1;
                    }
               }
             //int minh = item->want[0] * item->ah / item->aw;
             //if (item->want[1] < minh)
             //  item->want[1] = minh;
          }

        if (item->weight[0] < 0) item->weight[0] = 0;
        if (item->weight[1] < 0) item->weight[1] = 0;

        if (item->align[0] < 0) item->align[0] = -1;
        if (item->align[1] < 0) item->align[1] = -1;
        if (item->align[0] > 1) item->align[0] = 1;
        if (item->align[1] > 1) item->align[1] = 1;

        if (item->want[0] < 0) item->want[0] = 0;
        if (item->want[1] < 0) item->want[1] = 0;

        if (item->max[0] <= 0) item->max[0] = INT_MAX;
        if (item->max[1] <= 0) item->max[1] = INT_MAX;
        if (item->max[0] < item->want[0]) item->max[0] = item->want[0];
        if (item->max[1] < item->want[1]) item->max[1] = item->want[1];

        item->want[0] += item->pad[0] + item->pad[1];
        item->want[1] += item->pad[2] + item->pad[3];

        weight[0] += item->weight[0];
        weight[1] += item->weight[1];
        if (horiz)
          {
             wantw += item->want[0];
             if (item->want[1] > wanth)
               wanth = item->want[1];
          }
        else
          {
             wanth += item->want[1];
             if (item->want[0] > wantw)
               wantw = item->want[0];
          }

        D("want: %d %d :: %d %d", item->want[0], item->want[1], wantw, wanth);
        item->id = id++;
     }

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;


   //support aspect ratio
   /*id = 0;
   WRN("before: wantw: %d, wanth: %d", wantw, wanth);
   EINA_LIST_FOREACH(bd->children, li, opt)
     {
        item = &items[id];
        if (horiz)
          {
             wantw -= item->want[0];
             if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
               {
                  //item->want[0] = item->min[0];
                  item->want[1] = item->want[0] * item->aw / item->ah;
               }
             else if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_VERTICAL)
               {
                  item->want[1] = wanth;
                  item->want[0] = wanth * item->aw / item->ah;
               }
             else if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_BOTH)
               {
                  item->want[1] = wanth;
                  item->want[0] = wanth * item->aw / item->ah;
               }
             wantw += item->want[0];
          }
        else
          {
             wanth -= item->want[1];
             if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
               {
                  item->want[0] = wantw;
                  item->want[1] = wantw * item->ah / item->aw;
               }
             else if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_VERTICAL)
               {
                  //item->want[1] = item->min[1];
                  item->want[0] = item->want[1] * item->aw / item->ah;
               }
             else if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_BOTH)
               {
                  item->want[0] = wantw;
                  item->want[1] = wantw * item->ah / item->ah;
               }
             wanth += item->want[1];
          }

        id++;
     }
   WRN("after:  wantw: %d, wanth: %d", wantw, wanth);
   */
   //////

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

   /*
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
     }*/

   WRN("total weight: %.1f %.1f, extra: %.1f", weight[0], weight[1], extra);
   if (extra < 0) extra = 0;

   //double cx, cy, cw, ch, x, y, w, h;
   //work on items having aspect ratio first
   /*for (id = 0; id < count; id++)
     {
        item = &items[id];
        if (item->aspect >= EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
          {
             if (horiz)
               {
                  //consider aspect, min, max, fill
                  int old_w = item->want[0];
                  int w1 = item->want[0];
                  int h1 = item->want[1];
                  if (weight[0] > 0)
                    w1 = item->want[0] + extra * item->weight[0] / weight[0];
                  h1 = w1 * item->ah / item->aw;
                  D("w h: %d %d, want: %d %d, extra: %d, weight: %.1f / %.1f", w1, h1, item->want[0], item->want[1], extra, item->weight[0], weight[0]);
                  if (h1 < item->want[1])
                    {
                       h1 = item->want[1];
                       w1 = h1 * item->aw / item->ah;
                       D("w h: %d %d", w1, h1);
                    }
                  if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_BOTH ||
                      item->aspect == EFL_GFX_SIZE_HINT_ASPECT_VERTICAL)
                    {
                       if (h1 > boxh)
                         {
                            h1 = boxh;
                            w1 = h1 * item->aw / item->ah;
                         }
                    }

                  //aspect and fill: aspect does not respect fill
                  //if (item->align[1] < 0) //should check in first place
                  //  {
                  //     h1 = boxh;
                  //     w1 = h1 * item->aw / item->ah;
                  //     D("w, h: %d %d", w1, h1);
                  //  }

                  //aspect and max
                  double mar = 0.0;
                  if ((item->max[0] != INT_MAX) && (item->max[1] != INT_MAX))
                    {
                       mar = item->max[0] / (double)item->max[1];
                       D("mar: %.2f, max: %d %d", mar, item->max[0], item->max[1]);
                       if (item->ah > 0)
                         {
                            double ar = item->aw / (double)item->ah;
                            D("ar: %.2f, mar: %.2f", ar, mar);
                            if (ar < mar)
                              {
                                 if (h1 > item->max[1])
                                   {
                                      h1 = item->max[1];
                                      w1 = h1 * item->aw / item->ah;
                                   }
                                 D("w, h: %d %d", w1, h1);
                              }
                            else
                              {
                                 if (w1 > item->max[0])
                                   {
                                      w1 = item->max[0];
                                      h1 = w1 * item->ah / item->aw;
                                   }
                                 D("w, h: %d %d", w1, h1);
                              }
                         }
                    }
                  item->want[0] = w1;
                  item->want[1] = h1;
                  extra -= item->want[0] - old_w;
                  weight[0] -= item->weight[0];
                  D("horiz: w, h: %d %d, extra: %d", w1, h1, extra);
                  //ch = boxh;
                  //if (ch > item->max[1])
                  //  ch = item->max[1];
                  //cw = ch * item->aw / item->ah;
                  //if (cw > item->max[0])
                  //  {
                  //     cw = item->max[0];
                  //     ch = cw * item->ah / item->aw;
                  //  }
               }
             else
               {
                  int w1, h1;
                  int old_h = item->want[1];

                  h1 = item->want[1] + extra * item->weight[1] / weight[1];
                  w1 = h1 * item->aw / item->ah;

                  if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_BOTH ||
                      item->aspect == EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
                    {
                       if (w1 > boxw)
                         {
                            w1 = boxw;
                            h1 = w1 * item->ah / item->aw;
                         }
                    }
                  D("vertical: w,h: %d %d", w1, h1);
                  item->want[0] = w1;
                  item->want[1] = h1;
                  extra -= item->want[1] - old_h;
                  weight[1] -= item->weight[1];
                  //cw = boxw;
                  //if (cw > item->max[0])
                  //  cw = item->max[0];
                  //ch = cw * item->ah / item->aw;
                  //if (ch > item->max[1])
                  //  {
                  //     ch = item->max[1];
                  //     cw = ch * item->aw / item->ah;
                  //  }
               }
          }
     }*/

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
             cw = item->want[0] + rounding + (zeroweight ? 1.0 : item->weight[0]) * extra / weight[0];
             ch = boxh;
             cur_pos += cw + pad;
          }
        else
          {
             cx = boxx;
             cy = boxy + cur_pos;
             cw = boxw;
             ch = item->want[1] + rounding + (zeroweight ? 1.0 : item->weight[1]) * extra / weight[1];
             cur_pos += ch + pad;
          }

        if (item->aspect >= EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL)
          {
             if (horiz)
               {
                  //consider aspect, min, max, fill
                  //int old_w = item->want[0];
                  int w1 = item->want[0];
                  int h1 = item->want[1];
                  if (weight[0] > 0)
                    w1 = item->want[0] + extra * item->weight[0] / weight[0];
                  h1 = w1 * item->ah / item->aw;
                  D("w h: %d %d, want: %d %d, extra: %d, weight: %.1f / %.1f", w1, h1, item->want[0], item->want[1], extra, item->weight[0], weight[0]);
                  if (h1 < item->want[1]) //how?
                    {
                       h1 = item->want[1];
                       w1 = h1 * item->aw / item->ah;
                       D("w h: %d %d", w1, h1);
                    }
                  if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_BOTH ||
                      item->aspect == EFL_GFX_SIZE_HINT_ASPECT_VERTICAL)
                    {
                       if (h1 > boxh)
                         {
                            h1 = boxh;
                            w1 = h1 * item->aw / item->ah;
                         }
                    }

                  //aspect and fill: aspect does not respect fill
                  /*if (item->align[1] < 0) //should check in first place
                    {
                    h1 = boxh;
                    w1 = h1 * item->aw / item->ah;
                    D("w, h: %d %d", w1, h1);
                    }*/

                  //aspect and max
                  double mar = 0.0;
                  if ((item->max[0] != INT_MAX) && (item->max[1] != INT_MAX))
                    {
                       mar = item->max[0] / (double)item->max[1];
                       D("mar: %.2f, max: %d %d", mar, item->max[0], item->max[1]);
                       if (item->ah > 0)
                         {
                            double ar = item->aw / (double)item->ah;
                            D("ar: %.2f, mar: %.2f", ar, mar);
                            if (ar < mar)
                              {
                                 if (h1 > item->max[1])
                                   {
                                      h1 = item->max[1];
                                      w1 = h1 * item->aw / item->ah;
                                   }
                                 D("w, h: %d %d", w1, h1);
                              }
                            else
                              {
                                 if (w1 > item->max[0])
                                   {
                                      w1 = item->max[0];
                                      h1 = w1 * item->ah / item->aw;
                                   }
                                 D("w, h: %d %d", w1, h1);
                              }
                         }
                    }
                  w = w1 - item->pad[0] - item->pad[1];
                  h = h1 - item->pad[2] - item->pad[3];
                  if (item->align[0] < 0)
                    {
                       x = cx + (cw - w) * 0.5 + item->pad[0];
                    }
                  else
                    {
                       x = cx + (cw - w) * item->align[0] + item->pad[0];
                    }
                  if (item->align[1] < 0)
                    {
                       y = cy + (ch - h) * 0.5 + item->pad[2];
                    }
                  else
                    {
                       y = cy + (ch - h) * item->align[1] + item->pad[2];
                    }
                  //item->want[0] = w1;
                  //item->want[1] = h1;
                  //extra -= item->want[0] - old_w;
                  //weight[0] -= item->weight[0];
                  D("horiz: w, h: %d %d, extra: %d", w1, h1, extra);
               }
             else
               {
                  //FIXME: add handling for vertical case
               }
          }
        else
          {
             // horizontally
             if (item->max[0] < INT_MAX)
               {
                  w = MIN(MAX(item->want[0] - item->pad[0] - item->pad[1], item->max[0]), cw);
                  if (item->align[0] < 0)
                    {
                       // bad case: fill+max are not good together
                       x = cx + ((cw - w) * box_align[0]) + item->pad[0];
                    }
                  else
                    {
                       x = cx + ((cw - w) * item->align[0]) + item->pad[0];
                       D("x: %.1f, cx: %.1f", x, cx);
                    }
               }
             else if (item->align[0] < 0)
               {
                  // fill x
                  w = cw - item->pad[0] - item->pad[1];
                  x = cx + item->pad[0];
                  D("x: %.1f, cx: %.1f", x, cx);
               }
             else
               {
                  //lam sao support align + weight???
                  //w = item->want[0] - item->pad[0] - item->pad[1];
                  if (item->weight[0] > 0)
                    w = cw - item->pad[0] - item->pad[1];
                  else
                    w = item->want[0] - item->pad[0] - item->pad[1];
                  x = cx + ((cw - w) * item->align[0]) + item->pad[0];
                  D("x: %.1f, cx: %.1f", x, cx);
               }

             // vertically
             if (item->max[1] < INT_MAX)
               {
                  h = MIN(MAX(item->want[1] - item->pad[2] - item->pad[3], item->max[1]), ch);
                  if (horiz && item->align[1] < 0)
                    {
                       // bad case: fill+max are not good together
                       y = cy + ((ch - h) * box_align[1]) + item->pad[2];
                    }
                  else
                    y = cy + ((ch - h) * item->align[1]) + item->pad[2];
               }
             else if (item->align[1] < 0)
               {
                  // fill y
                  h = ch - item->pad[2] - item->pad[3];
                  y = cy + item->pad[2];
               }
             else
               {
                  //h = item->want[1] - item->pad[2] - item->pad[3];
                  if (!horiz && item->weight[1] > 0)
                    h = ch - item->pad[2] - item->pad[3];
                  else
                    h = item->want[1] - item->pad[2] - item->pad[3];
                  y = cy + ((ch - h) * item->align[1]) + item->pad[2];
               }
          }

        //DBG("[%2d/%2d] cell: %.0f,%.0f %.0fx%.0f item: %.0f,%.0f %.0fx%.0f",
        //    id, count, cx, cy, cw, ch, x, y, w, h);
        D("geo: %.0f %.0f %.0f %.0f", x, y, w, h);
        evas_object_geometry_set(item->obj, x, y, w, h);
     }

   int hh, hw;
   if (horiz)
     {
        evas_object_size_hint_min_set(ui_box,
                                      wantw + boxl + boxr + pad * (count - 1),
                                      wanth + boxt + boxb);
        hw = wantw + boxl + boxr + pad * (count - 1);
        hh = wanth + boxt + boxb;
     }
   else
     {
        evas_object_size_hint_min_set(ui_box,
                                      wantw + boxl + boxr,
                                      wanth + pad * (count - 1) + boxt + boxb);
        hw = wantw + boxl + boxr;
        hh = wanth + pad * (count - 1) + boxt + boxb;
     }
   WRN("box min: %d %d", hw, hh);
}
