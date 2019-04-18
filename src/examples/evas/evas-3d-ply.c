/**
 * Example of .ply file format import/export in Evas-3D.
 *
 * Read meshes from "tested_man_all_with_mods.ply", "tested_man_only_geometry.ply" and "tested_man_without_UVs.ply".
 * After that change some properties of material.
 * After that save material to "saved_man.mtl"
 * and geometry to "saved_man_all_with_mods.ply", "saved_man_only_geometry.ply" and "saved_man_without_UVs.ply".
 *
 * @verbatim
 * gcc -o evas-3d-ply evas-3d-ply.c `pkg-config --libs --cflags efl evas ecore ecore-evas ecore-file eo`
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
#include <Ecore_File.h>
#include "evas-common.h"

#define  WIDTH 1024
#define  HEIGHT 1024

#define NUMBER_OF_MESHES 32

static const char *image_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/star.jpg";
static const char *input_template = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/";
static const char *output_template = PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES "/";
static const char *file_name[8] = {"Normal_UVs_Colors.ply",
                                   "Normal_UVs_NoColors.ply",
                                   "Normal_NoUVs_Colors.ply",
                                   "Normal_NoUVs_NoColors.ply",
                                   "NoNormal_UVs_Colors.ply",
                                   "NoNormal_UVs_NoColors.ply",
                                   "NoNormal_NoUVs_Colors.ply",
                                   "NoNormal_NoUVs_NoColors.ply"};

int draw_mode[8] = {EVAS_CANVAS3D_SHADER_MODE_PHONG,
                    EVAS_CANVAS3D_SHADER_MODE_PHONG,
                    EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR,
                    EVAS_CANVAS3D_SHADER_MODE_SHADOW_MAP_RENDER,
                    EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR,
                    EVAS_CANVAS3D_SHADER_MODE_SHADOW_MAP_RENDER,
                    EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR,
                    EVAS_CANVAS3D_SHADER_MODE_SHADOW_MAP_RENDER};

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;

Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;

Eo *mesh_node[NUMBER_OF_MESHES];
Eo *mesh[NUMBER_OF_MESHES];

Eo *material = NULL;
Eo *texture = NULL;
Eo *light = NULL;
Ecore_Animator *anim = NULL;

static float angle = 0;

static Eina_Bool
_animate_scene(void *data)
{
   angle += 0.2;

   evas_canvas3d_node_orientation_angle_axis_set((Evas_Canvas3D_Node *)data, angle, 1.0, 1.0, 1.0);

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
   efl_gfx_entity_size_set(background, EINA_SIZE2D(w,  h));
   efl_gfx_entity_size_set(image, EINA_SIZE2D(w,  h));
}

int
main(void)
{
   char buffer[PATH_MAX];
   int i;

   for (i = 0; i < NUMBER_OF_MESHES; i++)
     {
        mesh_node[i] = NULL;
        mesh[i] = NULL;
     }

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
   evas_canvas3d_node_position_set(camera_node, 15.0, 0.0, 0.0);
   evas_canvas3d_node_look_at_set(camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0);
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

   material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(texture, image_path, NULL);
   evas_canvas3d_texture_filter_set(texture, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(texture, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   evas_canvas3d_material_texture_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, texture);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.01, 0.01, 0.01, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(material, 50.0);

   if (!ecore_file_mkpath(PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES))
     fprintf(stderr, "Failed to create folder %s\n\n",
             PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES);

   /* Add the meshes. */
   for (i = 0; i < NUMBER_OF_MESHES; i++)
     {
        mesh[i] = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);

        snprintf(buffer, PATH_MAX, "%s%s", input_template, file_name[i % 8]);
        efl_file_simple_load(mesh[i], buffer, NULL);
        evas_canvas3d_mesh_frame_material_set(mesh[i], 0, material);
        evas_canvas3d_mesh_shader_mode_set(mesh[i], draw_mode[(i % 8)]);

        snprintf(buffer, PATH_MAX, "%s%s", output_template, file_name[i % 8]);
        efl_file_save(mesh[i], buffer, NULL, NULL);

        if (i > 15)
          {
             efl_file_simple_load(mesh[i], buffer, NULL);
             evas_canvas3d_mesh_frame_material_set(mesh[i], 0, material);
             evas_canvas3d_mesh_shader_mode_set(mesh[i], draw_mode[(i % 8)]);
          }

        mesh_node[i] = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
        evas_canvas3d_node_member_add(root_node, mesh_node[i]);
        evas_canvas3d_node_mesh_add(mesh_node[i], mesh[i]);
        evas_canvas3d_node_position_set(mesh_node[i], 0, ((i % 4) * 4) + ((i / 16) * 1) - 6.5, (((i % 16) / 4) * 4) - 6);
     }

   /* Set up scene. */
   evas_canvas3d_scene_root_node_set(scene, root_node);
   evas_canvas3d_scene_camera_node_set(scene, camera_node);
   evas_canvas3d_scene_size_set(scene, WIDTH, HEIGHT);

   /* Add a background rectangle object. */
   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 100, 100, 100, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, scene);

   ecore_animator_frametime_set(0.03);
   for (i = 0; i < NUMBER_OF_MESHES; i++)
     anim = ecore_animator_add(_animate_scene, mesh_node[i]);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_animator_del(anim);
   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
