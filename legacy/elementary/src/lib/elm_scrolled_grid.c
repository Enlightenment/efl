/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Grid Scrolled Grid
 *
 * This widget aims to position objects in a grid layout while actually
 * building only the visible ones, using the same idea as genlist: the user
 * define a class for each cell, specifying functions that will be called at
 * object creation and deletion.
 *
 * Signals that you can add callbacks for are:
 *
 * clicked - The user has double-clicked a cell. The event_info parameter is
 * the grid cell that was double-clicked.
 *
 * selected - The user has made an item selected. The event_info parameter is
 * the grid cell that was selected.
 *
 * unselected - The user has made an item unselected. The event_info parameter
 * is the grid cell that was unselected.
 *
 * realized - This is called when the cell in the grid is created as a real
 * evas object. event_info is the grid cell that was created. The object may be
 * deleted at any time, so it is up to the caller to not use the object pointer
 * from elm_scrolled_grid_cell_object_get() in a way where it may point to
 * freed objects.
 *
 * drag,start,up - Called when the cell in the grid has been dragged (not
 * scrolled) up.
 *
 * drag,start,down - Called when the cell in the grid has been dragged (not
 * scrolled) down.
 *
 * drag,start,left - Called when the cell in the grid has been dragged (not
 * scrolled) left.
 *
 * drag,start,right - Called when the cell in the grid has been dragged (not
 * scrolled) right.
 *
 * drag,stop - Called when the cell in the grid has stopped being dragged.
 *
 * drag - Called when the cell in the grid is being dragged.
 *
 * scroll - called when the content has been scrolled (moved).
 *
 * scroll,drag,start - called when dragging the content has started.
 *
 * scroll,drag,stop - called when dragging the content has stopped.
 *
 *
 * A cell in the grid can have 0 or more text labels (they can be regular text
 * or textblock - that's up to the style to determine), 0 or more icons (which
 * are simply objects swallowed into the grid cell) and 0 or more boolean states
 * that can be used for check, radio or other indicators by the edje theme style.
 * A cell may be one of several styles (Elementary provides 1 by default -
 * "default", but this can be extended by system or application custom
 * themes/overlays/extensions).
 *
 * In order to implement the ability to add and delete cells on the fly, Grid
 * implements a class/callback system where the application provides a structure
 * with information about that type of cell (grid may contain multiple different
 * cells with different classes, states and styles). Grid will call the functions
 * in this struct (methods) when a cell is "realized" (that is created
 * dynamically while scrolling). All objects will simply be deleted when no
 * longer needed with evas_object_del(). The Elm_Genlist_Item_Class structure
 * contains the following members:
 *
 * cell_style - This is a constant string and simply defines the name of the
 * cell style. It must be specified and the default should be "default".
 *
 * func.label_get - This function is called when an actual cell object is
 * created. The data parameter is the one passed to elm_scrolled_grid_cell_add()
 * and related cell creation functions. The obj parameter is the grid object and
 * the part parameter is the string name of the text part in the edje design that
 * is listed as one of the possible labels that can be set. This function must
 * return a strdup'()ed string as the caller will free() it when done.
 *
 * func.icon_get - This function is called when an actual item object is
 * created. The data parameter is the one passed to elm_scrolled_grid_cell_add()
 * and related cell creation functions. The obj parameter is the grid object and
 * the part parameter is the string name of the icon part in the edje design that
 * is listed as one of the possible icons that can be set. This must return NULL
 * for no object or a valid object. The object will be deleted by grid on
 * shutdown or when the cell is unrealized.
 *
 * func.state_get - This function is called when an actual cell object is
 * created. The data parameter is the one passed to elm_scrolled_grid_cell_add()
 * and related cell creation functions. The obj parameter is the grid object and
 * the part parameter is the string name of th state part in the edje design that
 * is listed as one of the possible states that can be set. Return 0 for false
 * and 1 for true. Grid will emit a signal to the edje object with
 * "elm,state,XXX,active" "elm" when true (the default is false), where XXX is
 * the name of the part.
 *
 * func.del - This is called when elm_scrolled_grid_cell_del() is called on a
 * cell or elm_scrolled_grid_clear() is called on the grid. This is intended for
 * use when actual grid cells are deleted, so any backing data attached to the
 * cell (e.g. its data parameter on creation) can be deleted.
 *
 * If the application wants multiple cells to be able to be selected,
 * elm_scrolled_grid_multi_select_set() can enable this. If the grid is
 * single-selection only (the default), then elm_scrolled_grid_select_cell_get()
 * will return the selected cell, if any, or NULL if none is selected. If the
 * grid is multi-select then elm_scrolled_grid_selected_cells_get() will return a
 * list (that is only valid as long as no cells are modified (added, deleted,
 * selected or unselected).
 *
 * If a cell changes (state of boolean changes, label or icons change), then use
 * elm_scrolled_grid_cell_update() to have grid update the cell with the new
 * state. Grid will re-realize the cell thus call the functions in the
 * _Elm_Grid_Cell_Class for that cell.
 *
 * To programmatically (un)select a cell use elm_scrolled_grid_cell_selected_set().
 * To get its selected state use elm_scrolled_grid_cell_selected_get(). To make a
 * cell disabled (unable to be selected and appear differently) use
 * elm_scrolled_grid_cell_disable_set() to set this and
 * elm_scrolled_grid_cell_disable_get() to get the disabled state.
 *
 * Cells will only call their selection func and callback when first becoming
 * selected. Any further clicks will do nothing, unless you enable always
 * select with elm_scrolled_grid_always_select_mode_set(). This means event if
 * selected, every click will make the selected callbacks be called.
 * elm_scrolled_grid_no_select_mode_set() will turn off the ability to select
 * items entirely and they will neither appear selected nor call selected
 * callback function.
 *
 * Remember that you can create new styles and add your own theme augmentation
 * per application with elm_theme_extension_add(). If you absolutely must have a
 * specific style that overrides any theme the user or system sets up you can use
 * elm_theme_overlay_add() to add such a file.
 *
 * --
 * TODO:
 *  * Handle non-homogeneous objects too.
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Pan Pan;

#define PRELOAD 1

struct _Elm_Grid_Cell
{
   Evas_Object *base, *spacer;
   const Elm_Grid_Cell_Class *gcc;
   Ecore_Timer *long_timer;
   Widget_Data *wd;
   Eina_List *labels, *icons, *states, *icon_objs;
   const void *data;
   struct
     {
	Evas_Smart_Cb func;
	const void *data;
     } func;

   Evas_Coord x, y, dx, dy;
   int relcount;

   Eina_Bool want_unrealize : 1;
   Eina_Bool realized : 1;
   Eina_Bool dragging : 1;
   Eina_Bool down : 1;
   Eina_Bool delete_me : 1;
   Eina_Bool display_only : 1;
   Eina_Bool disabled : 1;
   Eina_Bool selected : 1;
   Eina_Bool hilighted : 1;
   Eina_Bool walking : 1;
};

struct _Widget_Data
{
   Evas_Object *self, *scr;
   Evas_Object *pan_smart;
   Pan *pan;
   Eina_List *cells;
   Ecore_Job *calc_job;
   Eina_List *selected;
   double align_x, align_y;

   Evas_Coord pan_x, pan_y;
   Evas_Coord cell_width, cell_height;	/* Each cell size */
   Evas_Coord minw, minh;		/* Total obj size */
   unsigned int nmax;

   Eina_Bool horizontal : 1;
   Eina_Bool on_hold : 1;
   Eina_Bool longpressed : 1;
   Eina_Bool multi : 1;
   Eina_Bool no_select : 1;
   Eina_Bool wasselected : 1;
   Eina_Bool always_select : 1;
};

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data *wd;
};

