#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Smart_Data Smart_Data;
typedef struct _Table_Item Table_Item;

struct _Smart_Data
{ 
   Evas_Coord       x, y, w, h;
   Evas_Object     *obj;
   Evas_Object     *clip;
   Evas_Bool        homogenous : 1;
   Evas_Bool        deleting : 1;
   Eina_List       *items;
   struct {
      int           cols, rows;
   } size;
}; 

struct _Table_Item
{
   Smart_Data      *sd;
   int              col, row, colspan, rowspan;
   Evas_Object     *obj;
};

/* local subsystem functions */
static Table_Item *_smart_adopt(Smart_Data *sd, Evas_Object *obj);
static void        _smart_disown(Evas_Object *obj);
static void        _smart_item_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void        _smart_item_changed_size_hints_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void        _smart_reconfigure(Smart_Data *sd);
static void        _smart_extents_calcuate(Smart_Data *sd);

static void _smart_init(void);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void _smart_clip_unset(Evas_Object *obj);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
_els_smart_table_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

void
_els_smart_table_homogenous_set(Evas_Object *obj, int homogenous)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (sd->homogenous == homogenous) return;
   sd->homogenous = homogenous;
   _smart_reconfigure(sd);
}

void
_els_smart_table_pack(Evas_Object *obj, Evas_Object *child, int col, int row, int colspan, int rowspan)
{
   Smart_Data *sd;
   Table_Item *ti;
   
   sd = evas_object_smart_data_get(obj);
   _smart_adopt(sd, child);
   sd->items = eina_list_append(sd->items, child);
   ti = evas_object_data_get(child, "e_table_data");
   if (ti)
     {
	ti->col = col;
	ti->row = row;
	ti->colspan = colspan;
	ti->rowspan = rowspan;
	if (sd->size.cols < (col + colspan)) sd->size.cols = col + colspan;
	if (sd->size.rows < (row + rowspan)) sd->size.rows = row + rowspan;
     }
   _smart_reconfigure(sd);
}

void
_els_smart_table_unpack(Evas_Object *obj)
{
   Table_Item *ti;
   Smart_Data *sd;
   
   ti = evas_object_data_get(obj, "e_table_data");
   if (!ti) return;
   sd = ti->sd;
   sd->items = eina_list_remove(sd->items, obj);
   _smart_disown(obj);
   if (!sd->deleting) _smart_reconfigure(sd);
}

void
_els_smart_table_col_row_size_get(Evas_Object *obj, int *cols, int *rows)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (cols) *cols = sd->size.cols;
   if (rows) *rows = sd->size.rows;
}

/* local subsystem functions */
static Table_Item *
_smart_adopt(Smart_Data *sd, Evas_Object *obj)
{
   Table_Item *ti;
   
   ti = calloc(1, sizeof(Table_Item));
   if (!ti) return NULL;
   ti->sd = sd;
   ti->obj = obj;
   /* defaults */
   ti->col = 0;
   ti->row = 0;
   ti->colspan = 1;
   ti->rowspan = 1;
   evas_object_clip_set(obj, sd->clip);
   evas_object_stack_above(obj, sd->obj);
   evas_object_smart_member_add(obj, ti->sd->obj);
   evas_object_data_set(obj, "e_table_data", ti);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
				  _smart_item_del_hook, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _smart_item_changed_size_hints_hook, NULL);
   evas_object_stack_below(obj, sd->obj);
   if ((!evas_object_visible_get(sd->clip)) &&
       (evas_object_visible_get(sd->obj)))
     evas_object_show(sd->clip);
   return ti;
}

static void
_smart_disown(Evas_Object *obj)
{
   Table_Item *ti;
   
   ti = evas_object_data_get(obj, "e_table_data");
   if (!ti) return;
   if (!ti->sd->items)
     {
	if (evas_object_visible_get(ti->sd->clip))
	  evas_object_hide(ti->sd->clip);
     }
   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
				  _smart_item_del_hook);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _smart_item_changed_size_hints_hook);
   evas_object_smart_member_del(obj);
   evas_object_data_del(obj, "e_table_data");
   free(ti);
}

static void
_smart_item_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _els_smart_table_unpack(obj);
}

static void
_smart_item_changed_size_hints_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(evas_object_smart_parent_get(obj));
   _smart_reconfigure(sd);
}

