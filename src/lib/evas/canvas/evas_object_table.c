#include "evas_common.h"
#include "evas_private.h"
#include <errno.h>

#include <Eo.h>

EAPI Eo_Op EVAS_OBJ_TABLE_BASE_ID = EO_NOOP;

#define MY_CLASS EVAS_OBJ_TABLE_CLASS

#define MY_CLASS_NAME "Evas_Object_Table"

typedef struct _Evas_Object_Table_Data       Evas_Object_Table_Data;
typedef struct _Evas_Object_Table_Option     Evas_Object_Table_Option;
typedef struct _Evas_Object_Table_Cache      Evas_Object_Table_Cache;
typedef struct _Evas_Object_Table_Iterator   Evas_Object_Table_Iterator;
typedef struct _Evas_Object_Table_Accessor   Evas_Object_Table_Accessor;

struct _Evas_Object_Table_Option
{
   Evas_Object *obj;
   unsigned short col, row, colspan, rowspan, end_col, end_row;
   struct {
      Evas_Coord w, h;
   } min, max;
   struct {
      double h, v;
   } align;
   struct {
      Evas_Coord l, r, t, b;
   } pad;
   Eina_Bool expand_h : 1; /* XXX required? */
   Eina_Bool expand_v : 1; /* XXX required? */
   Eina_Bool fill_h : 1;
   Eina_Bool fill_v : 1;
};

struct _Evas_Object_Table_Cache
{
   struct {
      struct {
         double h, v;
      } weights;
      struct {
         int h, v;
      } expands;
      struct {
         Evas_Coord w, h;
      } min;
   } total;
   struct {
      double *h, *v;
   } weights;
   struct {
      Evas_Coord *h, *v;
   } sizes;
   struct {
      Eina_Bool *h, *v;
   } expands;
   double ___pad; // padding to make sure doubles at end can be aligned
};

struct _Evas_Object_Table_Data
{
   Eina_List *children;
   struct {
      Evas_Coord h, v;
   } pad;
   struct {
      double h, v;
   } align;
   struct {
      int cols, rows;
   } size;
   Evas_Object_Table_Cache *cache;
   Evas_Object_Table_Homogeneous_Mode homogeneous;
   Eina_Bool hints_changed : 1;
   Eina_Bool expand_h : 1;
   Eina_Bool expand_v : 1;
   Eina_Bool is_mirrored : 1;
};

struct _Evas_Object_Table_Iterator
{
   Eina_Iterator iterator;

   Eina_Iterator *real_iterator;
   const Evas_Object *table;
};

struct _Evas_Object_Table_Accessor
{
   Eina_Accessor accessor;

   Eina_Accessor *real_accessor;
   const Evas_Object *table;
};

#define EVAS_OBJECT_TABLE_DATA_GET(o, ptr)                              \
   Evas_Object_Table_Data *ptr = eo_data_get(o, MY_CLASS)

#define EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(o, ptr)                    \
   EVAS_OBJECT_TABLE_DATA_GET(o, ptr);                                  \
if (!ptr)                                                               \
{                                                                       \
   CRIT("no widget data for object %p (%s)",                            \
        o, evas_object_type_get(o));                                    \
   return;                                                              \
}

#define EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, ptr, val)           \
   EVAS_OBJECT_TABLE_DATA_GET(o, ptr);                                  \
if (!ptr)                                                               \
{                                                                       \
   CRIT("No widget data for object %p (%s)",                            \
        o, evas_object_type_get(o));                                    \
   return val;                                                          \
}

static const char EVAS_OBJECT_TABLE_OPTION_KEY[] = "|EvTb";

static Eina_Bool
_evas_object_table_iterator_next(Evas_Object_Table_Iterator *it, void **data)
{
   Evas_Object_Table_Option *opt;

   if (!eina_iterator_next(it->real_iterator, (void **)&opt))
     return EINA_FALSE;
   if (data) *data = opt->obj;
   return EINA_TRUE;
}

static Evas_Object *
_evas_object_table_iterator_get_container(Evas_Object_Table_Iterator *it)
{
   return (Evas_Object *)it->table;
}