static const char *widtype = NULL;
static void _sizing_eval(Evas_Object *obj);
static void _cell_hilight(Elm_Grid_Cell *cell);
static void _cell_unrealize(Elm_Grid_Cell *cell);
static void _cell_select(Elm_Grid_Cell *cell);
static void _cell_unselect(Elm_Grid_Cell *cell);

static Evas_Smart_Class _pan_sc = EVAS_SMART_CLASS_INIT_VERSION;

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_object_theme_set(obj, wd->scr, "grid", "base",
                                       elm_widget_style_get(obj));
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
   elm_scrolled_grid_clear(obj);
   free(wd);
}

static void
_mouse_move(void *data, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info)
{
   Elm_Grid_Cell *cell = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
	if (!cell->wd->on_hold) {
	     cell->wd->on_hold = EINA_TRUE;
	     _cell_unselect(cell);
	}
     }
   if ((cell->dragging) && (cell->down))
     {
	if (cell->long_timer)
	  {
	     ecore_timer_del(cell->long_timer);
	     cell->long_timer = NULL;
	  }
	evas_object_smart_callback_call(cell->wd->self, "drag", cell);
	return;
     }
   if ((!cell->down) || (cell->wd->longpressed))
     {
	if (cell->long_timer)
	  {
	     ecore_timer_del(cell->long_timer);
	     cell->long_timer = NULL;
	  }
	return;
     }
   if (!cell->display_only)
     elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - cell->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   dy = y - cell->dy;
   ady = dy;
   if (ady < 0) ady = -dy;
   minw /= 2;
   minh /= 2;
   if ((adx > minw) || (ady > minh))
     {
	cell->dragging = 1;
	if (cell->long_timer)
	  {
	     ecore_timer_del(cell->long_timer);
	     cell->long_timer = NULL;
	  }
	if (cell->wd->wasselected)
	  _cell_unselect(cell);
	cell->wd->wasselected = 0;
	if (dy < 0)
	  {
	     if (ady > adx)
	       evas_object_smart_callback_call(cell->wd->self, "drag,start,up",
					       cell);
	     else
	       {
		  if (dx < 0)
		    evas_object_smart_callback_call(cell->wd->self,
						    "drag,start,left", cell);
	       }
	  }
	else
	  {
	     if (ady > adx)
	       evas_object_smart_callback_call(cell->wd->self,
					       "drag,start,down", cell);
	     else
	       {
		  if (dx < 0)
		    evas_object_smart_callback_call(cell->wd->self,
						    "drag,start,left", cell);
		  else
		    evas_object_smart_callback_call(cell->wd->self,
						    "drag,start,right", cell);
	       }
	  }
     }
}

