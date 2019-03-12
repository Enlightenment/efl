/**
 * Example of parallax occlusion shading and texture animation in Evas-3D.
 *
 * Press "n" for use normal mapping shading. Pres "p" for use parallax occlusion mapping shading.
 *
 * @verbatim
 * gcc -o evas-3d-parallax-occlusion evas-3d-parallax-occlusion.c `pkg-config --libs --cflags evas ecore ecore-evas eo efl` -lm
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

#define  WIDTH          400
#define  HEIGHT         400

static const char *rock_diffuse = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/rocks.jpg";
static const char *rock_n_and_height_map = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/rocks_NM_height.tga";
static const char *wood_diffuse = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/wood.jpg";
static const char *wood_n_and_height_map = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/four_NM_height.tga";

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
   Eo *cube;
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
     evas_canvas3d_mesh_shader_mode_set(scene->mesh, EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP);

   if (!strcmp("p", ev->key))
     evas_canvas3d_mesh_shader_mode_set(scene->mesh, EVAS_CANVAS3D_SHADER_MODE_PARALLAX_OCCLUSION);
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

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += 0.5;

   evas_canvas3d_node_orientation_angle_axis_set(scene->mesh_node, angle, 0.0, 1.0, 0.0);

   evas_canvas3d_node_mesh_frame_set(scene->mesh_node, scene->mesh, 50 * (1 + sin(angle / 180.0 * M_PI)));
   /* Rotate */
   if (angle > 360.0) angle = 0.0f;

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);

   evas_canvas3d_camera_projection_perspective_set(data->camera, 30.0, 1.0, 2.0, 50.0);

   data->camera_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 0.0, 0.0, 10.0);
   evas_canvas3d_node_look_at_set(data->camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(data->root_node, data->camera_node);
}

static void
_light_setup(Scene_Data *data)
{
   data->light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(data->light, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_light_diffuse_set(data->light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(data->light, 1.0, 1.0, 1.0, 1.0);

   data->light_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, -20.0, 20.0, 50.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

static void
_mesh_setup(Scene_Data *data)
{
   /* Setup material. */
   data->material_rocks = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   data->texture_rocks = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   data->texture_rocks_n = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(data->texture_rocks, rock_diffuse, NULL);
   evas_canvas3d_texture_wrap_set(data->texture_rocks, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   efl_file_simple_load(data->texture_rocks_n, rock_n_and_height_map, NULL);
   evas_canvas3d_texture_wrap_set(data->texture_rocks_n, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);

   evas_canvas3d_material_texture_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, data->texture_rocks_n);
   evas_canvas3d_material_texture_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, data->texture_rocks);
   evas_canvas3d_material_texture_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, data->texture_rocks);
   evas_canvas3d_material_texture_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, data->texture_rocks);
   evas_canvas3d_material_enable_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.1, 0.1, 0.1, 1.0);
   evas_canvas3d_material_color_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(data->material_rocks, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material_rocks, 100.0);

   data->material_wood = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   data->texture_wood = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   data->texture_four_n = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(data->texture_wood, wood_diffuse, NULL);
   evas_canvas3d_texture_wrap_set(data->texture_wood, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   efl_file_simple_load(data->texture_four_n, wood_n_and_height_map, NULL);
   evas_canvas3d_texture_wrap_set(data->texture_four_n, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);

   evas_canvas3d_material_texture_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, data->texture_four_n);
   evas_canvas3d_material_texture_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, data->texture_wood);
   evas_canvas3d_material_texture_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, data->texture_wood);
   evas_canvas3d_material_texture_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, data->texture_wood);
   evas_canvas3d_material_enable_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.1, 0.1, 0.1, 1.0);
   evas_canvas3d_material_color_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(data->material_wood, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material_wood, 100.0);

   /* Set data of primitive */
   data->cube = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->cube, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);

   /* Setup mesh. */
   data->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(data->mesh, 0, data->cube);
   evas_canvas3d_mesh_from_primitive_set(data->mesh, 100, data->cube);
   evas_canvas3d_mesh_shader_mode_set(data->mesh, EVAS_CANVAS3D_SHADER_MODE_PARALLAX_OCCLUSION);
   evas_canvas3d_mesh_frame_material_set(data->mesh, 0, data->material_rocks);

   evas_canvas3d_mesh_frame_material_set(data->mesh, 100, data->material_wood);

   data->mesh_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node);
   evas_canvas3d_node_mesh_add(data->mesh_node, data->mesh);
   evas_canvas3d_node_scale_set(data->mesh_node, 3, 3, 3);
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);
   evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(data->scene, 0.0, 0.0, 0.0, 0.0);

   data->root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
}

int
main(void)
{
   // Unless Evas 3D supports Software renderer, we force use of the gl backend.
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

   /* Add a background rectangle object. */
   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 0, 0, 0, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);
   evas_object_focus_set(image, EINA_TRUE);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, data.scene);

   /* Add animation timer callback. */
   ecore_timer_add(0.01, _animate_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
