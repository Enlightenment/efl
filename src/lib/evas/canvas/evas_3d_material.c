#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

static void
_material_free(Evas_3D_Object *obj)
{
   int i;
   Evas_3D_Material *material = (Evas_3D_Material *)obj;

   if (material->meshes)
     eina_hash_free(material->meshes);

   for (i = 0; i < EVAS_3D_MATERIAL_ATTRIB_COUNT; i++)
     {
        if (material->attribs[i].texture)
          {
             evas_3d_texture_material_del(material->attribs[i].texture, material);
             evas_3d_object_unreference(&material->attribs[i].texture->base);
          }
     }

   free(material);
}

static Eina_Bool
_material_mesh_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                        void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Mesh *m = *(Evas_3D_Mesh **)key;
   evas_3d_object_change(&m->base, EVAS_3D_STATE_MESH_MATERIAL, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_material_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED,
                 Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Material *material = (Evas_3D_Material *)obj;

   if (material->meshes)
     eina_hash_foreach(material->meshes, _material_mesh_change_notify, obj);
}

static void
_material_update(Evas_3D_Object *obj)
{
   int i;
   Evas_3D_Material *material = (Evas_3D_Material *)obj;

   for (i = 0; i < EVAS_3D_MATERIAL_ATTRIB_COUNT; i++)
     {
        if (material->attribs[i].enable)
          {
             if (material->attribs[i].texture)
               evas_3d_object_update(&material->attribs[i].texture->base);
          }
     }
}

static const Evas_3D_Object_Func material_func =
{
   _material_free,
   _material_change,
   _material_update,
};

void
evas_3d_material_mesh_add(Evas_3D_Material *material, Evas_3D_Mesh *mesh)
{
   int count = 0;

   if (material->meshes == NULL)
     {
        material->meshes = eina_hash_pointer_new(NULL);

        if (material->meshes == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(material->meshes, &mesh);

   eina_hash_set(material->meshes, &mesh, (const void *)(count + 1));
}

void
evas_3d_material_mesh_del(Evas_3D_Material *material, Evas_3D_Mesh *mesh)
{
   int count = 0;

   if (material->meshes == NULL)
     {
        ERR("No mesh to delete.");
        return;
     }

   count = (int)eina_hash_find(material->meshes, &mesh);

   if (count == 1)
     eina_hash_del(material->meshes, &mesh, NULL);
   else
     eina_hash_set(material->meshes, &mesh, (const void *)(count - 1));
}

Evas_3D_Material *
evas_3d_material_new(Evas *e)
{
   Evas_3D_Material *material = NULL;

   material = (Evas_3D_Material *)calloc(1, sizeof(Evas_3D_Material));

   if (material == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   evas_3d_object_init(&material->base, e, EVAS_3D_OBJECT_TYPE_MATERIAL, &material_func);

   evas_color_set(&material->attribs[EVAS_3D_MATERIAL_AMBIENT].color, 0.2, 0.2, 0.2, 1.0);
   evas_color_set(&material->attribs[EVAS_3D_MATERIAL_DIFFUSE].color, 0.8, 0.8, 0.8, 1.0);
   evas_color_set(&material->attribs[EVAS_3D_MATERIAL_SPECULAR].color, 1.0, 1.0, 1.0, 1.0);
   evas_color_set(&material->attribs[EVAS_3D_MATERIAL_EMISSION].color, 0.0, 0.0, 0.0, 1.0);
   material->shininess = 150.0;

   return material;
}

EAPI Evas_3D_Material *
evas_3d_material_add(Evas *e)
{
   return evas_3d_material_new(e);
}

EAPI void
evas_3d_material_del(Evas_3D_Material *material)
{
   evas_3d_object_unreference(&material->base);
}

EAPI Evas *
evas_3d_material_evas_get(const Evas_3D_Material *material)
{
   return material->base.evas;
}

EAPI void
evas_3d_material_enable_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib,
                            Eina_Bool enable)
{
   material->attribs[attrib].enable = enable;
}

EAPI Eina_Bool
evas_3d_material_enable_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib)
{
   return material->attribs[attrib].enable;
}

EAPI void
evas_3d_material_color_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib,
                           Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   evas_color_set(&material->attribs[attrib].color, r, g, b, a);
   evas_3d_object_change(&material->base, EVAS_3D_STATE_MATERIAL_COLOR, NULL);
}

EAPI void
evas_3d_material_color_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib,
                           Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r)  *r = material->attribs[attrib].color.r;
   if (g)  *g = material->attribs[attrib].color.g;
   if (b)  *b = material->attribs[attrib].color.b;
   if (a)  *a = material->attribs[attrib].color.a;
}

EAPI void
evas_3d_material_shininess_set(Evas_3D_Material *material, Evas_Real shininess)
{
   material->shininess = shininess;
}

EAPI Evas_Real
evas_3d_material_shininess_get(const Evas_3D_Material *material)
{
   return material->shininess;
}

EAPI void
evas_3d_material_texture_set(Evas_3D_Material *material, Evas_3D_Material_Attrib attrib,
                             Evas_3D_Texture *texture)
{
   if (material->attribs[attrib].texture != texture)
     {
        if (material->attribs[attrib].texture)
          {
             evas_3d_texture_material_del(material->attribs[attrib].texture, material);
             evas_3d_object_unreference(&material->attribs[attrib].texture->base);
          }

        material->attribs[attrib].texture = texture;
        evas_3d_texture_material_add(texture, material);
        evas_3d_object_reference(&texture->base);
     }

   evas_3d_object_change(&material->base, EVAS_3D_STATE_MATERIAL_TEXTURE, NULL);
}

EAPI Evas_3D_Texture *
evas_3d_material_texture_get(const Evas_3D_Material *material, Evas_3D_Material_Attrib attrib)
{
   return material->attribs[attrib].texture;
}
