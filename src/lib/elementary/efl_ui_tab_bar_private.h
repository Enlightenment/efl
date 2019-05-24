#ifndef EFL_UI_TAB_BAR_PRIVATE_H
#define EFL_UI_TAB_BAR_PRIVATE_H

typedef struct _Efl_Ui_Tab_Bar_Data                   Efl_Ui_Tab_Bar_Data;
typedef struct _Tab_Info                              Tab_Info;

struct _Efl_Ui_Tab_Bar_Data
{
   Efl_Canvas_Object                    *bx;
   int                                   cnt;
   int                                   cur;
   Efl_Ui_Layout_Orientation             dir;

   Eina_List                            *tab_infos;
   Tab_Info                             *selected_tab;
};

struct _Tab_Info
{
   Eo                                   *tab;

   const char                           *label;
   const char                           *icon_str;
   Eo                                   *icon;

   Eina_Bool                             selected : 1;
};


#define EFL_UI_TAB_BAR_DATA_GET(o, sd) \
  Efl_Ui_Tab_Bar_Data *sd = efl_data_scope_get(o, EFL_UI_TAB_BAR_CLASS)

#define EFL_UI_TAB_BAR_DATA_GET_OR_RETURN(o, sd, ...) \
  Efl_Ui_Tab_Bar_Data *sd = efl_data_scope_safe_get(o, EFL_UI_TAB_BAR_CLASS); \
  if (EINA_UNLIKELY(!sd))                            \
    {                                                \
       ERR("No widget data for object %p (%s)",      \
           o, evas_object_type_get(o));              \
       return __VA_ARGS__;                           \
    }


#endif
