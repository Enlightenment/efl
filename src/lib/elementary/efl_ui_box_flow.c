#include "efl_ui_box_private.h"
#include "efl_ui_container_layout.h"

#define MY_CLASS EFL_UI_BOX_FLOW_CLASS

typedef struct _Efl_Ui_Box_Flow_Data Efl_Ui_Box_Flow_Data;

struct _Efl_Ui_Box_Flow_Data
{
};

typedef struct _Item_Calc Item_Calc;
typedef struct _Row_Calc Row_Calc;

struct _Item_Calc
{
   EINA_INLIST;

   Evas_Object *obj;
   Row_Calc *row;
   double weight_factor;
   Efl_Ui_Container_Item_Hints hints[2]; /* 0 is x-axis, 1 is y-axis */
};

struct _Row_Calc
{
   EINA_INLIST;

   Evas_Object *obj;
   int item_count;
   int min_sum;
   int hgsize;
   double weight_sum;
   double cross_weight;
   double cross_space;
   double cur_pos;
   double weight_factor;
   Efl_Ui_Container_Item_Hints hint;
};

static int
_item_weight_sort_cb(const void *l1, const void *l2)
{
   Item_Calc *it1, *it2;

   it1 = EINA_INLIST_CONTAINER_GET(l1, Item_Calc);
   it2 = EINA_INLIST_CONTAINER_GET(l2, Item_Calc);

   return it2->weight_factor <= it1->weight_factor ? -1 : 1;
}

static int
_row_weight_sort_cb(const void *l1, const void *l2)
{
   Row_Calc *it1, *it2;

   it1 = EINA_INLIST_CONTAINER_GET(l1, Row_Calc);
   it2 = EINA_INLIST_CONTAINER_GET(l2, Row_Calc);

   return it2->weight_factor <= it1->weight_factor ? -1 : 1;
}