static void
_smart_reconfigure(Smart_Data *sd)
{
   Evas_Coord x, y, w, h, xx, yy;
   const Eina_List *l;
   Evas_Object *obj;
   Evas_Coord minw, minh;
   int expandw, expandh;
   double ax, ay;
   
   _smart_extents_calcuate(sd);
   
   x = sd->x;
   y = sd->y;
   w = sd->w;
   h = sd->h;

   evas_object_size_hint_min_get(sd->obj, &minw, &minh);
   evas_object_size_hint_align_get(sd->obj, &ax, &ay);
   expandw = 0;
   expandh = 0;
   if (w < minw)
     {
	x = x + ((w - minw) * (1.0 - ax));
	w = minw;
     }
   if (h < minh)
     {
	y = y + ((h - minh) * (1.0 - ay));
	h = minh;
     }
   EINA_LIST_FOREACH(sd->items, l, obj)
     {
	Table_Item *ti;
	int xw, xh;
	double wx, wy;

	ti = evas_object_data_get(obj, "e_table_data");
	evas_object_size_hint_weight_get(obj, &wx, &wy);
	xw = 0;
	xh = 0;
	if (wx > 0.0) xw = 1;
	if (wy > 0.0) xh = 1;
	if (xw) expandw++;
	if (xh) expandh++;
     }
   if (expandw == 0)
     {
	x += (w - minw) / 2;
	w = minw;
     }
   if (expandh == 0)
     {
	y += (h - minh) / 2;
	h = minh;
     }
   x = sd->x;
   y = sd->y;
   if (sd->homogenous)
     {
	EINA_LIST_FOREACH(sd->items, l, obj)
	  {
	     Table_Item *ti;
	     Evas_Coord ww, hh, ow, oh;
	     Evas_Coord mxw, mxh;
	     int xw, xh;
	     double wx, wy;

	     ti = evas_object_data_get(obj, "e_table_data");

	     xx = x + ((ti->col) * (w / (Evas_Coord)sd->size.cols));
	     yy = y + ((ti->row) * (h / (Evas_Coord)sd->size.rows));
	     ww = ((w / (Evas_Coord)sd->size.cols) * (ti->colspan));
	     hh = ((h / (Evas_Coord)sd->size.rows) * (ti->rowspan));
	     evas_object_size_hint_min_get(obj, &ow, &oh);
	     evas_object_size_hint_max_get(obj, &mxw, &mxh);
	     evas_object_size_hint_weight_get(obj, &wx, &wy);
	     evas_object_size_hint_align_get(obj, &ax, &ay);
	     xw = 0;
	     xh = 0;
	     if (wx > 0.0) xw = 1;
	     if (wy > 0.0) xh = 1;
	     if (xw) ow = ww;
	     if ((mxw >= 0) && (mxw < ow)) ow = mxw;
	     if (xh) oh = hh;
	     if ((mxh >= 0) && (mxh < oh)) oh = mxh;
	     evas_object_move(obj, 
			      xx + (Evas_Coord)(((double)(ww - ow)) * ax),
			      yy + (Evas_Coord)(((double)(hh - oh)) * ay));
	     evas_object_resize(obj, ow, oh);
	  }
     }
   else
     {
	int i, ex, tot, need, num, dif, left, nx;
	EINA_LIST_FOREACH(sd->items, l, obj)
	  {
	     Table_Item *ti;

	     ti = evas_object_data_get(obj, "e_table_data");	
	     if (sd->size.cols < (ti->col + ti->colspan))
	       sd->size.cols = ti->col + ti->colspan;
	     if (sd->size.rows < (ti->row + ti->rowspan))
	       sd->size.rows = ti->row + ti->rowspan;
	  }
	if ((sd->size.cols > 0) && (sd->size.rows > 0))
	  {
	     int *cols, *rows, *colsx, *rowsx;
	     
	     cols = calloc(sd->size.cols, sizeof(int));
	     rows = calloc(sd->size.rows, sizeof(int));
	     colsx = calloc(sd->size.cols, sizeof(int));
	     rowsx = calloc(sd->size.rows, sizeof(int));

	     EINA_LIST_FOREACH(sd->items, l, obj)
	       {
		  Table_Item *ti;
		  int xw, xh;
		  double wx, wy;

		  ti = evas_object_data_get(obj, "e_table_data");
		  evas_object_size_hint_weight_get(obj, &wx, &wy);
		  xw = 0;
		  xh = 0;
		  if (wx > 0.0) xw = 1;
		  if (wy > 0.0) xh = 1;
		  for (i = ti->col; i < (ti->col + ti->colspan); i++)
		    colsx[i] |= xw;
		  for (i = ti->row; i < (ti->row + ti->rowspan); i++)
		    rowsx[i] |= xh;
	       }

	     EINA_LIST_FOREACH(sd->items, l, obj)
	       {
		  Table_Item *ti;
		  Evas_Coord mnw, mnh, mxw, mxh;
		  int xw, xh;
		  double wx, wy;

		  ti = evas_object_data_get(obj, "e_table_data");

		  evas_object_size_hint_min_get(obj, &mnw, &mnh);
		  evas_object_size_hint_max_get(obj, &mxw, &mxh);
		  evas_object_size_hint_weight_get(obj, &wx, &wy);
		  evas_object_size_hint_align_get(obj, &ax, &ay);
		  xw = 0;
		  xh = 0;
		  if (wx > 0.0) xw = 1;
		  if (wy > 0.0) xh = 1;
		  
		  /* handle horizontal */
		  ex = 0;
		  tot = 0;
		  num = ti->colspan;
		  for (i = ti->col; i < (ti->col + num); i++)
		    {
		       if (colsx[i]) ex++;
		       tot += cols[i];
		    }
		  need = mnw;
		  if (tot < need)
		    {
		       dif = need - tot;
		       left = dif;
		       if (ex == 0)
			 {
			    nx = num;
			    for (i = ti->col; i < (ti->col + num); i++)
			      {
				 if (nx > 1)
				   {
				      cols[i] += dif / num;
				      left -= dif / num;
				   }
				 else
				   {
				      cols[i] += left;
				      left = 0;
				   }
				 nx--;
			      }
			 }
		       else
			 {
			    nx = ex;
			    for (i = ti->col; i < (ti->col + num); i++)
			      {
				 if (colsx[i])
				   {
				      if (nx > 1)
					{
					   cols[i] += dif / ex;
					   left -= dif / ex;
					}
				      else
					{
					   cols[i] += left;
					   left = 0;
					}
				      nx--;
				   }
			      }
			 }
		    }
		  
		  /* handle vertical */
		  ex = 0;
		  tot = 0;
		  num = ti->rowspan;
		  for (i = ti->row; i < (ti->row + num); i++)
		    {
		       if (rowsx[i]) ex++;
		       tot += rows[i];
		    }
		  need = mnh;
		  if (tot < need)
		    {
		       dif = need - tot;
		       left = dif;
		       if (ex == 0)
			 {
			    nx = num;
			    for (i = ti->row; i < (ti->row + num); i++)
			      {
				 if (nx > 1)
				   {
				      rows[i] += dif / num;
				      left -= dif / num;
				   }
				 else
				   {
				      rows[i] += left;
				      left = 0;
				   }
				 nx--;
			      }
			 }
		       else
			 {
			    nx = ex;
			    for (i = ti->row; i < (ti->row + num); i++)
			      {
				 if (rowsx[i])
				   {
				      if (nx > 1)
					{
					   rows[i] += dif / ex;
					   left -= dif / ex;
					}
				      else
					{
					   rows[i] += left;
					   left = 0;
					}
				      nx--;
				   }
			      }
			 }
		    }
	       }
	     
	     ex = 0;
	     for (i = 0; i < sd->size.cols; i++) { if (colsx[i]) ex++; }
	     tot = 0;
	     for (i = 0; i < sd->size.cols; i++) tot += cols[i];
	     dif = w - tot;
	     if ((ex > 0) && (dif > 0))
	       {
		  int exl;
		  
		  left = dif;
		  exl = ex;
		  for (i = 0; i < sd->size.cols; i++)
		    {
		       if (colsx[i])
			 {
			    if (exl == 1)
			      {
				 cols[i] += left;
				 exl--;
				 left = 0;
			      }
			    else
			      {			 
				 cols[i] += dif / ex;
				 exl--;
				 left -= dif / ex;
			      }
			 }
		    }
	       }
	     
	     ex = 0;
	     for (i = 0; i < sd->size.rows; i++) { if (rowsx[i]) ex++; }
	     tot = 0;
	     for (i = 0; i < sd->size.rows; i++) tot += rows[i];
	     dif = h - tot;
	     if ((ex > 0) && (dif > 0))
	       {
		  int exl;
		  
		  left = dif;
		  exl = ex;
		  for (i = 0; i < sd->size.rows; i++)
		    {
		       if (rowsx[i])
			 {
			    if (exl == 1)
			      {
				 rows[i] += left;
				 exl--;
				 left = 0;
			      }
			    else
			      {			 
				 rows[i] += dif / ex;
				 exl--;
				 left -= dif / ex;
			      }
			 }
		    }
	       }

	     EINA_LIST_FOREACH(sd->items, l, obj)
	       {
		  Table_Item *ti;
		  Evas_Coord ww, hh, ow, oh, i;
		  Evas_Coord mxw, mxh;

		  ti = evas_object_data_get(obj, "e_table_data");
		  evas_object_size_hint_min_get(obj, &ow, &oh);
		  evas_object_size_hint_max_get(obj, &mxw, &mxh);
		  evas_object_size_hint_align_get(obj, &ax, &ay);

		  xx = x;
		  for (i = 0; i < ti->col; i++) xx += cols[i];
		  ww = 0;
		  for (i = ti->col; i < (ti->col + ti->colspan); i++) ww += cols[i];
		  yy = y;
		  for (i = 0; i < ti->row; i++) yy += rows[i];
		  hh = 0;
		  for (i = ti->row; i < (ti->row + ti->rowspan); i++) hh += rows[i];

		  if (ax == -1.0) {ow = ww; ax = 0.0;}
		  if ((mxw >= 0) && (mxw < ow)) ow = mxw;
		  if (ay == -1.0) {oh = hh; ay = 0.0;}
		  if ((mxh >= 0) && (mxh < oh)) oh = mxh;
		  evas_object_move(obj, 
				   xx + (Evas_Coord)(((double)(ww - ow)) * ax),
				   yy + (Evas_Coord)(((double)(hh - oh)) * ay));
		  evas_object_resize(obj, ow, oh);
	       }
	     free(rows);
	     free(cols);
	     free(rowsx);
	     free(colsx);
	  }
     }
}

