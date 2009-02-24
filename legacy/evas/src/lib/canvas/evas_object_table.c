#include "evas_common.h"
#include <errno.h>

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
   Evas_Bool expand_h : 1; /* XXX required? */
   Evas_Bool expand_v : 1; /* XXX required? */
};

struct _Evas_Object_Table_Cache
{
   struct {
      struct {
	 int h, v;
      } expands;
      struct {
	 Evas_Coord w, h;
      } min;
   } total;
   struct {
      Evas_Coord *h, *v;
   } sizes;
   struct {
      Evas_Bool *h, *v;
   } expands;
};

struct _Evas_Object_Table_Data
{
   Evas_Object_Smart_Clipped_Data base;
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
   Evas_Bool hints_changed : 1;
   Evas_Bool expand_h : 1;
   Evas_Bool expand_v : 1;
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

/**
 * @addtogroup Evas_Object_Table
 * @{
 * @ingroup Evas_Smart_Object_Group
 */

#define EVAS_OBJECT_TABLE_DATA_GET(o, ptr)			\
  Evas_Object_Table_Data *ptr = evas_object_smart_data_get(o)

#define EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(o, ptr)			\
  EVAS_OBJECT_TABLE_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
       fprintf(stderr, "CRITICAL: no widget data for object %p (%s)\n",	\
	       o, evas_object_type_get(o));				\
       fflush(stderr);							\
       abort();								\
       return;								\
}

#define EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, ptr, val)		\
  EVAS_OBJECT_TABLE_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
       fprintf(stderr, "CRITICAL: no widget data for object %p (%s)\n",	\
	       o, evas_object_type_get(o));				\
       fflush(stderr);							\
       abort();								\
       return val;							\
    }

static const char EVAS_OBJECT_TABLE_OPTION_KEY[] = "Evas_Object_Table_Option";

