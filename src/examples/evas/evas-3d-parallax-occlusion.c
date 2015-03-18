/**
 * Example illustrating usage of parallax occlusion shading and texture animation.
 *
 * Press "n" for use normal mapping shading. Pres "p" for use parallax occlusion mapping shading.
 *
 * @verbatim
 * gcc -o evas-3d-parallax-occlusion evas-3d-parallax-occlusion.c evas-3d-primitives.c `pkg-config --libs --cflags evas ecore ecore-evas eo`-lm
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
#include "evas-3d-primitives.h"
#include "evas-common.h"

#define  WIDTH          400
#define  HEIGHT         400

static const char *rock_diffuse = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "rocks.jpg";
static const char *rock_n_and_height_map = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "rocks_NM_height.tga";
static const char *wood_diffuse = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "wood.jpg";
static const char *wood_n_and_height_map = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "four_NM_height.tga";

typedef struct _Scene_Data
{
   Eo *scene;
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node;

   Eo *camera;
   Eo *light;
   Eo *mesh;
   Eo *material_rocks;
   Eo *material_wood;
   Eo *texture_rocks;
   Eo *texture_rocks_n;
   Eo *texture_wood;
   Eo *texture_four_n;
} Scene_Data;

static Ecore_Evas *ecore_evas = NULL;
static Evas *evas = NULL;
static Eo *background = NULL;
static Eo *image = NULL;

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Scene_Data *scene = (Scene_Data *)data;
   Evas_Event_Key_Down *ev = event_info;

   if (!strcmp("n", ev->key))
     eo_do(scene->mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP));

   if (!strcmp("p", ev->key))
     eo_do(scene->mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PARALLAX_OCCLUSION));
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

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += 0.5;

   eo_do(scene->mesh_node,
         evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0));

   eo_do(scene->mesh_node, evas_3d_node_mesh_frame_set(scene->mesh, 50 * (1 + sin(angle / 180.0 * M_PI))));
   /* Rotate */
   if (angle > 360.0) angle = 0.0f;

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);

   eo_do(data->camera,
         evas_3d_camera_projection_perspective_set(30.0, 1.0, 2.0, 50.0));

   data->camera_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                              evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(0.0, 0.0, 10.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->camera_node));
}

static void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(data->light,
         evas_3d_light_ambient_set(0.2, 0.2, 0.2, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0));

   data->light_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(-20.0, 20.0, 50.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}

static void
_mesh_setup(Scene_Data *data)
{
   /* Setup material. */
   data->material_rocks = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   data->texture_rocks = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   data->texture_rocks_n = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(data->texture_rocks, evas_3d_texture_file_set(rock_diffuse, NULL),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   eo_do(data->texture_rocks_n, evas_3d_texture_file_set(rock_n_and_height_map, NULL),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_rocks,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, data->texture_rocks_n),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture_rocks),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT, data->texture_rocks),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_SPECULAR, data->texture_rocks),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.1, 0.1, 0.1, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   data->material_wood = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   data->texture_wood = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   data->texture_four_n = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(data->texture_wood, evas_3d_texture_file_set(wood_diffuse, NULL),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   eo_do(data->texture_four_n, evas_3d_texture_file_set(wood_n_and_height_map, NULL),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_wood,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, data->texture_four_n),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, data->texture_wood),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT, data->texture_wood),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_SPECULAR, data->texture_wood),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.1, 0.1, 0.1, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   /* Setup mesh. */
   data->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_cube_frame(data->mesh, 0);

   evas_3d_add_cube_frame(data->mesh, 100);
   eo_do(data->mesh,
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PARALLAX_OCCLUSION),
         evas_3d_mesh_frame_material_set(0, data->material_rocks));

   eo_do(data->mesh,
         evas_3d_mesh_frame_material_set(100, data->material_wood));

   data->mesh_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                            evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node));
   eo_do(data->mesh_node, evas_3d_node_mesh_add(data->mesh),
         evas_3d_node_scale_set(3, 3, 3));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);
   eo_do(data->scene,
         evas_3d_scene_size_set(WIDTH, HEIGHT);
         evas_3d_scene_background_color_set(0.0, 0.0, 0.0, 0.0));

   data->root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                            evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   eo_do(data->scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node));
}

int
main(void)
{
   //Unless Evas 3D supports Software renderer, we set gl backened forcely.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   Scene_Data data;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

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
         evas_obj_visibility_set(EINA_TRUE),
         evas_object_focus_set(image, EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(data.scene));

   /* Add animation timer callback. */
   ecore_timer_add(0.01, _animate_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
