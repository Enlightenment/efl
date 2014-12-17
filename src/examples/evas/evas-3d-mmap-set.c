/**
* Simple Evas example illustrating import from mmap.
*
* Open files to Eina_Files.
* Read meshes from Eina_Files.
* Show the results.
*
* @verbatim
* gcc -o evas-3d-mmap-set evas-3d-mmap-set.c `pkg-config --libs --cflags evas ecore ecore-evas eina eo` -lm
* @endverbatim
*/

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#include <math.h>

#define  WIDTH 1000
#define  HEIGHT 1000

#define LOAD_AND_ADD_MESH(extention, number)                                          \
   extention##_file = eina_file_open("mesh_mmap_set/mesh."#extention, 0);             \
   mesh_##extention = eo_add(EVAS_3D_MESH_CLASS, evas);                               \
   eo_do(mesh_##extention,                                                            \
         evas_3d_mesh_mmap_set(extention##_file, NULL),                               \
         evas_3d_mesh_frame_material_set(0, material),                                \
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));                      \
   node_##extention = eo_add(EVAS_3D_NODE_CLASS, evas,                                \
                          evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));          \
   eo_do(root_node, evas_3d_node_member_add(node_##extention));                       \
   eo_do(node_##extention, evas_3d_node_mesh_add(mesh_##extention),                   \
         evas_3d_node_position_set(initial_node_data[number * 10],                    \
                                   initial_node_data[number * 10 + 1],                \
                                   initial_node_data[number * 10 + 2]),               \
         evas_3d_node_scale_set(initial_node_data[number * 10 + 3],                   \
                                initial_node_data[number * 10 + 4],                   \
                                initial_node_data[number * 10 + 5]),                  \
         evas_3d_node_orientation_angle_axis_set(initial_node_data[number * 10 + 6],  \
                                                 initial_node_data[number * 10 + 7],  \
                                                 initial_node_data[number * 10 + 8],  \
                                                 initial_node_data[number * 10 + 9]));\
   ecore_timer_add(0.01, _animate_##extention, node_##extention);

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;

Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;
Eo *material = NULL;
Eo *light = NULL;

Eo *node_obj = NULL, *node_ply = NULL, *node_eet = NULL, *node_md2 = NULL;
Eo *mesh_obj = NULL, *mesh_ply = NULL, *mesh_eet = NULL, *mesh_md2 = NULL;

static float obj_animation_parameter = 1.0;
static float ply_animation_parameter = 180.0;
static float eet_animation_parameter = 0.0;
static float md2_animation_parameter = 0.0;

static float obj_animation_velocity = 1.0;
static float ply_animation_velocity = 1.0;
static float eet_animation_velocity = 1.0;
static float md2_animation_velocity = 32.0;

static const float initial_node_data[] =
 /*position              scale                 rotation*/
{
   /*obj*/
   3.0, 3.0, 0.0,      1.0, 1.0, 1.0,     0.0, 1.0, 0.0, 0.0,
   /*ply*/
   -3.0, 3.0, 0.0,     1.0, 1.0, 1.0,     0.0, 1.0, 0.0, 0.0,
   /*eet*/
   3.0, -3.0, 0.0,     1.0, 1.0, 1.0,     0.0, 1.0, 0.0, 0.0,
   /*md2*/
   -3.0, -3.0, 0.0,    0.001, 0.001, 0.001,     180.0, 0.0, 1.0, 0.0
};

static Eina_Bool
_animate_obj(void *data)
{
   obj_animation_parameter += obj_animation_velocity;
   float oap = obj_animation_parameter/200;

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_scale_set(oap, oap, pow(obj_animation_parameter, 2) / 2000),
         evas_3d_node_orientation_angle_axis_set(obj_animation_parameter, 0.0, 1.0, 0.0));

   if (obj_animation_parameter >= 360.0 || obj_animation_parameter <= 0.0)
     obj_animation_velocity *= -1.0;

   return EINA_TRUE;
}

static Eina_Bool
_animate_eet(void *data)
{
   eet_animation_parameter += eet_animation_velocity;

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_orientation_angle_axis_set(eet_animation_parameter, 1.0, 0.0, 0.0));
   eet_animation_velocity = sin(eet_animation_parameter / 180 * M_PI) + 1.1;

   return EINA_TRUE;
}

static Eina_Bool
_animate_ply(void *data)
{
   ply_animation_parameter += ply_animation_velocity;

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_orientation_angle_axis_set(ply_animation_parameter, 1.0, 0.0, 0.0));

   if (ply_animation_parameter > 360.0) ply_animation_parameter -= 180.0f;

   return EINA_TRUE;
}

static Eina_Bool
_animate_md2(void *data)
{
   md2_animation_parameter += md2_animation_velocity;

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_mesh_frame_set(mesh_md2, md2_animation_parameter));

   if (md2_animation_parameter > 256 * 100) md2_animation_parameter = 0;

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
   Eina_File *obj_file, *ply_file, *eet_file, *md2_file;

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
         evas_3d_camera_projection_perspective_set(10.0, 1.0, 1.0, 500.0));

   camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera));
   eo_do(root_node,
         evas_3d_node_member_add(camera_node));
   eo_do(camera_node,
         evas_3d_node_position_set(0.0, 0.0, 100.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
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
         evas_3d_node_position_set(1.0, 0.0, 100.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node,
         evas_3d_node_member_add(light_node));

   material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    1.0, 1.0, 0.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    0.5, 0.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));

   /* Add the meshes */
   LOAD_AND_ADD_MESH(obj, 0);
   LOAD_AND_ADD_MESH(eet, 1);
   LOAD_AND_ADD_MESH(ply, 2);
   LOAD_AND_ADD_MESH(md2, 3);

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));

   /* Add a background rectangle objects. */
   background = eo_add(EVAS_RECTANGLE_CLASS, evas);
   eo_do(background,
         evas_obj_color_set(20, 20, 200, 255),
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));

   /* Add an image object for 3D scene rendering. */
   image = evas_object_image_filled_add(evas);
   eo_do(image,
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(scene));

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   eina_file_close(obj_file);
   eina_file_close(ply_file);
   eina_file_close(eet_file);
   eina_file_close(md2_file);

   return 0;
}