EOLIAN static void
_efl_ui_box_flow_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Box_Flow_Data *pd EINA_UNUSED)
{
   Evas_Object_Box_Data *bd;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   bd = efl_data_scope_get(wd->resize_obj, EVAS_BOX_CLASS);
   Efl_Ui_Box_Data *bcd = efl_data_scope_get(obj, EFL_UI_BOX_CLASS);
   Evas_Object_Box_Option *opt;
   Eina_List *li;
   Eina_Inlist *inlist = NULL;
   Item_Calc *items, *item;
   Row_Calc *rows, *row;
   Efl_Ui_Container_Item_Hints *hints, *hint;
   Eina_Bool axis = !efl_ui_dir_is_horizontal(bcd->dir, EINA_FALSE);
   Eina_Bool c_axis = !axis;
   int want[2] = { 0, 0 };
   int rc = 0, count, i = 0, id, item_last = 0;
   double cur_pos, cross_weight_sum = 0, cross_min_sum = 0, min_sum = 0;
   Efl_Ui_Container_Layout_Calc box_calc[2]; /* 0 is x-axis, 1 is y-axis */

   count = eina_list_count(bd->children);
   if (!count)
     {
        efl_gfx_hint_size_min_set(obj, EINA_SIZE2D(0, 0));
        return;
     }

   _efl_ui_container_layout_init(obj, box_calc);

   items = alloca(count * sizeof(*items));
   rows = alloca(count * sizeof(*rows));
   memset(rows, 0, count * sizeof(*rows));

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

        if ((bcd->homogeneous && !axis) || box_calc[0].fill)
          hints[0].weight = 1;
        else if (hints[0].weight < 0)
          hints[0].weight = 0;

        if ((bcd->homogeneous && axis) || box_calc[1].fill)
          hints[1].weight = 1;
        else if (hints[1].weight < 0)
          hints[1].weight = 0;

        if (want[axis] < hints[axis].space)
          want[axis] = hints[axis].space;

        if (bcd->homogeneous)
          continue;

        if (i == 1)
          {
             min_sum = hints[axis].space;
          }
        else if (box_calc[axis].size < (min_sum + hints[axis].space + box_calc[axis].pad))
          {
             min_sum = hints[axis].space;
             rc++;
          }
        else
          {
             min_sum += (hints[axis].space + box_calc[axis].pad);
          }

        row = &rows[rc];
        item->row = row;

        if (row->cross_weight < hints[c_axis].weight)
          row->cross_weight = hints[c_axis].weight;
        if (row->cross_space < hints[c_axis].space)
          row->cross_space = hints[c_axis].space;
        row->weight_sum += hints[axis].weight;
        row->min_sum += hints[axis].space;
        row->item_count++;
     }

   // initialize homogeneous properties
   if (bcd->homogeneous)
     {
        min_sum = 0;
        for (i = 0; i < count; i++)
          {
             item = &items[i];
             hints = items[i].hints;

             if (i == 0)
               {
                  min_sum = want[axis];
               }
             else if (box_calc[axis].size < (min_sum + want[axis] + box_calc[axis].pad))
               {
                  min_sum = want[axis];
                  rc++;
               }
             else
               {
                  min_sum += (want[axis] + box_calc[axis].pad);
               }

             row = &rows[rc];
             item->row = row;

             if (row->cross_weight < hints[c_axis].weight)
               row->cross_weight = hints[c_axis].weight;
             if (row->cross_space < hints[c_axis].space)
               row->cross_space = hints[c_axis].space;
             row->item_count++;
          }
     }

   // calculate item space of each row
   for (id = 0, i = 0; id <= rc; id++)
     {
        int box_size;

        row = &rows[id];
        row->cur_pos = box_calc[axis].pos;

        box_size = box_calc[axis].size -
                   (box_calc[axis].pad * (row->item_count - 1));
        row->hgsize = box_size / row->item_count;

        cross_min_sum += row->cross_space;
        cross_weight_sum += row->cross_weight;

        if (bcd->homogeneous)
          continue;

        if (row->weight_sum > 0)
          {
             int calc_size;
             double orig_weight = row->weight_sum;

             calc_size = box_size;
             inlist = NULL;

             item_last += row->item_count;
             for (; i < item_last; i++)
               {
                  double denom;
                  hint = &items[i].hints[axis];

                  denom = (hint->weight * box_size) - (orig_weight * hint->space);
                  if (denom > 0)
                    {
                       items[i].weight_factor = (hint->weight * box_size) / denom;
                       inlist = eina_inlist_sorted_insert(inlist, EINA_INLIST_GET(&items[i]),
                                                          _item_weight_sort_cb);

                    }
                  else
                    {
                       calc_size -= hint->space;
                       row->weight_sum -= hint->weight;
                    }
               }

             EINA_INLIST_FOREACH(inlist, item)
               {
                  double weight_len;
                  hint = &item->hints[axis];

                  weight_len = (calc_size * hint->weight) / row->weight_sum;
                  if (hint->space < weight_len)
                    {
                       hint->space = weight_len;
                    }
                  else
                    {
                       row->weight_sum -= hint->weight;
                       calc_size -= hint->space;
                    }
               }
          }
        else if (EINA_DBL_EQ(row->weight_sum, 0))
          {
             row->cur_pos += (box_size - row->min_sum) * box_calc[axis].align;
          }
     }

   // calculate row space
   box_calc[c_axis].size -= (box_calc[c_axis].pad * rc);
   cur_pos = box_calc[c_axis].pos;
   if ((box_calc[c_axis].size > cross_min_sum))
     {
        if (cross_weight_sum > 0)
          {
             int orig_size, calc_size;
             double orig_weight = cross_weight_sum;

             calc_size = orig_size = box_calc[c_axis].size;
             inlist = NULL;

             for (i = 0; i <= rc; i++)
               {
                  double denom;
                  row = &rows[i];

                  denom = (row->cross_weight * orig_size) -
                          (orig_weight * row->cross_space);
                  if (denom > 0)
                    {
                       row->weight_factor = (row->cross_weight * orig_size) / denom;
                       inlist = eina_inlist_sorted_insert(inlist, EINA_INLIST_GET(row),
                                                          _row_weight_sort_cb);

                    }
                  else
                    {
                       calc_size -= row->cross_space;
                       cross_weight_sum -= row->cross_weight;
                    }
               }

             EINA_INLIST_FOREACH(inlist, row)
               {
                  double weight_len;

                  weight_len = (calc_size * row->cross_weight) / cross_weight_sum;
                  if (row->cross_space < weight_len)
                    {
                       row->cross_space = weight_len;
                    }
                  else
                    {
                       cross_weight_sum -= row->cross_weight;
                       calc_size -= row->cross_space;
                    }
               }
          }
        else if (EINA_DBL_EQ(cross_weight_sum, 0))
          {
             cur_pos += (box_calc[c_axis].size - cross_min_sum) * box_calc[c_axis].align;
          }
     }

   // calculate item geometry
   int item_size[2], item_pos[2], sw, sh;

   row = NULL;
   for (i = 0; i < count; i++)
     {
        item = &items[i];
        hints = items[i].hints;

        if (row && (row != item->row))
          cur_pos += row->cross_space + box_calc[c_axis].pad;

        row = item->row;

        if (bcd->homogeneous)
          hints[axis].space = row->hgsize;
        hints[c_axis].space = row->cross_space;
        sw = hints[0].space - (hints[0].margin[0] + hints[0].margin[1]);
        sh = hints[1].space - (hints[1].margin[0] + hints[1].margin[1]);

        item_size[0] = ((hints[0].weight > 0) && hints[0].fill) ? sw : 0;
        item_size[1] = ((hints[1].weight > 0) && hints[1].fill) ? sh : 0;

        _efl_ui_container_layout_min_max_calc(hints, &item_size[0], &item_size[1],
                                (hints[0].aspect > 0) && (hints[1].aspect > 0));

        item_pos[axis] = row->cur_pos + 0.5;
        item_pos[c_axis] = cur_pos + 0.5;

        item_pos[0] += (hints[0].margin[0] +
                        ((sw - item_size[0]) * hints[0].align));
        item_pos[1] += (hints[1].margin[0] +
                        ((sh - item_size[1]) * hints[1].align));

        row->cur_pos += hints[axis].space + box_calc[axis].pad;

        efl_gfx_entity_geometry_set(items[i].obj,
                                    EINA_RECT(item_pos[0], item_pos[1],
                                              item_size[0], item_size[1]));
     }

   want[axis] += (box_calc[axis].margin[0] + box_calc[axis].margin[1]);
   want[c_axis] = (box_calc[c_axis].margin[0] + box_calc[c_axis].margin[1]) +
                  (box_calc[c_axis].pad * rc) + cross_min_sum;

   efl_gfx_hint_size_restricted_min_set(obj, EINA_SIZE2D(want[0], want[1]));

   efl_event_callback_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

#include "efl_ui_box_flow.eo.c"
