#include "evas-3d-shooter-header.h"

void
evas_box3_set(Box3 *box, Evas_Real x0, Evas_Real y0, Evas_Real z0, Evas_Real x1, Evas_Real y1, Evas_Real z1)
{
   box->p0.x = x0;
   box->p0.y = y0;
   box->p0.z = z0;
   box->p1.x = x1;
   box->p1.y = y1;
   box->p1.z = z1;
}

void
_scale(Evas_Canvas3D_Node *node, Evas_Real scale)
{
   evas_canvas3d_node_scale_set(node, 1.0 * scale, 1.0 * scale, 1.0 * scale);
}

float
_distance(float x1, float z1, float x2, float z2)
{
   return sqrt(pow(x2 - x1, 2) + pow(z2 - z1, 2));
}

void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(data->camera, 65.0, 1.0, 1.0, 300.0);

   data->mediator_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));
   data->camera_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));

   evas_canvas3d_node_member_add(data->root_node, data->mediator_node);
   evas_canvas3d_node_member_add(data->mediator_node, data->camera_node);

   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 20.0, 80.0, 30.0);
   evas_canvas3d_node_look_at_set(data->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
}

void
_light_setup(Scene_Data *data)
{
   data->light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);

   evas_canvas3d_light_ambient_set(data->light, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_light_diffuse_set(data->light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(data->light, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_light_projection_perspective_set(data->light, 100.0, 1.0, 1.0, 200.0);

   data->light_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));

   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, -30.0, 70.0, 0.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0);

   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