static int
_long_press(void *data)
{
   Elm_Grid_Cell *cell = data;

   cell->long_timer = NULL;
   if ((cell->disabled) || (cell->dragging)) return 0;
   cell->wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(cell->wd->self, "longpressed", cell);
   return 0;
}

static void
_mouse_down(void *data, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info)
{
   Elm_Grid_Cell *cell = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y;

   if (ev->button != 1) return;
   cell->down = 1;
   cell->dragging = 0;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   cell->dx = ev->canvas.x - x;
   cell->dy = ev->canvas.y - y;
   cell->wd->longpressed = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) cell->wd->on_hold = EINA_TRUE;
   else cell->wd->on_hold = EINA_FALSE;
   cell->wd->wasselected = cell->selected;
   _cell_hilight(cell);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(cell->wd->self, "clicked", cell);
   if (cell->long_timer) ecore_timer_del(cell->long_timer);
   if (cell->realized)
     cell->long_timer = ecore_timer_add(1.0, _long_press, cell);
   else
     cell->long_timer = NULL;
}

static void
_mouse_up(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Grid_Cell *cell = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;

   if (ev->button != 1) return;
   cell->down = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) cell->wd->on_hold = EINA_TRUE;
   else cell->wd->on_hold = EINA_FALSE;
   if (cell->long_timer)
     {
	ecore_timer_del(cell->long_timer);
	cell->long_timer = NULL;
     }
   if (cell->dragging)
     {
	cell->dragging = EINA_FALSE;
	evas_object_smart_callback_call(cell->wd->self, "drag,stop", cell);
	dragged = EINA_TRUE;
     }
   if (cell->wd->on_hold)
     {
	cell->wd->longpressed = EINA_FALSE;
	cell->wd->on_hold = EINA_FALSE;
	return;
     }
   if (cell->wd->longpressed)
     {
	cell->wd->longpressed = EINA_FALSE;
	if (!cell->wd->wasselected)
	  _cell_unselect(cell);
	cell->wd->wasselected = EINA_FALSE;
	return;
     }
   if (dragged)
     {
	if (cell->want_unrealize)
	     _cell_unrealize(cell);
     }
   if ((cell->disabled) || dragged) return;
   if (cell->wd->multi)
     {
	if (!cell->selected)
	  {
	     _cell_hilight(cell);
	     _cell_select(cell);
	  }
	else _cell_unselect(cell);
     }
   else
     {
	if (!cell->selected)
	  {
	     while (cell->wd->selected) _cell_unselect(cell->wd->selected->data);
	  }
	else
	  {
	     const Eina_List *l, *l_next;
	     Elm_Grid_Cell *cell2;

	     EINA_LIST_FOREACH_SAFE(cell->wd->selected, l, l_next, cell2)
		if (cell2 != cell) _cell_unselect(cell2);
	  }
	_cell_hilight(cell);
	_cell_select(cell);
     }
}

static void
_cell_hilight(Elm_Grid_Cell *cell)
{
   if ((cell->wd->no_select) || (cell->delete_me) || (cell->hilighted)) return;
   edje_object_signal_emit(cell->base, "elm,state,selected", "elm");
   cell->hilighted = EINA_TRUE;
}

