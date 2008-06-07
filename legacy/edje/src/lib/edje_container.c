/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"
#include "edje_container.h"

#if 0

static void
_edje_container_relayout(Smart_Data *sd)
{
   Evas_List *l;
   Evas_Coord x, y, w, h, sw;

   if (sd->freeze > 0) return;
   if (!sd->need_layout) return;

   if (sd->w < sd->min_w) sw = sd->min_w;
   else if (sd->w > sd->max_w) sw = sd->max_w;
   else sw = sd->w;

   y = 0;
   x = 0;
   w = 0;
   h = 0;

   for (l = sd->children; l; l = l->next)
     {
	Edje_Item *ei;

	ei = l->data;
	if (sd->homogenous) h = sd->min_row_h;

	ei->y = y;
	ei->h = h;
//	ei->w = w;
//	ei->h = h;
     }

   sd->need_layout = 0;
}

static void
_edje_container_recalc(Smart_Data *sd)
{
   Evas_List *l;
   int any_max_h = 0, any_max_w = 0;
   int i;

   if (sd->freeze > 0) return;
   if (!sd->changed) return;

   sd->min_h = 0;
   sd->max_h = -1;
   sd->min_w = 0;
   sd->max_w = -1;
   sd->min_row_h = 0;
   sd->max_row_h = -1;
   sd->contents_h = 0;
   sd->contents_w = 0;

   for (i = 0; i < sd->cols; i++)
     {
	sd->min_w += sd->colinfo[i].minw;
	if (sd->colinfo[i].maxw >= 0)
	  {
	     if (sd->max_w >= 0)
	       sd->max_w += sd->colinfo[i].maxw;
	     else
	       sd->max_w = sd->colinfo[i].maxw;
	  }
	else
	  any_max_w = 1;
     }
   if (any_max_w) sd->max_w = -1;

   if (sd->w < sd->min_w)
     sd->contents_w = sd->min_w;
   else if ((sd->max_w >= 0) && (sd->w < sd->max_w))
     sd->w = sd->max_w;

   for (l = sd->children; l; l = l->next)
     {
	Edje_Item *ei;

	ei = l->data;
	if (ei->minh > sd->min_row_h)
	  sd->min_row_h = ei->minh;
	if (sd->max_row_h >= 0)
	  {
	     if (ei->maxh >= 0)
	       {
		  if (sd->max_row_h > ei->maxh)
		    sd->max_row_h = ei->maxh;
	       }
	     else
	       any_max_h = 1;
	  }
	sd->min_h += ei->minh;
	if (ei->maxh >= 0)
	  {
	     if (sd->max_h >= 0)
	       sd->max_h += ei->maxh;
	     else
	       sd->max_h = ei->maxh;
	  }
	else
	  any_max_h = 1;
     }
   if (any_max_h)
     {
	sd->max_h = -1;
	sd->max_row_h = -1;
     }
   if (sd->homogenous)
     {
	sd->min_h = evas_list_count(sd->children) * sd->min_row_h;
     }

   sd->changed = 0;
   sd->change_child = 0;
   sd->change_child_list = 0;
   sd->change_cols = 0;

   sd->need_layout = 1;
   _edje_container_relayout(sd);
}

static void
_edje_item_recalc(Edje_Item *ei)
{
   int i;

   if (ei->freeze > 0) return;
   if (!ei->recalc) return;
   if (!ei->sd) return;

   ei->minh = 0;
   ei->maxh = -1;
   for (i = 0; i < ((Smart_Data *)(ei->sd))->cols; i++)
     {
	if (ei->cells[i].minh > ei->minh) ei->minh = ei->cells[i].minh;
	if (ei->cells[i].maxh >= 0)
	  {
	     if (ei->maxh >= 0)
	       {
		  if (ei->cells[i].maxh < ei->maxh)
		    ei->maxh = ei->cells[i].maxh;
	       }
	     else
	       ei->maxh = ei->cells[i].maxh;
	  }
	if (((Smart_Data *)(ei->sd))->colinfo[i].minw < ei->cells[i].minw)
	  ((Smart_Data *)(ei->sd))->colinfo[i].minw = ei->cells[i].minw;
	if (((Smart_Data *)(ei->sd))->colinfo[i].maxw >= 0)
	  {
	     if (ei->cells[i].maxw >= 0)
	       {
		  if (((Smart_Data *)(ei->sd))->colinfo[i].maxw > ei->cells[i].maxw)
		    ((Smart_Data *)(ei->sd))->colinfo[i].maxw = ei->cells[i].maxw;
	       }
	  }
	else
	  ((Smart_Data *)(ei->sd))->colinfo[i].maxw = ei->cells[i].maxw;
     }

   ei->recalc = 0;

   _edje_container_recalc(ei->sd);
}


