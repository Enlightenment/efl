#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "evas_common_private.h"
#include "evas_private.h"

static void
_camera_free(Evas_3D_Object *obj)
{
   Evas_3D_Camera *camera = (Evas_3D_Camera *)obj;

   if (camera->nodes)
     eina_hash_free(camera->nodes);

   free(camera);
}

static Eina_Bool
_camera_node_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                        void *data EINA_UNUSED, void *fdata)
{
   Evas_3D_Node *n = *(Evas_3D_Node **)key;
   evas_3d_object_change(&n->base, EVAS_3D_STATE_NODE_CAMERA, (Evas_3D_Object *)fdata);
   return EINA_TRUE;
}

static void
_camera_change(Evas_3D_Object *obj, Evas_3D_State state EINA_UNUSED,
               Evas_3D_Object *ref EINA_UNUSED)
{
   Evas_3D_Camera *camera = (Evas_3D_Camera *)obj;

   if (camera->nodes)
     eina_hash_foreach(camera->nodes, _camera_node_change_notify, obj);
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

   if (camera->nodes == NULL)
     {
        camera->nodes = eina_hash_pointer_new(NULL);

        if (camera->nodes == NULL)
          {
             ERR("Failed to create hash table.");
             return;
          }
     }
   else
     count = (int)eina_hash_find(camera->nodes, &node);

   eina_hash_set(camera->nodes, &node, (const void *)(count + 1));
}

void
evas_3d_camera_node_del(Evas_3D_Camera *camera, Evas_3D_Node *node)
{
   int count = 0;

   if (camera->nodes == NULL)
     {
        ERR("No node to delete.");
        return;
     }

   count = (int)eina_hash_find(camera->nodes, &node);

   if (count == 1)
     eina_hash_del(camera->nodes, &node, NULL);
   else
     eina_hash_set(camera->nodes, &node, (const void *)(count - 1));
}

Evas_3D_Camera *
evas_3d_camera_new(Evas *e)
{
   Evas_3D_Camera *camera = NULL;

   camera = (Evas_3D_Camera *)calloc(1, sizeof(Evas_3D_Camera));

   if (camera == NULL)
     {
        ERR("Failed to allocate memory.");
        return NULL;
     }

   evas_3d_object_init(&camera->base, e, EVAS_3D_OBJECT_TYPE_CAMERA, &camera_func);
   return camera;
}

EAPI Evas_3D_Camera *
evas_3d_camera_add(Evas *e)
{
   return evas_3d_camera_new(e);
}

EAPI void
evas_3d_camera_del(Evas_3D_Camera *camera)
{
   evas_3d_object_unreference(&camera->base);
}

EAPI Evas *
evas_3d_camera_evas_get(const Evas_3D_Camera *camera)
{
   return camera->base.evas;
}

EAPI void
evas_3d_camera_projection_matrix_set(Evas_3D_Camera *camera, const Evas_Real *matrix)
{
   evas_mat4_array_set(&camera->projection, matrix);
   evas_3d_object_change(&camera->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}

EAPI void
evas_3d_camera_projection_matrix_get(const Evas_3D_Camera *camera, Evas_Real *matrix)
{
   memcpy(matrix, &camera->projection.m[0], sizeof(Evas_Real) * 16);
}

EAPI void
evas_3d_camera_projection_perspective_set(Evas_3D_Camera *camera, Evas_Real fovy, Evas_Real aspect, Evas_Real near, Evas_Real far)
{
   Evas_Real   xmax;
   Evas_Real   ymax;

   ymax = near * (Evas_Real)tan((double)fovy * M_PI / 360.0);
   xmax = ymax * aspect;

   evas_mat4_frustum_set(&camera->projection, -xmax, xmax, -ymax, ymax, near, far);
   evas_3d_object_change(&camera->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}

EAPI void
evas_3d_camera_projection_frustum_set(Evas_3D_Camera *camera, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far)
{
   evas_mat4_frustum_set(&camera->projection, left, right, bottom, top, near, far);
   evas_3d_object_change(&camera->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}

EAPI void
evas_3d_camera_projection_ortho_set(Evas_3D_Camera *camera, Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top, Evas_Real near, Evas_Real far)
{
   evas_mat4_ortho_set(&camera->projection, left, right, bottom, top, near, far);
   evas_3d_object_change(&camera->base, EVAS_3D_STATE_CAMERA_PROJECTION, NULL);
}
