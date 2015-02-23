/**
 * Simple Evas example illustrating import/export of .obj format.
 * Example demonstrate possibility to load and save mesh without tex_coords or/and normals.
 *
 * Read mesh from "sweet_home(parameters).obj".
 * After that cheange some properties of material.
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
#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
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
   mesh[num] = eo_add(EVAS_3D_MESH_CLASS, evas);                                  \
   snprintf(full_file_path, PATH_MAX, "%s%s", path, ".obj");                      \
   eo_do(mesh[num],                                                               \
         efl_file_set(full_file_path, NULL),                                      \
         evas_3d_mesh_frame_material_set(0, name_of_material),                    \
         evas_3d_mesh_shade_mode_set(shade_mode));                                \
   mesh_node[num] = eo_add(EVAS_3D_NODE_CLASS, evas,                              \
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));   \
   eo_do(root_node,                                                               \
         evas_3d_node_member_add(mesh_node[num]));                                \
   eo_do(mesh_node[num],                                                          \
         evas_3d_node_mesh_add(mesh[num]),                                        \
         evas_3d_node_position_set(0, Y, Z));                                     \

#define ADD_OBJ_MESH_AND_SAVED_COPY(path, Y, Z, num, shade_mode, name_of_material)\
   snprintf(buffer, PATH_MAX, "%s%s", input_template, #path);                     \
   ADD_OBJ_MESH(buffer, Y, Z, num, shade_mode, name_of_material)                  \
   snprintf(buffer, PATH_MAX, "%s%s%s", output_template, #path, ".obj");          \
   eo_do(mesh[num], efl_file_save(buffer, NULL, NULL));                           \
   snprintf(buffer, PATH_MAX, "%s%s", output_template, #path);                    \
   ADD_OBJ_MESH(buffer, Y + COPY_OFFSET, Z, num + 4, shade_mode, name_of_material)

#define ADD_TEXTURE(name, path)                                       \
   name = eo_add(EVAS_3D_TEXTURE_CLASS, evas);                        \
   eo_do(name,                                                        \
         evas_3d_texture_file_set(path, NULL),                        \
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,   \
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),  \
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,           \
                                  EVAS_3D_WRAP_MODE_REPEAT));

#define ADD_MATERIAL(name)                                                 \
   name = eo_add(EVAS_3D_MATERIAL_CLASS, evas);                            \
   eo_do(name,                                                             \
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE), \
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE), \
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),\
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),  \
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,              \
                                    COL_RED, 0.5),                         \
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,              \
                                    COL_GREEN, 0.5),                       \
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,             \
                                    COL_BLUE, 0.5),                        \
         evas_3d_material_shininess_set(100.0));

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

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, -1.0));

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
   int i;
   char buffer[PATH_MAX], full_file_path[PATH_MAX];
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
         evas_3d_camera_projection_perspective_set(20.0, 1.0, 1.0, 500.0));

   camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera));
   eo_do(root_node,
         evas_3d_node_member_add(camera_node));
   eo_do(camera_node,
         evas_3d_node_position_set(100.0, 0.0, 0.0),
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
         evas_3d_node_position_set(10.0, 0.0, 0.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node,
         evas_3d_node_member_add(light_node));

   ADD_TEXTURE(texture, texture_path)

   ADD_MATERIAL(material)

   ADD_MATERIAL(material_with_tex)
   eo_do(material_with_tex,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture));

   if (!ecore_file_mkpath(PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES))
     fprintf(stderr, "Failed to create folder %s\n\n",
             PACKAGE_EXAMPLES_DIR EVAS_SAVED_FILES);

   ADD_OBJ_MESH_AND_SAVED_COPY(home, -GRID_SIZE, -GRID_SIZE, 0,
                               EVAS_3D_SHADE_MODE_PHONG, material_with_tex)
   ADD_OBJ_MESH_AND_SAVED_COPY(home_without_normals, -GRID_SIZE, GRID_SIZE, 1,
                               EVAS_3D_SHADE_MODE_DIFFUSE, material_with_tex)
   ADD_OBJ_MESH_AND_SAVED_COPY(home_without_tex_coords, GRID_SIZE, -GRID_SIZE, 2,
                               EVAS_3D_SHADE_MODE_PHONG, material)
   ADD_OBJ_MESH_AND_SAVED_COPY(home_only_vertex_coords, GRID_SIZE, GRID_SIZE, 3,
                               EVAS_3D_SHADE_MODE_SHADOW_MAP_RENDER, NULL)

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));

   /* Add a background rectangle MESHES. */
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

   for (i = 0; i < NUMBER_OF_MESHES; i++)
     ecore_timer_add(ANIM_SPEED, _animate_scene, mesh_node[i]);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