static void
_cell_realize(Elm_Grid_Cell *cell)
{
   char buf[1024];

   if ((cell->realized) || (cell->delete_me)) return;
   cell->base = edje_object_add(evas_object_evas_get(cell->wd->self));
   edje_object_scale_set(cell->base, elm_widget_scale_get(cell->wd->self) *
			 _elm_config->scale);
   evas_object_smart_member_add(cell->base, cell->wd->pan_smart);
   elm_widget_sub_object_add(cell->wd->self, cell->base);
   _elm_theme_object_set(cell->wd->self, cell->base, "grid", "cell/default",
                         elm_widget_style_get(cell->wd->self));
   cell->spacer = evas_object_rectangle_add(evas_object_evas_get(cell->wd->self));
   evas_object_color_set(cell->spacer, 0, 0, 0, 0);
   elm_widget_sub_object_add(cell->wd->self, cell->spacer);
   evas_object_size_hint_min_set(cell->spacer, 2 * _elm_config->scale, 1);
   edje_object_part_swallow(cell->base, "elm.swallow.pad", cell->spacer);

   if (cell->gcc->func.label_get)
     {
	const Eina_List *l;
	const char *key;

	cell->labels = _elm_stringlist_get(edje_object_data_get(cell->base,
							        "labels"));
	EINA_LIST_FOREACH(cell->labels, l, key)
	  {
	     char *s = cell->gcc->func.label_get(cell->data, cell->wd->self,
						 l->data);
	     if (s)
	       {
		  edje_object_part_text_set(cell->base, l->data, s);
		  free(s);
	       }
	  }
     }

   if (cell->gcc->func.icon_get)
     {
	const Eina_List *l;
	const char *key;

	cell->icons = _elm_stringlist_get(edje_object_data_get(cell->base,
							       "icons"));
	EINA_LIST_FOREACH(cell->icons, l, key)
	  {
	     Evas_Object *ic = cell->gcc->func.icon_get(cell->data,
							cell->wd->self,
						        l->data);
	     if (ic)
	       {
		  cell->icon_objs = eina_list_append(cell->icon_objs, ic);
		  edje_object_part_swallow(cell->base, key, ic);
		  evas_object_show(ic);
		  elm_widget_sub_object_add(cell->wd->self, ic);
	       }
	  }
     }

   if (cell->gcc->func.state_get)
     {
	const Eina_List *l;
	const char *key;

	cell->states = _elm_stringlist_get(edje_object_data_get(cell->base,
							        "states"));
	EINA_LIST_FOREACH(cell->states, l, key)
	  {
	     Eina_Bool on = cell->gcc->func.state_get(cell->data,
		   cell->wd->self, l->data);
	     if (on)
	       {
		  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
		  edje_object_signal_emit(cell->base, buf, "elm");
	       }
	  }
     }

   if (!cell->wd->cell_width && !cell->wd->cell_height)
     {
	edje_object_size_min_restricted_calc(cell->base,
	      &cell->wd->cell_width, &cell->wd->cell_height,
	      cell->wd->cell_width, cell->wd->cell_height);
	elm_coords_finger_size_adjust(1, &cell->wd->cell_width,
				      1, &cell->wd->cell_height);
     }

   evas_object_event_callback_add(cell->base, EVAS_CALLBACK_MOUSE_DOWN,
				  _mouse_down, cell);
   evas_object_event_callback_add(cell->base, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up, cell);
   evas_object_event_callback_add(cell->base, EVAS_CALLBACK_MOUSE_MOVE,
				  _mouse_move, cell);

   if (cell->selected)
     edje_object_signal_emit(cell->base, "elm,state,selected", "elm");
   if (cell->disabled)
     edje_object_signal_emit(cell->base, "elm,state,disabled", "elm");

   evas_object_show(cell->base);
   cell->realized = EINA_TRUE;
   cell->want_unrealize = EINA_FALSE;
}

static void
_cell_unrealize(Elm_Grid_Cell *cell)
{
   Evas_Object *icon;

   if (!cell->realized) return;
   if (cell->long_timer)
     {
	ecore_timer_del(cell->long_timer);
	cell->long_timer = NULL;
     }
   evas_object_del(cell->base);
   cell->base = NULL;
   evas_object_del(cell->spacer);
   cell->spacer = NULL;
   _elm_stringlist_free(cell->labels);
   cell->labels = NULL;
   _elm_stringlist_free(cell->icons);
   cell->icons = NULL;
   _elm_stringlist_free(cell->states);

   EINA_LIST_FREE(cell->icon_objs, icon)
      evas_object_del(icon);

   cell->states = NULL;
   cell->realized = EINA_FALSE;
   cell->want_unrealize = EINA_FALSE;
}