static void
_smart_extents_calcuate(Smart_Data *sd)
{
   Evas_Coord minw, minh, maxw, maxh;

   minw = 0;
   minh = 0;
   maxw = -1; /* max < 0 == unlimited */
   maxh = -1;
   sd->size.cols = 0;
   sd->size.rows = 0;
   if (sd->homogenous)
     {
	const Eina_List *l;
	const Evas_Object *obj;
	EINA_LIST_FOREACH(sd->items, l, obj)
	  {
	     Table_Item *ti;
	     int mw, mh;
	     Evas_Coord w, h;

	     ti = evas_object_data_get(obj, "e_table_data");	
	     if (sd->size.cols < (ti->col + ti->colspan))
	       sd->size.cols = ti->col + ti->colspan;
	     if (sd->size.rows < (ti->row + ti->rowspan))
	       sd->size.rows = ti->row + ti->rowspan;
	     evas_object_size_hint_min_get(obj, &w, &h);
	     mw = (w + (ti->colspan - 1)) / ti->colspan;
	     mh = (h + (ti->rowspan - 1)) / ti->rowspan;
	     if (minw < mw) minw = mw;
	     if (minh < mh) minh = mh;
	  }
	minw *= sd->size.cols;
	minh *= sd->size.rows;
     }
   else
     {
	const Eina_List *l;
	const Evas_Object *obj;
	int i, ex, tot, need, num, dif, left, nx;
	EINA_LIST_FOREACH(sd->items, l, obj)
	  {
	     Table_Item *ti;

	     ti = evas_object_data_get(obj, "e_table_data");
	     if (sd->size.cols < (ti->col + ti->colspan))
	       sd->size.cols = ti->col + ti->colspan;
	     if (sd->size.rows < (ti->row + ti->rowspan))
	       sd->size.rows = ti->row + ti->rowspan;
	  }
	if ((sd->size.cols > 0) && (sd->size.rows > 0))
	  {
	     int *cols, *rows, *colsx, *rowsx;
	     
	     cols = calloc(sd->size.cols, sizeof(int));
	     rows = calloc(sd->size.rows, sizeof(int));
	     colsx = calloc(sd->size.cols, sizeof(int));
	     rowsx = calloc(sd->size.rows, sizeof(int));

	     EINA_LIST_FOREACH(sd->items, l, obj)
	       {
		  Table_Item *ti;
		  int xw, xh;
		  double wx, wy;

		  ti = evas_object_data_get(obj, "e_table_data");
		  evas_object_size_hint_weight_get(obj, &wx, &wy);
		  xw = 0;
		  xh = 0;
		  if (wx > 0.0) xw = 1;
		  if (wy > 0.0) xh = 1;
		  for (i = ti->col; i < (ti->col + ti->colspan); i++)
		    colsx[i] |= xw;
		  for (i = ti->row; i < (ti->row + ti->rowspan); i++)
		    rowsx[i] |= xh;
	       }

	     EINA_LIST_FOREACH(sd->items, l, obj)
	       {
		  Table_Item *ti;
		  Evas_Coord w, h;

		  ti = evas_object_data_get(obj, "e_table_data");
		  evas_object_size_hint_min_get(obj, &w, &h);

		  /* handle horizontal */
		  ex = 0;
		  tot = 0;
		  num = ti->colspan;
		  for (i = ti->col; i < (ti->col + num); i++)
		    {
		       if (colsx[i]) ex++;
		       tot += cols[i];
		    }
		  need = w;
		  if (tot < need)
		    {
		       dif = need - tot;
		       left = dif;
		       if (ex == 0)
			 {
			    nx = num;
			    for (i = ti->col; i < (ti->col + num); i++)
			      {
				 if (nx > 1)
				   {
				      cols[i] += dif / num;
				      left -= dif / num;
				   }
				 else
				   {
				      cols[i] += left;
				      left = 0;
				   }
				 nx--;
			      }
			 }
		       else
			 {
			    nx = ex;
			    for (i = ti->col; i < (ti->col + num); i++)
			      {
				 if (colsx[i])
				   {
				      if (nx > 1)
					{
					   cols[i] += dif / ex;
					   left -= dif / ex;
					}
				      else
					{
					   cols[i] += left;
					   left = 0;
					}
				      nx--;
				   }
			      }
			 }
		    }
		  
		  /* handle vertical */
		  ex = 0;
		  tot = 0;
		  num = ti->rowspan;
		  for (i = ti->row; i < (ti->row + num); i++)
		    {
		       if (rowsx[i]) ex++;
		       tot += rows[i];
		    }
		  need = h;
		  if (tot < need)
		    {
		       dif = need - tot;
		       left = dif;
		       if (ex == 0)
			 {
			    nx = num;
			    for (i = ti->row; i < (ti->row + num); i++)
			      {
				 if (nx > 1)
				   {
				      rows[i] += dif / num;
				      left -= dif / num;
				   }
				 else
				   {
				      rows[i] += left;
				      left = 0;
				   }
				 nx--;
			      }
			 }
		       else
			 {
			    nx = ex;
			    for (i = ti->row; i < (ti->row + num); i++)
			      {
				 if (rowsx[i])
				   {
				      if (nx > 1)
					{
					   rows[i] += dif / ex;
					   left -= dif / ex;
					}
				      else
					{
					   rows[i] += left;
					   left = 0;
					}
				      nx--;
				   }
			      }
			 }
		    }
	       }
	     for (i = 0; i < sd->size.cols; i++) minw += cols[i];
	     for (i = 0; i < sd->size.rows; i++) minh += rows[i];
	     free(rows);
	     free(cols);
	     free(rowsx);
	     free(colsx);
	  }
     }
   evas_object_size_hint_min_set(sd->obj, minw, minh);
}

