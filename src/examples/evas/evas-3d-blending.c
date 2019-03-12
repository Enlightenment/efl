/**
 * Example of different blending modes in Evas-3D.
 *
 * Press "Up" or "Down" key for change source blending factor.
 * Press "Left" or "Right" for change destination blending factors.
 * For more details see https://www.opengl.org/sdk/docs/man2/xhtml/glBlendFunc.xml
 *
 * @verbatim
 * gcc -o evas-3d-blending evas-3d-blending.c `pkg-config --libs --cflags efl evas ecore ecore-evas eo` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#define  WIDTH          1024
#define  HEIGHT         1024

static const char *blend_func_names[] =
{
   /**< The scale factors for color components is (0, 0, 0, 0)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ZERO",
   /**< The scale factors for color components is (1, 1, 1, 1)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ONE",
   /**< The scale factors for color components is (Rs/kR, Gs/kG, Bs/kB, As/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_SRC_COLOR",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Rs/kR, Gs/kG, Bs/kB, As/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_COLOR",
   /**< The scale factors for color components is (Rd/kR, Gd/kG, Bd/kB, Ad/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_DST_COLOR",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Rd/kR, Gd/kG, Bd/kB, Ad/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_DST_COLOR",
   /**< The scale factors for color components is (As/kA, As/kA, As/kA, As/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA",
   /**< The scale factors for color components is (1, 1, 1, 1) - (As/kA, As/kA, As/kA, As/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_ALPHA",
   /**< The scale factors for color components is (Ad/kA, Ad/kA, Ad/kA, Ad/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_DST_ALPHA",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Ad/kA, Ad/kA, Ad/kA, Ad/kA)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_DST_ALPHA",
   /**< The scale factors for color components is (Rc, Gc, Bc, Ac)*/
   "EVAS_CANVAS3D_BLEND_FUNC_CONSTANT_COLOR",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Rc, Gc, Bc, Ac)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR",
   /**< The scale factors for color components is (Ac, Ac, Ac, Ac)*/
   "EVAS_CANVAS3D_BLEND_FUNC_CONSTANT_ALPHA",
   /**< The scale factors for color components is (1, 1, 1, 1) - (Ac, Ac, Ac, Ac)*/
   "EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA",
   /**< The scale factors for color components is (i, i, i, 1) where i = min(As, kA, Ad)/kA*/
   "EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA_SATURATE",
};

typedef struct _Scene_Data
{
   Eo     *scene;
   Eo     *root_node;
   Eo     *camera_node;
   Eo     *light_node;
   Eo     *mesh_node;
   Eo     *mesh_node1;

   Eo     *camera;
   Eo     *light;
   Eo     *mesh;
   Eo     *mesh1;
   Eo     *sphere;
   Eo     *material;
   Eo     *material1;
} Scene_Data;

Evas             *evas        = NULL;
Eo               *background  = NULL;
Eo               *image       = NULL;

int func1 = EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA;
int func2 = EVAS_CANVAS3D_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Scene_Data *scene = (Scene_Data *)data;
   Evas_Event_Key_Down *ev = event_info;

   if (!strcmp("Up", ev->key))
     func1++;
   if (!strcmp("Down", ev->key))
     func1--;
   if (!strcmp("Left", ev->key))
     func2++;
   if (!strcmp("Right", ev->key))
     func2--;

   if(func1 < EVAS_CANVAS3D_BLEND_FUNC_ZERO)
     func1 = EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA_SATURATE;
   if(func1 > EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA_SATURATE)
     func1 = EVAS_CANVAS3D_BLEND_FUNC_ZERO;

   if(func2 < EVAS_CANVAS3D_BLEND_FUNC_ZERO)
     func2 = EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA_SATURATE;
   if(func2 > EVAS_CANVAS3D_BLEND_FUNC_SRC_ALPHA_SATURATE)
     func2 = EVAS_CANVAS3D_BLEND_FUNC_ZERO;
   evas_canvas3d_mesh_blending_func_set(scene->mesh1, func1, func2);
   printf("sfactor = %s, dfactor = %s\n", blend_func_names[func1], blend_func_names[func2]);
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
   efl_gfx_entity_size_set(background, EINA_SIZE2D(w,  h));
   efl_gfx_entity_size_set(image, EINA_SIZE2D(w,  h));
}

static Eina_Bool
_animate_scene(void *data)
{
   static float angle = 0.0f;
   Scene_Data *scene = (Scene_Data *)data;

   angle += 0.02;

   evas_canvas3d_node_position_set(scene->mesh_node, 3 * sin(angle), 0.0, 3 * cos(angle));

   if (angle > 2 * M_PI) angle = 0;

   return EINA_TRUE;
}

