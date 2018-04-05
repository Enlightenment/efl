/**
 * Example of .obj file format import in Evas-3D.
 *
 * Example demonstrates possibility to load and save mesh without tex_coords or/and normals.
 *
 * Read mesh from "sweet_home(parameters).obj".
 * After that change some properties of material.
 * After that save material to "saved_files/saved_home(parameters).mtl"
 * and geometry to "saved_files/saved_home(parameters).obj".
 * If material was not set it will be not saved.
 *
 * @verbatim
 * gcc -o evas-3d-obj evas-3d-obj.c `pkg-config --libs --cflags efl evas ecore ecore-evas ecore-file eo`
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

#define  WIDTH 1900
#define  HEIGHT 1080

#define  COL_LITTLE 0.2
#define  COL_BIG 1.0

#define COL_RED COL_BIG, COL_LITTLE, COL_LITTLE
#define COL_GREEN COL_LITTLE, COL_BIG, COL_LITTLE
#define COL_BLUE COL_LITTLE, COL_LITTLE, COL_BIG

#define ANIM_SPEED 0.02
#define COPY_OFFSET 5
#define GRID_SIZE 6
#define NUMBER_OF_MESHES 8

#define ADD_OBJ_MESH(path, Y, Z, num, shade_mode, name_of_material)               \
   mesh[num] = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);                                  \
   snprintf(full_file_path, PATH_MAX, "%s%s", path, ".obj");                      \
   efl_file_set(mesh[num], full_file_path, NULL); \
   evas_canvas3d_mesh_frame_material_set(mesh[num], 0, name_of_material); \
   evas_canvas3d_mesh_shader_mode_set(mesh[num], shade_mode);                                \
   mesh_node[num] = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));   \
   evas_canvas3d_node_member_add(root_node, mesh_node[num]);                                \
   evas_canvas3d_node_mesh_add(mesh_node[num], mesh[num]); \
   evas_canvas3d_node_position_set(mesh_node[num], 0, Y, Z);                                     \

#define ADD_OBJ_MESH_AND_SAVED_COPY(path, Y, Z, num, shade_mode, name_of_material)\
   snprintf(buffer, PATH_MAX, "%s%s", input_template, #path);                     \
   ADD_OBJ_MESH(buffer, Y, Z, num, shade_mode, name_of_material)                  \
   snprintf(buffer, PATH_MAX, "%s%s%s", output_template, #path, ".obj");          \
   efl_file_save(mesh[num], buffer, NULL, NULL);                           \
   snprintf(buffer, PATH_MAX, "%s%s", output_template, #path);                    \
   ADD_OBJ_MESH(buffer, Y + COPY_OFFSET, Z, num + 4, shade_mode, name_of_material)

#define ADD_TEXTURE(name, path)                                       \
   name = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);                        \
   efl_file_set(name, path, NULL); \
   evas_canvas3d_texture_filter_set(name, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, \
                                    EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST); \
   evas_canvas3d_texture_wrap_set(name, EVAS_CANVAS3D_WRAP_MODE_REPEAT, \
                                  EVAS_CANVAS3D_WRAP_MODE_REPEAT);

#define ADD_MATERIAL(name)                                                 \
   name = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);                            \
   evas_canvas3d_material_enable_set(name, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE); \
   evas_canvas3d_material_enable_set(name, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE); \
   evas_canvas3d_material_enable_set(name, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE); \
   evas_canvas3d_material_enable_set(name, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE); \
   evas_canvas3d_material_color_set(name, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, \
                                    COL_RED, 0.5); \
   evas_canvas3d_material_color_set(name, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, \
                                    COL_GREEN, 0.5); \
   evas_canvas3d_material_color_set(name, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, \
                                    COL_BLUE, 0.5); \
   evas_canvas3d_material_shininess_set(name, 100.0);

static const char *texture_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/sweet_home_reversed.png";
static const char *output_template = PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES "/saved_";
static const char *input_template = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sweet_";

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;

Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;
Eo *mesh[NUMBER_OF_MESHES];
Eo *mesh_node[NUMBER_OF_MESHES];
Eo *material_with_tex = NULL;
Eo *material = NULL;
Eo *texture = NULL;
Eo *light = NULL;

static float angle = 0;

static Eina_Bool
_animate_scene(void *data)
{
   angle += 0.2;

   evas_canvas3d_node_orientation_angle_axis_set((Evas_Canvas3D_Node *)data, angle, 1.0, 1.0, -1.0);

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
   int i;
   char buffer[PATH_MAX], full_file_path[PATH_MAX];

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
   evas_canvas3d_camera_projection_perspective_set(camera, 20.0, 1.0, 1.0, 500.0);

   camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(camera_node, camera);
   evas_canvas3d_node_member_add(root_node, camera_node);
   evas_canvas3d_node_position_set(camera_node, 100.0, 0.0, 0.0);
   evas_canvas3d_node_look_at_set(camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0);
   /* Add the light. */
   light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_diffuse_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_directional_set(light, EINA_TRUE);

   light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(light_node, light);
   evas_canvas3d_node_position_set(light_node, 10.0, 0.0, 0.0);
   evas_canvas3d_node_look_at_set(light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(root_node, light_node);

   ADD_TEXTURE(texture, texture_path)

   ADD_MATERIAL(material)

   ADD_MATERIAL(material_with_tex)
   evas_canvas3d_material_texture_set(material_with_tex, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, texture);

   if (!ecore_file_mkpath(PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES))
     fprintf(stderr, "Failed to create folder %s\n\n",
             PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES);

   ADD_OBJ_MESH_AND_SAVED_COPY(home, -GRID_SIZE, -GRID_SIZE, 0,
                               EVAS_CANVAS3D_SHADER_MODE_PHONG, material_with_tex)
   ADD_OBJ_MESH_AND_SAVED_COPY(home_without_normals, -GRID_SIZE, GRID_SIZE, 1,
                               EVAS_CANVAS3D_SHADER_MODE_DIFFUSE, material_with_tex)
   ADD_OBJ_MESH_AND_SAVED_COPY(home_without_tex_coords, GRID_SIZE, -GRID_SIZE, 2,
                               EVAS_CANVAS3D_SHADER_MODE_PHONG, material)
   ADD_OBJ_MESH_AND_SAVED_COPY(home_only_vertex_coords, GRID_SIZE, GRID_SIZE, 3,
                               EVAS_CANVAS3D_SHADER_MODE_SHADOW_MAP_RENDER, NULL)

   /* Set up scene. */
   evas_canvas3d_scene_root_node_set(scene, root_node);
   evas_canvas3d_scene_camera_node_set(scene, camera_node);
   evas_canvas3d_scene_size_set(scene, WIDTH, HEIGHT);

   /* Add a background rectangle MESHES. */
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

   for (i = 0; i < NUMBER_OF_MESHES; i++)
     ecore_timer_add(ANIM_SPEED, _animate_scene, mesh_node[i]);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