Eina_Bool _mesh_aabb(Evas_Canvas3D_Mesh **mesh, Scene_Data *scene, int index)
{
   Evas_Real x0, y0, z0, x1, y1, z1, r;
   Evas_Canvas3D_Node *node = NULL;

   if (fmod(index, 12) == 0)
     node = scene->mesh_node_warrior;
   else if (fmod(index, 12) == 1)
     node = scene->mesh_node_eagle;
   else if (fmod(index, 12) == 2)
     node = scene->mesh_node_grass;
   else if (fmod(index, 12) == 3)
     node = scene->mesh_node_soldier;
   else if (fmod(index, 12) == 4)
     node = scene->mesh_node_tommy;
   else if (fmod(index, 12) == 5)
     node = scene->mesh_node_gazebo;
   else if (fmod(index, 12) == 6)
     node = scene->mesh_node_carpet;
   else if (fmod(index, 12) == 7)
     node = scene->mesh_node_wall[0];
   else if (fmod(index, 12) == 8)
     node = scene->mesh_node_wall[1];
   else if (fmod(index, 12) == 9)
     node = scene->mesh_node_column[1];
   else if (fmod(index, 12) == 10)
     node = scene->mesh_node_column_c;
   else if (fmod(index, 12) == 11)
     node = scene->mesh_node_snake;

   evas_canvas3d_node_bounding_sphere_get(node, &x0, &y0, &z0, &r);

   evas_canvas3d_node_scale_set(scene->mesh_node_ball, r, r, r);
   evas_canvas3d_node_position_set(scene->mesh_node_ball, x0, y0, z0);

   evas_canvas3d_node_bounding_box_get(node, &x0, &y0, &z0, &x1, &y1, &z1);

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
        evas_canvas3d_mesh_vertex_assembly_set(*mesh, EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES);
        evas_canvas3d_mesh_frame_vertex_data_copy_set(*mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, 6 * sizeof(float), &vertixes[ 0]);
        evas_canvas3d_mesh_index_data_copy_set(*mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, 48, &indixes[0]);

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

Eina_Bool _horizontal_circle_resolution(Evas_Canvas3D_Node *node, Evas_Real x, Evas_Real z, Evas_Real r)
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   evas_canvas3d_node_bounding_box_get(node, &x0, &y0, &z0, &x1, &y1, &z1);

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

Eina_Bool _horizontal_circle_object_resolution(Evas_Canvas3D_Node *node, Evas_Canvas3D_Node *node2, Evas_Real r)
{
   Evas_Real x0, y0, z0, x1, y1, z1, x, y, z;

   evas_canvas3d_node_position_get(node2, EVAS_CANVAS3D_SPACE_WORLD, &x, &y, &z);
   evas_canvas3d_node_bounding_box_get(node, &x0, &y0, &z0, &x1, &y1, &z1);

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

Eina_Bool _horizontal_circle_position_resolution(Evas_Canvas3D_Node *node, Evas_Real x, Evas_Real z, Evas_Real r)
{
   Evas_Real x0, y0, z0;

   evas_canvas3d_node_position_get(node, EVAS_CANVAS3D_SPACE_PARENT, &x0, &y0, &z0);

   if (((x0 - x) * (x0 - x)) + ((z0 - z) * (z0 - z)) < r * r) return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Bool _horizontal_position_resolution(Evas_Canvas3D_Node *node, Evas_Real x, Evas_Real z)
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   evas_canvas3d_node_bounding_box_get(node, &x0, &y0, &z0, &x1, &y1, &z1);

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

/* Is actual for this example only */
Eina_Bool _horizontal_object_resolution(Evas_Canvas3D_Node *node, Evas_Real x1, Evas_Real z1, Evas_Real x2,
                                        Evas_Real z2, Evas_Real x_move, Evas_Real z_move)
{
   Evas_Real x3, y3, z3, x4, y4, z4;

   evas_canvas3d_node_bounding_box_get(node, &x3, &y3, &z3, &x4, &y4, &z4);

   if ((x4 + x_move < x1) || (x4 + x_move > x2) ||
       (z4 + z_move < z1) || (z4 + z_move > z2))
     return EINA_FALSE;


   return EINA_TRUE;
}

Eina_Bool _horizontal_frame_resolution(Evas_Canvas3D_Node *node, Evas_Real x1, Evas_Real z1, Evas_Real x2,
                                        Evas_Real z2, Evas_Real *x_move, Evas_Real *z_move)
{
   Evas_Real x3, y3, z3, x4, y4, z4;

   *x_move = 0;
   *z_move = 0;

   evas_canvas3d_node_bounding_box_get(node, &x3, &y3, &z3, &x4, &y4, &z4);

   if (x3 < x1)
     *x_move = x1 - x3;
   else if (x4 > x2)
     *x_move = x2 - x4;
   if (z3 < z1)
     *z_move = z1 - z3;
   else if (z4 > z2)
     *z_move = z2 - z4;

   if ((*x_move != 0) || (*z_move != 0))
        return EINA_FALSE;

   return EINA_TRUE;
}

static void
_generate_square_grid(unsigned short *indices, int count)
{
   int i, j;
   unsigned short *index = &indices[0];
   int vccount = count + 1;

   for(j = 0; j < count; j++)
     for(i = 0; i < count; i++)
       {
          *index++ = (unsigned short)(i + vccount * j);
          *index++ = i + vccount * (j + 1);
          *index++ = i + 1 + vccount * (j + 1);

          *index++ =  i + vccount * j;
          *index++ =  i + 1 +  vccount * j;
          *index++ =  i + vccount * (j + 1) + 1;
       }
}

void
_set_ball(Eo *mesh, int p)
{
   int vcount, icount, vccount, i, j;
   double dtheta, dfi, sinth, costh, fi, theta, sinfi, cosfi;
   unsigned short *indices;

   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   dtheta = M_PI / p;
   dfi = 2 * M_PI / p;

   vec3 *vertices = malloc(sizeof(vec3) * vcount);
   vec3 *normals = malloc(sizeof(vec3) * vcount);

   for (j = 0; j < vccount; j++)
     {
        theta = j * dtheta;
        sinth = sin(theta);
        costh = cos(theta);
        for (i = 0; i < vccount; i++)
          {
             fi = i * dfi;
             sinfi = sin(fi);
             cosfi = cos(fi);
             vertices[i + j * vccount].x = sinth * cosfi;
             vertices[i + j * vccount].y = sinth * sinfi;
             vertices[i + j * vccount].z = costh;

             normals[i + j * vccount].x = vertices[i + j * vccount].x;
             normals[i + j * vccount].y = vertices[i + j * vccount].y;
             normals[i + j * vccount].z = vertices[i + j * vccount].z;
          }
     }

   indices = malloc(sizeof(short) * icount);

   _generate_square_grid(indices, p);

   evas_canvas3d_mesh_vertex_count_set(mesh, vcount);
   evas_canvas3d_mesh_frame_add(mesh, 0);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, sizeof(vec3), &vertices[0]);
   evas_canvas3d_mesh_frame_vertex_data_copy_set(mesh, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, sizeof(vec3), &normals[0]);
   evas_canvas3d_mesh_index_data_copy_set(mesh, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, icount, &indices[0]);

   free(vertices);
   free(normals);
   free(indices);
}

