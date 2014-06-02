#include "evas_common_private.h"
#include "evas_private.h"

/* local calls */

static void evas_object_intercept_init(Evas_Object *eo_obj);
static void evas_object_intercept_deinit(Evas_Object *eo_obj);

static void
evas_object_intercept_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (!obj->interceptors)
     obj->interceptors = evas_mem_calloc(sizeof(Evas_Intercept_Func));
}

static void
evas_object_intercept_deinit(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (!obj->interceptors) return;
   if ((obj->interceptors->show.func) ||
       (obj->interceptors->hide.func) ||
       (obj->interceptors->move.func) ||
       (obj->interceptors->resize.func) ||
       (obj->interceptors->raise.func) ||
       (obj->interceptors->lower.func) ||
       (obj->interceptors->stack_above.func) ||
       (obj->interceptors->stack_below.func) ||
       (obj->interceptors->layer_set.func) ||
       (obj->interceptors->color_set.func) ||
       (obj->interceptors->clip_set.func) ||
       (obj->interceptors->clip_unset.func))
     return;
   free(obj->interceptors);
   obj->interceptors = NULL;
}

/* private calls */

void
evas_object_intercept_cleanup(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (obj->interceptors) free(obj->interceptors);
}

#define EVAS_OBJECT_INTERCEPT_CALL_SIMPLE(Type)                         \
  int                                                                   \
  evas_object_intercept_call_##Type(Evas_Object *eo_obj,                \
                                    Evas_Object_Protected_Data *obj)    \
  {									\
     int ret;								\
                                                                        \
     if (!obj->interceptors) return 0;					\
     if (obj->interceptors->Type.intercepted) return 0;			\
     obj->interceptors->Type.intercepted = EINA_TRUE;			\
     ret = !!(obj->interceptors->Type.func);				\
     if (ret)								\
       obj->interceptors->Type.func(obj->interceptors->Type.data, eo_obj); \
     obj->interceptors->Type.intercepted = EINA_FALSE;			\
     return ret;                                                        \
  }

EVAS_OBJECT_INTERCEPT_CALL_SIMPLE(show);
EVAS_OBJECT_INTERCEPT_CALL_SIMPLE(hide);
EVAS_OBJECT_INTERCEPT_CALL_SIMPLE(raise);
EVAS_OBJECT_INTERCEPT_CALL_SIMPLE(lower);

#define EVAS_OBJECT_INTERCEPT_CALL_GEOMETRY(Type)                       \
  int                                                                   \
  evas_object_intercept_call_##Type(Evas_Object *eo_obj,                \
                                    Evas_Object_Protected_Data *obj,    \
                                    Evas_Coord a, Evas_Coord b)         \
  {									\
     int ret;								\
                                                                        \
     if (!obj->interceptors) return 0;					\
     if (obj->interceptors->Type.intercepted) return 0;			\
     obj->interceptors->Type.intercepted = EINA_TRUE;			\
     ret = !!(obj->interceptors->Type.func);				\
     if (ret)								\
       obj->interceptors->Type.func(obj->interceptors->Type.data,       \
                                    eo_obj, a , b);                     \
     obj->interceptors->Type.intercepted = EINA_FALSE;			\
     return ret;                                                        \
  }

EVAS_OBJECT_INTERCEPT_CALL_GEOMETRY(move);
EVAS_OBJECT_INTERCEPT_CALL_GEOMETRY(resize);

#define EVAS_OBJECT_INTERCEPT_CALL_STACKING(Type)                       \
  int                                                                   \
  evas_object_intercept_call_##Type(Evas_Object *eo_obj,                \
                                    Evas_Object_Protected_Data *obj,    \
                                    Evas_Object *rel_to)                \
  {									\
     int ret;								\
                                                                        \
     if (!obj->interceptors) return 0;					\
     if (obj->interceptors->Type.intercepted) return 0;			\
     obj->interceptors->Type.intercepted = EINA_TRUE;			\
     ret = !!(obj->interceptors->Type.func);				\
     if (ret)								\
       obj->interceptors->Type.func(obj->interceptors->Type.data,       \
                                    eo_obj, rel_to);                    \
     obj->interceptors->Type.intercepted = EINA_FALSE;			\
     return ret;                                                        \
  }

