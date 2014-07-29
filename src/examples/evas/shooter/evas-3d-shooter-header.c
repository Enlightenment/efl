#include "evas-3d-shooter-header.h"

void evas_vec3_set(vec3 *dst, Evas_Real x, Evas_Real y, Evas_Real z)
{
   dst->x = x;
   dst->y = y;
   dst->z = z;
}

void evas_box3_empty_set(Box3 *box)
{
   evas_vec3_set(&box->p0, 0.0, 0.0, 0.0);
   evas_vec3_set(&box->p1, 0.0, 0.0, 0.0);
}

void evas_box3_set(Box3 *box, Evas_Real x0, Evas_Real y0, Evas_Real z0, Evas_Real x1, Evas_Real y1, Evas_Real z1)
{
   box->p0.x = x0;
   box->p0.y = y0;
   box->p0.z = z0;
   box->p1.x = x1;
   box->p1.y = y1;
   box->p1.z = z1;
}

void evas_box3_union(Box3 *out, const Box3 *a, const Box3 *b)
{
   evas_vec3_set(&out->p0, MIN(a->p0.x, b->p0.x), MIN(a->p0.y, b->p0.y), MIN(a->p0.z, b->p0.z));
   evas_vec3_set(&out->p1, MAX(a->p1.x, b->p1.x), MAX(a->p1.y, b->p1.y), MAX(a->p1.z, b->p1.z));
}

void _add_annulus(float * current_r1, int size, float r1, float r2, float z1, float z2)
{
   int i;
   float * current_r2 = current_r1 + 12 * size;
   float arc = (float)2 * 3.1415 / size;

   for( i = 0 ; i < size ; i++)
     {
        *current_r1 = r1 * (float)sin(i * arc);
        current_r1 += 1;
        *current_r1 = r1 * (float)cos(i * arc);
        current_r1 += 1;
        *current_r1 = z1;
        current_r1 += 1;
        *current_r1 = (float)sin(i * arc) * (r2 - r1);
        current_r1 += 1;
        *current_r1 = (float)cos(i * arc) * (r2 - r1);
        current_r1 += 1;
        *current_r1 = (z2 - z1);
        current_r1 += 7;
        *current_r2 = r2 * (float)sin(i * arc);
        current_r2 += 1;
        *current_r2 = r2 * (float)cos(i * arc);
        current_r2 += 1;
        *current_r2 = z2;
        current_r2 += 1;
        *current_r2 = (float)sin(i * arc) * (r2 -r1);
        current_r2 += 1;
        *current_r2 = (float)cos(i * arc) * (r2 - r1);
        current_r2 += 1;
        *current_r2 = (z2 - z1);
        current_r2 += 7;
     }
}

void _add_annulus_vertices(unsigned short * current_r1, int size, int first)
{
   int i;
   for( i = 0 ; i < size - 1 ; i++)
     {
        *current_r1 = first + i;
        current_r1 += 1;
        *current_r1 = first + 1 + i;
        current_r1 += 1;
        *current_r1 = first + size + i;
        current_r1 += 1;

        *current_r1 = first + size + i;
        current_r1 += 1;
        *current_r1 = first + 1 + size + i;
        current_r1 += 1;
        *current_r1 = first + 1 + i;
        current_r1 += 1;
     }
        *current_r1 = first + size - 1;
        current_r1 += 1;
        *current_r1 = first;
        current_r1 += 1;
        *current_r1 = first + 2 * size - 1;
        current_r1 += 1;

        *current_r1 = first + 2 * size - 1;
        current_r1 += 1;
        *current_r1 = first + size;
        current_r1 += 1;
        *current_r1 = first;
        current_r1 += 1;
}

void
_scale(Evas_3D_Node *node, Evas_Real scale)
{
   eo_do(node, evas_3d_node_scale_set(1.0 * scale, 1.0 * scale, 1.0 * scale));
}

void _add_solid_of_revolution(float * start, int size, float * vertic, unsigned short * indic)
{
   int i;
   int accurancy = 35;
   float * current = start;
   for( i = 0 ; i < size ; i++)
     {
        _add_annulus(&vertic[i*accurancy*24], accurancy, *current, *(current + 2), *(current + 1), *(current + 3));
        current+=2;
        _add_annulus_vertices(&indic[i*accurancy*6], accurancy, i*accurancy*2);
     }
}

void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(data->camera,
         evas_3d_camera_projection_perspective_set(65.0, 1.0, 1.0, 100.0));

   data->mediator_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                       evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));
   data->camera_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                     evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));

   eo_do(data->root_node, evas_3d_node_member_add(data->mediator_node));
   eo_do(data->mediator_node, evas_3d_node_member_add(data->camera_node));

   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(0.0, 0.0, 1.0);
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
}

void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_3D_LIGHT_CLASS, evas);

   eo_do(data->light,
         evas_3d_light_ambient_set(0.2, 0.2, 0.2, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0));

   data->light_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(15.0, 0.0, 30.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));

   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}

