#ifndef EFL_UI_WIDGET_STACK_H
#define EFL_UI_WIDGET_STACK_H


typedef struct _Content_Data Content_Data;
struct _Content_Data
{
   EINA_INLIST;

   Eo        *content;
   Eina_Bool  on_pushing : 1;
   Eina_Bool  on_popping : 1;
   Eina_Bool  popped_hidden : 1;
};

typedef struct _Transit_Data Transit_Data;
struct _Transit_Data
{
   Content_Data *cd;
};

typedef struct _Efl_Ui_Stack_Data Efl_Ui_Stack_Data;
struct _Efl_Ui_Stack_Data
{
   Eina_Inlist *stack; /* the last item is the top item */
   Efl_Canvas_Animation_Player *hide, *show;
   Transit_Data *show_td, *hide_td;

};

#endif
