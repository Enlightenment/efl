#include "efl_ui_table_private.h"
#include "efl_ui_container_layout.h"

typedef struct _Item_Calc Item_Calc;
typedef struct _Cell_Calc Cell_Calc;
typedef struct _Table_Calc Table_Calc;

struct _Item_Calc
{
   Evas_Object *obj;
   int cell_span[2];
   int cell_index[2];
   Efl_Ui_Container_Item_Hints hints[2]; /* 0 is x-axis, 1 is y-axis */
};

struct _Cell_Calc
{
   EINA_INLIST;

   int       index;
   int       next;
   double    acc;
   double    space;
   double    weight;
   double    weight_factor;
   Eina_Bool occupied : 1;
};

struct _Table_Calc
{
   /* 0 is x-axis, 1 is y-axis */

   int                           rows;
   int                           cols;
   int                           want[2];
   int                           hgsize[2];
   double                        weight_sum[2];
   Cell_Calc                    *cell_calc[2];
   Efl_Ui_Container_Layout_Calc  layout_calc[2];
};

static int
_weight_sort_cb(const void *l1, const void *l2)
{
   Cell_Calc *cc1, *cc2;

   cc1 = EINA_INLIST_CONTAINER_GET(l1, Cell_Calc);
   cc2 = EINA_INLIST_CONTAINER_GET(l2, Cell_Calc);

   return cc2->weight_factor <= cc1->weight_factor ? -1 : 1;
}

static void
_cell_weight_calc(Table_Calc *table_calc, Eina_Bool axis)
{
   int i, count, layout_size, calc_size;
   double denom, weight_sum, calc_weight;
   Eina_Inlist *inlist = NULL;
   Cell_Calc *cell_calc, *cc;

   layout_size = calc_size = table_calc->layout_calc[axis].size;
   weight_sum = calc_weight = table_calc->weight_sum[axis];
   cell_calc = table_calc->cell_calc[axis];
   count = axis ? table_calc->rows : table_calc->cols;

   for (i = 0; i < count; i = cell_calc[i].next)
     {
        denom = (cell_calc[i].weight * layout_size) -
                (weight_sum * cell_calc[i].space);
        if (denom > 0)
          {
             cell_calc[i].weight_factor = (cell_calc[i].weight * layout_size) / denom;
             inlist =  eina_inlist_sorted_insert(inlist,
                                                 EINA_INLIST_GET(&cell_calc[i]),
                                                 _weight_sort_cb);
          }
        else
          {
             calc_size -= cell_calc[i].space;
             calc_weight -= cell_calc[i].weight;
          }
     }

   EINA_INLIST_FOREACH(inlist, cc)
     {
        double weight_len;

        weight_len = (calc_size * cc->weight) / calc_weight;
        if (cc->space < weight_len)
          {
             cc->space = weight_len;
          }
        else
          {
             calc_size -= cc->space;
             calc_weight -= cc->weight;
          }
     }
}

static void
_efl_ui_table_homogeneous_cell_init(Table_Calc *table_calc, Eina_Bool axis)
{
   int i, index = 0, mmin = 0, count;
   Cell_Calc *prev_cell = NULL, *cell_calc;

   cell_calc = table_calc->cell_calc[axis];
   count = axis ? table_calc->rows : table_calc->cols;

   for (i = 0; i < count; i++)
     {
        if (!cell_calc[i].occupied) continue;

        cell_calc[i].index = index++;
        if (cell_calc[i].space > mmin)
          mmin = cell_calc[i].space;

        if (prev_cell)
          prev_cell->next = i;

        prev_cell = &cell_calc[i];
     }
   if (prev_cell)
     prev_cell->next = count;

   table_calc->layout_calc[axis].size -= (table_calc->layout_calc[axis].pad
                                          * (index - 1));

   table_calc->want[axis] = mmin * index;
   table_calc->weight_sum[axis] = index;

   if (table_calc->want[axis] > table_calc->layout_calc[axis].size)
     table_calc->hgsize[axis] = table_calc->want[axis] / index;
   else
     table_calc->hgsize[axis] = table_calc->layout_calc[axis].size / index;

   table_calc->hgsize[axis] += table_calc->layout_calc[axis].pad;
}