static Eina_Bool
_evas_object_table_iterator_next(Evas_Object_Table_Iterator *it, void **data)
{
   Evas_Object_Table_Option *opt;

   if (!eina_iterator_next(it->real_iterator, &opt))
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
_evas_object_table_accessor_get_at(Evas_Object_Table_Accessor *it, unsigned int index, void **data)
{
   Evas_Object_Table_Option *opt;

   if (!eina_accessor_data_get(it->real_accessor, index, &opt))
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

   size = (sizeof(Evas_Object_Table_Cache) +
	   (cols + rows) * (sizeof(Evas_Bool) + sizeof(Evas_Coord)));
   cache = malloc(size);
   if (!cache)
     {
	fprintf(stderr,
		"ERROR: could not allocate table cache %dx%d (%d bytes): %s\n",
		cols, rows, size, strerror(errno));
	return NULL;
     }

   cache->sizes.h = (Evas_Coord *)(cache + 1);
   cache->sizes.v = (Evas_Coord *)(cache->sizes.h + cols);
   cache->expands.h = (Evas_Bool *)(cache->sizes.v + rows);
   cache->expands.v = (Evas_Bool *)(cache->expands.h + cols);

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
	   (sizeof(Evas_Bool) + sizeof(Evas_Coord)));
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
_on_child_del(void *data, Evas *evas __UNUSED__, Evas_Object *child, void *einfo __UNUSED__)
{
   Evas_Object *table = data;
   evas_object_table_unpack(table, child);
}

static void
_on_child_hints_changed(void *data, Evas *evas __UNUSED__, Evas_Object *child __UNUSED__, void *einfo __UNUSED__)
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
   else
     cw = *w;

   *h -= opt->pad.t + opt->pad.b;
   if (*h < opt->min.h)
     ch = opt->min.h;
   else if ((opt->max.h > -1) && (*h > opt->max.h))
     ch = opt->max.h;
   else
     ch = *h;

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

/* static Evas_Bool */
/* _evas_object_table_check_hints_homogeneous_table(Evas_Object *child, double *align, Evas_Coord min, const char *axis_name) */
/* { */
/*    if (*align < 0.0) */
/*      { */
/* 	/\* assume expand and align to the center. */
/* 	 * this is compatible with evas_object_box behavior and is the */
/* 	 * same as weight > 0.0. */
/* 	 *\/ */
/* 	*align = 0.5; */
/* 	return 0; */
/*      } */
/*    else if (min < 1) */
/*      { */
/* 	fprintf(stderr, */
/* 		"WARNING: child %p [%s, %s] has no minimum width " */
/* 		"and no %s expand (weight is not > 0.0). " */
/* 		"Assuming weight > 0.0\n", */
/* 		child, evas_object_type_get(child), evas_object_name_get(child), */
/* 		axis_name); */
/* 	return 0; */
/*      } */

/*    return 1; */
/* } */

static void
_evas_object_table_calculate_hints_homogeneous(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   Eina_List *l;
   Evas_Object_Table_Option *opt;
   Evas_Coord minw, minh, o_minw, o_minh;
   Evas_Bool expand_h, expand_v;

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
/* 	else if ((priv->homogeneous == EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE) && */
/* 		 (!_evas_object_table_check_hints_homogeneous_table */
/* 		  (child, &opt->align.h, opt->min.w, "horizontal"))) */
/* 	  { */
/* 	     opt->expand_h = 1; */
/* 	     expand_h = 1; */
/* 	  } */


	opt->expand_v = 0;
	if ((weighth > 0.0) &&
	    ((opt->max.h < 0) ||
	     ((opt->max.h > -1) && (opt->min.h < opt->max.h))))
	  {
	     opt->expand_v = 1;
	     expand_v = 1;
	  }
/* 	else if ((priv->homogeneous == EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE) && */
/* 		 (!_evas_object_table_check_hints_homogeneous_table */
/* 		  (child, &opt->align.v, opt->min.h, "vertical"))) */
/* 	  { */
/* 	     opt->expand_v = 1; */
/* 	     expand_v = 1; */
/* 	  } */

	if (opt->align.h < 0.0)
	  opt->align.h = 0.5;
	if (opt->align.v < 0.0)
	  opt->align.v = 0.5;

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
	     fputs("ERROR: homogeneous table based on item size but no "
		   "horizontal mininum size specified! Using expand.\n",
		   stderr);
	     expand_h = 1;
	  }
	if (o_minh < 1)
	  {
	     fputs("ERROR: homogeneous table based on item size but no "
		   "vertical mininum size specified! Using expand.\n",
		   stderr);
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
   Evas_Bool expand_h, expand_v;

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
   Evas_Coord x, y, w, h, cellw, cellh;
   Eina_List *l;
   Evas_Object_Table_Option *opt;

   _evas_object_table_calculate_layout_homogeneous_sizes
     (o, priv, &x, &y, &w, &h, &cellw, &cellh);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
	Evas_Object *child = opt->obj;
	Evas_Coord cx, cy, cw, ch;

	cx = x + opt->col * (cellw + priv->pad.h);
	cy = y + opt->row * (cellh + priv->pad.v);

	cw = opt->colspan * cellw - priv->pad.h;
	ch = opt->rowspan * cellh - priv->pad.v;

	_evas_object_table_calculate_cell(opt, &cx, &cy, &cw, &ch);

	evas_object_move(child, cx, cy);
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
_evas_object_table_count_expands(const Evas_Bool *expands, int start, int end)
{
   const Evas_Bool *itr = expands + start, *itr_end = expands + end;
   int count = 0;

   for (; itr < itr_end; itr++)
     if (*itr)
       count++;

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
_evas_object_table_sizes_calc_expand(Evas_Coord *sizes, int start, int end, Evas_Coord space, const Evas_Bool *expands, int expand_count)
{
   Evas_Coord *itr = sizes + start, *itr_end = sizes + end;
   const Evas_Bool *itr_expand = expands + start;
   Evas_Coord step, last_space;

   /* XXX move to fixed point math and spread errors among cells */
   step = space / expand_count;
   last_space = space - step * (expand_count - 1);

   for (; itr < itr_end; itr++, itr_expand++)
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

static void
_evas_object_table_calculate_hints_regular(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   Evas_Object_Table_Option *opt;
   Evas_Object_Table_Cache *c;
   Eina_List *l;

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

	if (opt->align.h < 0.0)
	  opt->align.h = 0.5;
	if (opt->align.v < 0.0)
	  opt->align.v = 0.5;

	if (opt->expand_h)
	  memset(c->expands.h + opt->col, 1, opt->colspan);
	if (opt->expand_v)
	  memset(c->expands.v + opt->row, 1, opt->rowspan);
     }

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
		  c->expands.h, count);
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
		  c->expands.v, count);
	     else
	       _evas_object_table_sizes_calc_noexpand
		 (c->sizes.v, opt->row, opt->end_row, space);
	  }
     }

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
   Evas_Coord *cols, *rows;
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
	     fprintf(stderr,
		     "ERROR: could not allocate temp columns (%d bytes): %s\n",
		     size, strerror(errno));
	     return;
	  }
	memcpy(cols, c->sizes.h, size);
	_evas_object_table_sizes_calc_expand
	  (cols, 0, priv->size.cols, w - c->total.min.w,
	   c->expands.h, c->total.expands.h);
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
	     fprintf(stderr,
		     "ERROR: could not allocate temp rows (%d bytes): %s\n",
		     size, strerror(errno));
	     goto end;
	  }
	memcpy(rows, c->sizes.v, size);
	_evas_object_table_sizes_calc_expand
	  (rows, 0, priv->size.rows, h - c->total.min.h,
	   c->expands.v, c->total.expands.v);
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

	evas_object_move(child, cx, cy);
	evas_object_resize(child, cw, ch);
     }

 end:
   if (cols != c->sizes.h)
     free(cols);
   if (rows != c->sizes.v)
     free(rows);
}

