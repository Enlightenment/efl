#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* local calls */

static void evas_object_intercept_init(Evas_Object *obj);
static void evas_object_intercept_deinit(Evas_Object *obj);

static void
evas_object_intercept_init(Evas_Object *obj)
{
   /* MEM OK */
   if (!obj->interceptors)
     obj->interceptors = evas_mem_calloc(sizeof(Evas_Intercept_Func));
}

static void
evas_object_intercept_deinit(Evas_Object *obj)
{
   /* MEM OK */
   if (!obj->interceptors) return;
   if ((obj->interceptors->show.func) ||
       (obj->interceptors->hide.func) ||
       (obj->interceptors->move.func) ||
       (obj->interceptors->resize.func) ||
       (obj->interceptors->raise.func) ||
       (obj->interceptors->lower.func) ||
       (obj->interceptors->stack_above.func) ||
       (obj->interceptors->stack_below.func) ||
       (obj->interceptors->layer_set.func))
     return;
   free(obj->interceptors);
   obj->interceptors = NULL;
}

/* private calls */

void
evas_object_intercept_cleanup(Evas_Object *obj)
{
   /* MEM OK */
   if (obj->interceptors) free(obj->interceptors);
}

int
evas_object_intercept_call_show(Evas_Object *obj)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->show.func;
   if (obj->interceptors->show.func)
     obj->interceptors->show.func(obj->interceptors->show.data, obj);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_hide(Evas_Object *obj)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->hide.func;
   if (obj->interceptors->hide.func)
     obj->interceptors->hide.func(obj->interceptors->hide.data, obj);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_move(Evas_Object *obj, double x, double y)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->move.func;
   if (obj->interceptors->move.func)
     obj->interceptors->move.func(obj->interceptors->move.data, obj, x, y);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_resize(Evas_Object *obj, double w, double h)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->resize.func;
   if (obj->interceptors->resize.func)
     obj->interceptors->resize.func(obj->interceptors->resize.data, obj, w, h);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_raise(Evas_Object *obj)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->raise.func;
   if (obj->interceptors->raise.func)
     obj->interceptors->raise.func(obj->interceptors->raise.data, obj);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_lower(Evas_Object *obj)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->lower.func;
   if (obj->interceptors->lower.func)
     obj->interceptors->lower.func(obj->interceptors->lower.data, obj);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_stack_above(Evas_Object *obj, Evas_Object *above)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->stack_above.func;
   if (obj->interceptors->stack_above.func)
     obj->interceptors->stack_above.func(obj->interceptors->stack_above.data, obj, above);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_stack_below(Evas_Object *obj, Evas_Object *below)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->stack_below.func;
   if (obj->interceptors->stack_below.func)
     obj->interceptors->stack_below.func(obj->interceptors->stack_below.data, obj, below);
   obj->intercepted = 0;
   return ret;
}

int
evas_object_intercept_call_layer_set(Evas_Object *obj, int l)
{
   /* MEM OK */
   int ret;
   
   if (!obj->interceptors) return 0;
   if (obj->intercepted) return 0;
   obj->intercepted = 1;
   ret = (int)obj->interceptors->layer_set.func;
   if (obj->interceptors->layer_set.func)
     obj->interceptors->layer_set.func(obj->interceptors->layer_set.data, obj, l);
   obj->intercepted = 0;
   return ret;
}

/* public calls */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_show_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data)
{
   /* MEM OK */
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->show.func = func;
   obj->interceptors->show.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_show_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->show.func = NULL;
   data = obj->interceptors->show.data;
   obj->interceptors->show.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_hide_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->hide.func = func;
   obj->interceptors->hide.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_hide_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   if (!obj->interceptors) return NULL;
   obj->interceptors->hide.func = NULL;
   data = obj->interceptors->hide.data;
   obj->interceptors->hide.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_move_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double x, double y), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->move.func = func;
   obj->interceptors->move.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_move_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double x, double y))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->interceptors) return NULL;
   obj->interceptors->move.func = NULL;
   data = obj->interceptors->move.data;
   obj->interceptors->move.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_resize_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double w, double h), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->resize.func = func;
   obj->interceptors->resize.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_resize_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double w, double h))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->interceptors) return NULL;
   obj->interceptors->resize.func = NULL;
   data = obj->interceptors->resize.data;
   obj->interceptors->resize.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_raise_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->raise.func = func;
   obj->interceptors->raise.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_raise_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->interceptors) return NULL;
   obj->interceptors->raise.func = NULL;
   data = obj->interceptors->raise.data;
   obj->interceptors->raise.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_lower_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->lower.func = func;
   obj->interceptors->lower.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_lower_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->interceptors) return NULL;
   obj->interceptors->lower.func = NULL;
   data = obj->interceptors->lower.data;
   obj->interceptors->lower.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_stack_above_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->stack_above.func = func;
   obj->interceptors->stack_above.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_stack_above_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->interceptors) return NULL;
   obj->interceptors->stack_above.func = NULL;
   data = obj->interceptors->stack_above.data;
   obj->interceptors->stack_above.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_stack_below_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->stack_below.func = func;
   obj->interceptors->stack_below.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_stack_below_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->interceptors) return NULL;
   obj->interceptors->stack_below.func = NULL;
   data = obj->interceptors->stack_below.data;
   obj->interceptors->stack_below.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_intercept_layer_set_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l), const void *data)
{
   /* MEM OK */
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
   evas_object_intercept_init(obj);
   if (!obj->interceptors) return;
   obj->interceptors->layer_set.func = func;
   obj->interceptors->layer_set.data = (void *)data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void *
evas_object_intercept_layer_set_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l))
{
   /* MEM OK */
   void *data;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!obj->interceptors) return NULL;
   obj->interceptors->layer_set.func = NULL;
   data = obj->interceptors->layer_set.data;
   obj->interceptors->layer_set.data = NULL;
   evas_object_intercept_deinit(obj);
   return data;
}
