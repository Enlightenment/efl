/**
 * This example shows how to get and draw axis-aligned bounding box.
 *
 * @see _redraw_aabb();
 * Rotate axises (keys 1-4) for model and bounding box view from another angle.
 *
 * @verbatim
 * gcc -o evas-3d-aabb evas-3d-aabb.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
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
#include "evas-common.h"

#define  WIDTH 400
#define  HEIGHT 400

static const char *model_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sonic.md2";
static const char *image_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/sonic.png";

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Eo *background = NULL;
Eo *image = NULL;
Eo *cube = NULL;
Eo *scene = NULL;
Eo *root_node = NULL;
Eo *camera_node = NULL;
Eo *light_node = NULL;
Eo *camera = NULL;
Eo *mesh_node = NULL;
Eo *mesh_box_node = NULL;
Eo *mesh = NULL;
Eo *mesh_box = NULL;
Eo *material_box = NULL;
Eo *material = NULL;
Eo *texture = NULL;
Eo *light = NULL;


static Eina_Bool
_redraw_aabb();

static Eina_Bool
_animate_scene(void *data)
{
   static int frame = 0;

   eo_do((Evas_Canvas3D_Node *)data, evas_canvas3d_node_mesh_frame_set(mesh, frame));

   _redraw_aabb();

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
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Canvas3D_Node *node = (Evas_Canvas3D_Node *)data;

   switch(atoi(ev->key))
     {
      case 1:
        {
           eo_do(node, evas_canvas3d_node_orientation_angle_axis_set(90, 1.0, 0.0, 0.0));
           break;
        }
      case 2:
        {
           eo_do(node, evas_canvas3d_node_orientation_angle_axis_set(90, 0.0, 1.0, 0.0));
           break;
        }
      case 3:
        {
           eo_do(node, evas_canvas3d_node_orientation_angle_axis_set(90, 0.0, 0.0, 1.0));
           break;
        }
      case 4:
        {
           eo_do(node, evas_canvas3d_node_orientation_angle_axis_set(90, 1.0, 1.0, 0.0));
           break;
        }
     }
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   eo_do(background, efl_gfx_size_set(w, h));
   eo_do(image, efl_gfx_size_set(w, h));
}

static Eina_Bool
_redraw_aabb()
{
   Evas_Real x0, y0, z0, x1, y1, z1;

   eo_do(mesh_node, evas_canvas3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));
   eo_do(mesh_box_node, evas_canvas3d_node_position_set((x0 + x1)/2, (y0 + y1)/2, (z0 + z1)/2),
                        evas_canvas3d_node_scale_set(x1 - x0, y1 - y0, z1 - z0));

   return EINA_TRUE;
}

int
main(void)
{
   if (!ecore_evas_init()) return 0;

   ecore_evas = ecore_evas_new("opengl_x11", 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   scene = eo_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   root_node = eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                             evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_NODE));

   camera = eo_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   eo_do(camera,
         evas_canvas3d_camera_projection_perspective_set(60.0, 1.0, 1.0, 500.0));

   camera_node =
      eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   eo_do(camera_node,
         evas_canvas3d_node_camera_set(camera));
   eo_do(root_node,
         evas_canvas3d_node_member_add(camera_node));
   eo_do(camera_node,
         evas_canvas3d_node_position_set(100.0, 50.0, 20.0),
         evas_canvas3d_node_look_at_set(EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 20.0,
                                  EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 1.0));
   light = eo_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   eo_do(light,
         evas_canvas3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_light_directional_set(EINA_TRUE));

   light_node =
      eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                    evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   eo_do(light_node,
         evas_canvas3d_node_light_set(light),
         evas_canvas3d_node_position_set(1000.0, 0.0, 1000.0),
         evas_canvas3d_node_look_at_set(EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   eo_do(root_node,
         evas_canvas3d_node_member_add(light_node));

   mesh = eo_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   material = eo_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   eo_do(mesh,
         efl_file_set(model_path, NULL),
         evas_canvas3d_mesh_frame_material_set(0, material),
         evas_canvas3d_mesh_shade_mode_set(EVAS_CANVAS3D_SHADE_MODE_PHONG));

   texture = eo_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   eo_do(texture,
         evas_canvas3d_texture_file_set(image_path, NULL),
         evas_canvas3d_texture_filter_set(EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST),
         evas_canvas3d_texture_wrap_set(EVAS_CANVAS3D_WRAP_MODE_REPEAT,
                                  EVAS_CANVAS3D_WRAP_MODE_REPEAT));
   eo_do(material,
         evas_canvas3d_material_texture_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, texture),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_AMBIENT,
                                    0.01, 0.01, 0.01, 1.0),
         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE,
                                    1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_material_color_set(EVAS_CANVAS3D_MATERIAL_SPECULAR,
                                    1.0, 1.0, 1.0, 1.0),
         evas_canvas3d_material_shininess_set(50.0));

   mesh_node = eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                             evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_MESH));
   mesh_box_node = eo_add(EVAS_CANVAS3D_NODE_CLASS, evas,
                                 evas_canvas3d_node_constructor(EVAS_CANVAS3D_NODE_TYPE_MESH));

   material_box = eo_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   eo_do(material_box, evas_canvas3d_material_enable_set(EVAS_CANVAS3D_MATERIAL_DIFFUSE, EINA_TRUE));

   cube = eo_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   eo_do(cube,
         evas_canvas3d_primitive_form_set(EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE));

   mesh_box = eo_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   eo_do(mesh_box,
         evas_canvas3d_mesh_from_primitive_set(0, cube),
         evas_canvas3d_mesh_vertex_assembly_set(EVAS_CANVAS3D_VERTEX_ASSEMBLY_LINES),
         evas_canvas3d_mesh_shade_mode_set(EVAS_CANVAS3D_SHADE_MODE_VERTEX_COLOR),
         evas_canvas3d_mesh_frame_material_set(0, material_box));
    _redraw_aabb();

   eo_do(root_node,
         evas_canvas3d_node_member_add(mesh_box_node));
   eo_do(mesh_box_node,
         evas_canvas3d_node_mesh_add(mesh_box));

   eo_do(root_node,
         evas_canvas3d_node_member_add(mesh_node));
   eo_do(mesh_node,
         evas_canvas3d_node_mesh_add(mesh));

   eo_do(scene,
         evas_canvas3d_scene_root_node_set(root_node),
         evas_canvas3d_scene_camera_node_set(camera_node),
         evas_canvas3d_scene_size_set(WIDTH, HEIGHT));

   background = eo_add(EVAS_RECTANGLE_CLASS, evas);
   eo_do(background,
         efl_gfx_color_set(0, 0, 0, 255),
         efl_gfx_size_set(WIDTH, HEIGHT),
         efl_gfx_visible_set(EINA_TRUE));

   image = evas_object_image_filled_add(evas);
   eo_do(image,
         efl_gfx_size_set(WIDTH, HEIGHT),
         efl_gfx_visible_set(EINA_TRUE));
   evas_object_focus_set(image, EINA_TRUE);
   eo_do(image, evas_obj_image_scene_set(scene));

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, root_node);

   ecore_timer_add(0.01, _animate_scene, mesh_node);

   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
