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
        efl_gfx_size_hint_min_get(0, &item->min[0], &item->min[1]);
        if (item->aw == 0 || item->ah == 0)
          {
             item->aw = item->ah = 0;
             item->aspect = EFL_GFX_SIZE_HINT_ASPECT_NONE;
          }

        //aspect ratio support
        //lam sao de tinh chi tinh min + van support aspect ratio???
        //if (item->aspect == EFL_GFX_SIZE_HINT_ASPECT_HORIZONTAL ||
        //    item->aspect == EFL_GFX_SIZE_HINT_ASPECT_BOTH)
          {
             int minh = item->want[0] * item->ah / item->aw;
             if (item->want[1] < minh)
               item->want[1] = minh;
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

   double cx, cy, cw, ch, x, y, w, h;
   //work on items having aspect ratio first
   for (id = 0; id < count; id++)
     {
        item = &items[id];
        if (item->aspect)
          {
             if (horiz)
               {
                  ch = boxh;
                  if (ch > item->max[1])
                    ch = item->max[1];
                  cw = ch * item->aw / item->ah;
                  if (cw > item->max[0])
                    {
                       cw = item->max[0];
                       ch = cw * item->ah / item->aw;
                    }
               }
             else
               {
                  cw = boxw;
                  if (cw > item->max[0])
                    cw = item->max[0];
                  ch = cw * item->ah / item->aw;
                  if (ch > item->max[1])
                    {
                       ch = item->max[1];
                       cw = ch * item->aw / item->ah;
                    }
               }
          }
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
               x = cx + ((cw - w) * item->align[0]) + item->pad[0];
          }
        else if (item->align[0] < 0)
          {
             // fill x
             w = cw - item->pad[0] - item->pad[1];
             x = cx + item->pad[0];
          }
        else
          {
             w = item->want[0] - item->pad[0] - item->pad[1];
             x = cx + ((cw - w) * item->align[0]) + item->pad[0];
          }
        //aspect ratio
        if (item->aspect)
          {
             h = w * item->ah / item->aw;
             if (h > wanth)
               wanth = h;
          }
        /////

        // vertically
        if (item->max[1] < INT_MAX)
          {
             h = MIN(MAX(item->want[1] - item->pad[2] - item->pad[3], item->max[1]), ch);
             if (item->align[1] < 0)
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
             h = item->want[1] - item->pad[2] - item->pad[3];
             y = cy + ((ch - h) * item->align[1]) + item->pad[2];
          }
        if (item->aspect)
          {
             w = h * item->aw / item->ah;
             cur_pos = cur_pos - cw + w;
             if (w > wantw)
               wantw = w;
          }

        //DBG("[%2d/%2d] cell: %.0f,%.0f %.0fx%.0f item: %.0f,%.0f %.0fx%.0f",
        //    id, count, cx, cy, cw, ch, x, y, w, h);
        evas_object_geometry_set(item->obj, x, y, w, h);
     }

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
}