static void
_evas_object_table_iterator_free(Evas_Object_Table_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

static Eina_Bool
_evas_object_table_accessor_get_at(Evas_Object_Table_Accessor *it, unsigned int idx, void **data)
{
   Evas_Object_Table_Option *opt = NULL;

   if (!eina_accessor_data_get(it->real_accessor, idx, (void **)&opt))
     return EINA_FALSE;
   if (data) *data = opt->obj;
   return EINA_TRUE;
}

static Evas_Object *
_evas_object_table_accessor_get_container(Evas_Object_Table_Accessor *it)
{
   return (Evas_Object *)it->table;
}

static void
_evas_object_table_accessor_free(Evas_Object_Table_Accessor *it)
{
   eina_accessor_free(it->real_accessor);
   free(it);
}

static Evas_Object_Table_Cache *
_evas_object_table_cache_alloc(int cols, int rows)
{
   Evas_Object_Table_Cache *cache;
   int size;

   size = sizeof(Evas_Object_Table_Cache) +
      ((cols + rows) *
          (sizeof(double) + sizeof(Evas_Coord) + sizeof(Eina_Bool)));
   cache = malloc(size);
   if (!cache)
     {
        ERR("Could not allocate table cache %dx%d (%d bytes): %s",
            cols, rows, size, strerror(errno));
        return NULL;
     }

   cache->weights.h = (double *)(cache + 1);
   cache->weights.v = (double *)(cache->weights.h + cols);
   cache->sizes.h = (Evas_Coord *)(cache->weights.v + rows);
   cache->sizes.v = (Evas_Coord *)(cache->sizes.h + cols);
   cache->expands.h = (Eina_Bool *)(cache->sizes.v + rows);
   cache->expands.v = (Eina_Bool *)(cache->expands.h + cols);

   return cache;
}

static void
_evas_object_table_cache_free(Evas_Object_Table_Cache *cache)
{
   free(cache);
}

static void
_evas_object_table_cache_reset(Evas_Object_Table_Data *priv)
{
   Evas_Object_Table_Cache *c = priv->cache;
   int size;

   c->total.expands.v = 0;
   c->total.expands.h = 0;
   c->total.min.w = 0;
   c->total.min.h = 0;

   size = ((priv->size.rows + priv->size.cols) *
           (sizeof(double) + sizeof(Evas_Coord) + sizeof(Eina_Bool)));
   memset(c + 1, 0, size);
}

static void
_evas_object_table_cache_invalidate(Evas_Object_Table_Data *priv)
{
   priv->hints_changed = 1;
   if (priv->cache)
     {
        _evas_object_table_cache_free(priv->cache);
        priv->cache = NULL;
     }
}

static Evas_Object_Table_Option *
_evas_object_table_option_get(Evas_Object *o)
{
   return evas_object_data_get(o, EVAS_OBJECT_TABLE_OPTION_KEY);
}

static void
_evas_object_table_option_set(Evas_Object *o, const Evas_Object_Table_Option *opt)
{
   evas_object_data_set(o, EVAS_OBJECT_TABLE_OPTION_KEY, opt);
}

static Evas_Object_Table_Option *
_evas_object_table_option_del(Evas_Object *o)
{
   return evas_object_data_del(o, EVAS_OBJECT_TABLE_OPTION_KEY);
}

static void
_on_child_del(void *data, Evas *evas EINA_UNUSED, Evas_Object *child, void *einfo EINA_UNUSED)
{
   Evas_Object *table = data;
   evas_object_table_unpack(table, child);
}

static void
_on_child_hints_changed(void *data, Evas *evas EINA_UNUSED, Evas_Object *child EINA_UNUSED, void *einfo EINA_UNUSED)
{
   Evas_Object *table = data;
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(table, priv);
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(table);
}

static void
_evas_object_table_child_connect(Evas_Object *o, Evas_Object *child)
{
   evas_object_event_callback_add
     (child, EVAS_CALLBACK_DEL, _on_child_del, o);
   evas_object_event_callback_add
     (child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_child_hints_changed, o);
}

static void
_evas_object_table_child_disconnect(Evas_Object *o, Evas_Object *child)
{
   evas_object_event_callback_del_full
     (child, EVAS_CALLBACK_DEL, _on_child_del, o);
   evas_object_event_callback_del_full
     (child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_child_hints_changed, o);
}

static void
_evas_object_table_calculate_cell(const Evas_Object_Table_Option *opt, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Coord cw, ch;

   *w -= opt->pad.l + opt->pad.r;
   if (*w < opt->min.w)
     cw = opt->min.w;
   else if ((opt->max.w > -1) && (*w > opt->max.w))
     cw = opt->max.w;
   else if (opt->fill_h)
     cw = *w;
   else
     cw = opt->min.w;

   *h -= opt->pad.t + opt->pad.b;
   if (*h < opt->min.h)
     ch = opt->min.h;
   else if ((opt->max.h > -1) && (*h > opt->max.h))
     ch = opt->max.h;
   else if (opt->fill_v)
     ch = *h;
   else
     ch = opt->min.h;

   *x += opt->pad.l;
   if (cw != *w)
     {
        *x += (*w - cw) * opt->align.h;
        *w = cw;
     }

   *y += opt->pad.t;
   if (ch != *h)
     {
        *y += (*h - ch) * opt->align.v;
        *h = ch;
     }
}

static void
_evas_object_table_calculate_hints_homogeneous(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   Eina_List *l;
   Evas_Object_Table_Option *opt;
   Evas_Coord minw, minh, o_minw, o_minh;
   Eina_Bool expand_h, expand_v;

   o_minw = 0;
   o_minh = 0;
   minw = 0;
   minh = 0;
   expand_h = 0;
   expand_v = 0;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *child = opt->obj;
        Evas_Coord child_minw, child_minh, cell_minw, cell_minh;
        double weightw, weighth;

        evas_object_size_hint_min_get(child, &opt->min.w, &opt->min.h);
        evas_object_size_hint_max_get(child, &opt->max.w, &opt->max.h);
        evas_object_size_hint_padding_get
           (child, &opt->pad.l, &opt->pad.r, &opt->pad.t, &opt->pad.b);
        evas_object_size_hint_align_get(child, &opt->align.h, &opt->align.v);
        evas_object_size_hint_weight_get(child, &weightw, &weighth);

        child_minw = opt->min.w + opt->pad.l + opt->pad.r;
        child_minh = opt->min.h + opt->pad.t + opt->pad.b;

        cell_minw = (child_minw + opt->colspan - 1) / opt->colspan;
        cell_minh = (child_minh + opt->rowspan - 1) / opt->rowspan;

        opt->expand_h = 0;
        if ((weightw > 0.0) &&
            ((opt->max.w < 0) ||
             ((opt->max.w > -1) && (opt->min.w < opt->max.w))))
          {
             opt->expand_h = 1;
             expand_h = 1;
          }

        opt->expand_v = 0;
        if ((weighth > 0.0) &&
            ((opt->max.h < 0) ||
             ((opt->max.h > -1) && (opt->min.h < opt->max.h))))
          {
             opt->expand_v = 1;
             expand_v = 1;
          }

        opt->fill_h = 0;
        if (opt->align.h < 0.0)
          {
             opt->align.h = 0.5;
             opt->fill_h = 1;
          }
        opt->fill_v = 0;
        if (opt->align.v < 0.0)
          {
             opt->align.v = 0.5;
             opt->fill_v = 1;
          }

        /* greatest mininum values, with paddings */
        if (minw < cell_minw)
          minw = cell_minw;
        if (minh < cell_minh)
          minh = cell_minh;
        /* greatest mininum values, without paddings */
        if (o_minw < opt->min.w)
          o_minw = opt->min.w;
        if (o_minh < opt->min.h)
          o_minh = opt->min.h;
     }

   if (priv->homogeneous == EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM)
     {
        if (o_minw < 1)
          {
             ERR("homogeneous table based on item size but no "
                 "horizontal mininum size specified! Using expand.");
             expand_h = 1;
          }
        if (o_minh < 1)
          {
             ERR("homogeneous table based on item size but no "
                 "vertical mininum size specified! Using expand.");
             expand_v = 1;
          }
     }

   minw = priv->size.cols * (minw + priv->pad.h) - priv->pad.h;
   minh = priv->size.rows * (minh + priv->pad.v) - priv->pad.v;

   priv->hints_changed = 0;
   priv->expand_h = expand_h;
   priv->expand_v = expand_v;

   if ((minw > 0 ) || (minh > 0))
     evas_object_size_hint_min_set(o, minw, minh);

   // XXX hint max?
}

