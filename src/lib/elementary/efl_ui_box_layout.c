#include "efl_ui_box_private.h"

// FIXME: Aspect support is not implemented
// FIXME: handle RTL? just invert the horizontal order?

typedef struct _Item_Calc Item_Calc;

struct _Item_Calc
{
   Evas_Object *obj;
   double weight[2];
   double align[2];
   int min[2];
   int req[2];
   int max[2];
   int pad[4];
   int want[2];
   int id;
};

void
_efl_ui_box_custom_layout(Efl_Ui_Box *ui_box, Evas_Object_Box_Data *bd)
{
   Efl_Ui_Box_Data *pd = eo_data_scope_get(ui_box, EFL_UI_BOX_CLASS);
   Evas_Object_Box_Option *opt;
   Evas_Object *o;
   Eina_List *li;
   int wantw = 0, wanth = 0; // requested size
   int boxx, boxy, boxw, boxh;
   Item_Calc *items, *item;
   Eina_Bool horiz = _horiz(pd->orient), zeroweight = EINA_FALSE;
   int id = 0, count, boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   int length, want, deficit = 0, pad, extra = 0, rounding = 0;
   double cur_pos = 0, weight = 0, scale;
   double box_align[2];

   evas_object_geometry_get(ui_box, &boxx, &boxy, &boxw, &boxh);
   evas_object_size_hint_padding_get(ui_box, &boxl, &boxr, &boxt, &boxb);
   scale = evas_object_scale_get(ui_box);

   // Box align: used if "item has max size and fill" or "no item has a weight"
   //box_align[0] = bd->align.h;
   //box_align[1] = bd->align.v;
   evas_object_size_hint_align_get(ui_box, &box_align[0], &box_align[1]);

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

        evas_object_size_hint_weight_get(o, &item->weight[0], &item->weight[1]);
        if (item->weight[0] < 0) item->weight[0] = 0;
        if (item->weight[1] < 0) item->weight[1] = 0;

        evas_object_size_hint_align_get(o, &item->align[0], &item->align[1]);
        if (item->align[0] < 0) item->align[0] = -1;
        if (item->align[1] < 0) item->align[1] = -1;
        if (item->align[0] > 1) item->align[0] = 1;
        if (item->align[1] > 1) item->align[1] = 1;

        evas_object_size_hint_padding_get(o, &item->pad[0], &item->pad[1], &item->pad[2], &item->pad[3]);
        evas_object_size_hint_min_get(o, &item->min[0], &item->min[1]);
        if (item->min[0] < 0) item->min[0] = 0;
        if (item->min[1] < 0) item->min[1] = 0;

        evas_object_size_hint_max_get(o, &item->max[0], &item->max[1]);
        if (item->max[0] <= 0) item->max[0] = INT_MAX;
        if (item->max[1] <= 0) item->max[1] = INT_MAX;
        if (item->max[0] < item->min[0]) item->max[0] = item->min[0];
        if (item->max[1] < item->min[1]) item->max[1] = item->min[1];

        evas_object_size_hint_request_get(o, &item->req[0], &item->req[1]);
        if (item->req[0] < 0) item->req[0] = 0;
        if (item->req[1] < 0) item->req[1] = 0;

        item->want[0] = MAX(item->req[0], item->min[0]) + item->pad[0] + item->pad[1];
        item->want[1] = MAX(item->req[1], item->min[1]) + item->pad[2] + item->pad[3];

        if (horiz)
          {
             weight += item->weight[0];
             wantw += item->want[0];
             if (item->want[1] > wanth)
               wanth = item->want[1];
          }
        else
          {
             weight += item->weight[1];
             wanth += item->want[1];
             if (item->want[0] > wantw)
               wantw = item->want[0];
          }

        item->id = id++;
     }

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;

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

   if (extra < 0)
     {
        // note: deficit unused
        deficit = (-extra);
        extra = 0;
     }

   if (!weight)
     {
        double balign = box_align[!horiz];
        if (balign < 0)
          {
             // box is filled, set all weights to be equal
             zeroweight = EINA_TRUE;
          }
        else
          {
             // move bounding box according to box align
             cur_pos = extra * balign;
          }
        weight = count;
     }

   // reset box_align to 0.5 if filled (only used by items with max size)
   if (box_align[0] < 0) box_align[0] = 0.5;
   if (box_align[1] < 0) box_align[1] = 0.5;

   for (id = 0; id < count; id++)
     {
        double cx, cy, cw, ch, x, y, w, h;
        item = &items[id];

        // extra rounding up (compensate cumulative error)
        if ((item->id == (count - 1)) && (cur_pos - floor(cur_pos) >= 0.5))
          rounding = 1;

        if (horiz)
          {
             cx = boxx + boxl + cur_pos;
             cy = boxy + boxt;
             cw = item->want[0] + rounding + (zeroweight ? 1.0 : item->weight[0]) * extra / weight;
             ch = boxh;
             cur_pos += cw + pad;
          }
        else
          {
             cx = boxx + boxl;
             cy = boxy + boxt + cur_pos;
             cw = boxw;
             ch = item->want[1] + rounding + (zeroweight ? 1.0 : item->weight[1]) * extra / weight;
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

        //DBG("[%2d/%2d] cell: %.0f,%.0f %.0fx%.0f item: %.0f,%.0f %.0fx%.0f",
        //    id, count, cx, cy, cw, ch, x, y, w, h);
        evas_object_geometry_set(item->obj, x, y, w, h);
     }
}

EOLIAN static void
_efl_ui_box_flow_efl_pack_engine_layout_do(Eo_Class *klass EINA_UNUSED,
                                           void *_pd EINA_UNUSED,
                                           Eo *obj, const void *data)
{
   void (*func)(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data);
   const Efl_Ui_Box_Flow_Params *params = data;
   double ax = 0.5, ay = 0.5;
   Evas_Object_Box_Data *bd;
   Efl_Ui_Box_Data *pd;
   Eina_Bool homo = EINA_FALSE, maxsize = EINA_FALSE;

   EINA_SAFETY_ON_FALSE_RETURN(eo_isa(obj, EFL_UI_BOX_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   bd = eo_data_scope_get(wd->resize_obj, EVAS_BOX_CLASS);
   pd = eo_data_scope_get(obj, EFL_UI_BOX_CLASS);

   // FIXME: I wonder how this data will work with bindings? (lifetime, etc...)
   if (params)
     {
        ax = params->align_x;
        ay = params->align_y;
        homo = params->homogenous;
        maxsize = params->max_size;
     }

   if (_horiz(pd->orient))
     {
        if (homo)
          {
             if (maxsize)
               func = evas_object_box_layout_homogeneous_max_size_horizontal;
             else
               func = evas_object_box_layout_homogeneous_horizontal;
          }
        else
          func = evas_object_box_layout_flow_horizontal;
     }
   else
     {
        if (homo)
          {
             if (maxsize)
               func = evas_object_box_layout_homogeneous_max_size_vertical;
             else
               func = evas_object_box_layout_homogeneous_vertical;
          }
        else
          func = evas_object_box_layout_flow_vertical;
     }

   func(wd->resize_obj, bd, NULL);
}

#include "efl_ui_box_flow.eo.c"
