/**
 * Example of mmap import in Evas-3D.
 *
 * Open files to Eina_Files.
 * Read meshes from Eina_Files.
 * Show the results.
 *
 * @verbatim
 * gcc -o evas-3d-mmap-set evas-3d-mmap-set.c `pkg-config --libs --cflags efl evas ecore ecore-evas eina eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-common.h"

#include <math.h>

#define  WIDTH 1000
#define  HEIGHT 1000

#define LOAD_AND_ADD_MESH(extention, number)                                          \
   snprintf(buffer, PATH_MAX, "%s%s", template_path, #extention);                     \
   extention##_file = eina_file_open(buffer , 0);                                     \
   mesh_##extention = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);                               \
   efl_file_simple_mmap_load(mesh_##extention, extention##_file, NULL); \
   evas_canvas3d_mesh_frame_material_set(mesh_##extention, 0, material); \
   evas_canvas3d_mesh_shader_mode_set(mesh_##extention, EVAS_CANVAS3D_SHADER_MODE_PHONG);                      \
   node_##extention = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));          \
   evas_canvas3d_node_member_add(root_node, node_##extention);                       \
   evas_canvas3d_node_mesh_add(node_##extention, mesh_##extention); \
   evas_canvas3d_node_position_set(node_##extention, initial_node_data[number * 10], \
                                   initial_node_data[number * 10 + 1], \
                                   initial_node_data[number * 10 + 2]); \
   evas_canvas3d_node_scale_set(node_##extention, initial_node_data[number * 10 + 3], \
                                initial_node_data[number * 10 + 4], \
                                initial_node_data[number * 10 + 5]); \
   evas_canvas3d_node_orientation_angle_axis_set(node_##extention, initial_node_data[number * 10 + 6], \
                                                 initial_node_data[number * 10 + 7], \
                                                 initial_node_data[number * 10 + 8], \
                                                 initial_node_data[number * 10 + 9]);\
   ecore_timer_add(0.01, _animate_##extention, node_##extention);

static const char *template_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/mesh_for_mmap.";

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
static float md2_animation_velocity = 1.0;

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

   evas_canvas3d_node_scale_set((Evas_Canvas3D_Node *)data, oap, oap, oap);
   evas_canvas3d_node_orientation_angle_axis_set((Evas_Canvas3D_Node *)data, obj_animation_parameter, 0.0, 1.0, 0.0);

   if (obj_animation_parameter >= 360.0 || obj_animation_parameter <= 0.0)
     obj_animation_velocity *= -1.0;

   return EINA_TRUE;
}

static Eina_Bool
_animate_eet(void *data)
{
   eet_animation_parameter += eet_animation_velocity;

   evas_canvas3d_node_orientation_angle_axis_set((Evas_Canvas3D_Node *)data, eet_animation_parameter, 1.0, 0.0, 0.0);
   eet_animation_velocity = sin(eet_animation_parameter / 180 * M_PI) + 1.1;

   return EINA_TRUE;
}

static Eina_Bool
_animate_ply(void *data)
{
   ply_animation_parameter += ply_animation_velocity;

   evas_canvas3d_node_orientation_angle_axis_set((Evas_Canvas3D_Node *)data, ply_animation_parameter, 1.0, 0.0, 0.0);

   if (ply_animation_parameter > 360.0) ply_animation_parameter -= 180.0f;

   return EINA_TRUE;
}

static Eina_Bool
_animate_md2(void *data)
{
   md2_animation_parameter += md2_animation_velocity;

   evas_canvas3d_node_mesh_frame_set((Evas_Canvas3D_Node *)data, mesh_md2, md2_animation_parameter);

   if (md2_animation_parameter >= 360.0 || md2_animation_parameter <= 0.0)
     md2_animation_velocity *= -1.0;

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
   efl_gfx_entity_size_set(background, EINA_SIZE2D(w,  h));
   efl_gfx_entity_size_set(image, EINA_SIZE2D(w,  h));
}

int
main(void)
{
   char buffer[PATH_MAX];
   Eina_File *obj_file, *ply_file, *eet_file, *md2_file;

   // Unless Evas 3D supports Software renderer, we force use of the gl backend.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   /* Add a scene object .*/
   scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   /* Add the root node for the scene. */
   root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   /* Add the camera. */
   camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(camera, 10.0, 1.0, 1.0, 500.0);

   camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(camera_node, camera);
   evas_canvas3d_node_member_add(root_node, camera_node);
   evas_canvas3d_node_position_set(camera_node, 0.0, 0.0, 100.0);
   evas_canvas3d_node_look_at_set(camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   /* Add the light. */
   light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_diffuse_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_directional_set(light, EINA_TRUE);

   light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(light_node, light);
   evas_canvas3d_node_position_set(light_node, 1.0, 0.0, 100.0);
   evas_canvas3d_node_look_at_set(light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(root_node, light_node);

   material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.01, 0.01, 0.01, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 0.0, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 0.5, 0.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(material, 50.0);

   /* Add the meshes */
   LOAD_AND_ADD_MESH(obj, 0);
   LOAD_AND_ADD_MESH(eet, 1);
   LOAD_AND_ADD_MESH(ply, 2);
   LOAD_AND_ADD_MESH(md2, 3);

   /* Set up scene. */
   evas_canvas3d_scene_root_node_set(scene, root_node);
   evas_canvas3d_scene_camera_node_set(scene, camera_node);
   evas_canvas3d_scene_size_set(scene, WIDTH, HEIGHT);

   /* Add a background rectangle object. */
   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 20, 20, 200, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, scene);

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
