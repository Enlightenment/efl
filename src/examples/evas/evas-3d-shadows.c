/**
 * This example illustrating use of shadows in the scene and callbacks(clicked, collision).
 * Model and cube are clickable. Model detects collision with sphere.
 * Cube detects collision with sphere, model and cone.
 * @see evas_3d_scene_shadows_enable_set(Eina_Bool _shadows_enabled)
 * @see evas_3d_object_callback_register
 *
 * @verbatim
 * gcc -o evas-3d-shadows evas-3d-shadows.c evas-3d-primitives.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo eina` -lm
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
#include <Eina.h>
#include <math.h>
#include "evas-3d-primitives.h"
#include "evas-common.h"

#define  WIDTH 1024
#define  HEIGHT 1024

#define STEP 0.1
#define BG_COLOR 0.2, 0.2, 0.2
#define AMBIENT_LIGHT 0.2, 0.2, 0.2
#define DIFFUSE_LIGHT 1.0, 1.0, 1.0
#define SPECULAR_LIGHT 1.0, 1.0, 1.0

static const char *model_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sonic.md2";

static const vec2 tex_scale = {1, 1};
static const vec2 fence_tex_scale = {80, 6};

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;
Evas_3D_Node *choosed_node = NULL;

Eina_Bool
_cb_clicked(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Eina_List *meshes = NULL, *l;
   Evas_3D_Mesh *m;
   eo_do((Evas_3D_Node *)event_info, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, m)
     {
        eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
     }
   if (choosed_node != (Evas_3D_Node *)event_info)
     {
        eo_do(choosed_node, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
        EINA_LIST_FOREACH(meshes, l, m)
          {
             eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
          }
        choosed_node = (Evas_3D_Node *)event_info;
     }

   return EINA_TRUE;
}

Eina_Bool
_cb_collision(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Eina_List *meshes = NULL, *l;
   Evas_3D_Mesh *m;
   eo_do((Evas_3D_Node *)event_info, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, m)
     {
        eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
     }

   return EINA_TRUE;
}

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
   Body_3D     fence;
} Scene_Data;

static void
_show_help()
{
   fprintf(stdout, "Press 'w'/'s' key to move up/down object\n");
   fprintf(stdout, "Press 'a'/'d' key to move left/right object\n");
   fprintf(stdout, "Press 'q'/'e' key to to move near/far object\n");
   fprintf(stdout, "Cude and model can be moved.\n");
   fprintf(stdout, "Cube detects intersection with model, sphere, cone\n");
   fprintf(stdout, "Model detects intersection with sphere\n");
}

static Eina_Bool
_animate_scene(void *data)
{
   static int frame = 0;
   Body_3D *body = (Body_3D *)data;

   eo_do(body->node, evas_3d_node_mesh_frame_set(body->mesh, frame));

   /*frame += 32;*/

   if (frame > 256 * 20) frame = 0;

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
   evas_3d_add_sphere_frame(sphere->mesh, 0, 50, tex_scale);
   _body_material_set(sphere, 1, 0.0, 0.0);

   sphere->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH),
                    evas_3d_node_position_set(3.0, 3.0, 0.0));
   eo_do(sphere->node, evas_3d_node_mesh_add(sphere->mesh));
}

static void
_cone_setup(Body_3D *cone)
{
   cone->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_cone_frame(cone->mesh, 0, 100, tex_scale);
   _body_material_set(cone, 0.8, 0.5, 0.5);

   cone->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(cone->node, evas_3d_node_mesh_add(cone->mesh),
         evas_3d_node_position_set(-5.0, -1.0, -3.0));
}

static void
_cylinder_setup(Body_3D *cylinder)
{
   cylinder->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_cylinder_frame(cylinder->mesh, 0, 50, tex_scale);
   _body_material_set(cylinder, 0.0, 0.0, 1.0);

   cylinder->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(cylinder->node, evas_3d_node_mesh_add(cylinder->mesh),
         evas_3d_node_position_set(-2.0, 3.0, 1.0));
}

static void
_fence_setup(Body_3D *fence)
{

   Eo *texture = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture,
         evas_3d_texture_file_set(PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/grid.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   Eo *texture1 = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture1,
         evas_3d_texture_file_set(PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/grid_n.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   fence->material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   eo_do(fence->material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, texture),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_AMBIENT, texture),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_NORMAL, texture1),
         evas_3d_material_shininess_set(100.0));
   fence->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_cylinder_frame(fence->mesh, 0, 50, fence_tex_scale);

   eo_do(fence->mesh,
         evas_3d_mesh_frame_material_set(0, fence->material),
         evas_3d_mesh_alpha_func_set(EVAS_3D_COMPARISON_GREATER, 0),
         evas_3d_mesh_alpha_test_enable_set(EINA_TRUE),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_NORMAL_MAP));
   fence->node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(fence->node, evas_3d_node_mesh_add(fence->mesh),
         evas_3d_node_scale_set(7.0, 3.0, 7.0),
         evas_3d_node_position_set(0.0, 0.5, -5.0));
}

