#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

typedef struct Elm_Gen_Item_Type Elm_Gen_Item_Type;
typedef struct Elm_Gen_Item_Tooltip Elm_Gen_Item_Tooltip;
typedef struct _Widget_Data Widget_Data;
typedef struct _Pan Pan;

struct Elm_Gen_Item_Tooltip
{
   const void                 *data;
   Elm_Tooltip_Item_Content_Cb content_cb;
   Evas_Smart_Cb               del_cb;
   const char                 *style;
   Eina_Bool                   free_size : 1;
};

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data                   *wd;
   Ecore_Job                     *resize_job;
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
   Eina_List                    *labels, *icons, *states, *icon_objs;
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
};

Elm_Gen_Item *
elm_gen_item_new(Widget_Data              *wd,
                 const Elm_Gen_Item_Class *itc,
                 const void               *data,
                 Elm_Gen_Item             *parent,
                 Evas_Smart_Cb             func,
                 const void               *func_data);

void
elm_gen_item_unrealize(Elm_Gen_Item *it,
                       Eina_Bool     calc,
                       Ecore_Cb      extra_cb);
void
elm_gen_item_del_serious(Elm_Gen_Item *it, Ecore_Cb job);

void
elm_gen_item_del_notserious(Elm_Gen_Item *it);
