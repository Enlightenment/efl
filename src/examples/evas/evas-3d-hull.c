/**
 * Example to test convex hull models made in Evas-3D vs. Blender.
 *
 * The first model (from the right side) is the original model, the
 * second one is a convex hull made in Evas-3D, and the third one is a
 * convex hull made in Blender.
 *
 * Press "Right" or "Left" to switch models, the result of test (vertex count)
 * will be printed in console window.
 *
 * @verbatim
 * gcc -o evas-3d-hull evas-3d-hull.c -g `pkg-config --libs --cflags efl evas ecore ecore-evas eo eina` -lm
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

#define  WIDTH          1024
#define  HEIGHT         1024
#define  TESTS_COUNT    8
#define  SCALE_SPHERE   2.0
#define  SCALE_TORUS    2.7
#define  SCALE_HOME     0.7
#define  SCALE_SONIC    0.08
#define  SCALE_EAGLE    0.06

typedef struct _Scene_Data
{
   Eo *scene;
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node;
   Eo *mesh_node_convex_hull;
   Eo *mesh_node_blender;

   Eo *camera;
   Eo *light;
   Eo *mesh_sphere;
   Eo *mesh_torus;
   Eo *mesh_cube;
   Eo *mesh_plain;
   Eo *mesh_column;
   Eo *mesh_home;
   Eo *mesh_sonic;
   Eo *mesh_eagle;
   Eo *mesh_sphere_ch;
   Eo *mesh_torus_ch;
   Eo *mesh_cube_ch;
   Eo *mesh_plain_ch;
   Eo *mesh_column_ch;
   Eo *mesh_home_ch;
   Eo *mesh_sonic_ch;
   Eo *mesh_eagle_ch;
   Eo *mesh_blender_sphere;
   Eo *mesh_blender_torus;
   Eo *mesh_blender_cube;
   Eo *mesh_blender_plain;
   Eo *mesh_blender_column;
   Eo *mesh_blender_home;
   Eo *mesh_blender_sonic;
   Eo *mesh_blender_eagle;
   Eo *mesh_blender_test_sphere;
   Eo *mesh_blender_test_torus;
   Eo *mesh_blender_test_cube;
   Eo *mesh_blender_test_plain;
   Eo *mesh_blender_test_column;
   Eo *mesh_blender_test_home;
   Eo *mesh_blender_test_sonic;
   Eo *mesh_blender_test_eagle;
   Eo *material;
} Scene_Data;

int rr;

#define MODEL_MESH_INIT(name, model, shade)                                              \
   data->mesh_##name = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);                          \
   efl_file_simple_load(data->mesh_##name, model, NULL); \
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_##name, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES); \
   evas_canvas3d_mesh_shader_mode_set(data->mesh_##name, EVAS_CANVAS3D_SHADER_MODE_##shade); \
   evas_canvas3d_mesh_frame_material_set(data->mesh_##name, 0, data->material);

#define CONVEX_HULL_MESH_INIT(name)                                                              \
   vert = eina_inarray_new(sizeof(float), 1); \
   ind = eina_inarray_new(sizeof(unsigned short int), 1);\
   evas_canvas3d_mesh_convex_hull_data_get(data->mesh_##name, 0, vert, ind);  \
   vertex = (float*) vert->members;\
   index = ind->members;\
   data->mesh_##name##_ch = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);                                    \
   evas_canvas3d_mesh_vertex_count_set(data->mesh_##name##_ch, (vert->len / 10)); \
   evas_canvas3d_mesh_frame_add(data->mesh_##name##_ch, 0); \
   evas_canvas3d_mesh_frame_vertex_data_copy_set(data->mesh_##name##_ch, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_POSITION, \
                                            10 * sizeof(float), &vertex[ 0]); \
   evas_canvas3d_mesh_frame_vertex_data_copy_set(data->mesh_##name##_ch, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_NORMAL, \
                                            10 * sizeof(float), &vertex[ 3]); \
   evas_canvas3d_mesh_frame_vertex_data_copy_set(data->mesh_##name##_ch, 0, EVAS_CANVAS3D_VERTEX_ATTRIB_COLOR, \
                                            10 * sizeof(float), &vertex[ 6]); \
   evas_canvas3d_mesh_index_data_copy_set(data->mesh_##name##_ch, EVAS_CANVAS3D_INDEX_FORMAT_UNSIGNED_SHORT, \
                                     ind->len, &index[0]); \
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_##name##_ch, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES); \
   evas_canvas3d_mesh_shader_mode_set(data->mesh_##name##_ch, EVAS_CANVAS3D_SHADER_MODE_VERTEX_COLOR); \
   evas_canvas3d_mesh_frame_material_set(data->mesh_##name##_ch, 0, data->material);                                    \
         free(vert);                                                                             \
         free(ind);

#define SWITCH_MESH(index, name, scale)                                           \
   case index:                                                                    \
     {                                                                            \
        list = evas_canvas3d_node_mesh_list_get(scene->mesh_node);             \
        mesh = eina_list_nth(list, 0);                                            \
        evas_canvas3d_node_mesh_del(scene->mesh_node, mesh); \
        evas_canvas3d_node_mesh_add(scene->mesh_node, scene->mesh_##name); \
        evas_canvas3d_node_scale_set(scene->mesh_node, scale, scale, scale);                       \
        list = evas_canvas3d_node_mesh_list_get(scene->mesh_node_convex_hull); \
        mesh = eina_list_nth(list, 0);                                            \
        evas_canvas3d_node_mesh_del(scene->mesh_node_convex_hull, mesh); \
        evas_canvas3d_node_mesh_add(scene->mesh_node_convex_hull, scene->mesh_##name##_ch); \
        evas_canvas3d_node_scale_set(scene->mesh_node_convex_hull, scale, scale, scale);                       \
        list = evas_canvas3d_node_mesh_list_get(scene->mesh_node_blender);     \
        mesh = eina_list_nth(list, 0);                                            \
        evas_canvas3d_node_mesh_del(scene->mesh_node_blender, mesh); \
        evas_canvas3d_node_mesh_add(scene->mesh_node_blender, scene->mesh_blender_##name); \
        evas_canvas3d_node_scale_set(scene->mesh_node_blender, scale, scale, scale);                       \
        _print_result(scene->mesh_##name##_ch, scene->mesh_blender_test_##name);       \
        break;                                                                    \
     }

static const char *home = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sweet_home_without_tex_coords.obj";
static const char *sonic = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sonic.md2";
static const char *eagle = PACKAGE_EXAMPLES_DIR "/shooter/assets/models/eagle.md2";

static const char *column = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/column.ply";
static const char *plain = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/plain.ply";
static const char *plain_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/plain_blender_ch.ply";
static const char *sphere_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/sphere_blender_ch.ply";
static const char *torus_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/torus_blender_ch.ply";
static const char *cube_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/cube_blender_ch.ply";
static const char *column_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/column_blender_ch.ply";
static const char *home_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/home_blender_ch.obj";
static const char *sonic_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/sonic_blender_ch.ply";
static const char *eagle_ch = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/eagle_blender_ch.ply";
static const char *plain_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/plain_blender_ch_test.ply";
static const char *sphere_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/sphere_blender_ch_test.ply";
static const char *torus_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/torus_blender_ch_test.ply";
static const char *cube_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/cube_blender_ch_test.ply";
static const char *column_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/column_blender_ch_test.ply";
static const char *home_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/home_blender_ch_test.ply";
static const char *sonic_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/sonic_blender_ch_test.ply";
static const char *eagle_test = PACKAGE_EXAMPLES_DIR EVAS_CONVEX_HULL_FOLDER "/eagle_blender_ch_test.ply";

static Ecore_Evas *ecore_evas = NULL;
static Evas *evas = NULL;
static Eo *background = NULL;
static Eo *image = NULL;
static int next_model = 0;

static void
_print_result(Evas_Canvas3D_Mesh *mesh, Evas_Canvas3D_Mesh *convex_mesh)
{
   int v_count = 0;
   v_count = evas_canvas3d_mesh_vertex_count_get(mesh);

   printf("Vertex count is %d for convex hull\n",
          v_count);

   v_count = evas_canvas3d_mesh_vertex_count_get(convex_mesh);

   printf("Vertex count is %d for blender convex hull\n\n",
          v_count);

   return;
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
   if (angle >= 360.0)
     angle = 0.0;

   evas_canvas3d_node_orientation_angle_axis_set(scene->mesh_node, angle, 1.0, 1.0, 1.0);

   evas_canvas3d_node_orientation_angle_axis_set(scene->mesh_node_convex_hull, angle, 1.0, 1.0, 1.0);

   evas_canvas3d_node_orientation_angle_axis_set(scene->mesh_node_blender, angle, 1.0, 1.0, 1.0);

   return EINA_TRUE;
}

static void
_key_down(void *data,
         Evas *e EINA_UNUSED,
         Evas_Object *eo EINA_UNUSED,
         void *event_info)
{
   const Eina_List *list = NULL;
   Eo *mesh = NULL;
   Evas_Event_Key_Down *ev = event_info;
   Scene_Data *scene = (Scene_Data *)data;

   if (!strcmp(ev->key, "Right"))
     next_model++;
   else if (!strcmp(ev->key, "Left"))
     next_model--;

   if (next_model == -1)
     next_model = TESTS_COUNT - 1;
   else if (next_model == TESTS_COUNT)
     next_model = 0;

   switch (next_model)
     {
      SWITCH_MESH(0, sphere, SCALE_SPHERE)
      SWITCH_MESH(1, torus, SCALE_TORUS)
      SWITCH_MESH(2, cube, 1.0)
      SWITCH_MESH(3, plain, 1.0)
      SWITCH_MESH(4, column, 1.0)
      SWITCH_MESH(5, home, SCALE_HOME)
      SWITCH_MESH(6, sonic, SCALE_SONIC)
      SWITCH_MESH(7, eagle, SCALE_EAGLE)
      default:
        break;
     }
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);

   evas_canvas3d_camera_projection_perspective_set(data->camera, 60.0, 1.0, 2.0, 50.0);

   data->camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
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

   data->light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, 0.0, 0.0, 10.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

static void
_mesh_setup(Scene_Data *data)
{
   Eina_Inarray *vert, *ind;
   float *vertex;
   unsigned short int *index;
   Eo *primitive = NULL;
   /* Setup material. */

   data->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material, 100.0);

   /* Setup mesh sphere */
   primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE);
   evas_canvas3d_primitive_precision_set(primitive, 10);
   data->mesh_sphere = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(data->mesh_sphere, 0, primitive);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_sphere, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_sphere, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh_sphere, 0, data->material);

   /* Setup mesh torus */
   data->mesh_torus = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_primitive_form_set(primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_TORUS);
   evas_canvas3d_primitive_precision_set(primitive, 50);
   evas_canvas3d_mesh_from_primitive_set(data->mesh_torus, 0, primitive);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_torus, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_torus, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh_torus, 0, data->material);

   /* Setup mesh cube */
   data->mesh_cube = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_primitive_form_set(primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);
   evas_canvas3d_primitive_precision_set(primitive, 50);
   evas_canvas3d_mesh_from_primitive_set(data->mesh_cube, 0, primitive);
   evas_canvas3d_mesh_vertex_assembly_set(data->mesh_cube, EVAS_CANVAS3D_VERTEX_ASSEMBLY_TRIANGLES);
   evas_canvas3d_mesh_shader_mode_set(data->mesh_cube, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh_cube, 0, data->material);

   MODEL_MESH_INIT(plain, plain, PHONG)
   MODEL_MESH_INIT(column, column, PHONG)
   MODEL_MESH_INIT(home, home, PHONG)
   MODEL_MESH_INIT(sonic, sonic, PHONG)
   MODEL_MESH_INIT(eagle, eagle, PHONG)

   MODEL_MESH_INIT(blender_sphere, sphere_ch, PHONG)
   MODEL_MESH_INIT(blender_torus, torus_ch, PHONG)
   MODEL_MESH_INIT(blender_cube, cube_ch, PHONG)
   MODEL_MESH_INIT(blender_plain, plain_ch, PHONG)
   MODEL_MESH_INIT(blender_column, column_ch, PHONG)
   MODEL_MESH_INIT(blender_home, home_ch, PHONG)
   MODEL_MESH_INIT(blender_eagle, eagle_ch, PHONG)
   MODEL_MESH_INIT(blender_sonic, sonic_ch, PHONG)

   MODEL_MESH_INIT(blender_test_sphere, sphere_test, DIFFUSE)
   MODEL_MESH_INIT(blender_test_torus, torus_test, DIFFUSE)
   MODEL_MESH_INIT(blender_test_cube, cube_test, DIFFUSE)
   MODEL_MESH_INIT(blender_test_plain, plain_test, DIFFUSE)
   MODEL_MESH_INIT(blender_test_column, column_test, DIFFUSE)
   MODEL_MESH_INIT(blender_test_home, home_test, DIFFUSE)
   MODEL_MESH_INIT(blender_test_eagle, eagle_test, DIFFUSE)
   MODEL_MESH_INIT(blender_test_sonic, sonic_test, DIFFUSE)

   data->mesh_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node);
   evas_canvas3d_node_mesh_add(data->mesh_node, data->mesh_sphere);
   evas_canvas3d_node_scale_set(data->mesh_node, SCALE_SPHERE, SCALE_SPHERE, SCALE_SPHERE);
   evas_canvas3d_node_position_set(data->mesh_node, 3.0, 0.0, 0.0);

   CONVEX_HULL_MESH_INIT(sphere)
   CONVEX_HULL_MESH_INIT(torus)
   CONVEX_HULL_MESH_INIT(cube)
   CONVEX_HULL_MESH_INIT(plain)
   CONVEX_HULL_MESH_INIT(column)
   CONVEX_HULL_MESH_INIT(home)
   CONVEX_HULL_MESH_INIT(sonic)
   CONVEX_HULL_MESH_INIT(eagle)

   _print_result(data->mesh_sphere_ch, data->mesh_blender_test_sphere);

   data->mesh_node_convex_hull =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_convex_hull);
   evas_canvas3d_node_position_set(data->mesh_node_convex_hull, 0.0, 0.0, 0.0);
   evas_canvas3d_node_scale_set(data->mesh_node_convex_hull, SCALE_SPHERE, SCALE_SPHERE, SCALE_SPHERE);
   evas_canvas3d_node_mesh_add(data->mesh_node_convex_hull, data->mesh_sphere_ch);

   data->mesh_node_blender =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node_blender);
   evas_canvas3d_node_position_set(data->mesh_node_blender, -3.0, 0.0, 0.0);
   evas_canvas3d_node_scale_set(data->mesh_node_blender, SCALE_SPHERE, SCALE_SPHERE, SCALE_SPHERE);
   evas_canvas3d_node_mesh_add(data->mesh_node_blender, data->mesh_blender_sphere);
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);
   evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(data->scene, 0.0, 0.0, 0.0, 0.0);

   data->root_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
}

int
main(void)
{
   Ecore_Animator *anim;
   Scene_Data data;

   // Unless Evas 3D supports Software renderer, we force use of the gl backend.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   if (!ecore_evas_init()) return 0;

   printf("Loading 3d meshes. Press Left or Right to switch between models.\n");

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

   evas_object_show(image),
   evas_object_focus_set(image, EINA_TRUE);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, data.scene);

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _key_down, &data);

   /* Add animator. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();
   ecore_animator_del(anim);

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}


