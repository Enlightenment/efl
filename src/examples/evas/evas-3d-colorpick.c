/**
 * Example of finding nodes by color or geometry in an Evas-3D scene.
 *
 * Enable color pick mode of meshes and scene objects.
 * Click (left for color pick, right for geometry pick) on 3D object and see
 * in terminal time response of the found node.
 * Use key UP/DOWN for changing z coordinate of camera
 * Use key LEFT/RIGHT for scale each node
 * Use shortcut parameters of commandline: r - rows of objects, c - columns of objects,
 * p - precision of the spheres, t - path for a texture.
 *
 * @verbatim
 * gcc -o evas-3d-colorpick evas-3d-colorpick.c `pkg-config --libs --cflags evas ecore ecore-evas eo eina efl` -lm
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
#include <Ecore_Getopt.h>
#include <math.h>
#include "evas-common.h"

#define  WIDTH 800
#define  HEIGHT 600

#define INIT_SCALE 7.5
#define BIG_SCALE 10.0
#define SMALL_SCALE 5.0
#define SCALE_UNIT 0.5
#define VEC_3(value) value, value, value

static const char *image_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/wood.jpg";

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Evas_Object *image = NULL, *bg = NULL;

static const
Ecore_Getopt optdesc = {
   "",
   NULL,
   "0.0",
   "",
   "",
   "Example mesh color pick mechanism",
   0,
   {
      ECORE_GETOPT_STORE_INT('r', "row", "Rows of spheres"),
      ECORE_GETOPT_STORE_INT('c', "column", "Columns of spheres"),
      ECORE_GETOPT_STORE_INT('p', "precision", "Precision of spheres"),
      ECORE_GETOPT_STORE_STR('t', "texture", "Name of texture"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
    }
};

typedef struct _Object
{
   Eo *primitive;
   Eo *node;
   Eo *mesh;
   Eo *material;
   Eo *texture;

   Evas_Real speed;
   Ecore_Timer *animate;
   Eina_Bool (*sphere_init)(void *sphere, const char *texture);
   Eina_Bool (*sphere_animate)(void *data);

} Test_object;

typedef struct _Scene
{
   Eo *camera;
   Eo *light;
   Eo *scene;
   Eo *root_node;
   Eo *camera_node;
   Eo *light_node;

   int row;
   int col;
   int precision;

   Eina_List *spheres;

   Eina_Bool (*scene_init)(const char *texture);

} Scene_Data;

Scene_Data globalscene;

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   Eina_List *l;
   Test_object * item;
   EINA_LIST_FOREACH(globalscene.spheres, l, item)
     {
        free(item);
     }
   eina_list_free(globalscene.spheres);

   ecore_main_loop_quit();
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   efl_gfx_entity_size_set(bg, EINA_SIZE2D(w,  h));
   efl_gfx_entity_size_set(image, EINA_SIZE2D(w,  h));
}

static Eina_Bool
_animate_sphere(void *data)
{
   static int angle = 0.0;
   Eo *n = (Eo*)(((Test_object *)data)->node);
   Evas_Real s = ((Test_object *)data)->speed;

   evas_canvas3d_node_orientation_angle_axis_set(n, angle, s, s - 1, s + 1);
   angle++;
   if (angle > 360) angle = 0.0;

   return EINA_TRUE;
}

void _recalculate_position()
{
   int i = 0, j = 0, count = 0;
   Evas_Real x1, y1, shiftx = 0, shifty = 0;
   Test_object *m;

   evas_canvas3d_object_update(((Test_object *)eina_list_nth(globalscene.spheres, 0))->node);
   evas_canvas3d_node_bounding_box_get(((Test_object *)eina_list_nth(globalscene.spheres, 0))->node, NULL, NULL, NULL, &x1, &y1, NULL);

   for (i = 0; i < globalscene.row; ++i)
     {
        shiftx = (i * 2 - globalscene.row) * x1;
        for(j = 0; j < globalscene.col; ++j)
          {
              shifty = (j * 2 - globalscene.col) * y1;
              m = (Test_object *)eina_list_nth(globalscene.spheres, count);
              evas_canvas3d_node_position_set(m->node, shifty, 0.0, shiftx);
              count++;
          }
     }
}

static void
_on_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
             Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Evas_Real x, y, z;
   Eina_List *l;
   Test_object * item;

   if (!strcmp(ev->key, "Down"))
     {
        evas_canvas3d_node_position_get(globalscene.camera_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(globalscene.camera_node, x, y, (z - 5));
     }
   else if (!strcmp(ev->key, "Up"))
     {
        evas_canvas3d_node_position_get(globalscene.camera_node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        evas_canvas3d_node_position_set(globalscene.camera_node, x, y, (z + 5));
     }
   else if (!strcmp(ev->key, "Left"))
     {
        evas_canvas3d_node_scale_get(((Test_object *)eina_list_nth(globalscene.spheres, 0))->node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        EINA_LIST_FOREACH(globalscene.spheres, l, item)
          {
             evas_canvas3d_node_scale_set(item->node, (x - SCALE_UNIT), (y - SCALE_UNIT), (z - SCALE_UNIT));
          }
        _recalculate_position();
     }
   else if (!strcmp(ev->key, "Right"))
     {
        evas_canvas3d_node_scale_get(((Test_object *)eina_list_nth(globalscene.spheres, 0))->node, EVAS_CANVAS3D_SPACE_PARENT, &x, &y, &z);
        EINA_LIST_FOREACH(globalscene.spheres, l, item)
          {
             evas_canvas3d_node_scale_set(item->node, (x + SCALE_UNIT), (y + SCALE_UNIT), (z + SCALE_UNIT));
          }
        _recalculate_position();
     }
   else
     printf("Down, Up for position of camera; Left, Right for scale of each node");
}

static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Canvas3D_Node *n;
   Evas_Canvas3D_Mesh *m;
   Evas_Real s, t;
   clock_t time;
   float diff_sec;

   time = clock();
   Eina_Bool flag;
   if (ev->button == 1)
     {
        evas_canvas3d_scene_color_pick_enable_set(globalscene.scene, EINA_TRUE);
        flag = evas_canvas3d_scene_pick(globalscene.scene, ev->canvas.x, ev->canvas.y, &n, &m, NULL, NULL);
        time = clock() - time;
        diff_sec = ((float)time) / CLOCKS_PER_SEC / 10;
        if (flag)
          {
             printf("Boom! Time expended for color pick: %2.7f .\n", diff_sec);
             evas_canvas3d_node_scale_set(n, VEC_3(SMALL_SCALE));
          }
     }
   else
     {
        evas_canvas3d_scene_color_pick_enable_set(globalscene.scene, EINA_FALSE);
        flag = evas_canvas3d_scene_pick(globalscene.scene, ev->canvas.x, ev->canvas.y, &n, &m, &s, &t);
        time = clock() - time;
        diff_sec = ((float)time) / CLOCKS_PER_SEC / 10;
        if (flag)
          {
             printf("Boom! Time expended for geometry pick: %2.7f .\n", diff_sec);
             evas_canvas3d_node_scale_set(n, VEC_3(BIG_SCALE));
          }
      }
}

Eina_Bool
_init_sphere(void *this, const char *texture)
{
   Test_object *sphere  = (Test_object *)this;

   sphere->primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, evas);
   sphere->mesh = efl_add(EVAS_CANVAS3D_MESH_CLASS, evas);
   sphere->material = efl_add(EVAS_CANVAS3D_MATERIAL_CLASS, evas);
   evas_canvas3d_primitive_form_set(sphere->primitive, EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE);
   evas_canvas3d_primitive_precision_set(sphere->primitive, 50);
   evas_canvas3d_mesh_from_primitive_set(sphere->mesh, 0, sphere->primitive);
   evas_canvas3d_mesh_frame_material_set(sphere->mesh, 0, sphere->material);
   evas_canvas3d_mesh_shader_mode_set(sphere->mesh, EVAS_CANVAS3D_SHADER_MODE_PHONG);

   evas_canvas3d_mesh_color_pick_enable_set(sphere->mesh, EINA_TRUE);

   sphere->texture = efl_add(EVAS_CANVAS3D_TEXTURE_CLASS, evas);
   efl_file_simple_load(sphere->texture, texture, NULL);
   evas_canvas3d_texture_filter_set(sphere->texture, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST, EVAS_CANVAS3D_TEXTURE_FILTER_NEAREST);
   evas_canvas3d_texture_wrap_set(sphere->texture, EVAS_CANVAS3D_WRAP_MODE_REPEAT, EVAS_CANVAS3D_WRAP_MODE_REPEAT);
   evas_canvas3d_material_texture_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, sphere->texture);
   evas_canvas3d_material_enable_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
   evas_canvas3d_material_enable_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
   evas_canvas3d_material_enable_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
   evas_canvas3d_material_enable_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, EINA_TRUE);
   evas_canvas3d_material_color_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, 0.01, 0.01, 0.01, 1.0);
   evas_canvas3d_material_color_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_color_set(sphere->material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_material_shininess_set(sphere->material, 50.0);

   return EINA_TRUE;
}

Eina_Bool
_init_scene(const char *texture)
{
   int i = 0;
   Evas_Real tmp;
   Test_object *m;

   globalscene.scene = efl_add(EVAS_CANVAS3D_SCENE_CLASS, evas);

   globalscene.root_node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_NODE));

   globalscene.camera = efl_add(EVAS_CANVAS3D_CAMERA_CLASS, evas);
   evas_canvas3d_camera_projection_perspective_set(globalscene.camera, 30.0, 1.0, 1.0, 1000.0);

   globalscene.camera_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_CAMERA));
   evas_canvas3d_node_camera_set(globalscene.camera_node, globalscene.camera);
   evas_canvas3d_node_member_add(globalscene.root_node, globalscene.camera_node);
   evas_canvas3d_node_position_set(globalscene.camera_node, 0.0, 30.0, 800.0);
   evas_canvas3d_node_look_at_set(globalscene.camera_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, -1000.0, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);
   globalscene.light = efl_add(EVAS_CANVAS3D_LIGHT_CLASS, evas);
   evas_canvas3d_light_ambient_set(globalscene.light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_diffuse_set(globalscene.light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_specular_set(globalscene.light, 1.0, 1.0, 1.0, 1.0);
   evas_canvas3d_light_directional_set(globalscene.light, EINA_TRUE);

   globalscene.light_node =
      efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_LIGHT));
   evas_canvas3d_node_light_set(globalscene.light_node, globalscene.light);
   evas_canvas3d_node_position_set(globalscene.light_node, 100.0, 50.0, 300.0);
   evas_canvas3d_node_look_at_set(globalscene.light_node, EVAS_CANVAS3D_SPACE_PARENT, 0.0, 0.0, 0.0, EVAS_CANVAS3D_SPACE_PARENT, 1.0, 1.0, 1.0);
   evas_canvas3d_node_member_add(globalscene.root_node, globalscene.light_node);

   tmp = 0.01;
   for (i = 0; i < globalscene.col * globalscene.row; i++)
     {
        Test_object *spheretmp;

        m = malloc(sizeof(Test_object));
        m->sphere_init = _init_sphere;
        if (!i)
          m->sphere_init(m, texture);
        else
          {
             spheretmp = (Test_object *)eina_list_nth(globalscene.spheres, 0);
             m->mesh = spheretmp->mesh;
             m->material = spheretmp->material;
             m->texture = spheretmp->texture;
          }
        m->node = efl_add(EVAS_CANVAS3D_NODE_CLASS, evas, evas_canvas3d_node_type_set(efl_added, EVAS_CANVAS3D_NODE_TYPE_MESH));
        m->speed = tmp;
        m->sphere_animate = _animate_sphere;
        m->animate = ecore_timer_add(m->speed, m->sphere_animate, m);
        evas_canvas3d_node_member_add(globalscene.root_node, m->node);
        evas_canvas3d_node_mesh_add(m->node, m->mesh);
        evas_canvas3d_node_scale_set(m->node, VEC_3(INIT_SCALE));
        globalscene.spheres = eina_list_append(globalscene.spheres, m);
        tmp += 0.01;
     }

   _recalculate_position();

   evas_canvas3d_scene_root_node_set(globalscene.scene, globalscene.root_node);
   evas_canvas3d_scene_camera_node_set(globalscene.scene, globalscene.camera_node);
   evas_canvas3d_scene_size_set(globalscene.scene, WIDTH, HEIGHT);

   return EINA_TRUE;
}

int main(int argc, char **argv)
{
   int row = 0, col = 0, precision = 0;
   char *texture = NULL;
   Eina_Bool r;

   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_INT(row),
      ECORE_GETOPT_VALUE_INT(col),
      ECORE_GETOPT_VALUE_INT(precision),
      ECORE_GETOPT_VALUE_STR(texture),
      ECORE_GETOPT_VALUE_NONE
   };

   if (!ecore_evas_init()) return 0;
   if (!ecore_init()) return 0;

   ecore_getopt_parse(&optdesc, values, argc, argv);

   if (!row) row = 2;
   if (!col) col = 5;
   if (!precision) precision = 30;
   if (!texture) texture = (char *)image_path;

   printf("row - %d, col - %d, precision of spheres - %d, texture - %s\n",
          row, col, precision, texture);

   ecore_evas = ecore_evas_new("opengl_x11", 10, 10, WIDTH, HEIGHT, NULL);

   if (!ecore_evas) return 0;

   ecore_evas_callback_delete_request_set(ecore_evas, _on_delete);
   ecore_evas_callback_resize_set(ecore_evas, _on_canvas_resize);
   ecore_evas_show(ecore_evas);

   evas = ecore_evas_get(ecore_evas);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 100, 100, 100, 255);
   evas_object_move(bg, 0, 0);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_show(bg);

   globalscene.scene_init = _init_scene;
   globalscene.row = row;
   globalscene.col = col;
   globalscene.precision = precision;
   globalscene.scene_init(texture);

   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);

   efl_gfx_entity_size_set(image, EINA_SIZE2D(WIDTH,  HEIGHT));
   efl_gfx_entity_visible_set(image, EINA_TRUE);
   evas_object_focus_set(image, EINA_TRUE);
   efl_canvas_scene3d_set(image, globalscene.scene);

   r = evas_object_key_grab(image, "Down", 0, 0, EINA_TRUE);
   r = evas_object_key_grab(image, "Up", 0, 0, EINA_TRUE);
   r = evas_object_key_grab(image, "Left", 0, 0, EINA_TRUE);
   r = evas_object_key_grab(image, "Right", 0, 0, EINA_TRUE);

   (void) r;

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, NULL);
   evas_object_event_callback_add(image, EVAS_CALLBACK_KEY_DOWN, _on_key_down, NULL);

   ecore_main_loop_begin();

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();

   return 0;
}