static void
_efl_ui_table_regular_cell_init(Table_Calc *table_calc, Eina_Bool axis)
{
   int i, index = 0, acc, want = 0, count;
   double weight_sum = 0;
   Cell_Calc *prev_cell = NULL, *cell_calc;
   Efl_Ui_Container_Layout_Calc *layout_calc;

   layout_calc = &(table_calc->layout_calc[axis]);
   cell_calc = table_calc->cell_calc[axis];
   count = axis ? table_calc->rows : table_calc->cols;

   for (i = 0; i < count; i++)
     {
        if (!cell_calc[i].occupied) continue;
        else if (i && cell_calc[0].next == 0) cell_calc[0].next = i;

        cell_calc[i].index = index++;
        want += cell_calc[i].space;
        weight_sum += cell_calc[i].weight;

        if (prev_cell)
          prev_cell->next = i;

        prev_cell = &cell_calc[i];
     }
   if (prev_cell)
     prev_cell->next = count;

   table_calc->want[axis] = want;
   table_calc->weight_sum[axis] = weight_sum;
   table_calc->layout_calc[axis].size -= (table_calc->layout_calc[axis].pad
                                          * (index - 1));

   if ((layout_calc->size > want) && (weight_sum > 0))
     _cell_weight_calc(table_calc, axis);
   if (EINA_DBL_EQ(weight_sum, 0.0))
     layout_calc->pos += (layout_calc->size - want) * layout_calc->align;

   for (i = 0, acc = 0; i < count; acc += cell_calc[i].space, i = cell_calc[i].next)
     cell_calc[i].acc = acc;
}

static inline int
_efl_ui_table_homogeneous_item_pos_get(Table_Calc *table_calc, Item_Calc *item, Eina_Bool axis)
{
   return 0.5 + table_calc->layout_calc[axis].pos + (table_calc->hgsize[axis]
           * table_calc->cell_calc[axis][item->cell_index[axis]].index);
}

static inline int
_efl_ui_table_homogeneous_item_size_get(Table_Calc *table_calc, Item_Calc *item, Eina_Bool axis)
{
   return (table_calc->hgsize[axis] * item->cell_span[axis])
          - table_calc->layout_calc[axis].pad;
}

static inline int
_efl_ui_table_regular_item_pos_get(Table_Calc *table_calc, Item_Calc *item, Eina_Bool axis)
{
   return 0.5 + table_calc->layout_calc[axis].pos
          + table_calc->cell_calc[axis][item->cell_index[axis]].acc
          + (table_calc->cell_calc[axis][item->cell_index[axis]].index *
             table_calc->layout_calc[axis].pad);
}

static inline int
_efl_ui_table_regular_item_size_get(Table_Calc *table_calc, Item_Calc *item, Eina_Bool axis)
{
   int start, end;

   start = item->cell_index[axis];
   end = start + item->cell_span[axis] - 1;

   return table_calc->cell_calc[axis][end].acc
          - table_calc->cell_calc[axis][start].acc
          + table_calc->cell_calc[axis][end].space
          + ((item->cell_span[axis] - 1) * table_calc->layout_calc[axis].pad)
          - item->hints[axis].margin[0] - item->hints[axis].margin[1];
}

