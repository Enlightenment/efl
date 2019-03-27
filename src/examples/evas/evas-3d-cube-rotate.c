/**
 * Example of using mouse callback events for node rotation in Evas-3D.
 *
 * After clicking the mouse on cube callbacks began to emit automatically,
 * depending on mouse's position change the cube rotates with a certain
 * speed while mouse is still on the cube, also rotation slows down when
 * leaving the boundaries of the cube, all this goes on while the mouse
 * is pressed. Note, that rotation is taking place around one axis only.
 *
 * @verbatim
 * gcc -o evas-3d-cube-rotate evas-3d-cube-rotate.c -g `pkg-config --libs --cflags evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define EFL_BETA_API_SUPPORT
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Eo.h>

#define  WIDTH          400
#define  HEIGHT         400

typedef struct _Scene_Data
{
   Eo    *scene;
   Eo    *root_node;
   Eo    *camera_node;
   Eo    *light_node;
   Eo     *mesh_node;

   Eo               *camera;
   Eo               *light;
   Eo               *cube;
   Eo               *mesh;
   Eo               *material;
   float            angle;
   Evas_Coord       mouse_old_x;
   int              mouse_diff_x;
   int              mouse_indicator;
} Scene_Data;

Eo               *scene       = NULL;
Ecore_Evas       *ecore_evas  = NULL;
Evas             *evas        = NULL;
Eo               *background  = NULL;
Eo               *image       = NULL;
static float     angle        = 0.0f;
float            d_angle      = 0.5;

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

static Eina_Bool
_continue_scene(void *data)
{
   Scene_Data *d = (Scene_Data *)data;

   if (d->mouse_indicator == 1)
     {
        evas_event_feed_mouse_down(evas, 1, EVAS_BUTTON_NONE, 0, &data);
     }

   return EINA_TRUE;
}

static void
_rotate_x_reload(Evas_Canvas3D_Scene *eo_scene, Evas_Event_Mouse_Down *ev, Evas_Canvas3D_Node *node, Evas_Canvas3D_Mesh *mesh,
                             float *angle_diff, Evas_Coord *mouse_coord)
{
   Evas_Canvas3D_Node *n = NULL;
   Evas_Canvas3D_Mesh *m = NULL;
   Evas_Real s, t;
   int mouse_diff;

   evas_canvas3d_scene_pick(eo_scene, ev->canvas.x, ev->canvas.y, &n, &m, &s, &t);

   if ((n == node) && (m == mesh))
     {
        mouse_diff = ev->canvas.x - (*mouse_coord);
        *mouse_coord = ev->canvas.x;
        *angle_diff = (mouse_diff)/4;
     }
   else
     {
        if (*angle_diff > 2)
          (*angle_diff)-=1;
        else if (*angle_diff >0)
          (*angle_diff)-=0.25;
        else if(*angle_diff < -2)
          (*angle_diff)+=1;
        else if (*angle_diff <0)
          (*angle_diff)+=0.25;
     }
}

static void
_play_scene(void *data, Evas *_evas EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void   *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Scene_Data *d = (Scene_Data *)data;

   _rotate_x_reload(scene, ev, d->mesh_node, d->mesh, &d->angle, &d->mouse_old_x);

   /* Run this cycle only once to avoid the callback tree. */
   if (d->mouse_indicator == 0)
     ecore_timer_add(0.016, _continue_scene, data);

   d->mouse_indicator = 1;
}

static void
_stop_scene(void *data, Evas *_evas EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info EINA_UNUSED)
{
    Scene_Data *d = (Scene_Data *)data;

    d->angle = 0.0;
    d->mouse_indicator = 2;
}

static Eina_Bool
_animate_scene(void *data)
{
   Scene_Data *sc = (Scene_Data *)data;

   angle += sc->angle;

   evas_canvas3d_node_orientation_angle_axis_set(sc->mesh_node, angle, 0, 1.0, 0.0);

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(data->camera, 20.0, 1.0, 2.0, 50.0);

   data->camera_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 3.0, 3.0, 10.0);
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
   evas_canvas3d_node_position_set(data->light_node, 5.0, 5.0, 10.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);

   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

static void
_mesh_setup(Scene_Data *data)
{
   /* Setup material. */
   data->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.2, 0.2, 0.2, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.8, 0.8, 0.8, 1.0);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(data->material, 100.0);

   /* Setup primitive */
   data->cube = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->cube, EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE);

   /* Setup mesh. */
   data->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(data->mesh, 0, data->cube);
   evas_canvas3d_mesh_shader_mode_set(data->mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
   evas_canvas3d_mesh_frame_material_set(data->mesh, 0, data->material);

   data->mesh_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
   evas_canvas3d_node_member_add(data->root_node, data->mesh_node);
   evas_canvas3d_node_mesh_add(data->mesh_node, data->mesh);
}

static void
_scene_setup(Scene_Data *data)
{
   scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);
   data->root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   evas_canvas3d_scene_size_set(scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(scene, 0.0, 0.0, 0.0, 0.0);

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   evas_canvas3d_scene_root_node_set(scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(scene, data->camera_node);

   data->mouse_old_x = 0;
   data->mouse_diff_x = 0;
   data->mouse_indicator = 0;
}

int
main(void)
{
   Scene_Data data;

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   memset(&data, 0, sizeof(Scene_Data));
   _scene_setup(&data);

   /* Add a background rectangle object. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_gfx_entity_geometry_set(image, EINA_RECT(0, 0, WIDTH, HEIGHT));
   evas_object_show(image);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, scene);

   evas_object_focus_set(image, EINA_TRUE);

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _play_scene, &data);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_UP, _stop_scene, &data);

   /* Add animation timer callback. */
   ecore_timer_add(0.016, _animate_scene, &data);

   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
