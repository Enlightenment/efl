#define EFL_GFX_HINT_PROTECTED

#include "efl_ui_box_private.h"
#include "efl_ui_container_layout.h"

// FIXME: handle RTL? just invert the horizontal order?

typedef struct _Item_Calc Item_Calc;

struct _Item_Calc
{
   EINA_INLIST;

   Evas_Object *obj;
   double weight_factor;
   Efl_Ui_Container_Item_Hints hints[2]; /* 0 is x-axis, 1 is y-axis */
};

static int
_weight_sort_cb(const void *l1, const void *l2)
{
   Item_Calc *it1, *it2;

   it1 = EINA_INLIST_CONTAINER_GET(l1, Item_Calc);
   it2 = EINA_INLIST_CONTAINER_GET(l2, Item_Calc);

   return it2->weight_factor <= it1->weight_factor ? -1 : 1;
}

void
_efl_ui_box_custom_layout(Efl_Ui_Box *ui_box, Evas_Object_Box_Data *bd)
{
   Efl_Ui_Box_Data *pd = efl_data_scope_get(ui_box, EFL_UI_BOX_CLASS);
   Evas_Object_Box_Option *opt;
   Eina_List *li;
   Eina_Inlist *inlist = NULL;
   Item_Calc *items, *item;
   Efl_Ui_Container_Item_Hints *hints, *hint;
   Eina_Bool axis = !efl_ui_dir_is_horizontal(pd->dir, EINA_FALSE);
   Eina_Bool r_axis = !axis;
   int want[2] = { 0, 0 };
   int count, i = 0;
   double cur_pos, mmin = 0, weight_sum = 0;
   Efl_Ui_Container_Layout_Calc box_calc[2]; /* 0 is x-axis, 1 is y-axis */


   count = eina_list_count(bd->children);
   if (!count)
     {
        efl_gfx_hint_size_min_set(ui_box, EINA_SIZE2D(0, 0));
        return;
     }

   _efl_ui_container_layout_init(ui_box, box_calc);

   items = alloca(count * sizeof(*items));
#ifdef DEBUG
   memset(items, 0, count * sizeof(*items));
#endif

   // scan all items, get their properties, calculate total weight & min size
   EINA_LIST_FOREACH(bd->children, li, opt)
     {
        item = &items[i++];
        item->obj = opt->obj;
        hints = item->hints;

        _efl_ui_container_layout_item_init(item->obj, hints);

        if (pd->homogeneous || box_calc[0].fill)
          hints[0].weight = 1;
        else if (hints[0].weight < 0)
          hints[0].weight = 0;

        if (pd->homogeneous || box_calc[1].fill)
          hints[1].weight = 1;
        else if (hints[1].weight < 0)
          hints[1].weight = 0;

        weight_sum += hints[axis].weight;

        if (hints[r_axis].space > want[r_axis])
          want[r_axis] = hints[r_axis].space;

        if (pd->homogeneous)
          {
             if (hints[axis].space > mmin)
               mmin = hints[axis].space;
          }
        else
          {
             want[axis] += hints[axis].space;
          }
     }

   // total space & available space
   if (pd->homogeneous)
     want[axis] = mmin * count;

   if (box_calc[r_axis].size < want[r_axis])
     box_calc[r_axis].size = want[r_axis];

   // padding can not be squeezed (note: could make it an option)
   box_calc[axis].size -= (box_calc[axis].pad * (count - 1));
   box_calc[r_axis].pad = 0;
   cur_pos = box_calc[axis].pos;

   // calculate weight size
   if (!pd->homogeneous && (box_calc[axis].size > want[axis]) && (weight_sum > 0))
     {
        int orig_size, calc_size;
        double orig_weight = weight_sum;

        calc_size = orig_size = box_calc[axis].size;

        for (i = 0; i < count; i++)
          {
             double denom;
             hint = &items[i].hints[axis];

             denom = (hint->weight * orig_size) - (orig_weight * hint->space);
             if (denom > 0)
               {
                  items[i].weight_factor = (hint->weight * orig_size) / denom;
                  inlist = eina_inlist_sorted_insert(inlist, EINA_INLIST_GET(&items[i]),
                                                     _weight_sort_cb);

               }
             else
               {
                  calc_size -= hint->space;
                  weight_sum -= hint->weight;
               }
          }

        EINA_INLIST_FOREACH(inlist, item)
          {
             double weight_len;
             hint = &item->hints[axis];

             weight_len = (calc_size * hint->weight) / weight_sum;
             if (hint->space < weight_len)
               {
                  hint->space = weight_len;
               }
             else
               {
                  weight_sum -= hint->weight;
                  calc_size -= hint->space;
               }
          }
     }

   // calculate item geometry
     {
        int item_size[2], item_pos[2], sw, sh;

        if (box_calc[axis].size > want[axis])
          {
             if (pd->homogeneous)
               mmin = (double)box_calc[axis].size / count;
             else if (EINA_DBL_EQ(weight_sum, 0))
               cur_pos += (box_calc[axis].size - want[axis]) * box_calc[axis].align;
          }

        for (i = 0; i < count; i++)
          {
             hints = items[i].hints;

             if (pd->homogeneous)
               hints[axis].space = mmin;
             hints[r_axis].space = box_calc[r_axis].size;
             sw = hints[0].space - (hints[0].margin[0] + hints[0].margin[1]);
             sh = hints[1].space - (hints[1].margin[0] + hints[1].margin[1]);

             item_size[0] = ((hints[0].weight > 0) && hints[0].fill) ? sw : 0;
             item_size[1] = ((hints[1].weight > 0) && hints[1].fill) ? sh : 0;

             _efl_ui_container_layout_min_max_calc(hints, &item_size[0], &item_size[1],
                                (hints[0].aspect > 0) && (hints[1].aspect > 0));

             item_pos[axis] = cur_pos + 0.5;
             item_pos[r_axis] = box_calc[r_axis].pos;

             item_pos[0] += (hints[0].margin[0] +
                             ((sw - item_size[0]) * hints[0].align));
             item_pos[1] += (hints[1].margin[0] +
                             ((sh - item_size[1]) * hints[1].align));

             cur_pos += hints[axis].space + box_calc[axis].pad;

             efl_gfx_entity_geometry_set(items[i].obj,
                                         EINA_RECT(item_pos[0], item_pos[1],
                                                   item_size[0], item_size[1]));
          }
     }
   want[0] += (box_calc[0].margin[0] + box_calc[0].margin[1]) +
              (box_calc[0].pad * (count - 1));
   want[1] += (box_calc[1].margin[0] + box_calc[1].margin[1]) +
              (box_calc[1].pad * (count - 1));

   efl_gfx_hint_size_min_set(ui_box, EINA_SIZE2D(want[0], want[1]));
}
