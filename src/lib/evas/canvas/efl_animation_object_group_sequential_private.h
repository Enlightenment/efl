#define EFL_ANIMATION_OBJECT_PROTECTED

#include "evas_common_private.h"
#include <Ecore.h>

#define MY_CLASS EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_CLASS
#define MY_CLASS_NAME efl_class_name_get(MY_CLASS)

#define EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_DATA_GET(o, pd) \
   Efl_Animation_Object_Group_Sequential_Data *pd = efl_data_scope_get(o, EFL_ANIMATION_OBJECT_GROUP_SEQUENTIAL_CLASS)

typedef struct _Member_Object_Data
{
   Efl_Animation_Object *anim_obj;

   int                   repeated_count; /* How many times object has been
                                          * repeated. */
} Member_Object_Data;

typedef struct _Efl_Animation_Object_Group_Sequential_Data
{
   Eina_List   *member_anim_obj_data_list; //List of Member_Object_Data
} Efl_Animation_Object_Group_Sequential_Data;
