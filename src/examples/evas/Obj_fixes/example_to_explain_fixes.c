/**
* Temporary example to explain what changed in obj.
*
* @verbatim
* gcc -o example_to_explain_fixes example_to_explain_fixes.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo`
* @endverbatim
*/

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#define  WIDTH 1900
#define  HEIGHT 1080

#define ADD_OBJ_MESH(mesh_name, Y, Z)                                                      \
   mesh_name = eo_add(EVAS_3D_MESH_CLASS, evas);                                     \
   eo_do(mesh_name,                                                                  \
         efl_file_set("sweet_"#mesh_name".obj", NULL),                                  \
         evas_3d_mesh_frame_material_set(0, material),                          \
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));                \
   eo_do(mesh_name, efl_file_save("saved_"#mesh_name".obj", NULL, NULL));                    \
   mesh_name##_node = eo_add(EVAS_3D_NODE_CLASS, evas,                                 \
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH)); \
   eo_do(root_node,                                                             \
         evas_3d_node_member_add(mesh_name##_node));                                   \
   eo_do(mesh_name##_node,                                                             \
         evas_3d_node_mesh_add(mesh_name), \
         evas_3d_node_position_set(0, Y, Z));

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;

Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;
Eo *home_node = NULL;
Eo *home = NULL;
Eo *home_without_normals_node = NULL;
Eo *home_without_normals = NULL;;
Eo *home_without_tex_coords_node = NULL;
Eo *home_without_tex_coords = NULL;
Eo *home_only_vertex_coords_node = NULL;
Eo *home_only_vertex_coords = NULL;
Eo *material = NULL;
Eo *texture = NULL;
Eo *light = NULL;

static float angle = 0;

static Eina_Bool
_animate_scene(void *data)
{
   angle += 0.2;

   eo_do((Evas_3D_Node *)data, evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, -1.0));

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

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
   root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   /* Add the camera. */
   camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(camera,
         evas_3d_camera_projection_perspective_set(100.0, 1.0, 1.0, 500.0));

   camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera));
   eo_do(root_node,
         evas_3d_node_member_add(camera_node));
   eo_do(camera_node,
         evas_3d_node_position_set(10.0, 0.0, 0.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));
   /* Add the light. */
   light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(light,
         evas_3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_directional_set(EINA_TRUE));

   light_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(light_node,
         evas_3d_node_light_set(light),
         evas_3d_node_position_set(10.0, 0.0, 2.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node,
         evas_3d_node_member_add(light_node));

   texture = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   ADD_OBJ_MESH(home, -3, -3)
   ADD_OBJ_MESH(home_without_normals, -3, 3)
   ADD_OBJ_MESH(home_without_tex_coords, 3, -3)
   ADD_OBJ_MESH(home_only_vertex_coords, 3, 3)

   eo_do(texture,
         evas_3d_texture_file_set("sweet_home_temp.png", NULL),
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

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));

   /* Add a background rectangle objects. */
   background = eo_add(EVAS_RECTANGLE_CLASS, evas);
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

   ecore_timer_add(0.01, _animate_scene, home_node);
   ecore_timer_add(0.01, _animate_scene, home_without_normals_node);
   ecore_timer_add(0.01, _animate_scene, home_without_tex_coords_node);
   ecore_timer_add(0.01, _animate_scene, home_only_vertex_coords_node);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