static void
_evas_object_table_smart_calculate_regular(Evas_Object *o, Evas_Object_Table_Data *priv)
{
   if (priv->hints_changed)
     _evas_object_table_calculate_hints_regular(o, priv);
   _evas_object_table_calculate_layout_regular(o, priv);
}

static Evas_Smart_Class _parent_sc = {NULL};

static void
_evas_object_table_smart_add(Evas_Object *o)
{
   Evas_Object_Table_Data *priv;

   priv = evas_object_smart_data_get(o);
   if (!priv)
     {
	priv = calloc(1, sizeof(*priv));
	if (!priv)
	  {
	     fputs("ERROR: could not allocate object private data.\n", stderr);
	     return;
	  }
	evas_object_smart_data_set(o, priv);
     }
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

   _parent_sc.add(o);
}

static void
_evas_object_table_smart_del(Evas_Object *o)
{
   EVAS_OBJECT_TABLE_DATA_GET(o, priv);
   Eina_List *l;

   l = priv->children;
   while (l)
     {
	Evas_Object_Table_Option *opt = l->data;
	_evas_object_table_child_disconnect(o, opt->obj);
	_evas_object_table_option_del(opt->obj);
	free(opt);
	l = eina_list_remove_list(l, l);
     }

   if (priv->cache)
     _evas_object_table_cache_free(priv->cache);

   _parent_sc.del(o);
}

static void
_evas_object_table_smart_resize(Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord ow, oh;
   evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   evas_object_smart_changed(o);
}

static void
_evas_object_table_smart_calculate(Evas_Object *o)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(o, priv);

   if ((priv->size.cols < 1) || (priv->size.rows < 1))
     {
	fprintf(stderr, "DBG: nothing to do: cols=%d, rows=%d\n",
		priv->size.cols, priv->size.rows);
	return;
     }

   if (priv->homogeneous)
     _evas_object_table_smart_calculate_homogeneous(o, priv);
   else
     _evas_object_table_smart_calculate_regular(o, priv);
}

static void
_evas_object_table_smart_set(Evas_Smart_Class *sc)
{
   if (!sc)
     return;

   if (!_parent_sc.name)
     evas_object_smart_clipped_smart_set(&_parent_sc);

   sc->add = _evas_object_table_smart_add;
   sc->del = _evas_object_table_smart_del;
   sc->move = _parent_sc.move;
   sc->resize = _evas_object_table_smart_resize;
   sc->show = _parent_sc.show;
   sc->hide = _parent_sc.hide;
   sc->color_set = _parent_sc.color_set;
   sc->clip_set = _parent_sc.clip_set;
   sc->clip_unset = _parent_sc.clip_unset;
   sc->calculate = _evas_object_table_smart_calculate;
   sc->member_add = _parent_sc.member_add;
   sc->member_del = _parent_sc.member_del;
}

static Evas_Smart *
_evas_object_table_smart_class_new(void)
{
   static Evas_Smart_Class sc = {
     "Evas_Object_Table", EVAS_SMART_CLASS_VERSION,
   };

   if (!_parent_sc.name)
     _evas_object_table_smart_set(&sc);

   return evas_smart_class_new(&sc);
}

/**
 * Create a new table.
 *
 * It's set to non-homogeneous by default, add children with
 * evas_object_table_pack().
 */
Evas_Object *
evas_object_table_add(Evas *evas)
{
   static Evas_Smart *smart = NULL;
   Evas_Object *o;

   if (!smart)
     smart = _evas_object_table_smart_class_new();

   o = evas_object_smart_add(evas, smart);
   return o;
}

/**
 * Create a table that is child of a given element @a parent.
 *
 * @see evas_object_table_add()
 */
Evas_Object *
evas_object_table_add_to(Evas_Object *parent)
{
   Evas *evas;
   Evas_Object *o;

   evas = evas_object_evas_get(parent);
   o = evas_object_table_add(evas);
   evas_object_smart_member_add(o, parent);
   return o;
}

