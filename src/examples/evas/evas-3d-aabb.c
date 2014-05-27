/*
 * This example shows how to get and draw axis-aligned bounding box.

 * @see _mesh_aabb(Evas_3D_Mesh **mesh_b, const Evas_3D_Node *node);
 * Rotate axises (keys 1-4) for model and bounding box view from another angle.
 * Compile with "gcc -o evas-3d-aabb evas-3d-aabb.c `pkg-config --libs --cflags evas ecore ecore-evas eo`"
 */

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#define  WIDTH 400
#define  HEIGHT 400

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;

Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;
Eo *mesh_node = NULL;
Eo *mesh_box_node = NULL;
Eo *mesh = NULL;
Eo *mesh_box = NULL;
Eo *material_box = NULL;
Eo *material = NULL;
Eo *texture = NULL;
Eo *light = NULL;


static Eina_Bool
_mesh_aabb(Evas_3D_Mesh **mesh_b, const Evas_3D_Node *node);

static Eina_Bool
_animate_scene(void *data)
{
   static int frame = 0;

   eo_do((Evas_3D_Node *)data, evas_3d_node_mesh_frame_set(mesh, frame));

   _mesh_aabb(&mesh_box, mesh_box_node);

   frame += 32;

   if (frame > 256 * 50) frame = 0;

   return EINA_TRUE;
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_3D_Node *node = (Evas_3D_Node *)data;

   switch(atoi(ev->key))
     {
      case 1:
        {
           eo_do(node, evas_3d_node_orientation_angle_axis_set(90, 1.0, 0.0, 0.0));
           break;
        }
      case 2:
        {
           eo_do(node, evas_3d_node_orientation_angle_axis_set(90, 0.0, 1.0, 0.0));
           break;
        }
      case 3:
        {
           eo_do(node, evas_3d_node_orientation_angle_axis_set(90, 0.0, 0.0, 1.0));
           break;
        }
      case 4:
        {
           eo_do(node, evas_3d_node_orientation_angle_axis_set(90, 1.0, 1.0, 0.0));
           break;
        }
     }
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   eo_do(background, evas_obj_size_set(w, h));
   eo_do(image, evas_obj_size_set(w, h));
}

static Eina_Bool
_mesh_aabb(Evas_3D_Mesh **mesh_b, const Evas_3D_Node *node)
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   eo_do(mesh_node, evas_3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));

   float vertices[] =
   {
       x0,  y0,  z1,
       x0,  y1,  z1,
       x1,  y1,  z1,
       x1,  y0,  z1,

       x0,  y0,  z0,
       x1,  y0,  z0,
       x0,  y1,  z0,
       x1,  y1,  z0,

       x0,  y0,  z0,
       x0,  y1,  z0,
       x0,  y0,  z1,
       x0,  y1,  z1,

       x1,  y0,  z0,
       x1,  y1,  z0,
       x1,  y1,  z1,
       x1,  y0,  z1,

       x0,  y1,  z0,
       x1,  y1,  z0,
       x0,  y1,  z1,
       x1,  y1,  z1,

       x0,  y0,  z0,
       x1,  y0,  z0,
       x1,  y0,  z1,
       x0,  y0,  z1
   };

   unsigned short indices[] =
   {
      0,  1,  2,  3,  1,  2,  0,  3,
      4,  5,  5,  7,  7,  6,  6,  4,
      8,  9,  9,  11, 11, 10, 10, 8,
      12, 13, 13, 14, 14, 15, 15, 12,
      16, 17, 17, 19, 19, 18, 18, 16,
      20, 21, 21, 22, 22, 23, 23, 20
   };

   float *cube_vertices = (float *) malloc(1 * sizeof(vertices));
   unsigned short *cube_indices = (unsigned short *) malloc(1 * sizeof(indices));
   memcpy(cube_vertices, vertices, sizeof(vertices));
   memcpy(cube_indices, indices, sizeof(indices));

   eo_do(*mesh_b,
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add( 0),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION, 3 * sizeof(float), &cube_vertices[ 0]),
         evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, 48, &cube_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
         evas_3d_mesh_frame_material_set(0, material_box));

   free(cube_vertices);
   free(cube_indices);

   return EINA_TRUE;
}

int
main(void)
{
   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   root_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(camera,
         evas_3d_camera_projection_perspective_set(60.0, 1.0, 1.0, 500.0));

   camera_node =
      eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera));
   eo_do(root_node,
         evas_3d_node_member_add(camera_node));
   eo_do(camera_node,
         evas_3d_node_position_set(100.0, 50.0, 20.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));
   light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(light,
         evas_3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_directional_set(EINA_TRUE));

   light_node =
      eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(light_node,
         evas_3d_node_light_set(light),
         evas_3d_node_position_set(1000.0, 0.0, 1000.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node,
         evas_3d_node_member_add(light_node));

   mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(mesh,
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, "sonic.md2", NULL),
         evas_3d_mesh_frame_material_set(0, material),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));

   texture = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture,
         evas_3d_texture_file_set("sonic.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   eo_do(material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   mesh_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   mesh_box_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                                 evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   material_box = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(material_box, evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE));

    mesh_box = eo_add(EVAS_3D_MESH_CLASS, evas);
   _mesh_aabb(&mesh_box, mesh_box_node);

   eo_do(root_node,
         evas_3d_node_member_add(mesh_box_node));
   eo_do(mesh_box_node,
         evas_3d_node_mesh_add(mesh_box));

   eo_do(root_node,
         evas_3d_node_member_add(mesh_node));
   eo_do(mesh_node,
         evas_3d_node_mesh_add(mesh));

   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));

   background = eo_add(EVAS_OBJ_RECTANGLE_CLASS, evas);
   eo_unref(background);
   eo_do(background,
         evas_obj_color_set(0, 0, 0, 255),
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));

   image = evas_object_image_filled_add(evas);
   eo_do(image,
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));
   evas_object_focus_set(image, EINA_TRUE);
   eo_do(image, evas_obj_image_scene_set(scene));

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, root_node);

   ecore_timer_add(0.01, _animate_scene, mesh_node);

   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