static void
_evas_object_table_calculate_layout_homogeneous_sizes_item(const Evas_Object *o, const Evas_Object_Table_Data *priv, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Coord minw, minh;
   Eina_Bool expand_h, expand_v;

   evas_object_size_hint_min_get(o, &minw, &minh);
   expand_h = priv->expand_h;
   expand_v = priv->expand_v;

   if (*w < minw)
     expand_h = 0;
   if (!expand_h)
     {
        *x += (*w - minw) * priv->align.h;
        *w = minw;
     }

   if (*h < minh)
     expand_v = 0;
   if (!expand_v)
     {
        *y += (*h - minh) * priv->align.v;
        *h = minh;
     }
}

static void
_evas_object_table_calculate_layout_homogeneous_sizes(const Evas_Object *o, const Evas_Object_Table_Data *priv, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h, Evas_Coord *cellw, Evas_Coord *cellh)
{
   evas_object_geometry_get(o, x, y, w, h);
   if (priv->homogeneous == EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM)
     _evas_object_table_calculate_layout_homogeneous_sizes_item
       (o, priv, x, y, w, h);

   *cellw = (*w + priv->size.cols - 1) / priv->size.cols;
   *cellh = (*h + priv->size.rows - 1) / priv->size.rows;
}

static void
_evas_object_table_calculate_layout_homogeneous(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   Evas_Coord x = 0, y = 0, w = 0, h = 0, ww, hh, cellw = 0, cellh = 0;
   Eina_List *l;
   Evas_Object_Table_Option *opt;

   _evas_object_table_calculate_layout_homogeneous_sizes
      (o, priv, &x, &y, &w, &h, &cellw, &cellh);

   ww = w - ((priv->size.cols - 1) * priv->pad.h);
   hh = h - ((priv->size.rows - 1) * priv->pad.v);

   if (ww < 0) ww = 0;
   if (ww < 0) ww = 0;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *child = opt->obj;
        Evas_Coord cx, cy, cw, ch, cox, coy, cow, coh;

        cx = x + ((opt->col * ww) / priv->size.cols);
        cw = x + (((opt->col + opt->colspan) * ww) / priv->size.cols) - cx;
        cy = y + ((opt->row * hh) / priv->size.rows);
        ch = y + (((opt->row + opt->rowspan) * hh) / priv->size.rows) - cy;

        cx += (opt->col) * priv->pad.h;
        cy += (opt->row) * priv->pad.v;

        cox = cx;
        coy = cy;
        cow = cw;
        coh = ch;

        _evas_object_table_calculate_cell(opt, &cx, &cy, &cw, &ch);
        if (cw > cow)
          {
             cx = cox;
             cw = cow;
          }
        if (ch > coh)
          {
             cy = coy;
             ch = coh;
          }

        if (priv->is_mirrored)
          {
             evas_object_move(opt->obj, x + w - (cx - x + cw), cy);
          }
        else
          {
             evas_object_move(child, cx, cy);
          }
        evas_object_resize(child, cw, ch);
     }
}

static void
_evas_object_table_smart_calculate_homogeneous(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   if (priv->hints_changed)
     _evas_object_table_calculate_hints_homogeneous(o, priv);
   _evas_object_table_calculate_layout_homogeneous(o, priv);
}

static int
_evas_object_table_count_expands(const Eina_Bool *expands, int start, int end)
{
   const Eina_Bool *itr = expands + start, *itr_end = expands + end;
   int count = 0;

   for (; itr < itr_end; itr++)
     {
        if (*itr)
           count++;
     }

   return count;
}

static Evas_Coord
_evas_object_table_sum_sizes(const Evas_Coord *sizes, int start, int end)
{
   const Evas_Coord *itr = sizes + start, *itr_end = sizes + end;
   Evas_Coord sum = 0;

   for (; itr < itr_end; itr++)
     sum += *itr;

   return sum;
}

static void
_evas_object_table_sizes_calc_noexpand(Evas_Coord *sizes, int start, int end, Evas_Coord space)
{
   Evas_Coord *itr = sizes + start, *itr_end = sizes + end - 1;
   Evas_Coord step;
   int units;

   /* XXX move to fixed point math and spread errors among cells */
   units = end - start;
   step = space / units;
   for (; itr < itr_end; itr++)
     *itr += step;

   *itr += space - step * (units - 1);
}

