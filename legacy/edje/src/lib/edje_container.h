/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

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

typedef struct _Smart_Data         Smart_Data;
typedef struct _Smart_Data_Colinfo Smart_Data_Colinfo;

struct _Smart_Data
{
   Evas_Coord     x, y, w, h;
   Eina_List     *children;
   Evas_Object   *smart_obj;
   int            cols, rows;

   Evas_Coord     contents_w, contents_h;
   Evas_Coord     min_row_h, max_row_h;
   Evas_Coord     min_w, max_w, min_h, max_h;

   Smart_Data_Colinfo *colinfo;

   int            freeze;

   double         scroll_x, scroll_y;
   double         align_x, align_y;

   unsigned char  changed : 1;
   unsigned char  change_child : 1;
   unsigned char  change_child_list : 1;
   unsigned char  change_cols : 1;
   unsigned char  change_scroll : 1;

   unsigned char  need_layout : 1;

   unsigned char  homogenous : 1;
};

struct _Smart_Data_Colinfo
{
   Evas_Coord minw, maxw;
};

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

   Evas_Coord       minh, maxh;

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
 * 1 Edje object goes in-between each row as a spacer object (opt)
 * 1 Edje object goes in-between each column as a spacer object (opt)
 *
 * surround object:
 * 1 Edje object goes around each item - item swallowed into "item" part (opt)
 *   if no "item" part then just underlay it
 *   on select send "select" "" signal
 *   on unselect send "unselect" "" signal
 *   on focus send "focus" "" signal
 *   on unfocus send "unfocus" signal
 *
 *   if any list item/cell is an Edje object emit this to them too.
 *
 *   also call callbacks.
 *   if a surround object emits such a signal itself then call callbacks too
 *
 * only 1 or 0 items can be focused
 * disabled items cannot be focused or selected/deselected
 *
 */