/*****************************/

Edje_Item *
edje_item_add(Edje_Item_Class *cl, void *data)
{
   Edje_Item *ei;

   ei = calloc(sizeof(Edje_Item), 1);

   ei->class = cl;
   ei->class_data = data;

   return ei;
}

void
edje_item_del(Edje_Item *ei)
{
   Smart_Data *sd;

   sd = ei->sd;
   if (ei->object) evas_object_del(ei->object);
   if (ei->overlay_object) evas_object_del(ei->overlay_object);
   free(ei);
   if (!sd) return;
   sd->changed = 1;
   sd->change_child_list = 1;
   _edje_container_recalc(sd);
}

Evas_Object *
edje_item_container_get(Edje_Item *ei)
{
   if (!ei->sd) return NULL;
   return ((Smart_Data *)(ei->sd))->smart_obj;
}

/* an arbitary data pointer to use to track other data */

void
edje_item_data_set(Edje_Item *ei, void *data)
{
   ei->data = data;
}

void *
edje_item_data_get(Edje_Item *ei)
{
   return ei->data;
}

/* this object covers the entire item */
void
edje_item_overlay_object_set(Edje_Item *ei, Evas_Object *obj)
{
   if (ei->overlay_object)
     {
	/* FIXME: if it changed - remove...*/
     }
   ei->overlay_object = obj;
   if (ei->sd)
     evas_object_smart_member_add(((Smart_Data *)(ei->sd))->smart_obj, obj);
}

Evas_Object *
edje_item_overlay_object_get(Edje_Item *ei)
{
   return ei->overlay_object;
}

/* this object goes under entire item */
void
edje_item_object_set(Edje_Item *ei, Evas_Object *obj)
{
   if (ei->object)
     {
	/* FIXME: if it changed - remove...*/
     }
   ei->object = obj;
   if (ei->sd)
     evas_object_smart_member_add(((Smart_Data *)(ei->sd))->smart_obj, obj);
}

Evas_Object *
edje_item_object_get(Edje_Item *ei)
{
   return ei->object;
}

/* optionally you can manage each column's object yourself OR let Edje do it */
void
edje_item_object_column_set(Edje_Item *ei, int col, Evas_Object *obj)
{
   if (ei->cells_num <= (col + 1))
     {
	/* FIXME: unsafe realloc */
	ei->cells = realloc(ei->cells, sizeof(Edje_Item_Cell) * col);
	ei->cells_num = col + 1;
     }
   ei->cells[col].obj = obj;
}

Evas_Object *
edje_item_object_column_get(Edje_Item *ei, int col)
{
   if (ei->cells_num <= (col + 1)) return NULL;
   return ei->cells[col].obj;
}

/* query the item for the items preferred co-ords */
void
edje_item_geometry_get(Edje_Item *ei, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (!ei->sd)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   if (x) *x = ((Smart_Data *)(ei->sd))->x;
   if (y) *y = ((Smart_Data *)(ei->sd))->y + ei->y;
   if (w) *w = ((Smart_Data *)(ei->sd))->w;
   if (h) *h = ei->h;
}

/* freeze and thaw items if u are about to do a bunch of changes */
int
edje_item_freeze(Edje_Item *ei)
{
   ei->freeze++;
   return ei->freeze;
}

int
edje_item_thaw(Edje_Item *ei)
{
   ei->freeze--;
   if (ei->freeze > 0) return ei->freeze;
   if (!ei->sd) return ei->freeze;
   if (ei->recalc) _edje_item_recalc(ei);
   return ei->freeze;
}

/* column info */
void
edje_item_column_size_set(Edje_Item *ei, int col, Evas_Coord minw, Evas_Coord maxw, Evas_Coord minh, Evas_Coord maxh)
{
   if (ei->cells_num <= (col + 1))
     {
	/* FIXME: unsafe realloc */
	ei->cells = realloc(ei->cells, sizeof(Edje_Item_Cell) * col);
	ei->cells_num = col + 1;
     }
   if ((ei->cells[col].minw == minw) &&
       (ei->cells[col].minh == minh) &&
       (ei->cells[col].maxw == maxw) &&
       (ei->cells[col].maxh == maxh)) return;
   ei->cells[col].minh = minh;
   ei->cells[col].maxh = maxh;
   ei->cells[col].minw = minw;
   ei->cells[col].maxw = maxw;
   ei->recalc = 1;
   if (ei->sd)
     {
	((Smart_Data *)(ei->sd))->changed = 1;
	((Smart_Data *)(ei->sd))->change_child = 1;
     }
   _edje_item_recalc(ei);
}

