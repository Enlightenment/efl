#ifndef EFL_UI_POSITION_MANAGER_COMMON_H
#define EFL_UI_POSITION_MANAGER_COMMON_H 1

#include <Eina.h>
#include <Efl_Ui.h>
#include "efl_ui_position_manager_entity.eo.h"

typedef struct {
   void *data;
   Efl_Ui_Position_Manager_Batch_Access_Entity access; //this can also be the size accessor, but that does not matter here
   Eina_Free_Cb free_cb;
} Api_Callback;

static inline int
_fill_buffer(Api_Callback *cb , int start_id, int len, void *data)
{
   Eina_Rw_Slice slice;
   slice.mem = data;
   slice.len = len;

   return cb->access(cb->data, start_id, slice);
}

static inline void
vis_change_segment(Api_Callback *cb, int a, int b, Eina_Bool flag)
{
   const int len = 50;
   Efl_Gfx_Entity *data[len];

   if (a == b) return;

   for (int i = MIN(a, b); i < MAX(a, b); ++i)
     {
        Efl_Gfx_Entity *ent = NULL;
        int buffer_id = (i-MIN(a,b)) % len;

        if (buffer_id == 0)
          {
             EINA_SAFETY_ON_FALSE_RETURN(_fill_buffer(cb, MIN(a,b), len, data) >= 0);
          }
        ent = data[i - MIN(a,b)];
        if (ent && !efl_ui_focus_object_focus_get(ent))
          {
             efl_gfx_entity_visible_set(ent, flag);
          }
     }
}
#endif
