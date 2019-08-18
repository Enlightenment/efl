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

typedef struct {
   unsigned int start_id, end_id;
} Vis_Segment;

static inline int
_fill_buffer(Api_Callback *cb , int start_id, int len, int *group_id, void *data)
{
   Efl_Ui_Position_Manager_Batch_Result res;
   Eina_Rw_Slice slice;
   slice.mem = data;
   slice.len = len;

   res = cb->access(cb->data, start_id, slice);

   if (group_id)
     *group_id = res.group_id;

   return res.filled_items;
}

static inline void
vis_change_segment(Api_Callback *cb, int a, int b, Eina_Bool flag)
{
   const int len = 50;
   Efl_Ui_Position_Manager_Batch_Entity_Access data[len];

   if (a == b) return;

   for (int i = MIN(a, b); i < MAX(a, b); ++i)
     {
        Efl_Gfx_Entity *ent = NULL;
        int buffer_id = (i-MIN(a,b)) % len;

        if (buffer_id == 0)
          {
             EINA_SAFETY_ON_FALSE_RETURN(_fill_buffer(cb, MIN(a,b), len, NULL, data) >= 0);
          }
        ent = data[buffer_id].entity;
        if (ent && !flag && (efl_ui_focus_object_focus_get(ent) || efl_ui_focus_object_child_focus_get(ent)))
          {
             //we should not make focused object invisible, rather move it to some parking lot
             efl_gfx_entity_position_set(ent, EINA_POSITION2D(-9999,-9999));
          }
        if (ent && !efl_ui_focus_object_focus_get(ent))
          {
             efl_gfx_entity_visible_set(ent, flag);
          }
     }
}
#endif

static inline void
vis_segment_swap(Api_Callback *cb, Vis_Segment new, Vis_Segment old)
{
   if (new.end_id <= old.start_id || new.start_id >= old.end_id)
     {
        //it is important to first make the segment visible here, and then hide the rest
        //otherwise we get a state where item_container has 0 subchildren, which triggers a lot of focus logic.
        vis_change_segment(cb, new.start_id, new.end_id, EINA_TRUE);
        vis_change_segment(cb, old.start_id, old.end_id, EINA_FALSE);
     }
   else
     {
        vis_change_segment(cb, old.start_id, new.start_id, (old.start_id > new.start_id));
        vis_change_segment(cb, old.end_id, new.end_id, (old.end_id < new.end_id));
     }
}