/**
 * Set how this table should layout children.
 *
 * @todo consider aspect hint and respect it.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE
 * If table does not use homogeneous mode then columns and rows will
 * be calculated based on hints of individual cells. This operation
 * mode is more flexible, but more complex and heavy to calculate as
 * well. @b Weight properties are handled as a boolean
 * expand. Negative alignment will be considered as 0.5.
 *
 * @todo @c EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE should balance weight.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE
 * When homogeneous is relative to table the own table size is divided
 * equally among children, filling the whole table area. That is, if
 * table has @c WIDTH and @c COLUMNS, each cell will get <tt>WIDTH /
 * COLUMNS</tt> pixels. If children have minimum size that is larger
 * than this amount (including padding), then it will overflow and be
 * aligned respecting the alignment hint, possible overlapping sibling
 * cells. @b Weight hint is used as a boolean, if greater than zero it
 * will make the child expand in that axis, taking as much space as
 * possible (bounded to maximum size hint). Negative alignment will be
 * considered as 0.5.
 *
 * @par EVAS_OBJECT_TABLE_HOMOGENEOUS_ITEM
 * When homogeneous is relative to item it means the greatest minimum
 * cell size will be used. That is, if no element is set to expand,
 * the table will have its contents to a minimum size, the bounding
 * box of all these children will be aligned relatively to the table
 * object using evas_object_table_align_get(). If the table area is
 * too small to hold this minimum bounding box, then the objects will
 * keep their size and the bounding box will overflow the box area,
 * still respecting the alignment. @b Weight hint is used as a
 * boolean, if greater than zero it will make that cell expand in that
 * axis, toggling the <b>expand mode</b>, which makes the table behave
 * much like @b EVAS_OBJECT_TABLE_HOMOGENEOUS_TABLE, except that the
 * bounding box will overflow and items will not overlap siblings. If
 * no minimum size is provided at all then the table will fallback to
 * expand mode as well.
 */
void
evas_object_table_homogeneous_set(Evas_Object *o, Evas_Object_Table_Homogeneous_Mode homogeneous)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(o, priv);
   if (priv->homogeneous == homogeneous)
     return;
   priv->homogeneous = homogeneous;
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);
}

/**
 * Get the current layout homogeneous mode.
 *
 * @see evas_object_table_homogeneous_set()
 */
Evas_Object_Table_Homogeneous_Mode
evas_object_table_homogeneous_get(const Evas_Object *o)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   return priv->homogeneous;
}

/**
 * Set the alignment of the whole bounding box of contents.
 */
void
evas_object_table_align_set(Evas_Object *o, double horizontal, double vertical)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(o, priv);
   if (priv->align.h == horizontal && priv->align.v == vertical)
     return;
   priv->align.h = horizontal;
   priv->align.v = vertical;
   evas_object_smart_changed(o);
}

/**
 * Get alignment of the whole bounding box of contents.
 */
void
evas_object_table_align_get(const Evas_Object *o, double *horizontal, double *vertical)
{
   EVAS_OBJECT_TABLE_DATA_GET(o, priv);
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

/**
 * Set padding between cells.
 */
void
evas_object_table_padding_set(Evas_Object *o, Evas_Coord horizontal, Evas_Coord vertical)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(o, priv);
   if (priv->pad.h == horizontal && priv->pad.v == vertical)
     return;
   priv->pad.h = horizontal;
   priv->pad.v = vertical;
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);
}

/**
 * Get padding between cells.
 */
