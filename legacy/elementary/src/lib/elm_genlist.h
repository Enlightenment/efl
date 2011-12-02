#ifndef ELM_GEN_H_
# define ELM_GEN_H_

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#define ELM_GEN_ITEM_FROM_INLIST(it) \
   ((it) ? EINA_INLIST_CONTAINER_GET(it, Elm_Gen_Item) : NULL)

#define SWIPE_MOVES         12

typedef struct Elm_Gen_Item_Type Elm_Gen_Item_Type;
typedef struct Elm_Gen_Item_Tooltip Elm_Gen_Item_Tooltip;
typedef struct _Widget_Data Widget_Data;

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
   ELM_WIDGET_ITEM;
   EINA_INLIST;
   Widget_Data                  *wd;
   Elm_Gen_Item_Type            *item;
   const Elm_Gen_Item_Class     *itc;
   Evas_Coord                    x, y, dx, dy;
   Evas_Object                  *spacer;
   Elm_Gen_Item                 *parent;
   Eina_List                    *labels, *contents, *states, *content_objs;
   Ecore_Timer                  *long_timer;
   int                           relcount;
   int                           walking;
   int                           generation; /* a generation of an item. when the item is created, this value is set to the value of genlist generation. this value will be decreased when the item is going to be deleted */
   const char                   *mouse_cursor;

   struct
   {
      Evas_Smart_Cb func;
      const void   *data;
   } func;

   Elm_Gen_Item_Tooltip tooltip;
   Ecore_Cb    del_cb, sel_cb, highlight_cb;
   Ecore_Cb    unsel_cb, unhighlight_cb, unrealize_cb;

   Eina_Bool   want_unrealize : 1;
   Eina_Bool   display_only : 1;
   Eina_Bool   realized : 1;
   Eina_Bool   selected : 1;
   Eina_Bool   highlighted : 1;
   Eina_Bool   disabled : 1;
   Eina_Bool   dragging : 1;
   Eina_Bool   down : 1;
   Eina_Bool   group : 1;
   Eina_Bool   reorder : 1;
   Eina_Bool   mode_set : 1; /* item uses style mode for highlight/select */
};

typedef struct _Pan Pan;
struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data                   *wd;
   Ecore_Job                     *resize_job;
};