static void
_cell_place(Elm_Grid_Cell *cell, Evas_Coord cx, Evas_Coord cy)
{
   Evas_Coord x, y, ox, oy, cvx, cvy, cvw, cvh;
   Evas_Coord tch, tcw, alignw = 0, alignh = 0, vw, vh;

   cell->x = cx;
   cell->y = cy;
   evas_object_geometry_get(cell->wd->self, &ox, &oy, &vw, &vh);
   evas_output_viewport_get(evas_object_evas_get(cell->wd->self),
			    &cvx, &cvy, &cvw, &cvh);

   /* Preload rows/columns at each side of the Grid */
   cvx -= PRELOAD * cell->wd->cell_width;
   cvy -= PRELOAD * cell->wd->cell_height;
   cvw += 2 * PRELOAD * cell->wd->cell_width;
   cvh += 2 * PRELOAD * cell->wd->cell_height;

   tch = ((vh/cell->wd->cell_height)*cell->wd->cell_height);
   alignh = (vh - tch)*cell->wd->align_y;

   tcw = ((vw/cell->wd->cell_width)*cell->wd->cell_width);
   alignw = (vw - tcw)*cell->wd->align_x;

   if (cell->wd->horizontal && cell->wd->minw < vw)
     {
        int columns;

        columns = eina_list_count(cell->wd->cells)/(vh/cell->wd->cell_height);
        if (eina_list_count(cell->wd->cells) % (vh/cell->wd->cell_height))
             columns++;

        tcw = cell->wd->cell_width * columns;
	alignw = (vw - tcw)*cell->wd->align_x;
     }
   else if (cell->wd->horizontal && cell->wd->minw > vw)
        alignw = 0;   
   if (!cell->wd->horizontal && cell->wd->minh < vh)
     {
        int rows;

        rows = eina_list_count(cell->wd->cells)/(vw/cell->wd->cell_width);
        if (eina_list_count(cell->wd->cells) % (vw/cell->wd->cell_width))
             rows++;

        tch = cell->wd->cell_height * rows;
        alignh = (vh - tch)*cell->wd->align_y;
     }
   else if (!cell->wd->horizontal && cell->wd->minh > vh)
        alignh = 0;
   x = cx * cell->wd->cell_width - cell->wd->pan_x + ox + alignw;
   y = cy * cell->wd->cell_height - cell->wd->pan_y + oy + alignh;

   if (ELM_RECTS_INTERSECT(x, y, cell->wd->cell_width, cell->wd->cell_height,
			   cvx, cvy, cvw, cvh))
     {
	Eina_Bool was_realized = cell->realized;
	_cell_realize(cell);
	if (!was_realized)
	  evas_object_smart_callback_call(cell->wd->self, "realized", cell);
	evas_object_move(cell->base, x, y);
	evas_object_resize(cell->base, cell->wd->cell_width,
			   cell->wd->cell_height);
     }
   else
     _cell_unrealize(cell);
}

static Elm_Grid_Cell *
_cell_create(Widget_Data *wd, const Elm_Grid_Cell_Class *gcc,
      const void *data, Evas_Smart_Cb func, const void *func_data)
{
   Elm_Grid_Cell *cell;

   cell = calloc(1, sizeof(*cell));
   if (!cell) return NULL;
   cell->wd = wd;
   cell->gcc = gcc;
   cell->data = data;
   cell->func.func = func;
   cell->func.data = func_data;
   return cell;
}

static void
_cell_del(Elm_Grid_Cell *cell)
{
   if (cell->selected)
     cell->wd->selected = eina_list_remove(cell->wd->selected, cell);
   if (cell->realized) _cell_unrealize(cell);
   if ((!cell->delete_me) && (cell->gcc->func.del))
     cell->gcc->func.del(cell->data, cell->wd->self);
   cell->delete_me = EINA_TRUE;
   cell->wd->cells = eina_list_remove(cell->wd->cells, cell);
   if (cell->long_timer) ecore_timer_del(cell->long_timer);
   free(cell);
}

static void
_cell_select(Elm_Grid_Cell *cell)
{
   if ((cell->wd->no_select) || (cell->delete_me)) return;
   if (cell->selected)
     {
	if (cell->wd->always_select) goto call;
	return;
     }
   cell->selected = EINA_TRUE;
   cell->wd->selected = eina_list_append(cell->wd->selected, cell);
call:
   cell->walking++;
   if (cell->func.func) cell->func.func((void *)cell->func.data, cell->wd->self,
				        cell);
   if (!cell->delete_me)
     evas_object_smart_callback_call(cell->wd->self, "selected", cell);
   cell->walking--;
   if ((cell->walking == 0) && (cell->delete_me))
     if (cell->relcount == 0) _cell_del(cell);
}

static void
_cell_unselect(Elm_Grid_Cell *cell)
{
   if ((cell->delete_me) || (!cell->hilighted)) return;
   edje_object_signal_emit(cell->base, "elm,state,unselected", "elm");
   cell->hilighted = EINA_FALSE;
   if (cell->selected)
     {
	cell->selected = EINA_FALSE;
	cell->wd->selected = eina_list_remove(cell->wd->selected, cell);
	evas_object_smart_callback_call(cell->wd->self, "unselected", cell);
     }
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Evas_Coord minw = 0, minh = 0, nmax = 0, cvw, cvh;
   int count;

   evas_output_viewport_get(evas_object_evas_get(wd->self), NULL, NULL,
			    &cvw, &cvh);
   if (wd->horizontal && wd->cell_height)
     nmax = cvh / wd->cell_height;
   else if (wd->cell_width)
     nmax = cvw / wd->cell_width;

   if (nmax)
     {
	count = eina_list_count(wd->cells);
	if (wd->horizontal)
	  {
	     minw = ceil(count  / (float)nmax) * wd->cell_width;
	     minh = nmax * wd->cell_height;
	  }
	else
	  {
	     minw = nmax * wd->cell_width;
	     minh = ceil(count / (float)nmax) * wd->cell_height;
	  }
     }

   if ((minw != wd->minw) || (minh != wd->minh))
     {
	wd->minh = minh;
	wd->minw = minw;
	evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
	_sizing_eval(wd->self);
     }

   wd->nmax = nmax;
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = ELM_NEW(Pan);
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   _pan_sc.del(obj);
}