static void
_evas_object_table_sizes_calc_expand(Evas_Coord *sizes, int start, int end, Evas_Coord space, const Eina_Bool *expands, int expand_count, double *weights, double weighttot)
{
   Evas_Coord *itr = sizes + start, *itr_end = sizes + end;
   const Eina_Bool *itr_expand = expands + start;
   Evas_Coord step = 0, last_space = 0;
   int total = 0, i = start;

   /* XXX move to fixed point math and spread errors among cells */
   if (weighttot > 0.0)
     {
        step = space / expand_count;
        last_space = space - step * (expand_count - 1);
     }

   for (; itr < itr_end; itr++, itr_expand++, i++)
     {
        if (weighttot <= 0.0)
          {
             if (*itr_expand)
               {
                  expand_count--;
                  if (expand_count > 0)
                     *itr += step;
                  else
                    {
                       *itr += last_space;
                       break;
                    }
               }
          }
        else
          {
             if (*itr_expand)
               {
                  expand_count--;
                  if (expand_count > 0)
                    {
                       step = (weights[i] / weighttot) * space;
                       *itr += step;
                       total += step;
                    }
                  else
                    {
                       *itr += space - total;
                       break;
                    }
               }
          }
     }
}

static void
_evas_object_table_calculate_hints_regular(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   Evas_Object_Table_Option *opt;
   Evas_Object_Table_Cache *c;
   Eina_List *l;
   double totweightw = 0.0, totweighth = 0.0;
   int i;

   if (!priv->cache)
     {
        priv->cache = _evas_object_table_cache_alloc
           (priv->size.cols, priv->size.rows);
        if (!priv->cache)
          return;
     }
   c = priv->cache;
   _evas_object_table_cache_reset(priv);

   /* cache interesting data */
   memset(c->expands.h, 1, priv->size.cols);
   memset(c->expands.v, 1, priv->size.rows);
   memset(c->weights.h, 0, priv->size.cols);
   memset(c->weights.v, 0, priv->size.rows);
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *child = opt->obj;
        double weightw, weighth;

        evas_object_size_hint_min_get(child, &opt->min.w, &opt->min.h);
        evas_object_size_hint_max_get(child, &opt->max.w, &opt->max.h);
        evas_object_size_hint_padding_get
           (child, &opt->pad.l, &opt->pad.r, &opt->pad.t, &opt->pad.b);
        evas_object_size_hint_align_get(child, &opt->align.h, &opt->align.v);
        evas_object_size_hint_weight_get(child, &weightw, &weighth);

        opt->expand_h = 0;
        if ((weightw > 0.0) &&
            ((opt->max.w < 0) ||
             ((opt->max.w > -1) && (opt->min.w < opt->max.w))))
          opt->expand_h = 1;

        opt->expand_v = 0;
        if ((weighth > 0.0) &&
            ((opt->max.h < 0) ||
             ((opt->max.h > -1) && (opt->min.h < opt->max.h))))
          opt->expand_v = 1;

        opt->fill_h = 0;
        if (opt->align.h < 0.0)
          {
             opt->align.h = 0.5;
             opt->fill_h = 1;
          }
        opt->fill_v = 0;
        if (opt->align.v < 0.0)
          {
             opt->align.v = 0.5;
             opt->fill_v = 1;
          }

        if (!opt->expand_h)
          memset(c->expands.h + opt->col, 0, opt->colspan);
        else
          {
             for (i = opt->col; i < opt->col + opt->colspan; i++)
               c->weights.h[i] += (weightw / (double)opt->colspan);
          }
        if (!opt->expand_v)
          memset(c->expands.v + opt->row, 0, opt->rowspan);
        else
          {
             for (i = opt->row; i < opt->row + opt->rowspan; i++)
                c->weights.v[i] += (weighth / (double)opt->rowspan);
          }
     }
   for (i = 0; i < priv->size.cols; i++) totweightw += c->weights.h[i];
   for (i = 0; i < priv->size.rows; i++) totweighth += c->weights.v[i];

   /* calculate sizes for each row and column */
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Coord tot, need;

        /* handle horizontal */
        tot = _evas_object_table_sum_sizes(c->sizes.h, opt->col, opt->end_col);
        need = opt->min.w + opt->pad.l + opt->pad.r;
        if (tot < need)
          {
             Evas_Coord space = need - tot;
             int count;

             count = _evas_object_table_count_expands
                (c->expands.h, opt->col, opt->end_col);

             if (count > 0)
               _evas_object_table_sizes_calc_expand
                  (c->sizes.h, opt->col, opt->end_col, space,
                   c->expands.h, count, c->weights.h, totweightw);
             else
               _evas_object_table_sizes_calc_noexpand
                  (c->sizes.h, opt->col, opt->end_col, space);
          }

        /* handle vertical */
        tot = _evas_object_table_sum_sizes(c->sizes.v, opt->row, opt->end_row);
        need = opt->min.h + opt->pad.t + opt->pad.b;
        if (tot < opt->min.h)
          {
             Evas_Coord space = need - tot;
             int count;

             count = _evas_object_table_count_expands
                (c->expands.v, opt->row, opt->end_row);

             if (count > 0)
               _evas_object_table_sizes_calc_expand
                  (c->sizes.v, opt->row, opt->end_row, space,
                   c->expands.v, count, c->weights.v, totweighth);
             else
               _evas_object_table_sizes_calc_noexpand
                  (c->sizes.v, opt->row, opt->end_row, space);
          }
     }

   c->total.weights.h = totweightw;
   c->total.weights.v = totweighth;

   c->total.expands.h = _evas_object_table_count_expands
      (c->expands.h, 0, priv->size.cols);
   c->total.expands.v = _evas_object_table_count_expands
     (c->expands.v, 0, priv->size.rows);

   c->total.min.w = _evas_object_table_sum_sizes
     (c->sizes.h, 0, priv->size.cols);
   c->total.min.h = _evas_object_table_sum_sizes
     (c->sizes.v, 0, priv->size.rows);

   c->total.min.w += priv->pad.h * (priv->size.cols - 1);
   c->total.min.h += priv->pad.v * (priv->size.rows - 1);

   if ((c->total.min.w > 0) || (c->total.min.h > 0))
     evas_object_size_hint_min_set(o, c->total.min.w, c->total.min.h);

   // XXX hint max?
}

