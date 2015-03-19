/**
 * This example shows how to work frustum culling.
 * Use 'w'/'s' key to move far frustum plane from/to the camera.
 * Use 't'/'g' key to move near frustum plane from/to the camera.
 * Use '1'/'2' key to set camera to the first/second position.
 * Use '3'/'4'/'5' key to set bounding sphere/aabb/central point mode.
 * Use 'i', 'k', 'j', 'l', 'u' and 'o' keys to move mesh node (model) in 3D.
 * Use 'z', 'x', 'c', 'Z', 'X' and 'C' keys to change scaling constants of mesh (model).
 * See in terminal output value distance to far plane of frustum and value of visibility of node
 * @see evas_3d_camera_node_visible_get.
 *
 * @verbatim
 * gcc -o evas-3d-frustum evas-3d-frustum.c evas-3d-primitives.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Eo.h>
#include <math.h>
#include "evas-common.h"
#include "evas-3d-primitives.h"

#define  WIDTH          800
#define  HEIGHT         600

static const char *texture_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/sweet_home_reversed.png";
static const char *mesh_path = PACKAGE_EXAMPLES_DIR EVAS_MODEL_FOLDER "/sweet_home.obj";

typedef struct _Scene_Data
{
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;
   Eo *mesh_node_model;
   Eo *mesh_node;
   Eo *scene;
   Eo *camera;
   Eo *light;
   Eo *mesh_model;
   Eo *mesh_sphere;
   Eo *mesh_aabb;
   Eo *material_model;
   Eo *material;
   Eo *texture_model;
} Scene_Data;

Evas *evas;
Evas_Object *background,*image;
Evas_Real obj_x = 0.0, obj_y = 0.0, obj_z = 0.0, obj_sc_x = 10.0, obj_sc_y = 10.0, obj_sc_z = 10.0;
Evas_Real fleft = -5, fright = 5, fbottom = -5, fup = 5, fnear = 20, ffar = 1000;
Evas_Real radius = 0;
Evas_3D_Frustum_Mode key = EVAS_3D_FRUSTUM_MODE_AABB;
static const vec2 tex_scale = {1, 1};

static void
_show_help()
{
   fprintf(stdout, "Press 'w'/'s' key to move far frustum plane from/to the camera\n");
   fprintf(stdout, "Press 't'/'g' key to move near frustum plane from/to the camera\n");
   fprintf(stdout, "Press '1'/'2' key to set camera to the first/second position\n");
   fprintf(stdout, "Press '3'/'4'/'5' key to set bounding sphere/aabb/central point mode\n");
   fprintf(stdout, "Press 'i', 'k', 'j', 'l', 'u' and 'o' keys to move mesh node in 3D\n");
   fprintf(stdout, "Press 'z', 'x', 'c', 'Z', 'X' and 'C' keys to change scaling constants of mesh\n");
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

static Eina_Bool
_redraw_bounding_object(void *data)
{
   Evas_Real px, py, pz, sx, sy, sz;
   Evas_Real x0, y0, z0, x1, y1, z1, radius;
   Scene_Data *scene = (Scene_Data *)data;
   Eo *current_mesh;
   Eina_List *meshes = NULL, *l;

   eo_do(scene->mesh_node, meshes = (Eina_List *)evas_3d_node_mesh_list_get());
   EINA_LIST_FOREACH(meshes, l, current_mesh)
     {
        eo_do(scene->mesh_node, evas_3d_node_mesh_del(current_mesh));
     }

   switch (key)
     {
      case EVAS_3D_FRUSTUM_MODE_BSPHERE:
      {
         eo_do(scene->mesh_node_model,
               evas_3d_node_bounding_sphere_get(&x0, &y0, &z0, &radius));
         current_mesh = scene->mesh_sphere;
         px = x0;
         py = y0;
         pz = z0;
         sx = sy = sz = 2.0 * radius;
         break;
      }
      case EVAS_3D_FRUSTUM_MODE_AABB:
      {
         eo_do(scene->mesh_node_model,
               evas_3d_node_bounding_box_get(&x0, &y0, &z0,
                                             &x1, &y1, &z1));
         current_mesh = scene->mesh_aabb;
         px = (x1 + x0) / 2;
         py = (y1 + y0) / 2;
         pz = (z1 + z0) / 2;
         sx = x1 - x0;
         sy = y1 - y0;
         sz = z1 - z0;
         break;
      }
      case EVAS_3D_FRUSTUM_MODE_CENTRAL_POINT:
      {
         eo_do(scene->mesh_node_model,
               evas_3d_node_bounding_sphere_get(&x0, &y0, &z0, &radius));
         current_mesh = scene->mesh_sphere;
         px = x0;
         py = y0;
         pz = z0;
         sx = sy = sz = 0.1 * radius;
         break;
      }
      default:
         return ECORE_CALLBACK_RENEW;
     }

   eo_do(scene->mesh_node,
         evas_3d_node_mesh_add(current_mesh),
         evas_3d_node_position_set(px, py, pz),
         evas_3d_node_scale_set(sx, sy, sz));

   return ECORE_CALLBACK_RENEW;
}

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Scene_Data *scene = (Scene_Data *)data;
   Evas_Event_Key_Down *ev = event_info;
   Eina_Bool frustum;

   if (!strcmp("w", ev->key))
     {
        ffar += 20;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("s", ev->key))
     {
        ffar -= 20;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("t", ev->key))
     {
        fnear += 2;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("g", ev->key))
     {
        fnear -= 2;
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if(!strcmp("i", ev->key))
     {
        obj_y++;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("k", ev->key))
     {
        obj_y--;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("o", ev->key))
     {
        obj_z++;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("u", ev->key))
     {
        obj_z--;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("l", ev->key))
     {
        obj_x++;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("j", ev->key))
     {
        obj_x--;
        eo_do(scene->mesh_node_model,
              evas_3d_node_position_set(obj_x, obj_y, obj_z));
     }
   else if(!strcmp("z", ev->key))
     {
        obj_sc_y+=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("Z", ev->key))
     {
        obj_sc_y-=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("x", ev->key))
     {
        obj_sc_z+=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("X", ev->key))
     {
        obj_sc_z-=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("c", ev->key))
     {
        obj_sc_x+=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("C", ev->key))
     {
        obj_sc_x-=0.02;
        eo_do(scene->mesh_node_model,
              evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
     }
   else if(!strcmp("Return", ev->key))
     {
        eo_do(scene->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));
     }
   else if (!strcmp("1", ev->key))
     {
        eo_do(scene->camera_node, evas_3d_node_position_set(0.0, 0.0, 300.0),
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
        fprintf(stdout, "Changed position and view of camera\n");
     }
   else if (!strcmp("2", ev->key))
     {
        eo_do(scene->camera_node, evas_3d_node_position_set(300.0, 300.0, 300.0);
              evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
        fprintf(stdout, "Changed position and view of camera\n");
     }
   else if (!strcmp("3", ev->key))
     {
        key = EVAS_3D_FRUSTUM_MODE_BSPHERE;
     }
   else if (!strcmp("4", ev->key))
     {
        key = EVAS_3D_FRUSTUM_MODE_AABB;
     }
   else if (!strcmp("5", ev->key))
     {
        key = EVAS_3D_FRUSTUM_MODE_CENTRAL_POINT;
     }
   else
     {
        _show_help();
     }

   _redraw_bounding_object(data);
   eo_do(scene->camera, frustum = evas_3d_camera_node_visible_get(scene->camera_node, scene->mesh_node_model, key));
   fprintf(stdout, "far - %f frustum - %d radius - %f\n", ffar, frustum, radius);
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   data->camera_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                     evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(data->camera_node,
         evas_3d_node_camera_set(data->camera),
         evas_3d_node_position_set(0.0, 0.0, 300.0);
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));

   eo_do(data->camera, evas_3d_camera_projection_frustum_set(fleft, fright, fbottom, fup, fnear, ffar));

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
         evas_3d_node_position_set(0.0, 0.0, 200.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_3D_SPACE_PARENT, 0.0, 0.0, 1.0));

   eo_do(data->root_node, evas_3d_node_member_add(data->light_node));
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

   data->mesh_aabb = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_cube_frame(data->mesh_aabb, 0);
   eo_do(data->mesh_aabb,
          evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES),
          evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
          evas_3d_mesh_frame_material_set(0, data->material));

   data->mesh_sphere = eo_add(EVAS_3D_MESH_CLASS, evas);
   evas_3d_add_sphere_frame(data->mesh_sphere, 0, 20, tex_scale);
   eo_do(data->mesh_sphere,
          evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_LINES),
          evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE),
          evas_3d_mesh_frame_material_set(0, data->material));

   data->mesh_node =
      eo_add(EVAS_3D_NODE_CLASS, evas, evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node));
   eo_do(data->mesh_node, evas_3d_node_mesh_add(data->mesh_aabb));
}

static void
_mesh_setup_model(Scene_Data *data)
{
   data->mesh_model = eo_add(EVAS_3D_MESH_CLASS, evas);
   data->material_model = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   data->texture_model = eo_add(EVAS_3D_TEXTURE_CLASS, evas);

   eo_do(data->texture_model,
         evas_3d_texture_file_set(texture_path, NULL),
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
         efl_file_set(mesh_path, NULL),
         evas_3d_mesh_frame_material_set(0, data->material_model),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_DIFFUSE));
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   data->root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                                   evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   _light_setup(data);
   _mesh_setup_model(data);
   _camera_setup(data);

   data->mesh_node_model = eo_add(EVAS_3D_NODE_CLASS, evas,
                                      evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
   eo_do(data->mesh_node_model,
         evas_3d_node_position_set(obj_x, obj_y, obj_z),
         evas_3d_node_orientation_angle_axis_set(-90, 1.0, 0.0, 0.0),
         evas_3d_node_scale_set(obj_sc_x, obj_sc_y, obj_sc_z));
   eo_do(data->root_node, evas_3d_node_member_add(data->mesh_node_model));
   eo_do(data->mesh_node_model, evas_3d_node_mesh_add(data->mesh_model));

   _mesh_setup(data);

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
   Ecore_Evas *ecore_evas = NULL;

   if (!ecore_evas_init()) return 0;

   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
   ecore_evas = ecore_evas_new("opengl_x11", 0, 0, WIDTH, HEIGHT, NULL);

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

   _redraw_bounding_object(&data);
   _show_help();

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
