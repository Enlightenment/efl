#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

#include "Eo.h"

#define MY_CLASS EO_EVAS_3D_CAMERA_CLASS

static void
_camera_free(Evas_3D_Object *obj)
{
   Evas_3D_Camera_Data *pd = (Evas_3D_Camera_Data *)obj;

   if (pd->nodes)
     eina_hash_free(pd->nodes);

   //free(pd);
}

static Eina_Bool
_camera_node_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                        void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Node *n = *(Evas_3D_Node **)key;
   Evas_3D_Node_Data *pdnode = eo_data_scope_get(n, EO_EVAS_3D_NODE_CLASS);
   evas_3d_object_change(&pdnode->base, EVAS_3D_STATE_NODE_CAMERA, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_camera_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED,
               Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Camera_Data *pd = (Evas_3D_Camera_Data *)obj;

   if (pd->nodes)
     eina_hash_foreach(pd->nodes, _camera_node_change_notify, obj);
}

static const Evas_3D_Object_Func camera_func =
{
   _camera_free,
   _camera_change,
   NULL,
};

void
evas_3d_camera_node_add(Evas_3D_Camera *camera, Evas_3D_Node *node)
{
   int count = 0;
   Evas_3D_Camera_Data *pd = eo_data_scope_get(camera, MY_CLASS);
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
evas_3d_camera_node_del(Evas_3D_Camera *camera, Evas_3D_Node *node)
{
   int count = 0;
   Evas_3D_Camera_Data *pd = eo_data_scope_get(camera, MY_CLASS);

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

// Evas_3D_Camera *
// evas_3d_camera_new(Evas *e)
// {
//    Evas_3D_Camera *camera = NULL;

//    camera = (Evas_3D_Camera *)calloc(1, sizeof(Evas_3D_Camera));

//    if (camera == NULL)
//      {
//         ERR("Failed to allocate memory.");
//         return NULL;
//      }

//    evas_3d_object_init(&camera->base, e, EVAS_3D_OBJECT_TYPE_CAMERA, &camera_func);
//    return camera;
// }
EOLIAN static void
_eo_evas_3d_camera_eo_base_constructor(Eo *obj, Evas_3D_Camera_Data *pd)
{
   Eo *e;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, e = eo_parent_get());
   evas_3d_object_init(&pd->base, e, EVAS_3D_OBJECT_TYPE_CAMERA, &camera_func);
}

EOLIAN static void
_eo_evas_3d_camera_eo_base_destructor(Eo *obj EINA_UNUSED, Evas_3D_Camera_Data *pd)
{
   evas_3d_object_unreference(&pd->base);
}

EAPI Evas_3D_Camera *
evas_3d_camera_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = eo_add(MY_CLASS, e);
   eo_unref(eo_obj);
   return eo_obj;
   //return evas_3d_camera_new(e);
}

EOLIAN static Evas *
_eo_evas_3d_camera_evas_common_interface_evas_get(Eo *obj EINA_UNUSED, Evas_3D_Camera_Data *pd)
{
   return pd->base.evas;
}

EOLIAN static void
_eo_evas_3d_camera_projection_matrix_set(Eo *obj EINA_UNUSED, Evas_3D_Camera_Data *pd, const Evas_Real *matrix)
{
   evas_mat4_array_set(&pd->projection, matrix);
   evas_3d_object_change(&pd->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}

EOLIAN static void
_eo_evas_3d_camera_projection_matrix_get(Eo *obj EINA_UNUSED, Evas_3D_Camera_Data *pd, Evas_Real *matrix)
{
   memcpy(matrix, &pd->projection.m[0], sizeof(Evas_Real) * 16);
}

EOLIAN static void
_eo_evas_3d_camera_projection_perspective_set(Eo *obj EINA_UNUSED, Evas_3D_Camera_Data *pd, Evas_Real fovy, Evas_Real aspect, Evas_Real near, Evas_Real far)
{
   Evas_Real   xmax;
   Evas_Real   ymax;

   ymax = near * (Evas_Real)tan((double)fovy * M_PI / 360.0);
   xmax = ymax * aspect;

   evas_mat4_frustum_set(&pd->projection, -xmax, xmax, -ymax, ymax, near, far);
   evas_3d_object_change(&pd->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}

EOLIAN static void
_eo_evas_3d_camera_projection_frustum_set(Eo *obj EINA_UNUSED, Evas_3D_Camera_Data *pd, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far)
{
   evas_mat4_frustum_set(&pd->projection, left, right, bottom, top, near, far);
   evas_3d_object_change(&pd->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}

EOLIAN static void
_eo_evas_3d_camera_projection_ortho_set(Eo *obj EINA_UNUSED, Evas_3D_Camera_Data *pd, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far)
{
   evas_mat4_ortho_set(&pd->projection, left, right, bottom, top, near, far);
   evas_3d_object_change(&pd->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}

#include "canvas/evas_3d_camera.eo.c"

