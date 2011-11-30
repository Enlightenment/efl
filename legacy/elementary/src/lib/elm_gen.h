#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

#define ELM_GEN_ITEM_FROM_INLIST(it) \
   ((it) ? EINA_INLIST_CONTAINER_GET(it, Elm_Gen_Item) : NULL)

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
   Eina_Bool   delete_me : 1;
   Eina_Bool   down : 1;
   Eina_Bool   group : 1;
   Eina_Bool   reorder : 1;
   Eina_Bool   mode_set : 1; /* item uses style mode for highlight/select */
};
