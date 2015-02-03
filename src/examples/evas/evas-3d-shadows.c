/*
 * This example illustrating use of shadows in the scene.
 *
 * @see evas_3d_scene_shadows_enable_set(Eina_Bool _shadows_enabled)
 *
 * Compile with "gcc -o evas-3d-shadows evas-3d-shadows.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm"
 */

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <math.h>
#include "evas-3d-primitives.c"

#define  WIDTH 1024
#define  HEIGHT 1024

#define BG_COLOR 0.2, 0.2, 0.2
#define AMBIENT_LIGHT 0.2, 0.2, 0.2
#define DIFFUSE_LIGHT 1.0, 1.0, 1.0
#define SPECULAR_LIGHT 1.0, 1.0, 1.0

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;

typedef struct _Body_3D
{
   Eo     *material;
   Eo     *mesh;
   Eo     *node;
} Body_3D;

typedef struct _Scene_Data
{
   Eo     *scene;
   Eo     *root_node;
   Eo     *camera_node;
   Eo     *camera;
   Eo     *light_node;
   Eo     *light;

   Body_3D     sphere;
   Body_3D     cube;
   Body_3D     square;
   Body_3D     cylinder;
   Body_3D     model;
   Body_3D     cone;
} Scene_Data;

static Eina_Bool
_animate_scene(void *data)
{
   static int frame = 0;
   Body_3D *body = (Body_3D *)data;

   eo_do(body->node, evas_3d_node_mesh_frame_set(body->mesh, frame));

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
   eo_do(background, evas_obj_size_set(w, h));
   eo_do(image, evas_obj_size_set(w, h));
}

static void
_body_material_set(Body_3D *body, float r, float g, float b)
{
   body->material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(body->material,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, r, g, b, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, r, g, b, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(100.0));

   eo_do(body->mesh,
          evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG),
          evas_3d_mesh_frame_material_set(0, body->material));
}

static void
_sphere_setup(Body_3D *sphere)
{
   sphere->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   _set_ball(sphere->mesh, 50);
   _body_material_set(sphere, 1, 0.0, 0.0);

   sphere->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH),
                    evas_3d_node_position_set(2.0, 3.0, 1.0));
   eo_do(sphere->node, evas_3d_node_mesh_add(sphere->mesh));
}

static void
_cone_setup(Body_3D *cone)
{
   cone->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   _set_cone(cone->mesh, 100);
   _body_material_set(cone, 0.8, 0.5, 0.5);

   cone->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(cone->node, evas_3d_node_mesh_add(cone->mesh),
         evas_3d_node_position_set(-5.0, -1.0, -3.0),
         evas_3d_node_scale_set(1.0, 2.0, 1.0));
}

static void
_cylinder_setup(Body_3D *cylinder)
{
   cylinder->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   _set_cylinder(cylinder->mesh, 50);
   _body_material_set(cylinder, 0.0, 0.0, 1.0);

   cylinder->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(cylinder->node, evas_3d_node_mesh_add(cylinder->mesh),
         evas_3d_node_position_set(-2.0, 3.0, 1.0));
}

static void
_square_setup(Body_3D *square)
{
   square->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   _set_square(square->mesh);

   _body_material_set(square, 0.9, 1, 1);

   square->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(square->node, evas_3d_node_mesh_add(square->mesh),
         evas_3d_node_position_set(0.0, -1.0, 0.0),
         evas_3d_node_scale_set(30.0, 30.0, 30.0),
         evas_3d_node_orientation_angle_axis_set(90.0, 1.0, 0.0, 0.0));
}

static void
_box_setup(Body_3D *box)
{
   box->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   _set_cube(box->mesh);

   _body_material_set(box, 0, 1, 0);

   box->node = eo_add(EVAS_3D_NODE_CLASS, evas,
                      evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(box->node, evas_3d_node_mesh_add(box->mesh),
         evas_3d_node_position_set(5.0, 0.0, -3.0));
}

static void
_model_setup(Body_3D *model)
{
   Eo *texture = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture,
         evas_3d_texture_file_set("sonic.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   model->material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(model->material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT, texture),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_shininess_set(100.0));


   model->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);

   eo_do(model->mesh,
         efl_file_set("sonic.md2", NULL),
         evas_3d_mesh_frame_material_set(0, model->material),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));

   model->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(model->node, evas_3d_node_mesh_add(model->mesh),
         evas_3d_node_scale_set(0.1, 0.1, 0.1),
         evas_3d_node_orientation_angle_axis_set(120.0, -0.577, -0.577, -0.577));
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);

   eo_do(data->camera,
         evas_3d_camera_projection_perspective_set(50.0, 1.0, 2.0, 50.0));

  data->camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));

  eo_do(data->camera_node,
        evas_3d_node_camera_set(data->camera),
        evas_3d_node_position_set(0.0, 6.0, 12.0),
        evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 3.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 5.0, 0.0));

  eo_do(data->root_node, evas_3d_node_member_add(data->camera_node));
}

static void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(data->light,
         evas_3d_light_ambient_set(AMBIENT_LIGHT, 1.0),
         evas_3d_light_diffuse_set(DIFFUSE_LIGHT, 1.0),
         evas_3d_light_specular_set(SPECULAR_LIGHT, 1.0),
         evas_3d_light_projection_perspective_set(45.0, 1.0, 2.0, 1000.0));

   data->light_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(50.0, 50.0, 20.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 20.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   eo_do(data->scene,
         evas_3d_scene_size_set(WIDTH, HEIGHT);
         evas_3d_scene_background_color_set(BG_COLOR, 1));

   data->root_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));


   _camera_setup(data);
   _light_setup(data);

   _box_setup(&data->cube);
   _sphere_setup(&data->sphere);
   _cylinder_setup(&data->cylinder);
   _square_setup(&data->square);
   _model_setup(&data->model);
   _cone_setup(&data->cone);

   eo_do(data->root_node, evas_3d_node_member_add(data->sphere.node));
   eo_do(data->root_node, evas_3d_node_member_add(data->cube.node));
   eo_do(data->root_node, evas_3d_node_member_add(data->cylinder.node));
   eo_do(data->root_node, evas_3d_node_member_add(data->square.node));
   eo_do(data->root_node, evas_3d_node_member_add(data->model.node));
   eo_do(data->root_node, evas_3d_node_member_add(data->cone.node));

   eo_do(data->scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node),
         evas_3d_scene_shadows_enable_set(EINA_TRUE));
}

int
main(void)
{
   Scene_Data data;
   Ecore_Animator *anim;

   //Unless Evas 3D supports Software renderer, we set gl backened forcely.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
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
         evas_obj_visibility_set(EINA_TRUE));

   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(data.scene));

   /* Add animator. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, &data.model);

   /* Enter main loop. */
   ecore_main_loop_begin();
   ecore_animator_del(anim);

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
