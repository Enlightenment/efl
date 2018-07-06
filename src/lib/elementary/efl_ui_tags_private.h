#ifndef EFL_UI_TAGS_PRIVATE_H
#define EFL_UI_TAGS_PRIVATE_H

#include "elm_widget_layout.h"

typedef enum _Tags_View_State
{
   TAGS_VIEW_NONE,
   TAGS_VIEW_GUIDETEXT,
   TAGS_VIEW_ENTRY,
   TAGS_VIEW_SHRINK
} Tags_View_State;

typedef enum _Tags_It_State
{
   TAGS_IT_STATE_DEFAULT,
   TAGS_IT_STATE_SELECTED,
} Tags_It_State;

typedef struct _Efl_Ui_Tags_Data    Efl_Ui_Tags_Data;
struct _Efl_Ui_Tags_Data
{
   Eina_Array                         *it_array;
   Eina_List                          *layouts;

   Eo *parent, *box, *label, *end, *entry, *selected_it, *focused_it, *downed_it;

   Tags_View_State                     view_state;

   Ecore_Timer                        *longpress_timer;

   const char                         *label_str, *guide_text_str;
   int                                 n_str;
   Evas_Coord                          w_box, h_box;
   int                                 shrink;

   Efl_Ui_Format_Func_Cb               format_cb;
   Eina_Free_Cb                        format_free_cb;
   void                               *format_cb_data;
   Eina_Strbuf                        *format_strbuf;

   Eina_Bool                           last_it_select : 1;
   Eina_Bool                           editable : 1;
   Eina_Bool                           focused : 1;
   Eina_Bool                           label_packed : 1;
   Eina_Bool                           item_setting : 1;
};
#endif
