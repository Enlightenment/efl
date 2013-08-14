#include "eo_simple.h"
#include "eo_inherit.h"

EAPI Eo_Op EO_INHERIT_BASE_ID = 0;

#define MY_CLASS EO_INHERIT_CLASS

typedef struct
{
   int y;
} Private_Data;

static void
_get(Eo *obj EINA_UNUSED, void *obj_data, va_list *list)
{
   const Private_Data *pd = (Private_Data *) obj_data;
   int *r;

   r = va_arg(*list, int *);
   *r = pd->y;
}

static void
_inc(Eo *obj EINA_UNUSED, void *obj_data, va_list *list EINA_UNUSED)
{
   Private_Data *pd = (Private_Data *) obj_data;

   eo_do_super(obj, MY_CLASS, eo_inc());
   pd->y += 1;
}

static void
_constructor(Eo *obj, void *obj_data, va_list *list EINA_UNUSED)
{
   Private_Data *pd = (Private_Data *) obj_data;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   pd->y = 68;
}

static void
_destructor(Eo *obj, void *obj_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_INHERIT_ID(EO_INHERIT_SUB_ID_GET), _get),
        EO_OP_FUNC(EO_SIMPLE_ID(EO_SIMPLE_SUB_ID_INC), _inc),  // override
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EO_INHERIT_SUB_ID_GET, "Get Y"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Eo Inherit",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EO_INHERIT_BASE_ID, op_desc, EO_INHERIT_SUB_ID_LAST),
     NULL,
     sizeof(Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(inherit_class_get, &class_desc, EO_SIMPLE_CLASS, NULL);
