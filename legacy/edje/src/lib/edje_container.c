#include "Edje.h"
#include "edje_private.h"

#define E_SMART_OBJ_GET(smart, o, type) \
     { \
	char *_e_smart_str; \
	\
	if (!o) return; \
	smart = evas_object_smart_data_get(o); \
	if (!smart) return; \
	_e_smart_str = (char *)evas_object_type_get(o); \
	if (!_e_smart_str) return; \
	if (strcmp(_e_smart_str, type)) return; \
     }

#define E_SMART_OBJ_GET_RETURN(smart, o, type, ret) \
   { \
      char *_e_smart_str; \
      \
      if (!o) return ret; \
      smart = evas_object_smart_data_get(o); \
      if (!smart) return ret; \
      _e_smart_str = (char *)evas_object_type_get(o); \
      if (!_e_smart_str) return ret; \
      if (strcmp(_e_smart_str, type)) return ret; \
   }

#define E_OBJ_NAME "edje_container_object"

typedef struct _Smart_Data   Smart_Data;
typedef struct _Smart_Item   Smart_Item;
typedef struct _Smart_Column Smart_Column;

struct _Smart_Data
{
   int layout;
   Evas_List *children;
   struct {
      int w, h;
   } min, max;
};

struct _Smart_Item
{
   Evas_Object *container;
   Evas_Object *obj;
};

struct _Smart_Column
{
   int min, max;
};

#define EDJE_LAYOUT_NONE  0

/* the following are "linear" layout systems */

/* H & V LIST pack all items at their minimum size - no expanding in the lists
 * direction (lets say height), BUT all items are expanded to fill the "width"
 * of the list. if an item is too small to fit the width, the list object will
 * call a "min_size_change" callback indicating to the parent/swallower that
 * the parent should revise its use/view of this object. this is intended for
 * large lists of items - like a list of files, or titles etc.  this allows
 * for each item to have multiple columns. each column may be any size, with
 * the minimu size being the sum of all minimum column widths. as more items
 * are added column widths may be adjusted and all items told of this
 * adjustment
 */
#define EDJE_LAYOUT_VLIST 2
#define EDJE_LAYOUT_HLIST 1
/* H & V BOX pack items and may or may not expand an item in any direction and
 * may align an item smaller than its allocated space in a certain way. they
 * dont know about columns etc. like lists.
 */
#define EDJE_LAYOUT_VBOX  4
#define EDJE_LAYOUT_HBOX  3
/* H & V flow are like "file manager" views you see in explorer etc. wehere
 * items "line wrap" as they go along horizontally (or vertizally) as needed
 */
#define EDJE_LAYOUT_VFLOW 6
#define EDJE_LAYOUT_HFLOW 5

/* the following are "2 dimensional" layout systems */

/* tables are full 2-dimensional layouts which dont really have append and
 * prepend semantics... this will need working on later for 2d layouts
 */
#define EDJE_LAYOUT_TABLE 7
#define EDJE_LAYOUT_COUNT 8

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
_edje_container_object_add(Evas *evas)
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