void
evas_object_table_padding_get(const Evas_Object *o, Evas_Coord *horizontal, Evas_Coord *vertical)
{
   EVAS_OBJECT_TABLE_DATA_GET(o, priv);
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

/**
 * Add a new child to table.
 *
 * @param col relative-horizontal position to place child.
 * @param row relative-vertical position to place child.
 * @param colspan how many relative-horizontal position to use for this child.
 * @param rowspan how many relative-vertical position to use for this child.
 *
 * @return 1 on success, 0 on failure.
 */
Evas_Bool
evas_object_table_pack(Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   Evas_Object_Table_Option *opt;

   if (rowspan < 1)
     {
	fputs("ERROR: rowspan < 1\n", stderr);
	return 0;
     }
   if (colspan < 1)
     {
	fputs("ERROR: colspan < 1\n", stderr);
	return 0;
     }

   opt = _evas_object_table_option_get(child);
   if (opt)
     {
	fputs("ERROR: cannot add object that is already part of a table!\n",
	      stderr);
	return 0;
     }

   opt = malloc(sizeof(*opt));
   if (!opt)
     {
	fputs("ERROR: could not allocate table option data.\n", stderr);
	return 0;
     }

   opt->obj = child;
   opt->col = col;
   opt->row = row;
   opt->colspan = colspan;
   opt->rowspan = rowspan;
   opt->end_col = col + colspan;
   opt->end_row = row + rowspan;
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
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);

   return 1;
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

/**
 * Remove child from table.
 *
 * @note removing a child will immediately call a walk over children in order
 *       to recalculate numbers of columns and rows. If you plan to remove
 *       all children, use evas_object_table_clear() instead.
 *
 * @return 1 on success, 0 on failure.
 */
Evas_Bool
evas_object_table_unpack(Evas_Object *o, Evas_Object *child)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   Evas_Object_Table_Option *opt;

   if (o != evas_object_smart_parent_get(child))
     {
	fputs("ERROR: cannot unpack child from incorrect table!\n", stderr);
	return 0;
     }

   opt = _evas_object_table_option_del(child);
   if (!opt)
     {
	fputs("ERROR: cannot unpack child with no packing option!\n", stderr);
	return 0;
     }

   _evas_object_table_child_disconnect(o, child);
   _evas_object_table_remove_opt(priv, opt);
   evas_object_smart_member_del(child);
   free(opt);
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);

   return 1;
}

/**
 * Faster way to remove all child objects.
 *
 * @param clear if true, it will delete just removed children.
 */
void
evas_object_table_clear(Evas_Object *o, Evas_Bool clear)
{
   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN(o, priv);
   Eina_List *l;

   l = priv->children;
   while (l)
     {
	Evas_Object_Table_Option *opt = l->data;
	_evas_object_table_child_disconnect(o, opt->obj);
	_evas_object_table_option_del(opt->obj);
	evas_object_smart_member_del(opt->obj);
	if (clear)
	  evas_object_del(opt->obj);
	free(opt);
	l = eina_list_remove_list(l, l);
     }
   priv->children = NULL;
   priv->size.cols = 0;
   priv->size.rows = 0;
   _evas_object_table_cache_invalidate(priv);
   evas_object_smart_changed(o);
}

/**
 * Get the number of columns and rows this table takes.
 *
 * @note columns and rows are virtual entities, one can specify a table
 *       with a single object that takes 4 columns and 5 rows. The only
 *       difference for a single cell table is that paddings will be
 *       accounted proportionally.
 */
void
evas_object_table_col_row_size_get(const Evas_Object *o, int *cols, int *rows)
{
   EVAS_OBJECT_TABLE_DATA_GET(o, priv);
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

/**
 * Get an iterator to walk the list of children for the table.
 *
 * @note Do not remove or delete objects while walking the list.
 */
Eina_Iterator *
evas_object_table_iterator_new(const Evas_Object *o)
{
   Evas_Object_Table_Iterator *it;

   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

   if (!priv->children) return NULL;

   it = calloc(1, sizeof(Evas_Object_Table_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_list_iterator_new(priv->children);
   it->table = o;

   it->iterator.next = FUNC_ITERATOR_NEXT(_evas_object_table_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_evas_object_table_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_evas_object_table_iterator_free);

   return &it->iterator;
}

/**
 * Get an accessor to get random access to the list of children for the table.
 *
 * @note Do not remove or delete objects while walking the list.
 */
Eina_Accessor *
evas_object_table_accessor_new(const Evas_Object *o)
{
   Evas_Object_Table_Accessor *it;

   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

   if (!priv->children) return NULL;

   it = calloc(1, sizeof(Evas_Object_Table_Accessor));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->accessor, EINA_MAGIC_ACCESSOR);

   it->real_accessor = eina_list_accessor_new(priv->children);
   it->table = o;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(_evas_object_table_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(_evas_object_table_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(_evas_object_table_accessor_free);

   return &it->accessor;
}

/**
 * Get the list of children for the table.
 *
 * @note This is a duplicate of the list kept by the table internally.
 *       It's up to the user to destroy it when it no longer needs it.
 *       It's possible to remove objects from the table when walking this
 *       list, but these removals won't be reflected on it.
 */
Eina_List *
evas_object_table_children_get(const Evas_Object *o)
{
   Eina_List *new_list = NULL, *l;
   Evas_Object_Table_Option *opt;

   EVAS_OBJECT_TABLE_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

   EINA_LIST_FOREACH(priv->children, l, opt)
      new_list = eina_list_append(new_list, opt->obj);

   return new_list;
}
