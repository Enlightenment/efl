#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_CANVAS3D_CAMERA_CLASS

static Eina_Bool
_camera_node_change_notify(const Eina_Hash *hash EINA_UNUSED, const void *key,
                        void *data EINA_UNUSED, void *fdata)
{
   Evas_Canvas3D_Node *n = *(Evas_Canvas3D_Node **)key;
   evas_canvas3d_object_change(n, EVAS_CANVAS3D_STATE_NODE_CAMERA, (Evas_Canvas3D_Object *)fdata);
   return EINA_TRUE;
}

EOLIAN static void
_evas_canvas3d_camera_evas_canvas3d_object_change_notify(Eo *obj,Evas_Canvas3D_Camera_Data *pd, Evas_Canvas3D_State state EINA_UNUSED, Evas_Canvas3D_Object *ref EINA_UNUSED)
{
   if (pd->nodes) eina_hash_foreach(pd->nodes, _camera_node_change_notify, obj);
}

EOLIAN static void
_evas_canvas3d_camera_evas_canvas3d_object_update_notify(Eo *obj EINA_UNUSED,
                                                   Evas_Canvas3D_Camera_Data *pd EINA_UNUSED)
{

}

void
evas_canvas3d_camera_node_add(Evas_Canvas3D_Camera *camera, Evas_Canvas3D_Node *node)
{
   Evas_Canvas3D_Camera_Data *pd = efl_data_scope_get(camera, MY_CLASS);
   int count = 0;

   if (!pd->nodes)
     {
        pd->nodes = eina_hash_pointer_new(NULL);

        if (!pd->nodes)
          {
             ERR("Failed to create hash table. camera(%p)", camera);
             return;
          }
     }
   else count = (int)(uintptr_t)eina_hash_find(pd->nodes, &node);
   eina_hash_set(pd->nodes, &node, (const void *)(uintptr_t)(count + 1));
}

void
evas_canvas3d_camera_node_del(Evas_Canvas3D_Camera *camera, Evas_Canvas3D_Node *node)
{
   Evas_Canvas3D_Camera_Data *pd = efl_data_scope_get(camera, MY_CLASS);
   int count = 0;

   if (!pd->nodes)
     {
        ERR("No node to delete.");
        return;
     }

   count = (int)(uintptr_t)eina_hash_find(pd->nodes, &node);
   if (count == 1) eina_hash_del(pd->nodes, &node, NULL);
   else eina_hash_set(pd->nodes, &node, (const void *)(uintptr_t)(count - 1));
}

EOLIAN static Eo *
_evas_canvas3d_camera_efl_object_constructor(Eo *obj,
                                       Evas_Canvas3D_Camera_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_canvas3d_object_type_set(obj, EVAS_CANVAS3D_OBJECT_TYPE_CAMERA);

   return obj;
}

EOLIAN static void
_evas_canvas3d_camera_efl_object_destructor(Eo *obj,
                                      Evas_Canvas3D_Camera_Data *pd)
{
   Eina_Iterator *it = NULL;
   void *data = NULL;
   Evas_Canvas3D_Node_Data *node = NULL;

   if (pd->nodes)
     {
        it = eina_hash_iterator_key_new(pd->nodes);
        while (eina_iterator_next(it, &data))
          {
             node = efl_data_scope_get(data, EVAS_CANVAS3D_NODE_CLASS);
             node->data.camera.camera = NULL;
          }

        eina_hash_free(pd->nodes);
     }

   efl_destructor(efl_super(obj, MY_CLASS));
}

EAPI Evas_Canvas3D_Camera *
evas_canvas3d_camera_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return efl_add(MY_CLASS, e);
}

EOLIAN static void
_evas_canvas3d_camera_projection_matrix_set(Eo *obj, Evas_Canvas3D_Camera_Data *pd,
                                         const Evas_Real *matrix)
{
   eina_matrix4_array_set(&pd->projection, matrix);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_CAMERA_PROJECTION, NULL);
}

EOLIAN static void
_evas_canvas3d_camera_projection_matrix_get(const Eo *obj EINA_UNUSED,
                                         Evas_Canvas3D_Camera_Data *pd,
                                         Evas_Real *matrix)
{
   memcpy(matrix, &pd->projection.xx, sizeof(Evas_Real) * 16);
}

