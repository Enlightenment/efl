#define EFL_GFX_HINT_PROTECTED

#include "efl_ui_box_private.h"

// FIXME: handle RTL? just invert the horizontal order?

typedef struct _Item_Calc Item_Calc;

struct _Item_Calc
{
   EINA_INLIST;

   Evas_Object *obj;
   double weight[2];
   double align[2];
   double space[2];
   double comp_factor;
   Eina_Bool fill[2];
   Eina_Size2D max, min, aspect;
   int pad[4];
   Efl_Gfx_Hint_Aspect aspect_type;
   int id;
};

static int
weight_sort_cb(const void *l1, const void *l2)
{
   Item_Calc *it1, *it2;

   it1 = EINA_INLIST_CONTAINER_GET(l1, Item_Calc);
   it2 = EINA_INLIST_CONTAINER_GET(l2, Item_Calc);

   return it2->comp_factor <= it1->comp_factor ? -1 : 1;
}

static inline void
_min_max_calc(Item_Calc *item, int *cw, int *ch, Eina_Bool aspect_check)
{
   int w = *cw, h = *ch;

   if (aspect_check)
     {
        w = h * item->aspect.w / item->aspect.h;
        if (w > *cw)
          {
             w = *cw;
             h = w * item->aspect.h / item->aspect.w;
          }
     }

   if (w > item->max.w)
     {
        w = item->max.w;
        if (aspect_check) h = w * item->aspect.h / item->aspect.w;
     }
   if (h > item->max.h)
     {
        h = item->max.h;
        if (aspect_check) w = h * item->aspect.w / item->aspect.h;
     }
   if (w < item->min.w)
     {
        w = item->min.w;
        if (aspect_check) h = w * item->aspect.h / item->aspect.w;
     }
   if (h < item->min.h)
     {
        h = item->min.h;
        if (aspect_check) w = h * item->aspect.w / item->aspect.h;
     }
   *cw = w;
   *ch = h;
}

void
_efl_ui_box_custom_layout(Efl_Ui_Box *ui_box, Evas_Object_Box_Data *bd)
{
   Efl_Ui_Box_Data *pd = efl_data_scope_get(ui_box, EFL_UI_BOX_CLASS);
   Evas_Object_Box_Option *opt;
   Evas_Object *o;
   Eina_List *li;
   Eina_Inlist *inlist = NULL;
   int wantw = 0, wanth = 0; // requested size
   Eina_Rect boxs;
   Item_Calc *items, *item;
   Eina_Bool horiz = efl_ui_dir_is_horizontal(pd->dir, EINA_FALSE);
   int id = 0, count, boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   int length, want, pad;
   double cur_pos, weight[2] = { 0, 0 }, scale, mmin = 0;
   double box_align[2];
   Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE };

   boxs = efl_gfx_entity_geometry_get(ui_box);
   efl_gfx_hint_margin_get(ui_box, &boxl, &boxr, &boxt, &boxb);
   scale = efl_gfx_entity_scale_get(ui_box);

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
        efl_gfx_hint_size_min_set(ui_box, EINA_SIZE2D(0, 0));
        return;
     }

   items = alloca(count * sizeof(*items));
#ifdef DEBUG
   memset(items, 0, count * sizeof(*items));
