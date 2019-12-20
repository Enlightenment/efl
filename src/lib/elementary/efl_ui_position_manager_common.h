#ifndef EFL_UI_POSITION_MANAGER_COMMON_H
#define EFL_UI_POSITION_MANAGER_COMMON_H 1

#include <Eina.h>
#include <Efl_Ui.h>
#include "efl_ui_position_manager_entity.eo.h"

typedef struct {
   struct {
     void *data;
     Efl_Ui_Position_Manager_Size_Batch_Callback access; //this can also be the size accessor, but that does not matter here
     Eina_Free_Cb free_cb;
   } size;
   struct {
     void *data;
     Efl_Ui_Position_Manager_Object_Batch_Callback access; //this can also be the size accessor, but that does not matter here
     Eina_Free_Cb free_cb;
   } object;
} Api_Callbacks;

typedef struct {
   unsigned int start_id, end_id;
} Vis_Segment;

static Efl_Ui_Position_Manager_Size_Batch_Result
_batch_request_size(Api_Callbacks cb , int start_id, int end_id, int len, Eina_Bool cache, void *data)
{
   Efl_Ui_Position_Manager_Size_Batch_Result res;

   Eina_Rw_Slice slice;
   slice.mem = data;
   slice.len = len;

   Efl_Ui_Position_Manager_Size_Call_Config conf;
   conf.cache_request = cache;
   conf.range.start_id = start_id;
   conf.range.end_id = MIN(start_id + len, end_id);

   res = cb.size.access(cb.size.data, conf, slice);

   return res;
}

#define BATCH_ACCESS_SIZE(cb, start_id, end_id, len, cache, data) \
  do { \
    size_result = _batch_request_size((cb), (start_id), (end_id), (len), (cache), (data)); \
    EINA_SAFETY_ON_FALSE_RETURN(size_result.filled_items > 0); \
  } while(0);

#define BATCH_ACCESS_SIZE_VAL(cb, start_id, end_id, len, cache, data, V) \
  do { \
    size_result = _batch_request_size((cb), (start_id), (end_id), (len), (cache), (data)); \
    EINA_SAFETY_ON_FALSE_RETURN_VAL(size_result.filled_items > 0, V); \
  } while(0);


static Efl_Ui_Position_Manager_Object_Batch_Result
_batch_request_objects(Api_Callbacks cb , int start_id, int end_id, int len, void *data)
{
   Efl_Ui_Position_Manager_Object_Batch_Result res;

   Eina_Rw_Slice slice;
   slice.mem = data;
   slice.len = len;

   Efl_Ui_Position_Manager_Request_Range range;
   range.start_id = start_id;
   range.end_id = MIN(start_id + len, end_id);

   res = cb.object.access(cb.object.data, range, slice);

   return res;
}

#define BATCH_ACCESS_OBJECT(cb, start_id, end_id, len, data) \
  do { \
    object_result = _batch_request_objects((cb), (start_id), (end_id), (len), (data)); \
    EINA_SAFETY_ON_FALSE_RETURN(object_result.filled_items > 0); \
  } while(0);

#define BATCH_ACCESS_OBJECT_VAL(cb, start_id, end_id, len, data, v) \
  do { \
    object_result = _batch_request_objects((cb), (start_id), (end_id), (len), (data)); \
    EINA_SAFETY_ON_FALSE_RETURN_VAL(object_result.filled_items > 0, v); \
  } while(0);

static inline void
vis_change_segment(Api_Callbacks cb, int a, int b, Eina_Bool flag)
{
   const int len = 50;
   Efl_Ui_Position_Manager_Object_Batch_Entity data[len];
   Efl_Ui_Position_Manager_Object_Batch_Result object_result;

   if (a == b) return;

   for (int i = MIN(a, b); i < MAX(a, b); ++i)
     {
        Efl_Gfx_Entity *ent = NULL;
        int buffer_id = (i-MIN(a,b)) % len;

        if (buffer_id == 0)
          {
             BATCH_ACCESS_OBJECT(cb, i, MAX(a,b), len, data);
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

static inline void
vis_segment_swap(Api_Callbacks cb, Vis_Segment new, Vis_Segment old)
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

#endif
