/*
 * This example shows how to work frustum culling.
 * Use keys Up/Down for moving far plane of frustum.
 * Use keys Left/Right for changing camera view.
 * See in terminal output value of z coordinate of far plane of frustum
 * and check OBB's points inside frustum.
 * @see evas_3d_node_obb_frustum_check.
 * Compile with "gcc -o evas-3d-frustum evas-3d-frustum.c `pkg-config --libs --cflags evas ecore ecore-evas eo`"
 */

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Eo.h>

#define  WIDTH          800
#define  HEIGHT         600

typedef struct _Scene_Data
{
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node_model;
   Eo *scene;
   Eo *camera;
   Eo *light;
   Eo *mesh_model;
   Eo *material_model;
   Eo *texture_model;
} Scene_Data;

Evas *evas;
Evas_Object *background,*image;
Evas_Real fleft = -5, fright = 5, fbottom = -5, fup = 5, fnear = 20, ffar = 40;

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
   evas_object_resize(image, w, h);
   evas_object_move(image, 0, 0);
}

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{

   Scene_Data *scene = (Scene_Data *)data;
   Evas_Event_Key_Down *ev = event_info;
   int frustum;

   if (!strcmp("Up", ev->key))
     {
        ffar += 5;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
        eo_do(scene->mesh_node_model,
              frustum = evas_3d_node_obb_frustum_check(scene->camera_node));
        fprintf(stdout, "far - %f frustum - %d \n", ffar, frustum);
     }
   else if(!strcmp("Down", ev->key))
     {
        ffar -= 5;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
        eo_do(scene->mesh_node_model,
              frustum = evas_3d_node_obb_frustum_check(scene->camera_node));
        fprintf(stdout, "far - %f frustum - %d \n", ffar, frustum);
     }
   else if(!strcmp("Return", ev->key))
     {
        eo_do(scene->mesh_node_model,
              frustum = evas_3d_node_obb_frustum_check(scene->camera_node));
        fprintf(stdout, "far - %f frustum - %d \n", ffar, frustum);
     }
   else if (!strcmp("Left", ev->key))
     {
        eo_do(scene->camera_node, evas_3d_node_position_set(50.0, 0.0, 0.0),
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));
        fprintf(stdout, "Changed position and view of camera\n");
     }
   else if (!strcmp("Right", ev->key))
     {
        eo_do(scene->camera_node, evas_3d_node_position_set(0.0, 0.0, 50.0);
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, -1.0, 0.0));
        fprintf(stdout, "Changed position and view of camera\n");
     }
   else
     fprintf(stdout, "Press Right/Left/Up/Bottom keys\n");
}

static Eina_Bool
_animate_scene_model(void *data)
{
   static int frame = 0;
   Scene_Data *scene = (Scene_Data *)data;

   eo_do(scene->mesh_node_model,
         evas_3d_node_mesh_frame_set(scene->mesh_model, frame),
         evas_3d_node_orientation_angle_axis_set(90, 1.0, 0.0, 0.0));
   frame += 20;

   if (frame > 256 * 18) frame = 0;

   return ECORE_CALLBACK_RENEW;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   data->camera_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                     evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(0.0, 0.0, 50.0);
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, -1.0, 0.0));

   eo_do(data->root_node, evas_3d_node_member_add(data->camera_node));
}

static void
_light_setup(Scene_Data *data)
{
   data->light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(data->light,
         evas_3d_light_ambient_set( 0.2, 0.2, 0.2, 1.0);
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0);
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0));

   data->light_node = eo_add(EVAS_3D_NODE_CLASS,evas,
                                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(data->light_node,
         evas_3d_node_light_set(data->light),
         evas_3d_node_position_set(0.0, 0.0, 0.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));

   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
}

static void
_mesh_setup_model(Scene_Data *data)
{
   data->mesh_model = eo_add(EVAS_3D_MESH_CLASS, evas);
   data->material_model = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   data->texture_model = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture_model,
         evas_3d_texture_file_set("eagle.png", NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST, EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT, EVAS_3D_WRAP_MODE_REPEAT));

   eo_do(data->material_model,
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE);
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0, 1.0),
         evas_3d_material_texture_set( EVAS_3D_MATERIAL_DIFFUSE, data->texture_model),
         evas_3d_material_shininess_set(100.0));

   eo_do(data->mesh_model,
         evas_3d_mesh_file_set(EVAS_3D_MESH_FILE_TYPE_MD2, "eagle.md2", NULL),
         evas_3d_mesh_frame_material_set(0, data->material_model),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   data->root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                   evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));
   _camera_setup(data);
   _light_setup(data);
   _mesh_setup_model(data);

   data->mesh_node_model = eo_add(EVAS_3D_NODE_CLASS, evas,
                                      evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->mesh_node_model,
         evas_3d_node_position_set(0, 0, 0);
         evas_3d_node_scale_set(0.3, 0.3, 0.3),
         evas_3d_node_orientation_angle_axis_set(90, 1.0, 1.0, 0.0));
   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node_model));
   eo_do(data->mesh_node_model, evas_3d_node_mesh_add(data->mesh_model));

   eo_do(data->scene,
         evas_3d_scene_size_set( WIDTH, HEIGHT),
         evas_3d_scene_background_color_set(0.5, 0.5, 0.5, 0.0),
         evas_3d_scene_root_node_set(data->root_node);
         evas_3d_scene_camera_node_set(data->camera_node));
}

int
main(void)
{
   Scene_Data data;

   Ecore_Evas *ecore_evas;

   if (!ecore_evas_init()) return 0;

   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   ecore_evas = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   image = evas_object_image_filled_add(evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);

   evas_object_focus_set(image, EINA_TRUE);
   eo_do(image, evas_obj_image_scene_set(data.scene));

   ecore_timer_add(0.1, _animate_scene_model, &data);

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