#endif

   // box outer margin
   boxs.w -= boxl + boxr;
   boxs.h -= boxt + boxb;
   boxs.x += boxl;
   boxs.y += boxt;

   // scan all items, get their properties, calculate total weight & min size
   EINA_LIST_FOREACH(bd->children, li, opt)
     {
        item = &items[id];
        o = item->obj = opt->obj;

        efl_gfx_hint_weight_get(o, &item->weight[0], &item->weight[1]);
        efl_gfx_hint_align_get(o, &item->align[0], &item->align[1]);
        efl_gfx_hint_margin_get(o, &item->pad[0], &item->pad[1], &item->pad[2], &item->pad[3]);
        efl_gfx_hint_fill_get(o, &item->fill[0], &item->fill[1]);
        item->max = efl_gfx_hint_size_max_get(o);
        item->min = efl_gfx_hint_size_combined_min_get(o);
        efl_gfx_hint_aspect_get(o, &item->aspect_type, &item->aspect);

        if (horiz && (box_fill[0] || pd->homogeneous)) item->weight[0] = 1;
        else if (item->weight[0] < 0) item->weight[0] = 0;
        if (!horiz && (box_fill[1] || pd->homogeneous)) item->weight[1] = 1;
        else if (item->weight[1] < 0) item->weight[1] = 0;

        if (EINA_DBL_EQ(item->align[0], -1))
          {
             item->align[0] = 0.5;
             item->fill[0] = EINA_TRUE;
          }
        else if (item->align[0] < 0) item->align[0] = 0;
        else if (item->align[0] > 1) item->align[0] = 1;
        if (EINA_DBL_EQ(item->align[1], -1))
          {
             item->align[1] = 0.5;
             item->fill[1] = EINA_TRUE;
          }
        else if (item->align[1] < 0) item->align[1] = 0;
        else if (item->align[1] > 1) item->align[1] = 1;

        if (item->min.w < 0) item->min.w = 0;
        if (item->min.h < 0) item->min.h = 0;

        if (item->max.w < 0) item->max.w = INT_MAX;
        if (item->max.h < 0) item->max.h = INT_MAX;

        weight[0] += item->weight[0];
        weight[1] += item->weight[1];

        if ((item->aspect.w <= 0) || (item->aspect.h <= 0))
          {
             if ((item->aspect.w <= 0) ^ (item->aspect.h <= 0))
               {
                  ERR("Invalid aspect parameter for obj: %p", item->obj);
                  item->aspect.w = item->aspect.h = 0;
                  item->aspect_type = EFL_GFX_HINT_ASPECT_NONE;
               }
          }
        else
          {
             _min_max_calc(item, &item->min.w, &item->min.h, EINA_TRUE);
          }

        item->space[0] = item->min.w + item->pad[0] + item->pad[1];
        item->space[1] = item->min.h + item->pad[2] + item->pad[3];

        if (horiz)
          {
             if (item->space[1] > wanth)
               wanth = item->space[1];
             if (pd->homogeneous)
               {
                  if (item->space[0] > mmin)
                    mmin = item->space[0];
               }
             else
               {
                  wantw += item->space[0];
               }
          }
        else
          {
             if (item->space[0] > wantw)
               wantw = item->space[0];
             if (pd->homogeneous)
               {
                  if (item->space[1] > mmin)
                    mmin = item->space[1];
               }
             else
               {
                  wanth += item->space[1];
               }
          }

        item->id = id++;
     }

   // total space & available space
   if (horiz)
     {
        if (pd->homogeneous)
          wantw = mmin * count;
        want = wantw;
        length = boxs.w;
        pad = pd->pad.scalable ? (pd->pad.h * scale) : pd->pad.h;
        if (boxs.h < wanth)
          boxs.h = wanth;
     }
   else
     {
        if (pd->homogeneous)
          wanth = mmin * count;
        want = wanth;
        length = boxs.h;
        pad = pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v;
        if (boxs.w < wantw)
          boxs.w = wantw;
     }

   // padding can not be squeezed (note: could make it an option)
   length -= pad * (count - 1);
   cur_pos = horiz ? boxs.x : boxs.y;

   // calculate weight length
   if (!pd->homogeneous && (length > want) && (weight[!horiz] > 0))
     {
        int orig_length = length;
        double orig_weight = weight[!horiz];

        for (id = 0; id < count; id++)
          {
             double denom;
             item = &items[id];

             denom = (item->weight[!horiz] * orig_length) -
                     (orig_weight * item->space[!horiz]);
             if (denom > 0)
               {
                  item->comp_factor = (item->weight[!horiz] * orig_length) / denom;
                  inlist = eina_inlist_sorted_insert(inlist, EINA_INLIST_GET(item),
                                                     weight_sort_cb);
               }
             else
               {
                  length -= item->space[!horiz];
                  weight[!horiz] -= item->weight[!horiz];
               }
          }

        EINA_INLIST_FOREACH(inlist, item)
          {
             double weight_len;

             weight_len = (length * item->weight[!horiz]) / weight[!horiz];
             if (item->space[!horiz] < weight_len)
               {
                  item->space[!horiz] = weight_len;
               }
             else
               {
                  weight[!horiz] -= item->weight[!horiz];
                  length -= item->space[!horiz];
               }
          }
     }

   // calculate item geometry
     {
        int x, y, w, h, sw, sh;

        if (length > want)
          {
             if (pd->homogeneous)
               mmin = (double)length / count;
             else if (EINA_DBL_EQ(weight[!horiz], 0))
               cur_pos += (length - want) * box_align[!horiz];
          }

        for (id = 0; id < count; id++)
          {
             item = &items[id];
             if (pd->homogeneous)
               item->space[!horiz] = mmin;
             item->space[horiz] = horiz ? boxs.h : boxs.w;
             sw = item->space[0] - item->pad[0] - item->pad[1];
             sh = item->space[1] - item->pad[2] - item->pad[3];

             if ((item->weight[0] > 0) && item->fill[0])
               w = sw;
             else
               w = 0;

             if ((item->weight[1] > 0) && item->fill[1])
               h = sh;
             else
               h = 0;

             _min_max_calc(item, &w, &h, (item->aspect.w > 0) &&
                                         (item->aspect.h > 0));
             if (horiz)
               {
                  x = cur_pos + 0.5;
                  y = boxs.y;
               }
             else
               {
                  x = boxs.x;
                  y = cur_pos + 0.5;
               }
             x += item->pad[0] + ((sw - w) * item->align[0]);
             y += item->pad[2] + ((sh - h) * item->align[1]);

             cur_pos += item->space[!horiz] + pad;

             efl_gfx_entity_geometry_set(item->obj, EINA_RECT(x, y, w, h));
          }
     }

   if (horiz)
     {
        efl_gfx_hint_size_min_set(ui_box, EINA_SIZE2D(
                                  wantw + boxl + boxr + pad * (count - 1),
                                  wanth + boxt + boxb));
     }
   else
     {
        efl_gfx_hint_size_min_set(ui_box, EINA_SIZE2D(
                                  wantw + boxl + boxr,
                                  wanth + pad * (count - 1) + boxt + boxb));
     }
}
