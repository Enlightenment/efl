#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <math.h>
#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

#define MY_CLASS EO_EVAS_3D_LIGHT_CLASS

static void
_light_free(Evas_3D_Object *obj)
{
   Evas_3D_Light_Data *light = (Evas_3D_Light_Data *)obj;

   if (light->nodes)
     eina_hash_free(light->nodes);

   //free(light);
}

static Eina_Bool
_light_node_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                        void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Node *n = *(Evas_3D_Node **)key;
   Evas_3D_Node_Data *pdnode = eo_data_scope_get(n, EO_EVAS_3D_NODE_CLASS);
   evas_3d_object_change(&pdnode->base, EVAS_3D_STATE_NODE_LIGHT, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_light_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED,
              Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Light_Data *light = (Evas_3D_Light_Data *)obj;

   if (light->nodes)
     eina_hash_foreach(light->nodes, _light_node_change_notify, obj);
}

static const Evas_3D_Object_Func light_func =
{
   _light_free,
   _light_change,
   NULL,
};

void
evas_3d_light_node_add(Evas_3D_Light *light, Evas_3D_Node *node)
{
   int count = 0;
   Evas_3D_Light_Data *pd = eo_data_scope_get(light, MY_CLASS);
   if (pd->nodes == NULL)
     {
        pd->nodes = eina_hash_pointer_new(NULL);

        if (pd->nodes == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(pd->nodes, &node);

   eina_hash_set(pd->nodes, &node, (const void *)(count + 1));
}

void
evas_3d_light_node_del(Evas_3D_Light *light, Evas_3D_Node *node)
{
   int count = 0;
   Evas_3D_Light_Data *pd = eo_data_scope_get(light, MY_CLASS);
   if (pd->nodes == NULL)
     {
        ERR("No node to delete.");
        return;
     }

   count = (int)eina_hash_find(pd->nodes, &node);

   if (count == 1)
     eina_hash_del(pd->nodes, &node, NULL);
   else
     eina_hash_set(pd->nodes, &node, (const void *)(count - 1));
}


EAPI Evas_3D_Light *
evas_3d_light_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = eo_add(MY_CLASS, e);
   eo_unref(eo_obj);
   return eo_obj;
}

EOLIAN static void
_eo_evas_3d_light_eo_base_constructor(Eo *obj, Evas_3D_Light_Data *pd)
{
   Eo *e;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, e = eo_parent_get());
   evas_3d_object_init(&pd->base, e, EVAS_3D_OBJECT_TYPE_LIGHT, &light_func);
   evas_color_set(&pd->ambient, 0.0, 0.0, 0.0, 1.0);
   evas_color_set(&pd->diffuse, 1.0, 1.0, 1.0, 1.0);
   evas_color_set(&pd->specular, 1.0, 1.0, 1.0, 1.0);

   pd->spot_exp = 0.0;
   pd->spot_cutoff = 180.0;
   pd->spot_cutoff_cos = -1.0;

   pd->atten_const = 1.0;
   pd->atten_linear = 0.0;
   pd->atten_quad = 0.0;
}

EOLIAN static void
_eo_evas_3d_light_eo_base_destructor(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd)
{
   evas_3d_object_unreference(&pd->base);
}

EOLIAN static Evas *
_eo_evas_3d_light_evas_common_interface_evas_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd)
{
   return pd->base.evas;
}

EOLIAN static void
_eo_evas_3d_light_directional_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Eina_Bool directional)
{
   if (pd->directional != directional)
     {
        pd->directional = directional;
        evas_3d_object_change(&pd->base, EVAS_3D_STATE_ANY, NULL);
     }
}

EOLIAN static Eina_Bool
_eo_evas_3d_light_directional_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd)
{
   return pd->directional;
}

EOLIAN static void
_eo_evas_3d_light_ambient_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   pd->ambient.r = r;
   pd->ambient.g = g;
   pd->ambient.b = b;
   pd->ambient.a = a;

   evas_3d_object_change(&pd->base, EVAS_3D_STATE_LIGHT_AMBIENT, NULL);
}

EOLIAN static void
_eo_evas_3d_light_ambient_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = pd->ambient.r;
   if (g) *g = pd->ambient.g;
   if (b) *b = pd->ambient.b;
   if (a) *a = pd->ambient.a;
}

EOLIAN static void
_eo_evas_3d_light_diffuse_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   pd->diffuse.r = r;
   pd->diffuse.g = g;
   pd->diffuse.b = b;
   pd->diffuse.a = a;

   evas_3d_object_change(&pd->base, EVAS_3D_STATE_LIGHT_DIFFUSE, NULL);
}

EOLIAN static void
_eo_evas_3d_light_diffuse_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = pd->diffuse.r;
   if (g) *g = pd->diffuse.g;
   if (b) *b = pd->diffuse.b;
   if (a) *a = pd->diffuse.a;
}

EOLIAN static void
_eo_evas_3d_light_specular_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   pd->specular.r = r;
   pd->specular.g = g;
   pd->specular.b = b;
   pd->specular.a = a;

   evas_3d_object_change(&pd->base, EVAS_3D_STATE_LIGHT_SPECULAR, NULL);
}

EOLIAN static void
_eo_evas_3d_light_specular_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = pd->specular.r;
   if (g) *g = pd->specular.g;
   if (b) *b = pd->specular.b;
   if (a) *a = pd->specular.a;
}

EOLIAN static void
_eo_evas_3d_light_spot_exponent_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real exponent)
{
   pd->spot_exp = exponent;
   evas_3d_object_change(&pd->base, EVAS_3D_STATE_LIGHT_SPOT_EXP, NULL);
}

EOLIAN static Evas_Real
_eo_evas_3d_light_spot_exponent_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd)
{
   return pd->spot_exp;
}

EOLIAN static void
_eo_evas_3d_light_spot_cutoff_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real cutoff)
{
   pd->spot_cutoff = cutoff;
   pd->spot_cutoff_cos = cos(cutoff * M_PI / 180.0);
   evas_3d_object_change(&pd->base, EVAS_3D_STATE_LIGHT_SPOT_CUTOFF, NULL);
}

EOLIAN static Evas_Real
_eo_evas_3d_light_spot_cutoff_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd)
{
   return pd->spot_cutoff;
}

EOLIAN static void
_eo_evas_3d_light_attenuation_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real constant, Evas_Real linear, Evas_Real quadratic)
{
   pd->atten_const = constant;
   pd->atten_linear = linear;
   pd->atten_quad = quadratic;
   evas_3d_object_change(&pd->base, EVAS_3D_STATE_LIGHT_ATTENUATION, NULL);
}

EOLIAN static void
_eo_evas_3d_light_attenuation_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Evas_Real *constant, Evas_Real *linear, Evas_Real *quadratic)
{
   if (constant) *constant = pd->atten_const;
   if (linear) *linear = pd->atten_linear;
   if (quadratic) *quadratic = pd->atten_quad;
}

EOLIAN static void
_eo_evas_3d_light_attenuation_enable_set(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd, Eina_Bool enable)
{
   if (pd->enable_attenuation != enable)
     {
        pd->enable_attenuation = enable;
        evas_3d_object_change(&pd->base, EVAS_3D_STATE_LIGHT_ATTENUATION, NULL);
     }
}

EOLIAN static Eina_Bool
_eo_evas_3d_light_attenuation_enable_get(Eo *obj EINA_UNUSED, Evas_3D_Light_Data *pd)
{
   return pd->enable_attenuation;
}

#include "canvas/evas_3d_light.eo.c"