static void
_evas_object_table_calculate_layout_regular(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   Evas_Object_Table_Option *opt;
   Evas_Object_Table_Cache *c;
   Eina_List *l;
   Evas_Coord *cols = NULL, *rows = NULL;
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(o, &x, &y, &w, &h);
   c = priv->cache;

   /* handle horizontal */
   if ((c->total.expands.h <= 0) || (c->total.min.w >= w))
     {
        x += (w - c->total.min.w) * priv->align.h;
        w = c->total.min.w;
        cols = c->sizes.h;
     }
   else
     {
        int size = priv->size.cols * sizeof(Evas_Coord);
        cols = malloc(size);
        if (!cols)
          {
             ERR("Could not allocate temp columns (%d bytes): %s",
                 size, strerror(errno));
             goto end;
          }
        memcpy(cols, c->sizes.h, size);
        _evas_object_table_sizes_calc_expand
           (cols, 0, priv->size.cols, w - c->total.min.w,
            c->expands.h, c->total.expands.h, c->weights.h, c->total.weights.h);
     }

   /* handle vertical */
   if ((c->total.expands.v <= 0) || (c->total.min.h >= h))
     {
        y += (h - c->total.min.h) * priv->align.v;
        h = c->total.min.h;
        rows = c->sizes.v;
     }
   else
     {
        int size = priv->size.rows * sizeof(Evas_Coord);
        rows = malloc(size);
        if (!rows)
          {
             ERR("could not allocate temp rows (%d bytes): %s",
                 size, strerror(errno));
             goto end;
          }
        memcpy(rows, c->sizes.v, size);
        _evas_object_table_sizes_calc_expand
           (rows, 0, priv->size.rows, h - c->total.min.h,
            c->expands.v, c->total.expands.v, c->weights.v, c->total.weights.v);
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *child = opt->obj;
        Evas_Coord cx, cy, cw, ch;

        cx = x + opt->col * (priv->pad.h);
        cx += _evas_object_table_sum_sizes(cols, 0, opt->col);
        cw = _evas_object_table_sum_sizes(cols, opt->col, opt->end_col);

        cy = y + opt->row * (priv->pad.v);
        cy += _evas_object_table_sum_sizes(rows, 0, opt->row);
        ch = _evas_object_table_sum_sizes(rows, opt->row, opt->end_row);

        _evas_object_table_calculate_cell(opt, &cx, &cy, &cw, &ch);

        if (priv->is_mirrored)
          {
             evas_object_move(opt->obj, x + w - (cx - x + cw), cy);
          }
        else
          {
             evas_object_move(child, cx, cy);
          }
        evas_object_resize(child, cw, ch);
     }

 end:
   if (cols != c->sizes.h)
     {
        if (cols) free(cols);
     }
   if (rows != c->sizes.v)
     {
        if (rows) free(rows);
     }
}

static void
_evas_object_table_smart_calculate_regular(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   if (priv->hints_changed)
     _evas_object_table_calculate_hints_regular(o, priv);
   _evas_object_table_calculate_layout_regular(o, priv);
}

static void
_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object_Table_Data *priv = _pd;
   priv->pad.h = 0;
   priv->pad.v = 0;
   priv->align.h = 0.5;
   priv->align.v = 0.5;
   priv->size.cols = 0;
   priv->size.rows = 0;
   priv->cache = NULL;
   priv->homogeneous = EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE;
   priv->hints_changed = 1;
   priv->expand_h = 0;
   priv->expand_v = 0;

   eo_do_super(obj, evas_obj_smart_add());
}

static void
_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object_Table_Data *priv = _pd;
   Eina_List *l;

   l = priv->children;
   while (l)
     {
        Evas_Object_Table_Option *opt = l->data;
        _evas_object_table_child_disconnect(obj, opt->obj);
        _evas_object_table_option_del(opt->obj);
        free(opt);
        l = eina_list_remove_list(l, l);
     }

   if (priv->cache)
     {
        _evas_object_table_cache_free(priv->cache);
        priv->cache = NULL;
     }

   eo_do_super(obj, evas_obj_smart_del());
}

static void
_smart_resize(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   evas_object_smart_changed(obj);
}

static void
_smart_calculate(Eo *o, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object_Table_Data *priv = _pd;

   if ((priv->size.cols < 1) || (priv->size.rows < 1))
     {
        DBG("Nothing to do: cols=%d, rows=%d",
            priv->size.cols, priv->size.rows);
        return;
     }

   if (priv->homogeneous)
     _evas_object_table_smart_calculate_homogeneous(o, priv);
   else
     _evas_object_table_smart_calculate_regular(o, priv);
}

EAPI Evas_Object *
evas_object_table_add(Evas *evas)
{
   Evas_Object *obj = eo_add(MY_CLASS, evas);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME));
}

EAPI Evas_Object *
evas_object_table_add_to(Evas_Object *parent)
{
   Evas_Object *ret = NULL;
   eo_do(parent, evas_obj_table_add_to(&ret));
   return ret;
}

