#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_CANVAS3D_MATERIAL_CLASS

static Eina_Bool
_material_mesh_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                        void *data EINA_UNUSED, void *fdata)
{
   Evas_Canvas3D_Mesh *m = *(Evas_Canvas3D_Mesh **)key;
   evas_canvas3d_object_change(m, EVAS_CANVAS3D_STATE_MESH_MATERIAL, (Evas_Canvas3D_Object *)fdata);
   return EINA_TRUE;
}

EOLIAN static void
_evas_canvas3d_material_evas_canvas3d_object_change_notify(Eo *obj, Evas_Canvas3D_Material_Data *pd, Evas_Canvas3D_State state EINA_UNUSED, Evas_Canvas3D_Object *ref EINA_UNUSED)
{
   if (pd->meshes)
     eina_hash_foreach(pd->meshes, _material_mesh_change_notify, obj);
}

EOLIAN static void
_evas_canvas3d_material_evas_canvas3d_object_update_notify(Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd)
{
   int i;
   for (i = 0; i < EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT; i++)
     {
        if (pd->attribs[i].enable)
          {
             if (pd->attribs[i].texture)
               {
                  evas_canvas3d_object_update(pd->attribs[i].texture);
               }
          }
     }
}

void
evas_canvas3d_material_mesh_add(Evas_Canvas3D_Material *material, Evas_Canvas3D_Mesh *mesh)
{
   int count = 0;
   Evas_Canvas3D_Material_Data *pd = efl_data_scope_get(material, MY_CLASS);

   if (pd->meshes == NULL)
     {
        pd->meshes = eina_hash_pointer_new(NULL);

        if (pd->meshes == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)(uintptr_t)eina_hash_find(pd->meshes, &mesh);

   eina_hash_set(pd->meshes, &mesh, (const void *)(uintptr_t)(count + 1));
}

void
evas_canvas3d_material_mesh_del(Evas_Canvas3D_Material *material, Evas_Canvas3D_Mesh *mesh)
{
   int count = 0;
   Evas_Canvas3D_Material_Data *pd = efl_data_scope_get(material, MY_CLASS);

   if (pd->meshes == NULL)
     {
        ERR("No mesh to delete.");
        return;
     }

   count = (int)(uintptr_t)eina_hash_find(pd->meshes, &mesh);

   if (count == 1)
     eina_hash_del(pd->meshes, &mesh, NULL);
   else
     eina_hash_set(pd->meshes, &mesh, (const void *)(uintptr_t)(count - 1));
}


EAPI Evas_Canvas3D_Material *
evas_canvas3d_material_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return efl_add(MY_CLASS, e);
}

EOLIAN static Eo *
_evas_canvas3d_material_efl_object_constructor(Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_canvas3d_object_type_set(obj, EVAS_CANVAS3D_OBJECT_TYPE_MATERIAL);

   evas_color_set(&pd->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT].color, 0.2, 0.2, 0.2, 1.0);
   evas_color_set(&pd->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE].color, 0.8, 0.8, 0.8, 1.0);
   evas_color_set(&pd->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR].color, 1.0, 1.0, 1.0, 1.0);
   evas_color_set(&pd->attribs[EVAS_CANVAS3D_MATERIAL_ATTRIB_EMISSION].color, 0.0, 0.0, 0.0, 1.0);
   pd->shininess = 150.0;

   return obj;
}

EOLIAN static void
_evas_canvas3d_material_efl_object_destructor(Eo *obj, Evas_Canvas3D_Material_Data *pd)
{
   int i = 0;
   Eina_Iterator *it = NULL;
   void *data = NULL;
   Evas_Canvas3D_Mesh_Data *mesh = NULL;
   Eina_List *l = NULL;
   Evas_Canvas3D_Mesh_Frame *f = NULL;

   if (pd->meshes)
     {
        it = eina_hash_iterator_key_new(pd->meshes);
        while (eina_iterator_next(it, &data))
          {
             mesh = efl_data_scope_get(data, EVAS_CANVAS3D_MESH_CLASS);
             if (mesh->frames)
               {
                  EINA_LIST_FOREACH(mesh->frames, l, f)
                     f->material = NULL;
               }
          }
        eina_iterator_free(it);
        eina_hash_free(pd->meshes);
     }

   for (i = 0; i < EVAS_CANVAS3D_MATERIAL_ATTRIB_COUNT; i++)
     {
        if (pd->attribs[i].texture)
          {
             evas_canvas3d_texture_material_del(pd->attribs[i].texture, obj);
          }
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_canvas3d_material_enable_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd, Evas_Canvas3D_Material_Attrib attrib, Eina_Bool enable)
{
   pd->attribs[attrib].enable = enable;
}

EOLIAN static Eina_Bool
_evas_canvas3d_material_enable_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd, Evas_Canvas3D_Material_Attrib attrib)
{
   return pd->attribs[attrib].enable;
}

EOLIAN static void
_evas_canvas3d_material_color_set(Eo *obj, Evas_Canvas3D_Material_Data *pd, Evas_Canvas3D_Material_Attrib attrib, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   evas_color_set(&pd->attribs[attrib].color, r, g, b, a);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_MATERIAL_COLOR, NULL);
}

EOLIAN static void
_evas_canvas3d_material_color_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd, Evas_Canvas3D_Material_Attrib attrib, Evas_Real *r, Evas_Real *g, Evas_Real *b, Evas_Real *a)
{
   if (r)  *r = pd->attribs[attrib].color.r;
   if (g)  *g = pd->attribs[attrib].color.g;
   if (b)  *b = pd->attribs[attrib].color.b;
   if (a)  *a = pd->attribs[attrib].color.a;
}

EOLIAN static void
_evas_canvas3d_material_shininess_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd, Evas_Real shininess)
{
   pd->shininess = shininess;
}

EOLIAN static Evas_Real
_evas_canvas3d_material_shininess_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd)
{
   return pd->shininess;
}

EOLIAN static void
_evas_canvas3d_material_texture_set(Eo *obj, Evas_Canvas3D_Material_Data *pd, Evas_Canvas3D_Material_Attrib attrib, Evas_Canvas3D_Texture *texture)
{
   if (pd->attribs[attrib].texture != texture)
     {
        if (pd->attribs[attrib].texture)
          {
             evas_canvas3d_texture_material_del(pd->attribs[attrib].texture, obj);
             efl_unref(pd->attribs[attrib].texture);
          }

        pd->attribs[attrib].texture = texture;
        evas_canvas3d_texture_material_add(texture, obj);
        efl_ref(texture);
     }

   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_MATERIAL_TEXTURE, NULL);
}

EOLIAN static Evas_Canvas3D_Texture *
_evas_canvas3d_material_texture_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Material_Data *pd, Evas_Canvas3D_Material_Attrib attrib)
{
   return pd->attribs[attrib].texture;
}

#include "canvas/evas_canvas3d_material.eo.c"
