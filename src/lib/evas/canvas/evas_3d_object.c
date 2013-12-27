#include "evas_common_private.h"
#include "evas_private.h"

#define REF_COUNT_CHECK(obj)                    \
   do {                                         \
        if ((obj)->ref_count < 1)               \
          {                                     \
             ERR("Invalid reference count.")

#define REF_COUNT_CHECK_END()                   \
          }                                     \
   } while (0)

#define OBJ_TYPE_CHECK(obj, type)               \
   do {                                         \
        if ((obj)->type != type)                \
          {                                     \
             ERR("3D object type check failed.")

#define OBJ_TYPE_CHECK_END()                    \
          }                                     \
   } while (0)

void
evas_3d_object_init(Evas_3D_Object *obj,
                    Evas *e, Evas_3D_Object_Type type, const Evas_3D_Object_Func *func)
{
   obj->evas = e;
   obj->type = type;
   obj->ref_count = 1;
   obj->func = *func;
   memset(&obj->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_3D_STATE_MAX);
}

void
evas_3d_object_reference(Evas_3D_Object *obj)
{
   REF_COUNT_CHECK(obj);
   return;
   REF_COUNT_CHECK_END();

   obj->ref_count++;
}

void
evas_3d_object_unreference(Evas_3D_Object *obj)
{
   if (obj->ref_count < 1)
     {
        printf("gotcha\n");
     }

   REF_COUNT_CHECK(obj);
   return;
   REF_COUNT_CHECK_END();

   obj->ref_count--;

   if (obj->ref_count == 0)
     obj->func.free(obj);
}

int
evas_3d_object_reference_count_get(const Evas_3D_Object *obj)
{
   REF_COUNT_CHECK(obj);
   return 0;
   REF_COUNT_CHECK_END();

   return obj->ref_count;
}

Evas *
evas_3d_object_evas_get(const Evas_3D_Object *obj)
{
   REF_COUNT_CHECK(obj);
   return NULL;
   REF_COUNT_CHECK_END();

   return obj->evas;
}

Evas_3D_Object_Type
evas_3d_object_type_get(const Evas_3D_Object *obj)
{
   REF_COUNT_CHECK(obj);
   return EVAS_3D_OBJECT_TYPE_INVALID;
   REF_COUNT_CHECK_END();

   return obj->type;
}

Eina_Bool
evas_3d_object_dirty_get(const Evas_3D_Object *obj, Evas_3D_State state)
{
   return obj->dirty[state];
}

void
evas_3d_object_change(Evas_3D_Object *obj, Evas_3D_State state, Evas_3D_Object *ref)
{
   /* Skip already dirty properties. */
   if (obj->dirty[state])
     return;

   obj->dirty[state] = EINA_TRUE;
   obj->dirty[EVAS_3D_STATE_ANY] = EINA_TRUE;

   if (obj->func.change)
     obj->func.change(obj, state, ref);
}

void
evas_3d_object_update(Evas_3D_Object *obj)
{
   if (!obj->dirty[EVAS_3D_STATE_ANY])
     return;

   if (obj->func.update)
     obj->func.update(obj);

   evas_3d_object_update_done(obj);
}

void
evas_3d_object_update_done(Evas_3D_Object *obj)
{
   memset(&obj->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_3D_STATE_MAX);
}
