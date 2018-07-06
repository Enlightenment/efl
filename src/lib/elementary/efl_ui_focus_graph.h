#ifndef EFL_UI_FOCUS_GRAPH_H
#define EFL_UI_FOCUS_GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <Eina.h>

typedef struct _Opaque_Graph_Memeber Opaque_Graph_Member;


typedef struct {
  Eina_List *relation;
  int lvl;
  unsigned int distance;
} Efl_Ui_Focus_Graph_Calc_Direction_Result;

typedef struct {
  Efl_Ui_Focus_Graph_Calc_Direction_Result right, left, top, bottom;
} Efl_Ui_Focus_Graph_Calc_Result;

typedef struct {
   size_t offset_focusable; //offset to the focusable
} Efl_Ui_Focus_Graph_Context;

void efl_ui_focus_graph_calc(Efl_Ui_Focus_Graph_Context *context, Eina_Iterator *nodes, Opaque_Graph_Member *member, Efl_Ui_Focus_Graph_Calc_Result *result);

#endif