static void
_add_to(Eo *parent, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Evas *evas;

   evas = evas_object_evas_get(parent);
   *ret = evas_object_table_add(evas);
   evas_object_smart_member_add(*ret, parent);
}

EAPI void
evas_object_table_homogeneous_set(Evas_Object *o, Evas_Object_Table_Homogeneous_Mode homogeneous)
{
   eo_do(o, evas_obj_table_homogeneous_set(homogeneous));
}

static void
_homogeneous_set(Eo *o, void *_pd, va_list *list)
{
   Evas_Object_Table_Homogeneous_Mode homogeneous = va_arg(*list, Evas_Object_Table_Homogeneous_Mode);

   Evas_Object_Table_Data *priv = _pd;
   if (priv->homogeneous == homogeneous)
     return;
   priv->homogeneous = homogeneous;
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);
}

EAPI Evas_Object_Table_Homogeneous_Mode
evas_object_table_homogeneous_get(const Evas_Object *o)
{
   Evas_Object_Table_Homogeneous_Mode ret = EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE;
   eo_do((Eo *)o, evas_obj_table_homogeneous_get(&ret));
   return ret;
}

static void
_homogeneous_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object_Table_Homogeneous_Mode *ret = va_arg(*list, Evas_Object_Table_Homogeneous_Mode *);
   const Evas_Object_Table_Data *priv = _pd;
   *ret = priv->homogeneous;
}

EAPI void
evas_object_table_align_set(Evas_Object *o, double horizontal, double vertical)
{
   eo_do(o, evas_obj_table_align_set(horizontal, vertical));
}

static void
_align_set(Eo *o, void *_pd, va_list *list)
{
   double horizontal = va_arg(*list, double);
   double vertical = va_arg(*list, double);

   Evas_Object_Table_Data *priv = _pd;
   if (priv->align.h == horizontal && priv->align.v == vertical)
     return;
   priv->align.h = horizontal;
   priv->align.v = vertical;
   evas_object_smart_changed(o);
}

EAPI void
evas_object_table_align_get(const Evas_Object *o, double *horizontal, double *vertical)
{
   eo_do((Eo *)o, evas_obj_table_align_get(horizontal, vertical));
}

static void
_align_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   double *horizontal = va_arg(*list, double *);
   double *vertical = va_arg(*list, double *);

   const Evas_Object_Table_Data *priv = _pd;
   if (priv)
     {
        if (horizontal) *horizontal = priv->align.h;
        if (vertical) *vertical = priv->align.v;
     }
   else
     {
        if (horizontal) *horizontal = 0.5;
        if (vertical) *vertical = 0.5;
     }
}

EAPI void
evas_object_table_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical)
{
   eo_do(o, evas_obj_table_padding_set(horizontal, vertical));
}

static void
_padding_set(Eo *o, void *_pd, va_list *list)
{
   Evas_Coord horizontal = va_arg(*list, Evas_Coord);
   Evas_Coord vertical = va_arg(*list, Evas_Coord);

   Evas_Object_Table_Data *priv = _pd;
   if (priv->pad.h == horizontal && priv->pad.v == vertical)
     return;
   priv->pad.h = horizontal;
   priv->pad.v = vertical;
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);
}

EAPI void
evas_object_table_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical)
{
   eo_do((Eo *)o, evas_obj_table_padding_get(horizontal, vertical));
}

static void
_padding_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *horizontal = va_arg(*list, Evas_Coord *);
   Evas_Coord *vertical = va_arg(*list, Evas_Coord *);

   const Evas_Object_Table_Data *priv = _pd;
   if (priv)
     {
        if (horizontal) *horizontal = priv->pad.h;
        if (vertical) *vertical = priv->pad.v;
     }
   else
     {
        if (horizontal) *horizontal = 0;
        if (vertical) *vertical = 0;
     }
}

EAPI Eina_Bool
evas_object_table_pack_get(const Evas_Object *o, Evas_Object *child, unsigned short *col, unsigned short *row, unsigned short *colspan, unsigned short *rowspan)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)o, evas_obj_table_pack_get(child, col, row, colspan, rowspan, &ret));
   return ret;
}