void
_efl_ui_table_custom_layout(Efl_Ui_Table *ui_table, Efl_Ui_Table_Data *pd)
{
   Table_Item *ti;
   Item_Calc *items, *item;
   Efl_Ui_Container_Item_Hints *hints;
   int id = 0, i = 0, count, rows, cols;
   int (*_efl_ui_table_item_pos_get[2])(Table_Calc *, Item_Calc *, Eina_Bool);
   int (*_efl_ui_table_item_size_get[2])(Table_Calc *, Item_Calc *, Eina_Bool);
   Table_Calc table_calc;

   count = pd->count;

   if (!count)
     {
        efl_gfx_hint_size_restricted_min_set(ui_table, EINA_SIZE2D(0, 0));
        return;
     }

   _efl_ui_container_layout_init(ui_table, table_calc.layout_calc);

   table_calc.want[0] = table_calc.want[1] = 0;
   table_calc.weight_sum[0] = table_calc.weight_sum[1] = 0;

   efl_pack_table_size_get(ui_table, &cols, &rows);

   table_calc.cell_calc[0] = alloca(cols * sizeof(Cell_Calc));
   table_calc.cell_calc[1] = alloca(rows * sizeof(Cell_Calc));

   memset(table_calc.cell_calc[0], 0, cols * sizeof(Cell_Calc));
   memset(table_calc.cell_calc[1], 0, rows * sizeof(Cell_Calc));

   items = alloca(count * sizeof(*items));
#ifdef DEBUG
   memset(items, 0, count * sizeof(*items));
#endif

   table_calc.cols = cols;
   table_calc.rows = rows;
   // scan all items, get their properties, calculate total weight & min size
   EINA_INLIST_FOREACH(EINA_INLIST_GET(pd->items), ti)
     {
        if (((ti->col + ti->col_span) > cols) ||
            ((ti->row + ti->row_span) > rows))
          {
             efl_gfx_entity_geometry_set(ti->object, EINA_RECT(9999, 9999, 0, 0));
             count--;
             continue;
          }

        item = &items[id++];
        item->obj = ti->object;
        hints = item->hints;

        _efl_ui_container_layout_item_init(item->obj, hints);

        if (table_calc.layout_calc[0].fill || pd->homogeneoush)
          hints[0].weight = 1;
        else if (hints[0].weight < 0)
          hints[0].weight = 0;

        if (table_calc.layout_calc[1].fill || pd->homogeneousv)
          hints[1].weight = 1;
        else if (hints[1].weight < 0)
          hints[1].weight = 0;

        item->cell_index[0] = ti->col;
        item->cell_index[1] = ti->row;
        item->cell_span[0] = ti->col_span;
        item->cell_span[1] = ti->row_span;

        int end;
        double ispace, iweight;

        end = ti->col + ti->col_span;
        ispace = hints[0].space / ti->col_span;
        iweight = hints[0].weight / ti->col_span;
        for (i = ti->col; i < end; i++)
          {
             table_calc.cell_calc[0][i].occupied = EINA_TRUE;

             if (table_calc.cell_calc[0][i].space < ispace)
               table_calc.cell_calc[0][i].space = ispace;
             if (table_calc.cell_calc[0][i].weight < iweight)
               table_calc.cell_calc[0][i].weight = iweight;
          }

        end = ti->row + ti->row_span;
        ispace = hints[1].space / ti->row_span;
        iweight = hints[1].weight / ti->row_span;
        for (i = ti->row; i < end; i++)
          {
             table_calc.cell_calc[1][i].occupied = EINA_TRUE;

             if (table_calc.cell_calc[1][i].space < ispace)
               table_calc.cell_calc[1][i].space = ispace;
             if (table_calc.cell_calc[1][i].weight < iweight)
               table_calc.cell_calc[1][i].weight = iweight;
          }
     }

   if (pd->homogeneoush)
     {
        _efl_ui_table_homogeneous_cell_init(&table_calc, 0);
        _efl_ui_table_item_pos_get[0] = _efl_ui_table_homogeneous_item_pos_get;
        _efl_ui_table_item_size_get[0] = _efl_ui_table_homogeneous_item_size_get;
     }
   else
     {
        _efl_ui_table_regular_cell_init(&table_calc, 0);
        _efl_ui_table_item_pos_get[0] = _efl_ui_table_regular_item_pos_get;
        _efl_ui_table_item_size_get[0] = _efl_ui_table_regular_item_size_get;
     }

   if (pd->homogeneousv)
     {
        _efl_ui_table_homogeneous_cell_init(&table_calc, 1);
        _efl_ui_table_item_pos_get[1] = _efl_ui_table_homogeneous_item_pos_get;
        _efl_ui_table_item_size_get[1] = _efl_ui_table_homogeneous_item_size_get;
     }
   else
     {
        _efl_ui_table_regular_cell_init(&table_calc, 1);
        _efl_ui_table_item_pos_get[1] = _efl_ui_table_regular_item_pos_get;
        _efl_ui_table_item_size_get[1] = _efl_ui_table_regular_item_size_get;
     }

   for (i = 0; i < count; i++)
     {
        Eina_Rect space, item_geom;
        item = &items[i];
        hints = items[i].hints;

        space.x = _efl_ui_table_item_pos_get[0](&table_calc, item, 0);
        space.y = _efl_ui_table_item_pos_get[1](&table_calc, item, 1);
        space.w = _efl_ui_table_item_size_get[0](&table_calc, item, 0);
        space.h = _efl_ui_table_item_size_get[1](&table_calc, item, 1);

        item_geom.w = hints[0].fill ? space.w : hints[0].min;
        item_geom.h = hints[1].fill ? space.h : hints[1].min;

        _efl_ui_container_layout_min_max_calc(hints, &item_geom.w, &item_geom.h,
                                (hints[0].aspect > 0) && (hints[1].aspect > 0));

        item_geom.x = space.x + ((space.w - item_geom.w) * hints[0].align)
                      + hints[0].margin[0];
        item_geom.y = space.y + ((space.h - item_geom.h) * hints[1].align)
                      + hints[1].margin[0];

        efl_gfx_entity_geometry_set(item->obj, item_geom);
     }

   table_calc.want[0] += table_calc.layout_calc[0].margin[0]
                         + table_calc.layout_calc[0].margin[1]
                         + (table_calc.layout_calc[0].pad *
                            table_calc.cell_calc[0][cols - 1].index);

   table_calc.want[1] += table_calc.layout_calc[1].margin[0]
                         + table_calc.layout_calc[1].margin[1]
                         + (table_calc.layout_calc[1].pad *
                            table_calc.cell_calc[1][rows - 1].index);

   efl_gfx_hint_size_restricted_min_set(ui_table,
                                        EINA_SIZE2D(table_calc.want[0],
                                                    table_calc.want[1]));
}