struct _Widget_Data
{
   Eina_Inlist_Sorted_State *state;
   Evas_Object      *obj; /* the genlist object */
   Evas_Object      *scr; /* a smart scroller object which is used internally in genlist */
   Evas_Object      *pan_smart; /* "elm_genlist_pan" evas smart object. this is an extern pan of smart scroller(scr). */
   Eina_List        *selected;
   Eina_List        *group_items; /* list of groups index items */
   Eina_Inlist      *items; /* inlist of all items */
   Elm_Gen_Item     *reorder_it; /* item currently being repositioned */
   Elm_Gen_Item     *last_selected_item;
   Pan              *pan; /* pan_smart object's smart data */
   Ecore_Job        *calc_job;
   int               walking;
   int               item_width, item_height;
   int               group_item_width, group_item_height;
   int               minw, minh;
   long              count;
   Evas_Coord        pan_x, pan_y;
   Eina_Bool         reorder_mode : 1; /* a flag for reorder mode enable/disable */
   Eina_Bool         on_hold : 1;
   Eina_Bool         multi : 1; /* a flag for item multi selection */
   Eina_Bool         no_select : 1;
   Eina_Bool         wasselected : 1;
   Eina_Bool         always_select : 1;
   Eina_Bool         clear_me : 1; /* a flag whether genlist is marked as to be cleared or not. if this flag is true, genlist clear was already deferred.  */
   Eina_Bool         h_bounce : 1;
   Eina_Bool         v_bounce : 1;
   Ecore_Cb          del_cb, calc_cb, sizing_cb;
   Ecore_Cb          clear_cb;
   ////////////////////////////////////
   Eina_Inlist      *blocks; /* inlist of all blocks. a block consists of a certain number of items. maximum number of items in a block is 'max_items_per_block'. */
   Evas_Coord        reorder_old_pan_y, w, h, realminw, prev_viewport_w;
   Ecore_Job        *update_job;
   Ecore_Idle_Enterer *queue_idle_enterer;
   Ecore_Idler        *must_recalc_idler;
   Eina_List        *queue;
   Elm_Gen_Item *show_item, *anchor_item, *mode_item, *reorder_rel, *expanded_item;
   Eina_Inlist      *item_cache; /* an inlist of edje object it cache. */
   Evas_Coord        anchor_y;
   Evas_Coord        reorder_start_y; /* reorder it's initial y coordinate in the pan. */
   Elm_List_Mode     mode;
   Ecore_Timer      *multi_timer, *scr_hold_timer;
   Ecore_Animator   *reorder_move_animator;
   const char       *mode_type;
   const char       *mode_item_style;
   unsigned int      start_time;
   Evas_Coord        prev_x, prev_y, prev_mx, prev_my;
   Evas_Coord        cur_x, cur_y, cur_mx, cur_my;
   Eina_Bool         mouse_down : 1;
   Eina_Bool         multi_down : 1;
   Eina_Bool         multi_timeout : 1;
   Eina_Bool         multitouched : 1;
   Eina_Bool         longpressed : 1;
   Eina_Bool         bring_in : 1;
   Eina_Bool         compress : 1;
   Eina_Bool         height_for_width : 1;
   Eina_Bool         homogeneous : 1;
   Eina_Bool         swipe : 1;
   Eina_Bool         reorder_pan_move : 1;
   Eina_Bool         auto_scroll_enabled : 1;
   Eina_Bool         pan_resized : 1;
   struct
   {
      Evas_Coord x, y;
   } history[SWIPE_MOVES];
   int               multi_device;
   int               item_cache_count;
   int               item_cache_max; /* maximum number of cached items */
   int               movements;
   int               max_items_per_block; /* maximum number of items per block */
   double            longpress_timeout; /* longpress timeout. this value comes from _elm_config by default. this can be changed by elm_genlist_longpress_timeout_set() */
   int               generation; /* a generation of genlist. when genlist is cleared, this value will be increased and a new generation will start */

   /* The stuff below directly come from gengrid without any thinking */
   unsigned int      nmax;
   Evas_Coord        reorder_item_x, reorder_item_y;
   Evas_Coord        old_pan_x, old_pan_y;
   long              items_lost;
   double            align_x, align_y;

   Eina_Bool         horizontal : 1;
   Eina_Bool         move_effect_enabled : 1;
   Eina_Bool         reorder_item_changed : 1;
};

Elm_Gen_Item *
_elm_genlist_item_new(Widget_Data              *wd,
                      const Elm_Gen_Item_Class *itc,
                      const void               *data,
                      Elm_Gen_Item             *parent,
                      Evas_Smart_Cb             func,
                      const void               *func_data);

Evas_Object *
_elm_genlist_item_widget_get(const Elm_Gen_Item *it);

void
_elm_genlist_page_relative_set(Evas_Object *obj,
                               double       h_pagerel,
                               double       v_pagerel);

void
_elm_genlist_page_relative_get(const Evas_Object *obj,
                               double            *h_pagerel,
                               double            *v_pagerel);

void
_elm_genlist_page_size_set(Evas_Object *obj,
                           Evas_Coord   h_pagesize,
                           Evas_Coord   v_pagesize);

void
_elm_genlist_current_page_get(const Evas_Object *obj,
                              int               *h_pagenumber,
                              int               *v_pagenumber);

void
_elm_genlist_last_page_get(const Evas_Object *obj,
                           int               *h_pagenumber,
                           int               *v_pagenumber);

void
_elm_genlist_page_show(const Evas_Object *obj,
                       int                h_pagenumber,
                       int                v_pagenumber);

void
_elm_genlist_page_bring_in(const Evas_Object *obj,
                           int                h_pagenumber,
                           int                v_pagenumber);

void
_elm_genlist_item_unrealize(Elm_Gen_Item *it,
                            Eina_Bool     calc);
void
_elm_genlist_item_del_serious(Elm_Gen_Item *it);

void
_elm_genlist_item_del_notserious(Elm_Gen_Item *it);

#endif
