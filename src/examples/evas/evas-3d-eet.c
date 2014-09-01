/**
* Simple Evas example illustrating .eet import/export
*
* Take mesh from md2.
* Set material to it.
* Show it in left side.
* Save the first frame of this mesh to eet. (static only)
* Take result to another mesh from eet.
* Show it in right side.
*
* @verbatim
* gcc -o evas-3d-eet evas-3d-eet.c `pkg-config --libs --cflags evas ecore ecore-evas eo`
* @endverbatim
*/

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#define  WIDTH 1024
#define  HEIGHT 1024

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
Eo *mesh = NULL;
Eo *mesh_node2 = NULL;
Eo *mesh2 = NULL;
Eo *material = NULL;
Eo *light = NULL;

static float angle = 0;

static Eina_Bool
_animate_eet(void *data)
{
   angle += 0.5;

   eo_do((Evas_3D_Node *)data, evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 1.0));

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static Eina_Bool
_animate_md2(void *data)
{
   static int frame = 0;

   eo_do((Evas_3D_Node *)data, evas_3d_node_mesh_frame_set(mesh, frame));

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
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   eo_do(background, evas_obj_size_set(w, h));
   eo_do(image, evas_obj_size_set(w, h));
}

int
main(void)
{
   //Unless Evas 3D supports Software renderer, we set gl backened forcely.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   /* Add a scene object .*/
   scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   /* Add the root node for the scene. */
   root_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   /* Add the camera. */
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
         evas_3d_node_position_set(200.0, 0.0, 40.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));
   /* Add the light. */
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

   /* Add the meshes. */
   mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   mesh2 = eo_add(EVAS_3D_MESH_CLASS, evas);
   material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(mesh,
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, "sonic.md2", NULL),
         evas_3d_mesh_frame_material_set(0, material),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));

   eo_do(material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    0.50, 0.50, 0.00, 0.30),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    0.00, 0.50, 0.50, 0.30),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    0.50, 0.00, 0.50, 0.30),
         evas_3d_material_shininess_set(50.0));

   eo_do(mesh, evas_3d_mesh_save(EVAS_3D_MESH_FILE_TYPE_EET, "saved_Sonic_EET.eet", NULL));
   eo_do(mesh2,
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_EET, "saved_Sonic_EET.eet", NULL),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));

   mesh_node = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(root_node,
         evas_3d_node_member_add(mesh_node));
   eo_do(mesh_node,
         evas_3d_node_mesh_add(mesh),
         evas_3d_node_position_set(0.0, -40.0, 0.0));

   mesh_node2 = eo_add_custom(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(root_node,
         evas_3d_node_member_add(mesh_node2));
   eo_do(mesh_node2,
         evas_3d_node_mesh_add(mesh2),
         evas_3d_node_position_set(0.0, 40.0, 0.0));

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));

   /* Add a background rectangle objects. */
   background = eo_add(EVAS_RECTANGLE_CLASS, evas);
   eo_unref(background);
   eo_do(background,
         evas_obj_color_set(0, 0, 0, 255),
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   eo_do(image,
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(scene));

   ecore_timer_add(0.01, _animate_md2, mesh_node);
   ecore_timer_add(0.01, _animate_eet, mesh_node2);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