EOLIAN static void
_evas_canvas3d_camera_projection_perspective_set(Eo *obj, Evas_Canvas3D_Camera_Data *pd,
                                              Evas_Real fovy, Evas_Real aspect,
                                              Evas_Real dnear, Evas_Real dfar)
{
   Evas_Real   xmax;
   Evas_Real   ymax;

   ymax = dnear * (Evas_Real)tan((double)fovy * M_PI / 360.0);
   xmax = ymax * aspect;

   evas_mat4_frustum_set(&pd->projection, -xmax, xmax, -ymax, ymax, dnear, dfar);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_CAMERA_PROJECTION, NULL);
}

EOLIAN static void
_evas_canvas3d_camera_projection_frustum_set(Eo *obj, Evas_Canvas3D_Camera_Data *pd,
                                          Evas_Real left, Evas_Real right,
                                          Evas_Real bottom, Evas_Real top,
                                          Evas_Real dnear, Evas_Real dfar)
{
   evas_mat4_frustum_set(&pd->projection, left, right, bottom, top, dnear, dfar);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_CAMERA_PROJECTION, NULL);
}

EOLIAN static void
_evas_canvas3d_camera_projection_ortho_set(Eo *obj, Evas_Canvas3D_Camera_Data *pd,
                                        Evas_Real left, Evas_Real right,
                                        Evas_Real bottom, Evas_Real top,
                                        Evas_Real dnear, Evas_Real dfar)
{
   eina_matrix4_ortho_set(&pd->projection, left, right, bottom, top, dnear, dfar);
   evas_canvas3d_object_change(obj, EVAS_CANVAS3D_STATE_CAMERA_PROJECTION, NULL);
}

EOLIAN static Eina_Bool
_evas_canvas3d_camera_node_visible_get(Eo *obj EINA_UNUSED, Evas_Canvas3D_Camera_Data *pd, Evas_Canvas3D_Node *camera_node, Evas_Canvas3D_Node *node, Evas_Canvas3D_Frustum_Mode key)
{
   Eina_Matrix4 matrix_vp;
   Eina_Quaternion planes[6];
   Evas_Canvas3D_Node_Data *pd_node = efl_data_scope_get(node, EVAS_CANVAS3D_NODE_CLASS);
   Evas_Canvas3D_Node_Data *pd_camera = efl_data_scope_get(camera_node, EVAS_CANVAS3D_NODE_CLASS);
   Eina_Vector3 central_point;

   if (!node || pd_node->type != EVAS_CANVAS3D_NODE_TYPE_MESH)
     {
        ERR("Mesh node %p type mismatch.", node);
        return EINA_FALSE;
     }

   if (!camera_node || pd_camera->type != EVAS_CANVAS3D_NODE_TYPE_CAMERA)
     {
        ERR("Camera node %p type mismatch.", camera_node);
        return EINA_FALSE;
     }

   /*get need matrix like multiply projection matrix with view matrix*/
   eina_matrix4_multiply(&matrix_vp, &pd->projection, &pd_camera->data.camera.matrix_world_to_eye);

   evas_frustum_calculate(planes, &matrix_vp);

   if (key == EVAS_CANVAS3D_FRUSTUM_MODE_BSPHERE)
     return evas_is_sphere_in_frustum(&pd_node->bsphere, planes);
   else if (key == EVAS_CANVAS3D_FRUSTUM_MODE_AABB)
     return evas_is_box_in_frustum(&pd_node->aabb, planes);
   else if (key == EVAS_CANVAS3D_FRUSTUM_MODE_CENTRAL_POINT)
     {
        central_point.x = (pd_node->aabb.p0.x + pd_node->aabb.p1.x) / 2;
        central_point.y = (pd_node->aabb.p0.y + pd_node->aabb.p1.y) / 2;
        central_point.z = (pd_node->aabb.p0.z + pd_node->aabb.p1.z) / 2;
        return evas_is_point_in_frustum(&central_point, planes);
     }
   else
     {
        ERR("Unknown frustun mode.");
        return EINA_TRUE;
     }
}

#include "canvas/evas_canvas3d_camera.eo.c"

