#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <math.h>
#include "evas_common_private.h"
#include "evas_private.h"

static void
_light_free(Evas_3D_Object *obj)
{
   Evas_3D_Light *light = (Evas_3D_Light *)obj;

   if (light->nodes)
     eina_hash_free(light->nodes);

   free(light);
}

static Eina_Bool
_light_node_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                        void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Node *n = *(Evas_3D_Node **)key;
   evas_3d_object_change(&n->base, EVAS_3D_STATE_NODE_LIGHT, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_light_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED,
              Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Light *light = (Evas_3D_Light *)obj;

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

   if (light->nodes == NULL)
     {
        light->nodes = eina_hash_pointer_new(NULL);

        if (light->nodes == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(light->nodes, &node);

   eina_hash_set(light->nodes, &node, (const void *)(count + 1));
}

void
evas_3d_light_node_del(Evas_3D_Light *light, Evas_3D_Node *node)
{
   int count = 0;

   if (light->nodes == NULL)
     {
        ERR("No node to delete.");
        return;
     }

   count = (int)eina_hash_find(light->nodes, &node);

   if (count == 1)
     eina_hash_del(light->nodes, &node, NULL);
   else
     eina_hash_set(light->nodes, &node, (const void *)(count - 1));
}

Evas_3D_Light *
evas_3d_light_new(Evas *e)
{
   Evas_3D_Light *light = NULL;

   light = (Evas_3D_Light *)calloc(1, sizeof(Evas_3D_Light));

   if (light == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   evas_3d_object_init(&light->base, e, EVAS_3D_OBJECT_TYPE_LIGHT, &light_func);

   evas_color_set(&light->ambient, 0.0, 0.0, 0.0, 1.0);
   evas_color_set(&light->diffuse, 1.0, 1.0, 1.0, 1.0);
   evas_color_set(&light->specular, 1.0, 1.0, 1.0, 1.0);

   light->spot_exp = 0.0;
   light->spot_cutoff = 180.0;
   light->spot_cutoff_cos = -1.0;

   light->atten_const = 1.0;
   light->atten_linear = 0.0;
   light->atten_quad = 0.0;

   return light;
}

EAPI Evas_3D_Light *
evas_3d_light_add(Evas *e)
{
   return evas_3d_light_new(e);
}

EAPI void
evas_3d_light_del(Evas_3D_Light *light)
{
   evas_3d_object_unreference(&light->base);
}

EAPI Evas *
evas_3d_light_evas_get(const Evas_3D_Light *light)
{
   return light->base.evas;
}

EAPI void
evas_3d_light_directional_set(Evas_3D_Light *light, Eina_Bool directional)
{
   if (light->directional != directional)
     {
        light->directional = directional;
        evas_3d_object_change(&light->base, EVAS_3D_STATE_ANY, NULL);
     }
}

EAPI Eina_Bool
evas_3d_light_directional_get(const Evas_3D_Light *light)
{
   return light->directional;
}

EAPI void
evas_3d_light_ambient_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   light->ambient.r = r;
   light->ambient.g = g;
   light->ambient.b = b;
   light->ambient.a = a;

   evas_3d_object_change(&light->base, EVAS_3D_STATE_LIGHT_AMBIENT, NULL);
}

EAPI void
evas_3d_light_ambient_get(const Evas_3D_Light *light,
                          Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = light->ambient.r;
   if (g) *g = light->ambient.g;
   if (b) *b = light->ambient.b;
   if (a) *a = light->ambient.a;
}

EAPI void
evas_3d_light_diffuse_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   light->diffuse.r = r;
   light->diffuse.g = g;
   light->diffuse.b = b;
   light->diffuse.a = a;

   evas_3d_object_change(&light->base, EVAS_3D_STATE_LIGHT_DIFFUSE, NULL);
}

EAPI void
evas_3d_light_diffuse_get(const Evas_3D_Light *light,
                          Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = light->diffuse.r;
   if (g) *g = light->diffuse.g;
   if (b) *b = light->diffuse.b;
   if (a) *a = light->diffuse.a;
}

EAPI void
evas_3d_light_specular_set(Evas_3D_Light *light, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   light->specular.r = r;
   light->specular.g = g;
   light->specular.b = b;
   light->specular.a = a;

   evas_3d_object_change(&light->base, EVAS_3D_STATE_LIGHT_SPECULAR, NULL);
}

EAPI void
evas_3d_light_specular_get(const Evas_3D_Light *light,
                           Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r) *r = light->specular.r;
   if (g) *g = light->specular.g;
   if (b) *b = light->specular.b;
   if (a) *a = light->specular.a;
}

EAPI void
evas_3d_light_spot_exponent_set(Evas_3D_Light *light, Evas_Real exponent)
{
   light->spot_exp = exponent;
   evas_3d_object_change(&light->base, EVAS_3D_STATE_LIGHT_SPOT_EXP, NULL);
}

EAPI Evas_Real
evas_3d_light_spot_exponent_get(const Evas_3D_Light *light)
{
   return light->spot_exp;
}

EAPI void
evas_3d_light_spot_cutoff_set(Evas_3D_Light *light, Evas_Real cutoff)
{
   light->spot_cutoff = cutoff;
   light->spot_cutoff_cos = cos(cutoff * M_PI / 180.0);
   evas_3d_object_change(&light->base, EVAS_3D_STATE_LIGHT_SPOT_CUTOFF, NULL);
}

EAPI Evas_Real
evas_3d_light_spot_cutoff_get(const Evas_3D_Light *light)
{
   return light->spot_cutoff;
}

EAPI void
evas_3d_light_attenuation_set(Evas_3D_Light *light,
                              Evas_Real constant, Evas_Real linear, Evas_Real quadratic)
{
   light->atten_const = constant;
   light->atten_linear = linear;
   light->atten_quad = quadratic;
   evas_3d_object_change(&light->base, EVAS_3D_STATE_LIGHT_ATTENUATION, NULL);
}

EAPI void
evas_3d_light_attenuation_get(const Evas_3D_Light *light, Evas_Real *constant, Evas_Real *linear, Evas_Real *quadratic)
{
   if (constant) *constant = light->atten_const;
   if (linear) *linear = light->atten_linear;
   if (quadratic) *quadratic = light->atten_quad;
}

EAPI void
evas_3d_light_attenuation_enable_set(Evas_3D_Light *light, Eina_Bool enable)
{
   if (light->enable_attenuation != enable)
     {
        light->enable_attenuation = enable;
        evas_3d_object_change(&light->base, EVAS_3D_STATE_LIGHT_ATTENUATION, NULL);
     }
}

EAPI Eina_Bool
evas_3d_light_attenuation_enable_get(const Evas_3D_Light *light)
{
   return light->enable_attenuation;
}