static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   if (!sd) return;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if (x)
     *x = (ow < sd->wd->minw) ? sd->wd->minw - ow : 0;
   if (y)
     *y = (oh < sd->wd->minh) ? sd->wd->minh - oh : 0;
}

static void
_pan_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord cx = 0, cy = 0;
   Eina_List *l;
   Elm_Grid_Cell *cell;

   if (!sd) return;
   if (!sd->wd->nmax) return;

   EINA_LIST_FOREACH(sd->wd->cells, l, cell)
     {
	_cell_place(cell, cx, cy);
	if (sd->wd->horizontal)
	  {
	     cy = (cy + 1) % sd->wd->nmax;
	     if (!cy) cx++;
	  }
	else
	  {
	     cx = (cx + 1) % sd->wd->nmax;
	     if (!cx) cy++;
	  }
     }
}

static void
_pan_move(Evas_Object *obj, Evas_Coord x __UNUSED__, Evas_Coord y __UNUSED__)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_hold_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_scr_drag_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "scroll,drag,start", NULL);
}

static void
_scr_drag_stop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "scroll,drag,stop", NULL);
}

static void
_scr_scroll(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "scroll", NULL);
}

/**
 * Add a new Scrolled Grid object.
 *
 * @param parent The parent object.
 * @return  The new object or NULL if it cannot be created.
 *
 * @see elm_scrolled_grid_cell_size_set()
 * @see elm_scrolled_grid_horizontal_set()
 * @see elm_scrolled_grid_cell_add()
 * @see elm_scrolled_grid_cell_del()
 * @see elm_scrolled_grid_clear()
 *
 * @ingroup Grid
 */
EAPI Evas_Object *
elm_scrolled_grid_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Evas_Coord minw, minh;
   Widget_Data *wd;
   static Evas_Smart *smart = NULL;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "grid");
   elm_widget_type_set(obj, "grid");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "grid", "base", "default");
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "drag,start", _scr_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scr_drag_stop, obj);
   evas_object_smart_callback_add(wd->scr, "scroll", _scr_scroll, obj);

   elm_smart_scroller_bounce_allow_set(wd->scr, 1, 1);

   wd->self = obj;
   wd->align_x = 0.5;
   wd->align_y = 0.5;

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   if (!smart)
     {
	static Evas_Smart_Class sc;

	evas_object_smart_clipped_smart_set(&_pan_sc);
	sc = _pan_sc;
	sc.name = "elm_scrolled_grid_pan";
	sc.version = EVAS_SMART_CLASS_VERSION;
	sc.add = _pan_add;
	sc.del = _pan_del;
	sc.resize = _pan_resize;
	sc.move = _pan_move;
	sc.calculate = _pan_calculate;
	smart = evas_smart_class_new(&sc);
     }
   if (smart)
     {
	wd->pan_smart = evas_object_smart_add(e, smart);
	wd->pan = evas_object_smart_data_get(wd->pan_smart);
	wd->pan->wd = wd;
     }

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
				     _pan_set, _pan_get,
				     _pan_max_get, _pan_child_size_get);

   _sizing_eval(obj);

   return obj;
}

/**
 * Set the size for the cell of the Grid.
 *
 * @param obj The Grid object.
 * @param w The cell's width.
 * @param h The cell's height;
 *
 * @see elm_scrolled_grid_cell_size_get()
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_cell_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->cell_width == w && wd->cell_height == h) return;
   wd->cell_width = w;
   wd->cell_height = h;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

/**
 * Get the size of the cell of the Grid.
 *
 * @param obj The Grid object.
 * @param w Pointer to the cell's width.
 * @param h Pointer to the cell's height.
 *
 * @see elm_scrolled_grid_cell_size_get()
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_cell_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (w) *w = wd->cell_width;
   if (h) *h = wd->cell_height;
}

/**
 * Set cell's alignment within the scroller.
 *
 * @param obj The grid object.
 * @param align_x The x alignment (0 <= x <= 1).
 * @param align_y The y alignment (0 <= y <= 1).
 *
 * @see elm_scrolled_grid_align_get()
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_align_set(Evas_Object *obj, double align_x, double align_y)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (align_x > 1.0)
     align_x = 1.0;
   else if (align_x < 0.0)
     align_x = 0.0;
   wd->align_x = align_x;

   if (align_y > 1.0)
     align_y = 1.0;
   else if (align_y < 0.0)
     align_y = 0.0;
   wd->align_y = align_y;
}

/**
 * Get the alignenment set for the grid object.
 *
 * @param obj The grid object.
 * @param align_x Pointer to x alignenment.
 * @param align_y Pointer to y alignenment.
 *
 * @see elm_scrolled_grid_align_set()
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_align_get(const Evas_Object *obj, double *align_x, double *align_y)
{
    ELM_CHECK_WIDTYPE(obj, widtype);
    Widget_Data *wd = elm_widget_data_get(obj);
    if (align_x) *align_x = wd->align_x;
    if (align_y) *align_y = wd->align_y;
}

/**
 * Add cell to the end of the Grid.
 *
 * @param obj The Grid object.
 * @param gcc The cell class for the cell.
 * @param data The cell data.
 * @param func Convenience function called when cell is selected.
 * @param func_data Data passed to @p func above.
 * @return A handle to the cell added or NULL if not possible.
 *
 * @see elm_scrolled_grid_cell_del()
 *
 * @ingroup Grid
 */