static void
_smart_init(void)
{
   if (_e_smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     "e_table",
	       EVAS_SMART_CLASS_VERSION,
	       _smart_add,
	       _smart_del,
	       _smart_move,
	       _smart_resize,
	       _smart_show,
	       _smart_hide,
	       _smart_color_set,
	       _smart_clip_set,
	       _smart_clip_unset,
	       NULL,
	       NULL,
	       NULL,
	       NULL
	  };
	_e_smart = evas_smart_class_new(&sc);
     }
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(sd->clip, obj);
   evas_object_move(sd->clip, -100002, -100002);
   evas_object_resize(sd->clip, 200004, 200004);
   evas_object_color_set(sd->clip, 255, 255, 255, 255);
   evas_object_smart_data_set(obj, sd);
}
   
static void
_smart_del(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->deleting = 1;
   while (sd->items)
     {
	Evas_Object *child;
	
	child = sd->items->data;
	_els_smart_table_unpack(child);
     }
   evas_object_del(sd->clip);
   free(sd);
   evas_object_smart_data_set(obj, NULL);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd;
   const Eina_List *l;
   Evas_Object *child;
   Evas_Coord dx, dy;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   dx = x - sd->x;
   dy = y - sd->y;
   EINA_LIST_FOREACH(sd->items, l, child)
     {
	Evas_Coord ox, oy;

	evas_object_geometry_get(child, &ox, &oy, NULL, NULL);
	evas_object_move(child, ox + dx, oy + dy);
     }
   sd->x = x;
   sd->y = y;
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
}

static void
_smart_show(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->items) evas_object_show(sd->clip);
}

static void
_smart_hide(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->clip);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;   
   evas_object_color_set(sd->clip, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->clip, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->clip);
}  