Eina_Bool _mesh_aabb(Evas_3D_Mesh **mesh, Scene_Data *scene)
{
   Evas_Real x0, y0, z0, x1, y1, z1;
   eo_do(scene->mesh_node_level[10],
         evas_3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));

   float vertixes[] =
   {
       x0,  y0,  z1,     0.0,  0.0,  1.0,
       x0,  y1,  z1,     0.0,  0.0,  1.0,
       x1,  y1,  z1,     0.0,  0.0,  1.0,
       x1,  y0,  z1,     0.0,  0.0,  1.0,

       x0,  y0,  z0,     0.0,  0.0, -1.0,
       x1,  y0,  z0,     0.0,  0.0, -1.0,
       x0,  y1,  z0,     0.0,  0.0, -1.0,
       x1,  y1,  z0,     0.0,  0.0, -1.0,

       x0,  y0,  z0,    -1.0,  0.0,  0.0,
       x0,  y1,  z0,    -1.0,  0.0,  0.0,
       x0,  y0,  z1,    -1.0,  0.0,  0.0,
       x0,  y1,  z1,    -1.0,  0.0,  0.0,

       x1,  y0,  z0,     1.0,  0.0,  0.0,
       x1,  y1,  z0,     1.0,  0.0,  0.0,
       x1,  y1,  z1,     1.0,  0.0,  0.0,
       x1,  y0,  z1,     1.0,  0.0,  0.0,

       x0,  y1,  z0,     0.0,  1.0,  0.0,
       x1,  y1,  z0,     0.0,  1.0,  0.0,
       x0,  y1,  z1,     0.0,  1.0,  0.0,
       x1,  y1,  z1,     0.0,  1.0,  0.0,

       x0,  y0,  z0,     0.0, -1.0,  0.0,
       x1,  y0,  z0,     0.0, -1.0,  0.0,
       x1,  y0,  z1,     0.0, -1.0,  0.0,
       x0,  y0,  z1,     0.0, -1.0,  0.0
   };

   unsigned short indixes[] =
   {
      0,  1,  2,  3,  1,  2,  0,  3,
      4,  5,  5,  7,  7,  6,  6,  4,
      8,  9,  9,  11, 11, 10, 10, 8,
      12, 13, 13, 14, 14, 15, 15, 12,
      16, 17, 17, 19, 19, 18, 18, 16,
      20, 21, 21, 22, 22, 23, 23, 20
   };

   if (*mesh)
     {
        eo_do(*mesh,
              evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION, 6 * sizeof(float), &vertixes[ 0]),
              evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 48, &indixes[0]));

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

void _on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

void _on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   evas_object_resize(background, w, h);
   evas_object_resize(image, w, h);
   evas_object_move(image, 0, 0);
}

Eina_Bool _horizontal_circle_resolution(Evas_3D_Node *node, Evas_Real x, Evas_Real z, Evas_Real r)
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   eo_do(node,
         evas_3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));

   if (((x0 - x) * (x0 - x)) + ((z0 - z) * (z0 - z)) < r * r) return EINA_FALSE;
   if (((x0 - x) * (x0 - x)) + ((z1 - z) * (z1 - z)) < r * r) return EINA_FALSE;
   if (((x1 - x) * (x1 - x)) + ((z0 - z) * (z0 - z)) < r * r) return EINA_FALSE;
   if (((x1 - x) * (x1 - x)) + ((z1 - z) * (z1 - z)) < r * r) return EINA_FALSE;

   if ((((x + r < x0) && (x + r > x1)) || ((x + r > x0) && (x+r < x1))) && (((z < z0) && (z > z1)) || ((z > z0) && (z < z1))))
     return EINA_FALSE;
   if ((((x - r < x0) && (x - r > x1)) || ((x - r > x0) && (x-r < x1))) && (((z < z0) && (z > z1)) || ((z > z0) && (z < z1))))
     return EINA_FALSE;
   if ((((z + r < z0) && (z + r > z1)) || ((z + r > z0) && (z+r < z1))) && (((x < x0) && (x > x1)) || ((x > x0) && (x < x1))))
     return EINA_FALSE;
   if ((((z - r < z0) && (z - r > z1)) || ((z - r > z0) && (z-r < z1))) && (((x < x0) && (x > x1)) || ((x > x0) && (x < x1))))
     return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Bool _horizontal_position_resolution(Evas_3D_Node *node, Evas_Real x, Evas_Real z)
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   eo_do(node,
         evas_3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));

   if ((x > x0) && (x < x1) && (z > z0) && (z < z1))
     return EINA_FALSE;
   if ((x > x0) && (x < x1) && (z < z0) && (z > z1))
     return EINA_FALSE;
   if ((x < x0) && (x > x1) && (z > z0) && (z < z1))
     return EINA_FALSE;
   if ((x < x0) && (x > x1) && (z < z0) && (z > z1))
     return EINA_FALSE;

   return EINA_TRUE;
}