static void
_square_setup(Body_3D *square)
{
   square->mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_square_frame(square->mesh, 0);

   _body_material_set(square, 0.4, 0.4, 0.4);

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
   evas_3d_add_cube_frame(box->mesh, 0);

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
         evas_3d_texture_file_set(PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/sonic.png", NULL),
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
         efl_file_set(model_path, NULL),
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
         evas_3d_light_spot_cutoff_set(20),
         evas_3d_light_projection_perspective_set(40.0, 1.0, 2.0, 1000.0));

   data->light_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(50.0, 50.0, 20.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 15.0, 0.0, -5.0,
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
   _fence_setup(&data->fence);

   eo_do(data->root_node, 
         evas_3d_node_member_add(data->sphere.node),
         evas_3d_node_member_add(data->cube.node),
         evas_3d_node_member_add(data->cylinder.node),
         evas_3d_node_member_add(data->square.node),
         evas_3d_node_member_add(data->model.node),
         evas_3d_node_member_add(data->cone.node),
         evas_3d_node_member_add(data->fence.node));

   eo_do(data->scene,
         evas_3d_scene_root_node_set(data->root_node),
         evas_3d_scene_camera_node_set(data->camera_node),
         evas_3d_scene_shadows_enable_set(EINA_TRUE));
}

static void
_on_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   if (!strcmp("w", ev->key))
     {
        Evas_Real x, y, z;
        eo_do(choosed_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(choosed_node, evas_3d_node_position_set(x, y + STEP, z));
     }
   else if(!strcmp("s", ev->key))
     {
        Evas_Real x, y, z;
        eo_do(choosed_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(choosed_node, evas_3d_node_position_set(x, y - STEP, z));
     }
   else if(!strcmp("a", ev->key))
     {
        Evas_Real x, y, z;
        eo_do(choosed_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(choosed_node, evas_3d_node_position_set(x - STEP, y, z));
     }
   else if(!strcmp("d", ev->key))
     {
        Evas_Real x, y, z;
        eo_do(choosed_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(choosed_node, evas_3d_node_position_set(x + STEP, y, z));
     }
   else if(!strcmp("q", ev->key))
     {
        Evas_Real x, y, z;
        eo_do(choosed_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(choosed_node, evas_3d_node_position_set(x, y, z - STEP));
     }
   else if(!strcmp("e", ev->key))
     {
        Evas_Real x, y, z;
        eo_do(choosed_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(choosed_node, evas_3d_node_position_set(x, y, z + STEP));
     }
   else
     {
        _show_help();
     }
}

static void _init(Scene_Data *data)
{
   Eina_List *meshes = NULL, *l;
   Evas_3D_Mesh *m;
   eo_do(data->sphere.node, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, m)
     {
        eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
     }
   eo_do(data->cube.node, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, m)
     {
        eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
     }
   eo_do(data->cylinder.node, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, m)
     {
        eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
     }
   eo_do(data->model.node, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, m)
     {
        eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
     }
   eo_do(data->cone.node, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, m)
     {
        eo_do(m, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
     }
}

static void
_on_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Scene_Data *d = (Scene_Data *)data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_3D_Node *n = NULL;
   Evas_3D_Mesh *m = NULL;
   Evas_Real s, t;
   if (ev->button == 3)
     {
        _init(d);
        return;
     }
   eo_do(d->scene, evas_3d_scene_pick(ev->canvas.x, ev->canvas.y, &n, &m, &s, &t));

}
int
main(void)
{
   Scene_Data data;
   Ecore_Animator *anim;
   Eina_List *nodes1 = NULL, *nodes2 = NULL;
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

   evas_object_focus_set(image, EINA_TRUE);
   /* Set the image object as render target for 3D scene. */
   eo_do(image, evas_obj_image_scene_set(data.scene));

   nodes1 = eina_list_append(nodes1, data.sphere.node);
   nodes2 = eina_list_append(nodes2, data.sphere.node);
   nodes2 = eina_list_append(nodes2, data.model.node);
   nodes2 = eina_list_append(nodes2, data.cone.node);

   /*Set callbacks*/
   eo_do(data.cube.node, eo_event_callback_add(EVAS_3D_OBJECT_EVENT_CLICKED, _cb_clicked, NULL));
   eo_do(data.cube.node, eo_event_callback_add(EVAS_3D_OBJECT_EVENT_COLLISION, _cb_collision, nodes2));

   eo_do(data.model.node, eo_event_callback_add(EVAS_3D_OBJECT_EVENT_CLICKED, _cb_clicked, NULL));
   eo_do(data.model.node, eo_event_callback_add(EVAS_3D_OBJECT_EVENT_COLLISION, _cb_collision, nodes1));

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);

   /* Add animator. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, &data.model);

   /* Enter main loop. */
   ecore_main_loop_begin();
   ecore_animator_del(anim);
   eina_list_free(nodes1);
   eina_list_free(nodes2);
   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