static void
_camera_setup(Scene_Data *data)
{
   data->camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);

   evas_canvas3d_camera_projection_perspective_set(data->camera, 60.0, 1.0, 2.0, 50.0);

   data->camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));

   evas_canvas3d_node_camera_set(data->camera_node, data->camera);
   evas_canvas3d_node_position_set(data->camera_node, 0.0, 0.0, 10.0);
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

   data->light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(data->light_node, data->light);
   evas_canvas3d_node_position_set(data->light_node, 0.0, 0.0, 10.0);
   evas_canvas3d_node_look_at_set(data->light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   evas_canvas3d_node_member_add(data->root_node, data->light_node);
}

static void
_set_ball(Eo *mesh, Eo *sphere, Evas_Canvas3D_Material *material)
{
   evas_canvas3d_mesh_from_primitive_set(mesh, 0, sphere);
   evas_canvas3d_mesh_frame_material_set(mesh, 0, material);
   evas_canvas3d_mesh_shader_mode_set(mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);
}

static void
_mesh_setup(Scene_Data *data)
{
   data->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 1.0, 0.2, 0.2, 0.2);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 0.0, 0.0, 0.2);
   evas_canvas3d_material_color_set(data->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 0.2);
   evas_canvas3d_material_shininess_set(data->material, 100.0);

   data->material1 = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);

   evas_canvas3d_material_enable_set(data->material1, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material1, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(data->material1, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_color_set(data->material1, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.0, 0.2, 0.2, 0.2);
   evas_canvas3d_material_color_set(data->material1, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 0.0, 0.0, 1.0, 0.2);
   evas_canvas3d_material_color_set(data->material1, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 0.2);
   evas_canvas3d_material_shininess_set(data->material1, 100.0);

   data->sphere = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   evas_canvas3d_primitive_form_set(data->sphere, EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE);
   evas_canvas3d_primitive_precision_set(data->sphere, 50);

   data->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   data->mesh1 = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);

   _set_ball(data->mesh, data->sphere, data->material);
   _set_ball(data->mesh1, data->sphere, data->material1);

   data->mesh_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   evas_canvas3d_node_member_add(data->root_node, data->mesh_node);
   evas_canvas3d_node_mesh_add(data->mesh_node, data->mesh);

   data->mesh_node1 =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));

   evas_canvas3d_node_member_add(data->root_node, data->mesh_node1);
   evas_canvas3d_node_mesh_add(data->mesh_node1, data->mesh1);

   evas_canvas3d_mesh_blending_enable_set(data->mesh1, EINA_TRUE);
   evas_canvas3d_mesh_blending_func_set(data->mesh1, func1, func2);

   evas_canvas3d_node_scale_set(data->mesh_node, 2.0, 2.0, 2.0);

   evas_canvas3d_node_scale_set(data->mesh_node1, 5.0, 5.0, 5.0);
}

static void
_scene_setup(Scene_Data *data)
{
   data->scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   evas_canvas3d_scene_size_set(data->scene, WIDTH, HEIGHT);
   evas_canvas3d_scene_background_color_set(data->scene, 0.5, 0.5, 0.5, 1);

   data->root_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   _camera_setup(data);
   _light_setup(data);
   _mesh_setup(data);

   evas_canvas3d_scene_root_node_set(data->scene, data->root_node);
   evas_canvas3d_scene_camera_node_set(data->scene, data->camera_node);
}

int
main(void)
{
   Scene_Data data;
   Ecore_Animator *anim;
   Ecore_Evas *ecore_evas = NULL;

   if (!ecore_evas_init()) return 0;

   setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
   ecore_evas = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   _scene_setup(&data);

   /* Add a background rectangle object. */
   background = evas_object_rectangle_add(evas);
   evas_object_color_set(background, 0, 0, 0, 255);
   evas_object_move(background, 0, 0);
   evas_object_resize(background, WIDTH, HEIGHT);
   evas_object_show(background);

   /* Add an image object for 3D scene rendering. */
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);
   evas_object_move(image, 0, 0);
   evas_object_resize(image, WIDTH, HEIGHT);
   evas_object_show(image);
   evas_object_focus_set(image, EINA_TRUE);

   /* Set the image object as render target for 3D scene. */
   efl_canvas_scene3d_set(image, data.scene);

   /* Add animation timer callback. */
   ecore_animator_frametime_set(0.008);
   anim = ecore_animator_add(_animate_scene, &data);

   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, &data);
   /* Enter main loop. */
   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
   ecore_animator_del(anim);

   return 0;
}
