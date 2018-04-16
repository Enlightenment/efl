#ifndef ELM_GEN_H_
#define ELM_GEN_H_

#include <Elementary.h>
#include <Elementary_Cursor.h>

#define ELM_GEN_ITEM_FROM_INLIST(it) \
  ((it) ? EINA_INLIST_CONTAINER_GET(it, Elm_Gen_Item) : NULL)

#define ELM_GEN_ITEM_PREV(_it) ((_it) ? ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(_it)->prev) : NULL)
#define ELM_GEN_ITEM_NEXT(_it) ((_it) ? ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(_it)->next) : NULL)

#define SWIPE_MOVES 12

/* common item handles for genlist/gengrid */

typedef struct Elm_Gen_Item_Type    Elm_Gen_Item_Type;
typedef struct Elm_Gen_Item_Tooltip Elm_Gen_Item_Tooltip;

struct Elm_Gen_Item_Tooltip
{
   const void                 *data;
   Elm_Tooltip_Item_Content_Cb content_cb;
   Evas_Smart_Cb               del_cb;
   const char                 *style;
   Eina_Bool                   free_size : 1;
};

struct Elm_Gen_Item
{
   Elm_Widget_Item_Data     *base;
   EINA_INLIST;

   Elm_Gen_Item_Type        *item;
   const Elm_Gen_Item_Class *itc;
   Evas_Coord                x, y, dx, dy;
   Evas_Object              *spacer, *deco_all_view;
   Elm_Gen_Item             *parent;
   Eina_List                *texts, *contents, *states, *content_objs;
   Ecore_Timer              *long_timer;
   int                       generation; /**< a generation of an item. when the item is created, this value is set to the value of genlist/gengrid generation. this value will be decreased when the item is going to be deleted */
   const char               *mouse_cursor;
   Eina_List                *item_focus_chain;

   struct
   {
      Evas_Smart_Cb func;
      const void   *data;
   } func;

   Elm_Gen_Item_Tooltip      tooltip;
   Ecore_Cb                  del_cb, unrealize_cb;
   Ecore_Cb                  sel_cb, unsel_cb;
   Ecore_Cb                  highlight_cb, unhighlight_cb;

   int                       position;
   Elm_Object_Select_Mode    select_mode;

   Eina_Bool                 position_update : 1;
   Eina_Bool                 want_unrealize : 1;
   Eina_Bool                 realized : 1;
   Eina_Bool                 selected : 1;
   Eina_Bool                 highlighted : 1;
   Eina_Bool                 dragging : 1; /**< this is set true when an item is being dragged. this is set false on multidown/mouseup/mousedown. when this is true, the item should not be unrealized. or evas mouse down/up event will be corrupted. */
   Eina_Bool                 down : 1;
   Eina_Bool                 group : 1;
   Eina_Bool                 reorder : 1;
   Eina_Bool                 decorate_it_set : 1; /**< item uses style mode for highlight/select */
   Eina_Bool                 flipped : 1; /**< a flag that shows the flip status of the item. */
   Eina_Bool                 has_contents : 1; /**< content objs have or previously did exist (size calcs) */
   Eina_Bool                 cursor_engine_only : 1;
   Eina_Bool                 hide : 1; /**< flag used for hiding the items which do not match filter text */
   Eina_Bool                 filtered: 1; /**<flag used to indicate whether or not item has been filtered */
};

#endif