void
edje_item_column_size_get(Edje_Item *ei, int col, Evas_Coord *minw, Evas_Coord *maxw, Evas_Coord *minh, Evas_Coord *maxh)
{
   if (ei->cells_num <= (col + 1))
     {
	if (minw) *minw = 0;
	if (minh) *minh = 0;
	if (maxw) *maxw = -1;
	if (maxh) *maxh = -1;
     }
   if (minw) *minw = ei->cells[col].minw;
   if (minh) *minh = ei->cells[col].minh;
   if (maxw) *maxw = ei->cells[col].maxw;
   if (maxh) *maxh = ei->cells[col].maxh;
}

/* selection stuff */
void
edje_item_select(Edje_Item *ei)
{
   ei->selected = 1;
   /* FIXME: trigger item to change visually */
}

void
edje_item_unselect(Edje_Item *ei)
{
   ei->selected = 0;
   /* FIXME: trigger item to change visually */
}

/* focus stuff - only 1 can be focuesd */
void
edje_item_focus(Edje_Item *ei)
{
//   ei->focused = 1;
}

void
edje_item_unfocus(Edje_Item *ei)
{
//   ei->focused = 0;
}

/* disable/enable stuff - stops focus and selection working on these items */
void
edje_item_enable(Edje_Item *ei)
{
//   ei->disabled = 0;
}

void
edje_item_disable(Edje_Item *ei)
{
//   ei->disabled = 1;
}

/* item utils */
int
edje_item_selected_get(Edje_Item *ei)
{
   return ei->selected;
}

int
edje_item_focused_get(Edje_Item *ei)
{
   return ei->focused;
}

int
edje_item_disabled_get(Edje_Item *ei)
{
   return ei->disabled;
}

/***** container calls *****/

int
edje_container_freeze(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   sd->freeze++;
   return sd->freeze;
}

int
edje_container_thaw(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   sd->freeze--;
   if (sd->freeze <= 0) _edje_container_recalc(sd);
   return sd->freeze;
}

void
edje_container_item_append(Evas_Object *obj, Edje_Item *ei)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->children = evas_list_append(sd->children, ei);
   sd->changed = 1;
   sd->change_child_list = 1;
   sd->rows = evas_list_count(sd->children);
   _edje_container_recalc(sd);
}

void
edje_container_item_prepend(Evas_Object *obj, Edje_Item *ei)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->children = evas_list_prepend(sd->children, ei);
   sd->changed = 1;
   sd->change_child_list = 1;
   sd->rows = evas_list_count(sd->children);
   _edje_container_recalc(sd);
}

void
edje_container_item_append_relative(Evas_Object *obj, Edje_Item *ei, Edje_Item *rel)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->children = evas_list_append_relative(sd->children, ei, rel);
   sd->changed = 1;
   sd->change_child_list = 1;
   sd->rows = evas_list_count(sd->children);
   _edje_container_recalc(sd);
}

void
edje_container_item_prepend_relative(Evas_Object *obj, Edje_Item *ei, Edje_Item *rel)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->children = evas_list_prepend_relative(sd->children, ei, rel);
   sd->changed = 1;
   sd->change_child_list = 1;
   sd->rows = evas_list_count(sd->children);
   _edje_container_recalc(sd);
}

void
edje_container_item_insert(Evas_Object *obj, Edje_Item *ei, int n)
{
   Smart_Data *sd;
   void *rel;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   rel = evas_list_nth(sd->children, n);
   if (!rel)
     sd->children = evas_list_append(sd->children, ei);
   else
     sd->children = evas_list_append_relative(sd->children, ei, rel);
   sd->changed = 1;
   sd->change_child_list = 1;
   sd->rows = evas_list_count(sd->children);
   _edje_container_recalc(sd);
}

void
edje_container_item_remove(Evas_Object *obj, Edje_Item *ei)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->children = evas_list_remove(sd->children, ei);
   sd->changed = 1;
   sd->change_child_list = 1;
   sd->rows = evas_list_count(sd->children);
   _edje_container_recalc(sd);
}