static void
_pack_get(Eo *o EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   unsigned short *col = va_arg(*list, unsigned short *);
   unsigned short *row = va_arg(*list, unsigned short *);
   unsigned short *colspan = va_arg(*list, unsigned short *);
   unsigned short *rowspan = va_arg(*list, unsigned short *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Object_Table_Option *opt;

   opt = _evas_object_table_option_get(child);
   if (!opt)
     {
        if (col) *col = 0;
        if (row) *row = 0;
        if (colspan) *colspan = 0;
        if (rowspan) *rowspan = 0;
        return;
     }
   if (col) *col = opt->col;
   if (row) *row = opt->row;
   if (colspan) *colspan = opt->colspan;
   if (rowspan) *rowspan = opt->rowspan;
   if (ret) *ret = EINA_TRUE;
}

EAPI Eina_Bool
evas_object_table_pack(Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do(o, evas_obj_table_pack(child, col, row, colspan, rowspan, &ret));
   return ret;
}

static void
_pack(Eo *o, void *_pd, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   unsigned short col = va_arg(*list, int);
   unsigned short row = va_arg(*list, int);
   unsigned short colspan = va_arg(*list, int);
   unsigned short rowspan = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Object_Table_Option *opt;

   Evas_Object_Table_Data *priv = _pd;

   if (rowspan < 1)
     {
        ERR("rowspan < 1");
        return;
     }
   if (colspan < 1)
     {
        ERR("colspan < 1");
        return;
     }

   opt = _evas_object_table_option_get(child);
   if (!opt)
     {
        opt = malloc(sizeof(*opt));
        if (!opt)
          {
             ERR("could not allocate table option data.");
             return;
          }
     }

   opt->obj = child;
   opt->col = col;
   opt->row = row;
   opt->colspan = colspan;
   opt->rowspan = rowspan;
   opt->end_col = col + colspan;
   opt->end_row = row + rowspan;

   if (evas_object_smart_parent_get(child) == o)
     {
        Eina_Bool need_shrink = EINA_FALSE;

        if (priv->size.cols < opt->end_col)
          priv->size.cols = opt->end_col;
        else
          need_shrink = EINA_TRUE;
        if (priv->size.rows < opt->end_row)
          priv->size.rows = opt->end_row;
        else
          need_shrink = EINA_TRUE;

        if (need_shrink)
          {
             Eina_List *l;
             Evas_Object_Table_Option *opt2;
             int max_row = 0, max_col = 0;

             EINA_LIST_FOREACH(priv->children, l, opt2)
               {
                  if (max_col < opt2->end_col) max_col = opt2->end_col;
                  if (max_row < opt2->end_row) max_row = opt2->end_row;
               }
             priv->size.cols = max_col;
             priv->size.rows = max_row;
          }
     }
   else
     {
        opt->min.w = 0;
        opt->min.h = 0;
        opt->max.w = 0;
        opt->max.h = 0;
        opt->align.h = 0.5;
        opt->align.v = 0.5;
        opt->pad.l = 0;
        opt->pad.r = 0;
        opt->pad.t = 0;
        opt->pad.b = 0;
        opt->expand_h = 0;
        opt->expand_v = 0;

        priv->children = eina_list_append(priv->children, opt);

        if (priv->size.cols < opt->end_col)
          priv->size.cols = opt->end_col;
        if (priv->size.rows < opt->end_row)
          priv->size.rows = opt->end_row;

        _evas_object_table_option_set(child, opt);
        evas_object_smart_member_add(child, o);
        _evas_object_table_child_connect(o, child);
     }
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);
   if (ret) *ret = EINA_TRUE;
}

static void
_evas_object_table_remove_opt(Evas_Object_Table_Data *priv, Evas_Object_Table_Option *opt)
{
   Eina_List *l;
   int max_row, max_col, was_greatest;

   max_row = 0;
   max_col = 0;
   was_greatest = 0;
   l = priv->children;
   while (l)
     {
        Evas_Object_Table_Option *cur_opt = l->data;

        if (cur_opt != opt)
          {
             if (max_col < cur_opt->end_col)
               max_col = cur_opt->end_col;
             if (max_row < cur_opt->end_row)
               max_row = cur_opt->end_row;

             l = l->next;
          }
        else
          {
             Eina_List *tmp = l->next;
             priv->children = eina_list_remove_list(priv->children, l);

             if ((priv->size.cols > opt->end_col) &&
                 (priv->size.rows > opt->end_row))
               break;
             else
               {
                  was_greatest = 1;
                  l = tmp;
               }
          }
     }

   if (was_greatest)
     {
        priv->size.cols = max_col;
        priv->size.rows = max_row;
     }
}

EAPI Eina_Bool
evas_object_table_unpack(Evas_Object *o, Evas_Object *child)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do(o, evas_obj_table_unpack(child, &ret));
   return ret;
}

static void
_unpack(Eo *o, void *_pd, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Object_Table_Option *opt;

   Evas_Object_Table_Data *priv = _pd;

   if (o != evas_object_smart_parent_get(child))
     {
        ERR("cannot unpack child from incorrect table!");
        return;
     }

   opt = _evas_object_table_option_del(child);
   if (!opt)
     {
        ERR("cannot unpack child with no packing option!");
        return;
     }

   _evas_object_table_child_disconnect(o, child);
   _evas_object_table_remove_opt(priv, opt);
   evas_object_smart_member_del(child);
   free(opt);
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);

   if (ret) *ret = EINA_TRUE;
}

EAPI void
evas_object_table_clear(Evas_Object *o, Eina_Bool clear)
{
   eo_do(o, evas_obj_table_clear(clear));
}

static void
_clear(Eo *o, void *_pd, va_list *list)
{
   Eina_Bool clear = va_arg(*list, int);

   Evas_Object_Table_Option *opt;

   Evas_Object_Table_Data *priv = _pd;

   EINA_LIST_FREE(priv->children, opt)
     {
        _evas_object_table_child_disconnect(o, opt->obj);
        _evas_object_table_option_del(opt->obj);
        evas_object_smart_member_del(opt->obj);
        if (clear)
          evas_object_del(opt->obj);
        free(opt);
     }
   priv->size.cols = 0;
   priv->size.rows = 0;
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);
}

EAPI void
evas_object_table_col_row_size_get(const Evas_Object *o, int *cols, int *rows)
{
   eo_do((Eo *)o, evas_obj_table_col_row_size_get(cols, rows));
}

static void
_col_row_size_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   int *cols = va_arg(*list, int *);
   int *rows = va_arg(*list, int *);

   const Evas_Object_Table_Data *priv = _pd;
   if (priv)
     {
        if (cols) *cols = priv->size.cols;
        if (rows) *rows = priv->size.rows;
     }
   else
     {
        if (cols) *cols = -1;
        if (rows) *rows = -1;
     }
}

EAPI Eina_Iterator *
evas_object_table_iterator_new(const Evas_Object *o)
{
   Eina_Iterator *ret = NULL;
   eo_do((Eo *)o, evas_obj_table_iterator_new(&ret));
   return ret;
}

