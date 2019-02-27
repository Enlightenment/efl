/**
 * Example of .eet file format import/export in Evas-3D.
 *
 * Take mesh from md2.
 * Set material to it.
 * Show it in left side.
 * Save the first frame of this mesh to eet. (static only)
 * Take result to another mesh from eet.
 * Show it in right side.
 *
 * @verbatim
 * gcc -o evas-3d-eet evas-3d-eet.c `pkg-config --libs --cflags efl eina evas ecore ecore-evas ecore-file eo`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include "evas-common.h"

#define  WIDTH 1024
#define  HEIGHT 1024

static const char *input_model_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sonic.md2";
static const char *output_model_path = PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES "/saved_Sonic_EET.eet";

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

   evas_canvas3d_node_orientation_angle_axis_set((Evas_Canvas3D_Node *)data, angle, 1.0, 1.0, 1.0);

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static Eina_Bool
_animate_md2(void *data)
{
   static int frame = 0;

   evas_canvas3d_node_mesh_frame_set((Evas_Canvas3D_Node *)data, mesh, frame);

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
   efl_gfx_entity_size_set(background, EINA_SIZE2D(w,  h));
   efl_gfx_entity_size_set(image, EINA_SIZE2D(w,  h));
}

int
main(void)
{
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
   evas_canvas3d_camera_projection_perspective_set(camera, 60.0, 1.0, 1.0, 500.0);

   camera_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(camera_node, camera);
   evas_canvas3d_node_member_add(root_node, camera_node);
   evas_canvas3d_node_position_set(camera_node, 200.0, 0.0, 40.0);
   evas_canvas3d_node_look_at_set(camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 20.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0);
   /* Add the light. */
   light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_diffuse_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_directional_set(light, EINA_TRUE);

   light_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(light_node, light);
   evas_canvas3d_node_position_set(light_node, 1000.0, 0.0, 1000.0);
   evas_canvas3d_node_look_at_set(light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(root_node, light_node);

   /* Add the meshes. */
   mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   mesh2 = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   efl_file_simple_load(mesh, input_model_path, NULL);
   evas_canvas3d_mesh_frame_material_set(mesh, 0, material);
   evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);

   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.50, 0.50, 0.00, 0.30);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.00, 0.50, 0.50, 0.30);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 0.50, 0.00, 0.50, 0.30);
   evas_canvas3d_material_shininess_set(material, 50.0);

   if (!ecore_file_mkpath(PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES))
     fprintf(stderr, "Failed to create folder %s\n\n",
             PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES);

   efl_file_save(mesh, output_model_path, NULL, NULL);

   efl_file_simple_load(mesh2, output_model_path, NULL);
   evas_canvas3d_mesh_shader_mode_set(mesh2, EVAS_CANVAS3D_SHADER_MODE_PHONG);

   mesh_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   evas_canvas3d_node_member_add(root_node, mesh_node);
   evas_canvas3d_node_mesh_add(mesh_node, mesh);
   evas_canvas3d_node_position_set(mesh_node, 0.0, -40.0, 0.0);

   mesh_node2 = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(root_node, mesh_node2);
   evas_canvas3d_node_mesh_add(mesh_node2, mesh2);
   evas_canvas3d_node_position_set(mesh_node2, 0.0, 40.0, 0.0);

   /* Set up scene. */
   evas_canvas3d_scene_root_node_set(scene, root_node);
   evas_canvas3d_scene_camera_node_set(scene, camera_node);
   evas_canvas3d_scene_size_set(scene, WIDTH, HEIGHT);

   /* Add a background rectangle object. */
   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 0, 0, 0, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, scene);

   ecore_timer_add(0.01, _animate_md2, mesh_node);
   ecore_timer_add(0.01, _animate_eet, mesh_node2);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