void
edje_container_columns_set(Evas_Object *obj, int cols)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->cols == cols) return;
   sd->colinfo = realloc(sd->colinfo, cols * sizeof(Smart_Data_Colinfo));
   if (cols > sd->cols)
     {
	int i;

	for (i = sd->cols; i < cols; i++)
	  {
	     sd->colinfo[i].minw = 0;
	     sd->colinfo[i].maxw = -1;
	  }
     }
   sd->cols = cols;
   sd->changed = 1;
   sd->change_cols = 1;
   _edje_container_recalc(sd);
}

int
edje_container_columns_get(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->cols;
}

void
edje_container_min_size_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->changed)
     {
	int freeze;

	freeze = sd->freeze;
	sd->freeze = 0;
	_edje_container_recalc(sd);
	sd->freeze = freeze;
     }
   if (minw) *minw = sd->min_w;
   if (minh) *minh = sd->min_h;
}

void
edje_container_max_size_get(Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->changed)
     {
	int freeze;

	freeze = sd->freeze;
	sd->freeze = 0;
	_edje_container_recalc(sd);
	sd->freeze = freeze;
     }
   if (maxw) *maxw = sd->max_w;
   if (maxh) *maxh = sd->max_h;
}

void
edje_containter_align_set(Evas_Object *obj, double halign, double valign)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->align_x == halign) && (sd->align_y == valign)) return;
   sd->align_x = halign;
   sd->align_y = valign;
   sd->need_layout = 1;
   _edje_container_relayout(sd);
}

void
edje_container_align_get(Evas_Object *obj, double *halign, double *valign)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (halign) *halign = sd->align_x;
   if (valign) *valign = sd->align_y;
}

int
edje_container_count_get(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return evas_list_count(sd->children);
}

Edje_Item *
edje_container_item_first_get(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   if (!sd->children) return NULL;
   return sd->children->data;
}

Edje_Item *
edje_container_item_last_get(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   if (!sd->children) return NULL;
   return evas_list_last(sd->children)->data;
}

Edje_Item *
edje_container_item_nth_get(Evas_Object *obj, int n)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   return evas_list_nth(sd->children, n);
}

void
edje_container_homogenous_size_set(Evas_Object *obj, int homog)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((homog) && (sd->homogenous)) ||
       ((!homog) && (!sd->homogenous))) return;
   sd->homogenous = homog;
   sd->changed = 1;
   sd->change_child = 1;
   _edje_container_recalc(sd);
}

int
edje_container_homogenous_size_get(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->homogenous;
}

void
edje_container_scroll_set(Evas_Object *obj, double pos, double shift)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->scroll_y == pos) && (sd->scroll_x == shift)) return;
   sd->scroll_y = pos;
   sd->scroll_x = shift;
   sd->need_layout = 1;
   _edje_container_relayout(sd);
}

void
edje_container_scroll_get(Evas_Object *obj, double *pos, double *shift)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (pos) *pos = sd->scroll_y;
   if (shift) *shift = sd->scroll_x;
}

static void _smart_init(void);
static void _smart_add(Evas_Object * obj);
static void _smart_del(Evas_Object * obj);
static void _smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object * obj);
static void _smart_hide(Evas_Object * obj);
static void _smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object * obj);

static Evas_Smart  *smart = NULL;

Evas_Object *
edje_container_object_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, smart);
}

/*******************************************/
/* Internal smart object required routines */
/*******************************************/
static void
_smart_init(void)
{
   if (smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     E_OBJ_NAME,
	       _smart_add,
	       _smart_del,
	       _smart_move,
	       _smart_resize,
	       _smart_show,
	       _smart_hide,
	       _smart_color_set,
	       _smart_clip_set,
	       _smart_clip_unset,
	       NULL
	  };
        smart = evas_smart_class_new(&sc);
     }
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
//   evas_object_smart_member_add(sd->obj, obj);
   evas_object_smart_data_set(obj, sd);
   sd->smart_obj = obj;
}

static void
_smart_del(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->colinfo) free(sd->colinfo);
//   evas_object_del(sd->obj);
   free(sd);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_move(sd->obj, x, y);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_resize(sd->obj, w, h);
}

static void
_smart_show(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_show(sd->obj);
}

static void
_smart_hide(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_hide(sd->obj);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_color_set(sd->obj, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_clip_set(sd->obj, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_clip_unset(sd->obj);
}

#endif