EAPI Elm_Grid_Cell *
elm_scrolled_grid_cell_add(Evas_Object *obj, const Elm_Grid_Cell_Class *gcc,
			   const void *data, Evas_Smart_Cb func,
			   const void *func_data)
{
   Elm_Grid_Cell *cell;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   cell = _cell_create(wd, gcc, data, func, func_data);
   if (!cell) return NULL;

   wd->cells = eina_list_append(wd->cells, cell);
   wd->no_select = EINA_FALSE;

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return cell;
}

/**
 * Remove a cell from the Grid.
 *
 * @param cell The cell to be removed.
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise.
 *
 * @see elm_scrolled_grid_clear() to remove all cells of the grid.
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_cell_del(Elm_Grid_Cell *cell)
{
   if (!cell) return;
   if ((cell->relcount > 0) || (cell->walking > 0))
     {
	cell->delete_me = EINA_TRUE;
	if (cell->selected)
	  cell->wd->selected = eina_list_remove(cell->wd->selected, cell);
	if (cell->gcc->func.del) cell->gcc->func.del(cell->data, cell->wd->self);
	return;
     }

   _cell_del(cell);

   if (cell->wd->calc_job) ecore_job_del(cell->wd->calc_job);
   cell->wd->calc_job = ecore_job_add(_calc_job, cell->wd);
}

/**
 * Set for what direction the grid will expand.
 *
 * @param obj The Grid object.
 * @param setting If @c EINA_TRUE the grid will expand horizontally or
 * vertically if @c EINA_FALSE.
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_horizontal_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (setting == wd->horizontal) return;
   wd->horizontal = setting;

   /* Update the cells to conform to the new layout */
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

/**
 * Clear the Grid
 *
 * This clears all cells in the grid, leaving it empty.
 *
 * @param obj The Grid object.
 *
 * @see elm_scrolled_grid_cell_del() to remove just one cell.
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_clear(Evas_Object *obj)
{
   Eina_List *l, *l_next;
   Elm_Grid_Cell *cell;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->calc_job)
     {
	ecore_job_del(wd->calc_job);
	wd->calc_job = NULL;
     }

   EINA_LIST_FOREACH_SAFE(wd->cells, l, l_next, cell)
     {
	if (cell->realized) _cell_unrealize(cell);
	if (cell->gcc->func.del) cell->gcc->func.del(cell->data, wd->self);
	if (cell->long_timer) ecore_timer_del(cell->long_timer);
	free(cell);
	wd->cells = eina_list_remove_list(wd->cells, l);
     }

   if (wd->selected)
     {
	eina_list_free(wd->selected);
	wd->selected = NULL;
     }

   wd->pan_x = 0;
   wd->pan_y = 0;
   wd->minw = 0;
   wd->minh = 0;
   evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
   evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
   _sizing_eval(obj);
}

/**
 * Get the real evas object of the grid cell
 *
 * This returns the actual evas object used for the specified grid cell.
 * This may be NULL as it may not be created, and may be deleted at any time
 * by grid. Do not modify this object (move, resize, show, hide etc.) as grid
 * is controlling it. This function is for querying, emitting custom signals
 * or hooking lower level callbacks for events. Do not delete this object
 * under any circumstances.
 *
 * @param cell The Grid cell.
 * @return the evas object associated to this cell.
 *
 * @see elm_scrolled_grid_cell_data_get()
 *
 * @ingroup Grid
 */
EAPI const Evas_Object *
elm_scrolled_grid_cell_object_get(Elm_Grid_Cell *cell)
{
   if (!cell) return NULL;
   return cell->base;
}

/**
 * Returns the data associated to a cell
 *
 * This returns the data value passed on the elm_scrolled_grid_cell_add() and
 * related cell addition calls.
 *
 * @param cell The Grid cell.
 * @return the data associated to this cell.
 *
 * @see elm_scrolled_grid_cell_add()
 * @see elm_scrolled_grid_cell_object_get()
 *
 * @ingroup Grid
 */
EAPI void *
elm_scrolled_grid_cell_data_get(Elm_Grid_Cell *cell)
{
   if (!cell) return NULL;
   return (void *)cell->data;
}

