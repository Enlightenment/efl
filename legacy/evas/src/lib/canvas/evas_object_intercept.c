#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"
   
void
evas_object_intercept_show_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_show_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_hide_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_hide_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_move_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double x, double y), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_move_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double x, double y))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_resize_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double w, double h), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_resize_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, double w, double h))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_raise_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_raise_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_lower_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_lower_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_stack_above_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_stack_above_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_stack_below_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_stack_below_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}

void
evas_object_intercept_layer_set_callback_add(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l), void *data)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!func) return;
}

void *
evas_object_intercept_layer_set_callback_del(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l))
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!func) return NULL;
   return NULL;
}
