#include "Edje.h"
#include "edje_private.h"
#include "edje_container.h"

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
   if (ei->object) evas_object_del(ei->object);
   if (ei->overlay_object) evas_object_del(ei->overlay_object);
   free(ei);
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

/* optionally you can manage each column's object yourself OR let edje do it */
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
   if (ei->recalc)
     {
	/* FIXME: recalc item */
     }
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
   ei->cells[col].minw = minw;
   ei->cells[col].minh = minh;
   ei->cells[col].maxw = maxw;
   ei->cells[col].maxh = maxh;
   ei->recalc = 1;
   if (ei->freeze > 0) return;
   /* FIXME: recalc item */
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
}

void
edje_item_unselect(Edje_Item *ei)
{
   ei->selected = 0;
}

/* focus stuff - only 1 can be focuesd */
void
edje_item_focus(Edje_Item *ei)
{
   ei->focused = 1;
}

void
edje_item_unfocus(Edje_Item *ei)
{
   ei->focused = 0;
}

/* disable/enable stuff - stops focus and selection working on these items */
void
edje_item_enable(Edje_Item *ei)
{
   ei->disabled = 0;
}

void
edje_item_disable(Edje_Item *ei)
{
   ei->disabled = 1;
}

/* item utils */
Edje_Item *
edje_item_next_get(Edje_Item *ei)
{
}

Edje_Item *
edje_item_prev_get(Edje_Item *ei)
{
}

int
edje_item_selected_get(Edje_Item *ei)
{
}

int
edje_item_focused_get(Edje_Item *ei)
{
}

int
edje_item_disabled_get(Edje_Item *ei)
{
}

double
edje_item_position_get(Edje_Item *ei)
{
}

void
edje_item_offset_set(Edje_Item *ei, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
}

/***** container calls *****/

void
edje_container_item_append(Evas_Object *obj, Edje_Item *ei)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_item_prepend(Evas_Object *obj, Edje_Item *ei)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_item_append_relative(Evas_Object *obj, Edje_Item *ei, Edje_Item *rel)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_item_prepend_relative(Evas_Object *obj, Edje_Item *ei, Edje_Item *rel)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_item_insert(Evas_Object *obj, Edje_Item *ei, int n)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_item_remove(Evas_Object *obj, Edje_Item *ei)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_columns_set(Evas_Object *obj, int cols)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

int
edje_container_columns_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_min_size_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_max_size_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_containter_align_set(Evas_Object *obj, double halign, double valign)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_align_get(Evas_Object *obj, double *halign, double *valign)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

int
edje_container_count_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

Edje_Item *
edje_container_item_first_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

Edje_Item *
edje_container_item_last_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

Edje_Item *
edje_container_item_nth_get(Evas_Object *obj, int n)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_homogenous_size_set(Evas_Object *obj, int homog)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

int
edje_container_homogenous_size_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_orientation_set(Evas_Object *obj, int orient)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

int
edje_container_orientation_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_scroll_set(Evas_Object *obj, double pos, double shift)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

void
edje_container_scroll_get(Evas_Object *obj, double *pos, double *shift)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
}

static void _smart_init(void);
static void _smart_add(Evas_Object * obj);
static void _smart_del(Evas_Object * obj);
static void _smart_layer_set(Evas_Object * obj, int layer);
static void _smart_raise(Evas_Object * obj);
static void _smart_lower(Evas_Object * obj);
static void _smart_stack_above(Evas_Object * obj, Evas_Object * above);
static void _smart_stack_below(Evas_Object * obj, Evas_Object * below);
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
   smart = evas_smart_new(E_OBJ_NAME,
			  _smart_add,
			  _smart_del,
			  _smart_layer_set,
			  _smart_raise,
			  _smart_lower,
			  _smart_stack_above,
			  _smart_stack_below,
			  _smart_move,
			  _smart_resize,
			  _smart_show,
			  _smart_hide,
			  _smart_color_set,
			  _smart_clip_set,
			  _smart_clip_unset,
			  NULL);
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
//   evas_object_del(sd->obj);
   free(sd);
}

static void
_smart_layer_set(Evas_Object *obj, int layer)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_layer_set(sd->obj, layer);
}
   
static void
_smart_raise(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_raise(sd->obj);
}

static void
_smart_lower(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_lower(sd->obj);
}
   
static void
_smart_stack_above(Evas_Object *obj, Evas_Object *above)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_stack_above(sd->obj, above);
}

static void
_smart_stack_below(Evas_Object *obj, Evas_Object *below)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
//   evas_object_stack_below(sd->obj, below);
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
