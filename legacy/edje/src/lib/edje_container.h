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

struct _Smart_Data
{
   Evas_Coord     x, y, w, h;
   Evas_List     *children;
   struct {
      Evas_Coord  w, h;
   } min, max;
   Evas_Object   *smart_obj;
   int            cols, rows;
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
#define EDJE_LAYOUT_VLIST 1
#define EDJE_LAYOUT_HLIST 2
/* H & V BOX pack items and may or may not expand an item in any direction and
 * may align an item smaller than its allocated space in a certain way. they
 * dont know about columns etc. like lists.
 */
#define EDJE_LAYOUT_VBOX  3
#define EDJE_LAYOUT_HBOX  4
/* H & V flow are like "file manager" views you see in explorer etc. wehere
 * items "line wrap" as they go along horizontally (or vertizally) as needed
 */
#define EDJE_LAYOUT_VFLOW 5
#define EDJE_LAYOUT_HFLOW 6

/* the following are "2 dimensional" layout systems */

/* tables are full 2-dimensional layouts which dont really have append and
 * prepend semantics... this will need working on later for 2d layouts. dont
 * worry about these yet - but keep this as a marker for things to do later
 */
#define EDJE_LAYOUT_TABLE 7
/* count
 */
#define EDJE_LAYOUT_COUNT 8


/* All items are virtual constructs that provide Evas_Objects at some point.
 * Edje may move, resize, show, hide, clip, unclip, raise, lower etc. this
 * item AFTER it calls the item's add() method and before it calls the del()
 * method. Edje may call add() and del() at any time as often items may not
 * be visible and so may not need to exist at all - they are merely information
 * used for layout, and nothing more. this helps save cpu and memory keeping
 * things responsive for BIG lists of items. you create an item from an item
 * class then ask that item to be appended/prepended etc. to the container.
 */
typedef struct _Edje_Item       Edje_Item;
typedef struct _Edje_Item_Cell  Edje_Item_Cell;
typedef struct _Edje_Item_Class Edje_Item_Class;

struct _Edje_Item_Class
{
   Evas_Object *(*add)      (Edje_Item *ei);
   void         (*del)      (Edje_Item *ei);
   void         (*select)   (Edje_Item *ei);
   void         (*deselect) (Edje_Item *ei);
   void         (*focus)    (Edje_Item *ei);
   void         (*unfocus)  (Edje_Item *ei);
};

/* private */
struct _Edje_Item
{
   Edje_Item_Class *class;
   void            *class_data;
   
   void            *sd;
   
   void            *data;
   
   Evas_Object     *object;
   Evas_Object     *overlay_object;
   int              freeze;
   Evas_Coord       y, h;
   
   int              cells_num;
   Edje_Item_Cell  *cells;
   
   unsigned char    accessible : 1;
   unsigned char    recalc : 1;
   unsigned char    selected : 1;
   unsigned char    disabled : 1;
   unsigned char    focused : 1;
};

struct _Edje_Item_Cell
{
   Evas_Object *obj;
   Evas_Coord   x, w;
   Evas_Coord   minw, minh, maxw, maxh;
};

/* here is an item for a vertical list - with 1 or more columns. this has 3 */
/* just rotate for a horizontal list */

/*
 *             COL 0                 COL 1          COL 2
 * 
 * +-----------------------------+ +-------+ +----------------+
 * |          pad_top            | |       | |                |
 * | pad_left  OBJECT  pad_right | |  OBJ  | |     OBJECT     | ROW 0
 * |         pad_bottom          | |       | |                |
 * +-----------------------------+ +-------+ +----------------+
 *               /\              /|\
 *     space_row ||               +-- space_col
 *               \/
 * +-----------------------------+ +-------+ +----------------+
 * |                             | |       | |                |
 * |                             | |       | |                | ROW 1
 * |                             | |       | |                |
 * +-----------------------------+ +-------+ +----------------+
 * 
 * spacer object:
 * 1 edje object goes inbetween each row as a spacer object (opt)
 * 1 edje object goes inbetween each column as a spacer object (opt)
 * 
 * surround object:
 * 1 edje object goes around each item - item swallowed into "item" part (opt)
 *   if no "item" part then just underlay it
 *   on select send "select" "" signal
 *   on unselect send "unselect" "" signal
 *   on focus send "focus" "" signal
 *   on unfocus send "unfocus" signal
 * 
 *   if any list item/cell is an edje object emit this to them too.
 * 
 *   also call callbacks.
 *   if a surround object emits such a signal itself then call callbacks too
 * 
 * only 1 or 0 items can be focused
 * disabled items cannot be focused or selected/deselected
 * 
 */
