/**
 * Example illustrating usage of fog effect.
 *
 * @verbatim
 * gcc -o evas-3d-fog evas-3d-fog.c evas-3d-primitives.c `pkg-config --libs --cflags evas ecore ecore-evas eo`-lm
 * @endverbatim
 */

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <math.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-3d-primitives.h"

#define  WIDTH          1024
#define  HEIGHT         1024
#define  FOG_COLOR      0.5, 0.5, 0.5
#define  FOG_FACTOR     0.2

static double pi = 3.14159265359;

typedef struct _Scene_Data
{
   Eo     *scene;
   Eo     *root_node;
   Eo     *camera_node;
   Eo     *light_node;
   Eo     *mesh_node;
   Eo     *mesh_node1;
   Eo     *animation_node;

   Eo     *camera;
   Eo     *light;
   Eo     *mesh;
   Eo     *mesh1;
   Eo     *material;
} Scene_Data;

static const vec2 tex_scale = {1, 1};

Ecore_Evas       *ecore_evas  = NULL;
Evas             *evas        = NULL;
Eo               *background  = NULL;
Eo               *image       = NULL;

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

   evas_object_resize(background, w, h);
   eo_do(background, evas_obj_size_set(w, h));
   eo_do(image, evas_obj_size_set(w, h));
}

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += 0.5;

   eo_do(scene->animation_node,
         evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0));

   if (angle > 360.0) angle -= 360.0f;

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);

   eo_do(data->camera,
         evas_3d_camera_projection_perspective_set(60.0, 1.0, 2.0, 50.0));

  data->camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
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

   data->light_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(0.0, 0.0, 10.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}

static void
_set_ball(Eo *mesh, int p, Evas_3D_Material *material)
{
   evas_3d_add_sphere_frame(mesh, 0, p, tex_scale);

   eo_do(mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
               evas_3d_mesh_frame_material_set(0, material));
}


static void
_mesh_setup(Scene_Data *data)
{
   data->material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(data->material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.2, 0.2, 0.2, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 0.8, 0.8, 0.8, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   data->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   data->mesh1 = eo_add(EVAS_3D_MESH_CLASS, evas);

   _set_ball(data->mesh, 100, data->material);
   _set_ball(data->mesh1, 100, data->material);

   data->animation_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   data->mesh_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->animation_node, evas_3d_node_member_add(data->mesh_node));
   eo_do(data->root_node, evas_3d_node_member_add(data->animation_node));
   eo_do(data->mesh_node, evas_3d_node_mesh_add(data->mesh),
                          evas_3d_node_position_set(0.0, 0.0, 3.0));

   data->mesh_node1 =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node1));
   eo_do(data->mesh_node1, evas_3d_node_mesh_add(data->mesh1),
                           evas_3d_node_scale_set(3, 3, 3));

   eo_do(data->mesh, evas_3d_mesh_fog_enable_set(EINA_TRUE), evas_3d_mesh_fog_color_set(FOG_COLOR, FOG_FACTOR));
   eo_do(data->mesh1, evas_3d_mesh_fog_enable_set(EINA_TRUE), evas_3d_mesh_fog_color_set(FOG_COLOR, FOG_FACTOR));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   eo_do(data->scene,
         evas_3d_scene_size_set(WIDTH, HEIGHT);
         evas_3d_scene_background_color_set(FOG_COLOR, 1));

   data->root_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
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
   Ecore_Animator *anim;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 10, 10, WIDTH, HEIGHT, NULL);

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
         evas_obj_visibility_set(EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(data.scene));

   /* Add animation timer callback. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
   ecore_animator_del(anim);

   return 0;
}
