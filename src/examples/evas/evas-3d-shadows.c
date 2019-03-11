/**
 * Example of setting up an animated Evas-3D scene with lighting and anti-aliasing.
 *
 * This example illustrates use of shadows, callbacks(clicked, collision),
 * technic of the billboard and post proccesing render with anti-aliasing.
 *
 * Model and cube are clickable. Model detects collision with sphere.
 * Cube detects collision with sphere, model and cone. Model and cude are moveable.
 * @see evas_canvas3d_scene_shadows_enable_set(Eina_Bool _shadows_enabled)
 * @see evas_canvas3d_object_callback_register
 * @see evas_canvas3d_billboard_set/get
 * @see evas_object_anti_alias_set/get
 *
 * Control keys and description:
 * 'w'/'s' key to move up/down object;
 * 'a'/'d' key to move left/right object;
 * 'q'/'e' key to move near/far object;
 * '1'/'2' key to change kind of node - billboard/normal model\n");
 * '3'/'4' key to enable/disable post proccesing render;
 * '5'/'6' key to enable/disable shadow effect;
 * Up/Down key to change position of camera;
 * 'i' key to return initial view of scene;

 * @verbatim
 * gcc -o evas-3d-shadows evas-3d-shadows.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo eina` -lm
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
#include <Eina.h>
#include <math.h>
#include "evas-common.h"

#define  WIDTH 1024
#define  HEIGHT 1024

#define STEP 0.1
#define BG_COLOR 0.2, 0.2, 0.2
#define AMBIENT_LIGHT 0.2, 0.2, 0.2
#define DIFFUSE_LIGHT 1.0, 1.0, 1.0
#define SPECULAR_LIGHT 1.0, 1.0, 1.0

static const char *model_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sonic.md2";
static const char *image_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/sonic.png";
static const char *b_image_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/billboard.png";

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;
Evas_Canvas3D_Node *choosed_node = NULL;

typedef struct _Body_3D
{
   Eo     *primitive;
   Eo     *material;
   Eo     *mesh;
   Eo     *node;
   Eo     *texture;
} Body_3D;

typedef struct _Scene_Data
{
   Eo     *scene;
   Eo     *root_node;
   Eo     *camera_node;
   Eo     *camera;
   Eo     *light_node;
   Eo     *light;
   Eo     *mediator;

   Body_3D     sphere;
   Body_3D     cube;
   Body_3D     square;
   Body_3D     cylinder;
   Body_3D     model;
   Body_3D     cone;
   Body_3D     fence;
   Body_3D     billboard;

   Eina_Bool   init;
} Scene_Data;

void
_cb_clicked(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_List *meshes = NULL, *l;
   Evas_Canvas3D_Mesh *m;
   Evas_Canvas3D_Node *billboard = NULL;
   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get((Evas_Canvas3D_Node *)event->info);
   EINA_LIST_FOREACH(meshes, l, m)
     {
        evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
     }
   if (choosed_node != (Evas_Canvas3D_Node *)event->info)
     {
        billboard = evas_canvas3d_node_billboard_target_get(choosed_node);
        if (!billboard)
          {
             meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(choosed_node);
             EINA_LIST_FOREACH(meshes, l, m)
               {
                  evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_PHONG);
               }
          }
        choosed_node = (Evas_Canvas3D_Node *)event->info;
     }
}

void
_cb_collision(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eina_List *meshes = NULL, *l;
   Evas_Canvas3D_Mesh *m;
   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get((Evas_Canvas3D_Node *)event->info);
   EINA_LIST_FOREACH(meshes, l, m)
     {
        evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
     }
}

static void
_show_help()
{
   printf("Press 'w'/'s' key to move up/down object\n");
   printf("Press 'a'/'d' key to move left/right object\n");
   printf("Press 'q'/'e' key to move near/far object\n");
   printf("Cude and model can be moved.\n");
   printf("Cube detects intersection with model, sphere, cone\n");
   printf("Model detects intersection with sphere\n");
   printf("Press '1'/'2' key to change kind of node - billboard/normal model\n");
   printf("Press '3'/'4' key to enable/disable post render with anti-aliasing\n");
   printf("Press '5'/'6' key to enable/disable shadow effect\n");
   printf("Press Up/Down key to change position of camera\n");
   printf("Press 'i' key to return initial view of scene\n");
}

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0;
   Evas_Real x, y, z;
   Scene_Data *scene = (Scene_Data *)data;
   if (scene->init)
     {
        evas_canvas3d_node_position_get(scene->mediator, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(scene->mediator, sin(angle) * 20, y, cos(angle) * 20);
        evas_canvas3d_node_look_at_set(scene->mediator, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 3.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 5.0, 0.0);
        angle += 0.005;
        if (angle > 360) angle = 0.0;
     }
   else
     {
        evas_canvas3d_node_position_set(scene->mediator, 0.0, 6.0, 12.0);
        evas_canvas3d_node_look_at_set(scene->mediator, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 3.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 5.0, 0.0);
     }

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

static void
_body_material_set(Body_3D *body, float r, float g, float b)
{
   body->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_enable_set(body->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(body->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(body->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(body->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, r, g, b, 1.0);
   evas_canvas3d_material_color_set(body->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, r, g, b, 1.0);
   evas_canvas3d_material_color_set(body->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(body->material, 100.0);

   evas_canvas3d_mesh_shader_mode_set(body->mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(body->mesh, 0, body->material);
}

static void
_sphere_setup(Body_3D *sphere)
{
   sphere->primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(sphere->primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE);
   evas_canvas3d_primitive_precision_set(sphere->primitive, 50);

   sphere->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(sphere->mesh, 0, sphere->primitive);

   _body_material_set(sphere, 1, 0.0, 0.0);

   sphere->node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH), evas_canvas3d_node_position_set(efl_added, 3.0, 3.0, 0.0));
   evas_canvas3d_node_mesh_add(sphere->node, sphere->mesh);
}

static void
_cone_setup(Body_3D *cone)
{
   cone->primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(cone->primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_CONE);
   evas_canvas3d_primitive_precision_set(cone->primitive, 50);

   cone->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(cone->mesh, 0, cone->primitive);

   _body_material_set(cone, 0.8, 0.5, 0.5);

   cone->node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_mesh_add(cone->node, cone->mesh);
   evas_canvas3d_node_orientation_angle_axis_set(cone->node, -90.0, 1.0, 0.0, 0.0);
   evas_canvas3d_node_position_set(cone->node, -4.0, 0.0, -3.0);
}

static void
_cylinder_setup(Body_3D *cylinder)
{
   cylinder->primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(cylinder->primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_CYLINDER);
   evas_canvas3d_primitive_precision_set(cylinder->primitive, 50);

   cylinder->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(cylinder->mesh, 0, cylinder->primitive);

   _body_material_set(cylinder, 0.0, 0.0, 1.0);

   cylinder->node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_mesh_add(cylinder->node, cylinder->mesh);
   evas_canvas3d_node_orientation_angle_axis_set(cylinder->node, -90.0, 1.0, 0.0, 0.0);
   evas_canvas3d_node_position_set(cylinder->node, -2.0, 3.0, 1.0);
}

static void
_fence_setup(Body_3D *fence)
{

   Eo *texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   evas_canvas3d_texture_atlas_enable_set(texture, EINA_FALSE);
   efl_file_simple_load(texture, PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/grid.png", NULL);
   evas_canvas3d_texture_filter_set(texture, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(texture, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   Eo *texture1 = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   evas_canvas3d_texture_atlas_enable_set(texture1, EINA_FALSE);
   efl_file_simple_load(texture1, PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/grid_n.png", NULL);
   evas_canvas3d_texture_filter_set(texture1, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(texture1, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   fence->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_texture_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, texture);
   evas_canvas3d_material_texture_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, texture);
   evas_canvas3d_material_enable_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_texture_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, texture1);
   evas_canvas3d_material_color_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(fence->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(fence->material, 100.0);

   fence->primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(fence->primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_CYLINDER);
   evas_canvas3d_primitive_mode_set(fence->primitive, EVAS_CANVAS3D_PRIMITIVE_MODE_WITHOUT_BASE);
   evas_canvas3d_primitive_tex_scale_set(fence->primitive, 160.0, 12.0);
   evas_canvas3d_primitive_precision_set(fence->primitive, 50);

   fence->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(fence->mesh, 0, fence->primitive);

   evas_canvas3d_mesh_frame_material_set(fence->mesh, 0, fence->material);
   evas_canvas3d_mesh_alpha_func_set(fence->mesh, EVAS_CANVAS3D_COMPARISON_GREATER, 0);
   evas_canvas3d_mesh_alpha_test_enable_set(fence->mesh, EINA_TRUE);
   evas_canvas3d_mesh_shader_mode_set(fence->mesh, EVAS_CANVAS3D_SHADER_MODE_NORMAL_MAP);
   fence->node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_mesh_add(fence->node, fence->mesh);
   evas_canvas3d_node_orientation_angle_axis_set(fence->node, -90.0, 1.0, 0.0, 0.0);
   evas_canvas3d_node_scale_set(fence->node, 10.0, 10.0, 5.0);
   evas_canvas3d_node_position_set(fence->node, 0.0, -1.0, -2.0);
}

static void
_square_setup(Body_3D *square)
{
   square->primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(square->primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_SQUARE);

   square->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(square->mesh, 0, square->primitive);

   _body_material_set(square, 0.4, 0.4, 0.4);

   square->node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_mesh_add(square->node, square->mesh);
   evas_canvas3d_node_position_set(square->node, 0.0, -1.0, 0.0);
   evas_canvas3d_node_scale_set(square->node, 30.0, 30.0, 30.0);
   evas_canvas3d_node_orientation_angle_axis_set(square->node, 90.0, 1.0, 0.0, 0.0);
}

static void
_box_setup(Body_3D *box)
{
   box->primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(box->primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);

   box->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(box->mesh, 0, box->primitive);

   _body_material_set(box, 0, 1, 0);

   box->node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_mesh_add(box->node, box->mesh);
   evas_canvas3d_node_position_set(box->node, 3.0, 0.0, -3.0);
}

static void
_model_setup(Body_3D *model)
{
   model->texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(model->texture, image_path, NULL);
   evas_canvas3d_texture_filter_set(model->texture, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(model->texture, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   model->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_texture_set(model->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, model->texture);
   evas_canvas3d_material_texture_set(model->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, model->texture);
   evas_canvas3d_material_enable_set(model->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(model->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(model->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_shininess_set(model->material, 100.0);


   model->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   efl_file_simple_load(model->mesh, model_path, NULL);
   evas_canvas3d_mesh_frame_material_set(model->mesh, 0, model->material);
   evas_canvas3d_mesh_shader_mode_set(model->mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);

   model->node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_mesh_add(model->node, model->mesh);
   evas_canvas3d_node_scale_set(model->node, 0.1, 0.1, 0.1);
   evas_canvas3d_node_orientation_angle_axis_set(model->node, 120.0, -0.577, -0.577, -0.577);
}

static void
_billboard_setup(Scene_Data *data)
{
   data->billboard.texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(data->billboard.texture, b_image_path, NULL);
   evas_canvas3d_texture_filter_set(data->billboard.texture, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(data->billboard.texture, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);

   data->billboard.primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->billboard.primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_SQUARE);

   data->billboard.mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(data->billboard.mesh, 0, data->billboard.primitive);

   _body_material_set(&(data->billboard), 1.0, 1.0, 1.0);

   evas_canvas3d_material_texture_set(data->billboard.material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, data->billboard.texture);

   evas_canvas3d_mesh_frame_material_set(data->billboard.mesh, 0, data->billboard.material);
   evas_canvas3d_mesh_alpha_func_set(data->billboard.mesh, EVAS_CANVAS3D_COMPARISON_GREATER, 0);
   evas_canvas3d_mesh_alpha_test_enable_set(data->billboard.mesh, EINA_TRUE);
   evas_canvas3d_mesh_shader_mode_set(data->billboard.mesh, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
   evas_canvas3d_mesh_blending_enable_set(data->billboard.mesh, EINA_TRUE);
   evas_canvas3d_mesh_blending_func_set(data->billboard.mesh, EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA, EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);

   data->billboard.node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_mesh_add(data->billboard.node, data->billboard.mesh);
   evas_canvas3d_node_position_set(data->billboard.node, 0.0, 2.0, 0.0);
   evas_canvas3d_node_scale_set(data->billboard.node, 2.2, 4.6, 4.0);

   evas_canvas3d_node_billboard_target_set(data->billboard.node, data->mediator);
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   data->mediator = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   evas_canvas3d_camera_projection_perspective_set(data->camera, 50.0, 1.0, 2.0, 100.0);

  data->camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));

  evas_canvas3d_node_camera_set(data->camera_node, data->camera);


  evas_canvas3d_node_position_set(data->mediator, 0.0, 6.0, 12.0);
  evas_canvas3d_node_look_at_set(data->mediator, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 3.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 5.0, 0.0);

  evas_canvas3d_node_member_add(data->mediator, data->camera_node);
  evas_canvas3d_node_member_add(data->root_node, data->mediator);
}

static void
_light_setup(Scene_Data *data)
{
   data->light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(data->light, AMBIENT_LIGHT, 1.0);
   evas_canvas3d_light_diffuse_set(data->light, DIFFUSE_LIGHT, 1.0);
   evas_canvas3d_light_specular_set(data->light, SPECULAR_LIGHT, 1.0);
   evas_canvas3d_light_spot_cutoff_set(data->light, 20);
   evas_canvas3d_light_projection_perspective_set(data->light, 40.0, 1.0, 2.0, 1000.0);

   data->light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, 50.0, 50.0, 70.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 15.0, 0.0, -5.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0);
   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

static void
_scene_setup(Scene_Data *data)
{
   data->init = EINA_FALSE;

   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(data->scene, BG_COLOR, 1);

   data->root_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));


   _camera_setup(data);
   _light_setup(data);

   _box_setup(&data->cube);
   _sphere_setup(&data->sphere);
   _cylinder_setup(&data->cylinder);
   _square_setup(&data->square);
   _model_setup(&data->model);
   _cone_setup(&data->cone);
   _fence_setup(&data->fence);
   _billboard_setup(data);

   evas_canvas3d_node_member_add(data->root_node, data->sphere.node);
   evas_canvas3d_node_member_add(data->root_node, data->cube.node);
   evas_canvas3d_node_member_add(data->root_node, data->cylinder.node);
   evas_canvas3d_node_member_add(data->root_node, data->square.node);
   evas_canvas3d_node_member_add(data->root_node, data->model.node);
   evas_canvas3d_node_member_add(data->root_node, data->cone.node);
   evas_canvas3d_node_member_add(data->root_node, data->fence.node);

   evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
   evas_canvas3d_scene_shadows_enable_set(data->scene, EINA_TRUE);
}

static void
_on_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Scene_Data *scene = (Scene_Data *)data;
   if (!strcmp("w", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(choosed_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(choosed_node, x, y + STEP, z);
     }
   else if(!strcmp("s", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(choosed_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(choosed_node, x, y - STEP, z);
     }
   else if(!strcmp("a", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(choosed_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(choosed_node, x - STEP, y, z);
     }
   else if(!strcmp("d", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(choosed_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(choosed_node, x + STEP, y, z);
     }
   else if(!strcmp("q", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(choosed_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(choosed_node, x, y, z - STEP);
     }
   else if(!strcmp("e", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(choosed_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(choosed_node, x, y, z + STEP);
     }
   else if(!strcmp("1", ev->key))
     {
        evas_canvas3d_node_member_del(scene->root_node, scene->model.node);
        evas_canvas3d_node_member_add(scene->root_node, scene->billboard.node);
     }
   else if(!strcmp("2", ev->key))
     {
        evas_canvas3d_node_member_add(scene->root_node, scene->model.node);
        evas_canvas3d_node_member_del(scene->root_node, scene->billboard.node);
     }
   else if(!strcmp("3", ev->key))
     {
        evas_object_anti_alias_set(image, EINA_TRUE);
     }
   else if(!strcmp("4", ev->key))
     {
        evas_object_anti_alias_set(image, EINA_FALSE);
     }
   else if(!strcmp("5", ev->key))
     {
        evas_canvas3d_scene_shadows_enable_set(scene->scene, EINA_TRUE);
     }
   else if(!strcmp("6", ev->key))
     {
        evas_canvas3d_scene_shadows_enable_set(scene->scene, EINA_FALSE);
     }
   else if(!strcmp("Up", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(scene->camera_node, x, y, z + STEP);
     }
   else if(!strcmp("Down", ev->key))
     {
        Evas_Real x, y, z;
        evas_canvas3d_node_position_get(scene->camera_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(scene->camera_node, x, y, z - STEP);
     }
   else if (!strcmp("i", ev->key))
     {
        scene->init = !scene->init;
        evas_canvas3d_node_position_set(scene->model.node, 0.0, 0.0, 0.0);
        evas_canvas3d_node_position_set(scene->billboard.node, 0.0, 2.0, 0.0);
     }
   else
     {
        _show_help();
     }
}

static void _init(Scene_Data *data)
{
   Eina_List *meshes = NULL, *l;
   Evas_Canvas3D_Mesh *m;
   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(data->sphere.node);
   EINA_LIST_FOREACH(meshes, l, m)
     {
        evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_PHONG);
     }
   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(data->cube.node);
   EINA_LIST_FOREACH(meshes, l, m)
     {
        evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_PHONG);
     }
   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(data->cylinder.node);
   EINA_LIST_FOREACH(meshes, l, m)
     {
        evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_PHONG);
     }
   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(data->model.node);
   EINA_LIST_FOREACH(meshes, l, m)
     {
        evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_PHONG);
     }
   meshes = (Eina_List *)evas_canvas3d_node_mesh_list_get(data->cone.node);
   EINA_LIST_FOREACH(meshes, l, m)
     {
        evas_canvas3d_mesh_shader_mode_set(m, EVAS_CANVAS3D_SHADER_MODE_PHONG);
     }
}

static void
_on_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Scene_Data *d = (Scene_Data *)data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Canvas3D_Node *n = NULL;
   Evas_Canvas3D_Mesh *m = NULL;
   Evas_Real s, t;
   if (ev->button == 3)
     {
        _init(d);
        return;
     }
   evas_canvas3d_scene_pick(d->scene, ev->canvas.x, ev->canvas.y, &n, &m, &s, &t);

}

int
main(void)
{
   Scene_Data data;
   Ecore_Animator *anim;
   Eina_List *nodes1 = NULL, *nodes2 = NULL;

   // Unless Evas 3D supports Software renderer, we force use of the gl backend.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
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
   evas_object_anti_alias_set(image, EINA_TRUE);
   evas_object_focus_set(image, EINA_TRUE);
   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, data.scene);

   nodes1 = eina_list_append(nodes1, data.sphere.node);
   nodes2 = eina_list_append(nodes2, data.sphere.node);
   nodes2 = eina_list_append(nodes2, data.model.node);
   nodes2 = eina_list_append(nodes2, data.cone.node);

   /*Set callbacks*/
   efl_event_callback_add(data.cube.node, EVAS_CANVAS3D_OBJECT_EVENT_CLICKED, _cb_clicked, NULL);
   efl_event_callback_add(data.cube.node, EVAS_CANVAS3D_OBJECT_EVENT_COLLISION, _cb_collision, nodes2);

   efl_event_callback_add(data.model.node, EVAS_CANVAS3D_OBJECT_EVENT_CLICKED, _cb_clicked, NULL);
   efl_event_callback_add(data.model.node, EVAS_CANVAS3D_OBJECT_EVENT_COLLISION, _cb_collision, nodes1);

   efl_event_callback_add(data.billboard.node, EVAS_CANVAS3D_OBJECT_EVENT_CLICKED, _cb_clicked, NULL);
   efl_event_callback_add(data.billboard.node, EVAS_CANVAS3D_OBJECT_EVENT_COLLISION, _cb_collision, nodes1);

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);

   /* Add animator. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();
   ecore_animator_del(anim);
   eina_list_free(nodes1);
   eina_list_free(nodes2);
   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