EVAS_OBJECT_INTERCEPT_CALL_STACKING(stack_above);
EVAS_OBJECT_INTERCEPT_CALL_STACKING(stack_below);

int
evas_object_intercept_call_layer_set(Evas_Object *eo_obj,
                                     Evas_Object_Protected_Data *obj,
                                     int l)
{
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->interceptors->layer_set.intercepted) return 0;
   obj->interceptors->layer_set.intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->layer_set.func);
   if (ret)
     obj->interceptors->layer_set.func(obj->interceptors->layer_set.data, eo_obj, l);
   obj->interceptors->layer_set.intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_focus_set(Evas_Object *eo_obj,
                                     Evas_Object_Protected_Data *obj,
                                     Eina_Bool focus)
{
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->interceptors->focus_set.intercepted) return 0;
   obj->interceptors->focus_set.intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->focus_set.func);
   if (ret)
     obj->interceptors->focus_set.func(obj->interceptors->focus_set.data, eo_obj, focus);
   obj->interceptors->focus_set.intercepted = EINA_FALSE;
   return ret;
}


int
evas_object_intercept_call_color_set(Evas_Object *eo_obj,
                                     Evas_Object_Protected_Data *obj,
                                     int r, int g, int b, int a)
{
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->interceptors->color_set.intercepted) return 0;
   obj->interceptors->color_set.intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->color_set.func);
   if (ret)
     obj->interceptors->color_set.func(obj->interceptors->color_set.data, eo_obj, r, g, b, a);
   obj->interceptors->color_set.intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_clip_set(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Object *clip)
{
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->interceptors->clip_set.intercepted) return 0;
   obj->interceptors->clip_set.intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->clip_set.func);
   if (ret)
     obj->interceptors->clip_set.func(obj->interceptors->clip_set.data, eo_obj, clip);
   obj->interceptors->clip_set.intercepted = EINA_FALSE;
   return ret;
}

int
evas_object_intercept_call_clip_unset(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int ret;

   if (!obj->interceptors) return 0;
   if (obj->interceptors->clip_unset.intercepted) return 0;
   obj->interceptors->clip_unset.intercepted = EINA_TRUE;
   ret = !!(obj->interceptors->clip_unset.func);
   if (ret)
     obj->interceptors->clip_unset.func(obj->interceptors->clip_unset.data, eo_obj);
   obj->interceptors->clip_unset.intercepted = EINA_FALSE;
   return ret;
}

/* public calls */

#define EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Up_Type, Lower_Type)      \
  EAPI void                                                             \
  evas_object_intercept_##Lower_Type##_callback_add(Evas_Object *eo_obj,\
                                                    Evas_Object_Intercept_##Up_Type##_Cb func, const void *data) \
  {                                                                     \
     MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);                       \
     return;                                                            \
     MAGIC_CHECK_END();                                                 \
     Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS); \
     if (!func) return;                                                 \
     evas_object_intercept_init(eo_obj);                                \
     if (!obj->interceptors) return;                                    \
     obj->interceptors->Lower_Type.func = func;                         \
     obj->interceptors->Lower_Type.data = (void *)data;                 \
  }                                                                     \
                                                                        \
  EAPI void *                                                           \
  evas_object_intercept_##Lower_Type##_callback_del(Evas_Object *eo_obj,\
                                                    Evas_Object_Intercept_##Up_Type##_Cb func) \
  {                                                                     \
     void *data;                                                        \
                                                                        \
     MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);                       \
     return NULL;                                                       \
     MAGIC_CHECK_END();                                                 \
     Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS); \
     if (!func) return NULL;                                            \
     if (!obj->interceptors) return NULL;                               \
     obj->interceptors->Lower_Type.func = NULL;                         \
     data = obj->interceptors->Lower_Type.data;                         \
     obj->interceptors->Lower_Type.data = NULL;                         \
     evas_object_intercept_deinit(eo_obj);                              \
     return data;                                                       \
  }

EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Show, show);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Hide, hide);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Move, move);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Resize, resize);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Raise, raise);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Lower, lower);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Stack_Above, stack_above);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Stack_Below, stack_below);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Layer_Set, layer_set);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Focus_Set, focus_set);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Color_Set, color_set);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Clip_Set, clip_set);
EVAS_OBJECT_INTERCEPT_CALLBACK_DEFINE(Clip_Unset, clip_unset);