static void
_iterator_new(Eo *o, void *_pd, va_list *list)
{
   Eina_Iterator **ret = va_arg(*list, Eina_Iterator **);

   Evas_Object_Table_Iterator *it;

   const Evas_Object_Table_Data *priv = _pd;

   if (!priv->children)
     {
        *ret = NULL;
        return;
     }

   it = calloc(1, sizeof(Evas_Object_Table_Iterator));
   if (!it)
     {
        *ret = NULL;
        return;
     }

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_list_iterator_new(priv->children);
   it->table = o;

   it->iterator.next = FUNC_ITERATOR_NEXT(_evas_object_table_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_evas_object_table_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_evas_object_table_iterator_free);

   *ret = &it->iterator;
}

EAPI Eina_Accessor *
evas_object_table_accessor_new(const Evas_Object *o)
{
   Eina_Accessor *ret = NULL;
   eo_do((Eo *)o, evas_obj_table_accessor_new(&ret));
   return ret;
}

static void
_accessor_new(Eo *o, void *_pd, va_list *list)
{
   Eina_Accessor **ret = va_arg(*list, Eina_Accessor **);

   Evas_Object_Table_Accessor *it;

   const Evas_Object_Table_Data *priv = _pd;

   if (!priv->children)
     {
        *ret = NULL;
        return;
     }

   it = calloc(1, sizeof(Evas_Object_Table_Accessor));
   if (!it)
     {
        *ret = NULL;
        return;
     }

   EINA_MAGIC_SET(&it->accessor, EINA_MAGIC_ACCESSOR);

   it->real_accessor = eina_list_accessor_new(priv->children);
   it->table = o;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(_evas_object_table_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(_evas_object_table_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(_evas_object_table_accessor_free);

   *ret = &it->accessor;
}

EAPI Eina_List *
evas_object_table_children_get(const Evas_Object *o)
{
   Eina_List *ret = NULL;
   eo_do((Eo *)o, evas_obj_table_children_get(&ret));
   return ret;
}

static void
_children_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_List **ret = va_arg(*list, Eina_List **);

   Eina_List *new_list = NULL, *l;
   Evas_Object_Table_Option *opt;

   const Evas_Object_Table_Data *priv = _pd;

   EINA_LIST_FOREACH(priv->children, l, opt)
      new_list = eina_list_append(new_list, opt->obj);

   *ret = new_list;
}

Evas_Object *
evas_object_table_child_get(const Evas_Object *o, unsigned short col, unsigned short row)
{
   Eina_List *l;
   Evas_Object_Table_Option *opt;

   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

   EINA_LIST_FOREACH(priv->children, l, opt)
      if (opt->col == col && opt->row == row)
         return opt->obj;
   return NULL;
}

EAPI Eina_Bool
evas_object_table_mirrored_get(const Evas_Object *obj)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)obj, evas_obj_table_mirrored_get(&ret));
   return ret;
}

static void
_mirrored_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   const Evas_Object_Table_Data *priv = _pd;

   *ret = priv->is_mirrored;
}

EAPI void
evas_object_table_mirrored_set(Evas_Object *obj, Eina_Bool mirrored)
{
   eo_do(obj, evas_obj_table_mirrored_set(mirrored));
}

static void
_mirrored_set(Eo *o, void *_pd, va_list *list)
{
   Eina_Bool mirrored = va_arg(*list, int);

   Evas_Object_Table_Data *priv = _pd;
   if (priv->is_mirrored != mirrored)
     {
        priv->is_mirrored = mirrored;
        eo_do(o, evas_obj_smart_calculate());
     }
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ADD_TO), _add_to),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET), _homogeneous_set),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET), _homogeneous_get),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ALIGN_SET), _align_set),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ALIGN_GET), _align_get),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PADDING_SET), _padding_set),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PADDING_GET), _padding_get),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PACK_GET), _pack_get),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_PACK), _pack),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_UNPACK), _unpack),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_CLEAR), _clear),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_COL_ROW_SIZE_GET), _col_row_size_get),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ITERATOR_NEW), _iterator_new),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_ACCESSOR_NEW), _accessor_new),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_CHILDREN_GET), _children_get),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_GET), _mirrored_get),
        EO_OP_FUNC(EVAS_OBJ_TABLE_ID(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_SET), _mirrored_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CALCULATE), _smart_calculate),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_ADD_TO, "Create a table that is child of a given element parent."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_SET, "Set how this table should layout children."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_HOMOGENEOUS_GET, "Get the current layout homogeneous mode."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_ALIGN_SET, "Set the alignment of the whole bounding box of contents."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_ALIGN_GET, "Get alignment of the whole bounding box of contents."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_PADDING_SET, "Set padding between cells."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_PADDING_GET, "Get padding between cells."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_PACK_GET, "Get packing location of a child of table."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_PACK, "Add a new child to a table object or set its current packing."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_UNPACK, "Remove child from table."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_CLEAR, "Faster way to remove all child objects from a table object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_COL_ROW_SIZE_GET, "Get the number of columns and rows this table takes."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_ITERATOR_NEW, "Get an iterator to walk the list of children for the table."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_ACCESSOR_NEW, "Get an accessor to get random access to the list of children for the table."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_CHILDREN_GET, "Get the list of children for the table."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_GET, "Gets the mirrored mode of the table."),
     EO_OP_DESCRIPTION(EVAS_OBJ_TABLE_SUB_ID_MIRRORED_SET, "Sets the mirrored mode of the table."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Evas_Object_Table",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_OBJ_TABLE_BASE_ID, op_desc, EVAS_OBJ_TABLE_SUB_ID_LAST),
     NULL,
     sizeof(Evas_Object_Table_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_object_table_class_get, &class_desc, EVAS_OBJ_SMART_CLIPPED_CLASS, NULL);

