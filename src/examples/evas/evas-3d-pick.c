/**
 * This example shows how to attach mechanism of pick. 
 *
 * Here shown which transformation should be applied to event_info of 
 * _on_mouse_down to make them usable in evas_3d_scene_pick()
 * and parameters which can be got from this function.
 *
 * @verbatim
 * gcc -o evas-3d-pick evas-3d-pick.c evas-3d-primitives.c `pkg-config --libs --cflags evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <math.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-common.h"
#include "evas-3d-primitives.h"

#define  WIDTH          400
#define  HEIGHT         400

static const char *image_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/wood.jpg";

static Ecore_Evas *ecore_evas = NULL;
static Evas *evas = NULL;
static Eo *background = NULL;
static Eo *image = NULL;
static Eo *scene = NULL;
static Eo *root_node = NULL;
static Eo *camera_node = NULL;
static Eo *camera = NULL;
static Eo *mesh_node = NULL;
static Eo *mesh = NULL;
static Eo *material = NULL;
static Eo *texture_diffuse = NULL;
static const vec2 tex_scale = {1, 1};

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;

   angle += 0.3;

   eo_do((Evas_3D_Node *)data,
         evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 0.0));

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
   evas_object_resize(background, w, h);
   evas_object_resize(image, w, h);
   evas_object_move(image, 0, 0);
}

static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   Evas_Coord obj_x, obj_y;
   int scene_w, scene_h;
   Evas_Real scene_x, scene_y;
   Evas_Real s, t;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Eina_Bool pick;

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   obj_x = ev->canvas.x - x;
   obj_y = ev->canvas.y - y;

   eo_do(scene, evas_3d_scene_size_get(&scene_w, &scene_h));

   scene_x = obj_x * scene_w / (Evas_Real)w;
   scene_y = obj_y * scene_h / (Evas_Real)h;

   eo_do(scene, pick = evas_3d_scene_pick(scene_x, scene_y, &n, &m, &s, &t));
   if (pick) printf("Picked     : ");
   else printf("Not picked : ");

   printf("output(%d, %d) canvas(%d, %d) object(%d, %d) scene(%f, %f) texcoord(%f, %f) "
          "node(%p) mesh(%p)\n",
          ev->output.x, ev->output.y,
          ev->canvas.x, ev->canvas.y,
          obj_x, obj_y,
          scene_x, scene_y,
          s, t, n, m);
}

int
main(void)
{
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
         evas_3d_camera_projection_perspective_set(30.0, 1.0, 1.0, 100.0));

   camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_3d_node_camera_set(camera),
         evas_3d_node_position_set(0.0, 0.0, 2.5),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node, evas_3d_node_member_add(camera_node));

   /* Add the cube mesh. */

   mesh = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_sphere_frame(mesh, 0, 100, tex_scale);

   material = eo_add(EVAS_3D_MATERIAL_CLASS, evas);

   texture_diffuse = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(texture_diffuse,
         evas_3d_texture_file_set(image_path, NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_LINEAR,
                                    EVAS_3D_TEXTURE_FILTER_LINEAR));
   eo_do(material,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE,
                                      texture_diffuse),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),

         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT, 0.01, 0.01, 0.01,
                                    1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE, 1.0, 1.0, 1.0,
                                    1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR, 1.0, 1.0, 1.0,
                                    1.0),
         evas_3d_material_shininess_set(50.0));

   mesh_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));

   eo_do(root_node, evas_3d_node_member_add(mesh_node));
   eo_do(mesh_node, evas_3d_node_mesh_add(mesh));
   eo_do(mesh, evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
               evas_3d_mesh_frame_material_set(0, material));

   /* Set up scene. */
   eo_do(scene,
         evas_3d_scene_root_node_set(root_node),
         evas_3d_scene_camera_node_set(camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));

   /* Add evas objects. */
   background = eo_add(EVAS_RECTANGLE_CLASS, evas);
   eo_do(background,
         efl_gfx_color_set(0, 0, 0, 255),
         efl_gfx_size_set(WIDTH, HEIGHT),
         efl_gfx_visible_set(EINA_TRUE));

   image = evas_object_image_filled_add(evas);
   eo_do(image,
         evas_obj_image_scene_set(scene),
         efl_gfx_size_set(WIDTH, HEIGHT),
         efl_gfx_visible_set(EINA_TRUE));
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN,
                                  _on_mouse_down, NULL);

   ecore_timer_add(0.01, _animate_scene, mesh_node);

   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
