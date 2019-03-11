/**
 * Example of picking nodes and meshes by screen coordinates using Evas-3D.
 *
 * Here shown which transformation should be applied to event_info of
 * _on_mouse_down to make them usable in evas_canvas3d_scene_pick()
 * and parameters which can be got from this function.
 *
 * @verbatim
 * gcc -o evas-3d-pick evas-3d-pick.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#include <math.h>
#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-common.h"

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
static Eo *sphere = NULL;

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;

   angle += 0.3;

   evas_canvas3d_node_orientation_angle_axis_set((Evas_Canvas3D_Node *)data, angle, 0.0, 1.0, 0.0);

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
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Real s, t;
   Evas_Canvas3D_Node *n;
   Evas_Canvas3D_Mesh *m;
   Eina_Bool pick;

   pick = evas_canvas3d_scene_pick(scene, ev->canvas.x, ev->canvas.y, &n, &m, &s, &t);

   if (pick) printf("Picked     : ");
   else printf("Not picked : ");

   printf("output(%d, %d) canvas(%d, %d) texcoord(%f, %f) "
          "node(%p) mesh(%p)\n",
          ev->output.x, ev->output.y,
          ev->canvas.x, ev->canvas.y,
          s, t, n, m);
}

int
main(void)
{
   // Unless Evas 3D supports Software renderer, we force use of the gl backend.
   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);

   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   /* Add a scene object .*/
   scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   /* Add the root node for the scene. */
   root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   /* Add the camera. */
   camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(camera, 30.0, 1.0, 1.0, 100.0);

   camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(camera_node, camera);
   evas_canvas3d_node_position_set(camera_node, 0.0, 0.0, 2.5);
   evas_canvas3d_node_look_at_set(camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(root_node, camera_node);

   sphere = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(sphere, EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE);
   evas_canvas3d_primitive_precision_set(sphere, 50);

   mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   evas_canvas3d_mesh_from_primitive_set(mesh, 0, sphere);

   material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   texture_diffuse = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(texture_diffuse, image_path, NULL);
   evas_canvas3d_texture_filter_set(texture_diffuse, EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR, EVAS_CANVAS3D_TEXTURE_FILTER_LINEAR);
   evas_canvas3d_material_texture_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, texture_diffuse);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.01, 0.01, 0.01, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(material, 50.0);

   mesh_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   evas_canvas3d_node_member_add(root_node, mesh_node);
   evas_canvas3d_node_mesh_add(mesh_node, mesh);
   evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_DIFFUSE);
   evas_canvas3d_mesh_frame_material_set(mesh, 0, material);

   /* Set up scene. */
   evas_canvas3d_scene_root_node_set(scene, root_node);
   evas_canvas3d_scene_camera_node_set(scene, camera_node);
   evas_canvas3d_scene_size_set(scene, WIDTH, HEIGHT);

   /* Add evas objects. */
   background = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_color_set(background, 0, 0, 0, 255);
   efl_gfx_entity_size_set(background, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(background, EINA_TRUE);

   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   efl_canvas_scene3d_set(image, scene);
   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);
   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN,
                                  _on_mouse_down, NULL);

   ecore_timer_add(0.01, _animate_scene, mesh_node);

   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
