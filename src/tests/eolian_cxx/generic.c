
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include "generic.eo.h"

struct _Generic_Data
{
   int        req_ctor_a_val;
   Ecore_Cb   req_ctor_b_cb;
   void      *req_ctor_b_data;
   int        opt_ctor_a_val;
   Ecore_Cb   opt_ctor_b_cb;
   void      *opt_ctor_b_data;
};
typedef struct _Generic_Data Generic_Data;

#define MY_CLASS GENERIC_CLASS

static void _generic_eo_base_constructor(Eo *obj, Generic_Data *pd)
{
   pd->req_ctor_a_val = 0;
   pd->req_ctor_b_cb = NULL;
   pd->req_ctor_b_data = NULL;
   pd->opt_ctor_a_val = 0;
   pd->opt_ctor_b_cb = NULL;
   pd->opt_ctor_b_data = NULL;
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

static void _generic_required_ctor_a(Eo *obj EINA_UNUSED, Generic_Data *pd, int value)
{
   pd->req_ctor_a_val = value;
}

static void _generic_required_ctor_b(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data)
{
   cb(data);
}

static void _generic_optional_ctor_a(Eo *obj EINA_UNUSED, Generic_Data *pd, int value)
{
   pd->opt_ctor_a_val = value;
}

static void _generic_optional_ctor_b(Eo *obj EINA_UNUSED, Generic_Data *pd EINA_UNUSED, Ecore_Cb cb, void *data)
{
   cb(data);
}

static int _generic_req_ctor_a_value_get(Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   return pd->req_ctor_a_val;
}

static int _generic_opt_ctor_a_value_get(Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   return pd->opt_ctor_a_val;
}

static void _generic_call_req_ctor_b_callback(Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   if (pd->req_ctor_b_cb)
     pd->req_ctor_b_cb(pd->req_ctor_b_data);
}

static void _generic_call_opt_ctor_b_callback(Eo *obj EINA_UNUSED, Generic_Data *pd)
{
   if (pd->opt_ctor_b_cb)
     pd->opt_ctor_b_cb(pd->opt_ctor_b_data);
}

#include "generic.eo.c"