/**
 * Get the cell's coordinates.
 *
 * This returns the logical position of the cell whithin the Grid.
 *
 * @param cell The Grid cell.
 * @param x The x-axis coordinate pointer.
 * @param y The y-axis coordinate pointer.
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_cell_pos_get(const Elm_Grid_Cell *cell, unsigned int *x, unsigned int *y)
{
   if (!cell) return;
   if (x) *x = cell->x;
   if (y) *y = cell->y;
}

/**
 * Enable or disable multi-select in the grid.
 *
 * This enables (EINA_TRUE) or disables (EINA_FALSE) multi-select in the grid.
 * This allows more than 1 cell to be selected.
 *
 * @param obj The grid object.
 * @param multi Multi-select enabled/disabled
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_multi_select_set(Evas_Object *obj, Eina_Bool multi)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->multi = multi;
}

/**
 * Get if multi-select in grid is enabled or disabled
 *
 * @param obj The grid object
 * @return Multi-select enable/disable
 * (EINA_TRUE = enabled / EINA_FALSE = disabled)
 *
 * @ingroup Grid
 */
EAPI Eina_Bool
elm_scrolled_grid_multi_select_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->multi;
}

/**
 * Get the selected cell in the grid
 *
 * This gets the selected cell in the grid (if multi-select is enabled only
 * the first cell in the list is selected - which is not very useful, so see
 * elm_scrolled_grid_selected_cells_get() for when multi-select is used).
 *
 * If no cell is selected, NULL is returned.
 *
 * @param obj The grid object.
 * @return The selected cell, or NULL if none.
 *
 * @ingroup Grid
 */
EAPI Elm_Grid_Cell *
elm_scrolled_grid_selected_cell_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (wd->selected) return wd->selected->data;
   return NULL;
}

/**
 * Get a list of selected cells in the grid.
 *
 * This returns a list of the selected cells. This list pointer is only valid
 * so long as no cells are selected or unselected (or unselected implictly by
 * deletion). The list contains Elm_Grid_Cell pointers.
 *
 * @param obj The grid object.
 * @return The list of selected cells, or NULL if none are selected.
 *
 * @ingroup Grid
 */
EAPI const Eina_List *
elm_scrolled_grid_selected_cells_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->selected;
}

/**
 * Get the selected state of a cell.
 *
 * This gets the selected state of a cell (1 selected, 0 not selected).
 *
 * @param cell The cell
 * @return The selected state
 *
 * @ingroup Grid
 */
EAPI Eina_Bool
elm_scrolled_grid_cell_selected_get(const Elm_Grid_Cell *cell)
{
   if (!cell) return EINA_FALSE;
   return cell->selected;
}

/**
 * Sets the disabled state of a cell.
 *
 * A disabled cell cannot be selected or unselected. It will also change
 * appearance to disabled. This sets the disabled state (1 disabled, 0 not
 * disabled).
 *
 * @param cell The cell
 * @param disabled The disabled state
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_cell_disabled_set(Elm_Grid_Cell *cell, Eina_Bool disabled)
{
   if (!cell) return;
   if (cell->disabled == disabled) return;
   if (cell->delete_me) return;
   cell->disabled = disabled;
   if (cell->realized)
     {
	if (cell->disabled)
	  edje_object_signal_emit(cell->base, "elm,state,disabled", "elm");
	else
	  edje_object_signal_emit(cell->base, "elm,state,enabled", "elm");
     }
}

/**
 * Get the disabled state of a cell.
 *
 * This gets the disabled state of the given cell.
 *
 * @param cell The cell
 * @return The disabled state
 *
 * @ingroup Grid
 */
EAPI Eina_Bool
elm_scrolled_grid_cell_disabled_get(const Elm_Grid_Cell *cell)
{
   if (!cell) return EINA_FALSE;
   if (cell->delete_me) return EINA_FALSE;
   return cell->disabled;
}

/**
 * Set the always select mode.
 *
 * Cells will only call their selection func and callback when first becoming
 * selected. Any further clicks will do nothing, unless you enable always select
 * with elm_scrolled_grid_always_select_mode_set(). This means even if selected,
 * every click will make the selected callbacks be called.
 *
 * @param obj The grid object
 * @param always_select The always select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->always_select = always_select;
}

/**
 * Get the always select mode.
 *
 * @param obj The grid object.
 * @return The always select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Grid
 */
EAPI Eina_Bool
elm_scrolled_grid_always_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->always_select;
}

/**
 * Set no select mode.
 *
 * This will turn off the ability to select items entirely and they will
 * neither appear selected nor call selected callback functions.
 *
 * @param obj The grid object
 * @param no_select The no select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->no_select = no_select;
}

/**
 * Gets no select mode.
 *
 * @param obj The grid object
 * @return The no select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Grid
 */
EAPI Eina_Bool
elm_scrolled_grid_no_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->no_select;
}

/**
 * Set bounce mode.
 *
 * This will enable or disable the scroller bounce mode for the grid. See
 * elm_scroller_bounce_set() for details.
 *
 * @param obj The grid object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
}

/**
 * Get the bounce mode
 *
 * @param obj The grid object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Grid
 */
EAPI void
elm_scrolled_grid_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_get(wd->scr, h_bounce, v_bounce);
}
