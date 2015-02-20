/*
 * This example shows how to use color pick algorithm for finding node at scene.
 * Enable color pick mode of meshes and scene objects.
 * Click (left for color pick, right for geometry pick) on 3D object and see
 * in terminal time response of the found node.
 * Use key UP/DOWN for changing z coordinate of camera
 * Use key LEFT/RIGHT for scale each node
 * Use shortcut parameters of commanline: r - rows of objects, c - columns of objects, m - path for model name,
 * f - path for first texture, s - path for second texture.
 * Compile with "gcc -o evas-3d-colorpick evas-3d-colorpick.c `pkg-config --libs --cflags evas ecore ecore-evas eo eina efl`"
 */
//TODO new resources

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Getopt.h>
#include "evas-3d-common.h"

#define  WIDTH 800
#define  HEIGHT 600

#define ANIMATION_COUNT 3
#define MAX_PATH 128

static const char *model_path = PACKAGE_EXAMPLES_DIR EVAS_3D_MODEL_FOLDER "/M15.obj";
static const char *image1_path = PACKAGE_EXAMPLES_DIR EVAS_3D_IMAGE_FOLDER "/M15.png";
static const char *image2_path = PACKAGE_EXAMPLES_DIR EVAS_3D_IMAGE_FOLDER "/M15_1.png";

Ecore_Evas *ecore_evas = NULL;
Evas *evas = NULL;
Evas_Object *image = NULL, *bg = NULL;

Eina_Bool (*animate_func[ANIMATION_COUNT])(void*);

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
      ECORE_GETOPT_STORE_INT('r', "row", "Rows of models"),
      ECORE_GETOPT_STORE_INT('c', "column", "Columns of models"),
      ECORE_GETOPT_STORE_STR('m', "name model", "Name of model"),
      ECORE_GETOPT_STORE_STR('f', "texture1", "Name1 of texture"),
      ECORE_GETOPT_STORE_STR('s', "texture2", "Name2 of texture"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
    }
};
 typedef struct _Mine
 {
   Eo *node;
   Eo *mesh1;
   Eo *mesh2;
   Eo *material1;
   Eo *material2;
   Eo *texture1;
   Eo *texture2;

   Evas_Real speed;
   Ecore_Timer *animate;
   Eina_Bool (*m15_init)(void *m15, const char *model,
                         const char *texture1, const char *texture2);
   Eina_Bool (*m15_animate)(void *data);

 } MineM15;

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

   Eina_List *mines;

   Eina_Bool (*scene_init)(const char *model,
                           const char *texture1, const char *texture2);

} Scene_Data;

Scene_Data globalscene;

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   Eina_List *l;
   MineM15 * item;
   EINA_LIST_FOREACH(globalscene.mines, l, item)
     {
        free(item);
     }
   eina_list_free(globalscene.mines);


   ecore_main_loop_quit();
}
static void
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   eo_do(bg, evas_obj_size_set(w, h));
   eo_do(image, evas_obj_size_set(w, h));
}

static Eina_Bool
_animate_mine1(void *data)
{
   static int angle = 0.0;
   Eo *n = (Eo*)(((MineM15 *)data)->node);
   eo_do(n, evas_3d_node_orientation_angle_axis_set(angle, 1.0, 0.0, 1.0));
   angle++;
   if (angle > 360) angle = 0.0;
   return EINA_TRUE;
}
static Eina_Bool
_animate_mine2(void *data)
{
   static int angle = 0.0;
   Eo *n = (Eo*)(((MineM15 *)data)->node);
   eo_do(n, evas_3d_node_orientation_angle_axis_set(angle, 0.0, 1.0, 1.0));
   angle++;
   if (angle > 360) angle = 0.0;
   return EINA_TRUE;
}
static Eina_Bool
_animate_mine3(void *data)
{
   static int angle = 0.0;
   Eo *n = (Eo*)(((MineM15 *)data)->node);
   eo_do(n, evas_3d_node_orientation_angle_axis_set(angle, 1.0, 1.0, 0.0));
   angle++;
   if (angle > 360) angle = 0.0;
   return EINA_TRUE;
}

void _recalculate_position()
{
   int i = 0, j = 0, count = 0;
   Evas_Real x0, y0, z0, x1, y1, z1, shiftx = 0, shifty = 0;
   MineM15 *m;

   eo_do(((MineM15 *)eina_list_nth(globalscene.mines, 0))->node, evas_3d_object_update());
   eo_do(((MineM15 *)eina_list_nth(globalscene.mines, 0))->node,
         evas_3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));

   for (i = 0; i < globalscene.row; ++i)
     {
        shiftx = i * 2 * x1;
        for(j = 0; j < globalscene.col; ++j)
          {
              shifty = j * 2 * y1;
              m = (MineM15 *)eina_list_nth(globalscene.mines, count);
              eo_do(m->node, evas_3d_node_position_set(shifty, 0.0, shiftx));
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
   MineM15 * item;
   if (!strcmp(ev->key, "Down"))
     {
        eo_do(globalscene.camera_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(globalscene.camera_node, evas_3d_node_position_set(x, y, (z - 5)));
     }
   else if (!strcmp(ev->key, "Up"))
     {
        eo_do(globalscene.camera_node, evas_3d_node_position_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        eo_do(globalscene.camera_node, evas_3d_node_position_set(x, y, (z + 5)));
     }
   else if (!strcmp(ev->key, "Left"))
     {
        eo_do(((MineM15 *)eina_list_nth(globalscene.mines, 0))->node,
              evas_3d_node_scale_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        EINA_LIST_FOREACH(globalscene.mines, l, item)
          {
             eo_do(item->node, evas_3d_node_scale_set((x - 0.1), (y - 0.1), (z - 0.1)));
          }
        _recalculate_position();
     }
   else if (!strcmp(ev->key, "Right"))
     {
        eo_do(((MineM15 *)eina_list_nth(globalscene.mines, 0))->node,
              evas_3d_node_scale_get(EVAS_3D_SPACE_PARENT, &x, &y, &z));
        EINA_LIST_FOREACH(globalscene.mines, l, item)
          {
             eo_do(item->node, evas_3d_node_scale_set((x + 0.1), (y + 0.1), (z + 0.1)));
          }
        _recalculate_position();
     }
   else
     fprintf(stdout, "Down, Up for position of camera; Left, Right for scale of each node");
}
static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *eo EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_3D_Node *n;
   Evas_3D_Mesh *m;
   Evas_Real s, t;
   clock_t time;
   float diff_sec;

   time = clock();
   Eina_Bool flag;
   if (ev->button == 1)
     {
        eo_do(globalscene.scene, evas_3d_scene_color_pick_enable_set(EINA_TRUE));
        eo_do(globalscene.scene,
              flag = evas_3d_scene_pick(ev->canvas.x, ev->canvas.y, &n, &m, NULL, NULL));
        time = clock() - time;
        diff_sec = ((float)time) / CLOCKS_PER_SEC / 10;
        if (flag)
          {
             fprintf(stdout, "Boom! Time expended for color pick: %2.7f .\n", diff_sec);
             eo_do(n, evas_3d_node_scale_set(0.5, 0.5, 0.5));
          }
     }
   else
     {
        eo_do(globalscene.scene, evas_3d_scene_color_pick_enable_set(EINA_FALSE));
        eo_do(globalscene.scene, flag = evas_3d_scene_pick(ev->canvas.x, ev->canvas.y, &n, &m, &s, &t));
        time = clock() - time;
        diff_sec = ((float)time) / CLOCKS_PER_SEC / 10;
        if (flag)
          {
             fprintf(stdout, "Boom! Time expended for geometry pick: %2.7f .\n", diff_sec);
             eo_do(n, evas_3d_node_scale_set(1.0, 1.0, 1.0));
          }
      }
}

Eina_Bool
_init_m15(void *this, const char *model, const char *texture1,
          const char *texture2)
{
   MineM15 *m15  = (MineM15 *)this;
   m15->mesh1 = eo_add(EVAS_3D_MESH_CLASS, evas);
   m15->mesh2 = eo_add(EVAS_3D_MESH_CLASS, evas);
   m15->material1 = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   m15->material2 = eo_add(EVAS_3D_MATERIAL_CLASS, evas);
   eo_do(m15->mesh1,
         efl_file_set(model, NULL),
         evas_3d_mesh_frame_material_set(0, m15->material1),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
   eo_do(m15->mesh2,
         efl_file_set(model, NULL),
         evas_3d_mesh_frame_material_set(0, m15->material2),
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));


   eo_do(m15->mesh1,  evas_3d_mesh_color_pick_enable_set(EINA_TRUE));
   eo_do(m15->mesh2,  evas_3d_mesh_color_pick_enable_set(EINA_TRUE));

   m15->texture1 = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(m15->texture1,
         evas_3d_texture_file_set(texture1, NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   m15->texture2 = eo_add(EVAS_3D_TEXTURE_CLASS, evas);
   eo_do(m15->texture2,
         evas_3d_texture_file_set(texture2, NULL),
         evas_3d_texture_filter_set(EVAS_3D_TEXTURE_FILTER_NEAREST,
                                    EVAS_3D_TEXTURE_FILTER_NEAREST),
         evas_3d_texture_wrap_set(EVAS_3D_WRAP_MODE_REPEAT,
                                  EVAS_3D_WRAP_MODE_REPEAT));
   eo_do(m15->material1,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, m15->texture1),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));
   eo_do(m15->material2,
         evas_3d_material_texture_set(EVAS_3D_MATERIAL_DIFFUSE, m15->texture2),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_AMBIENT, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_DIFFUSE, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_SPECULAR, EINA_TRUE),
         evas_3d_material_enable_set(EVAS_3D_MATERIAL_NORMAL, EINA_TRUE),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_AMBIENT,
                                    0.01, 0.01, 0.01, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_DIFFUSE,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_color_set(EVAS_3D_MATERIAL_SPECULAR,
                                    1.0, 1.0, 1.0, 1.0),
         evas_3d_material_shininess_set(50.0));
   return EINA_TRUE;
}


Eina_Bool
_init_scene(const char *model, const char *texture1,
            const char *texture2)
{
   int i = 0, j = 0, count = 0;
   Evas_Real x0, y0, z0, x1, y1, z1, tmp, shiftx = 0, shifty = 0;
   MineM15 *m;

   animate_func[0] = _animate_mine1;
   animate_func[1] = _animate_mine2;
   animate_func[2] = _animate_mine3;

   globalscene.scene = eo_add(EVAS_3D_SCENE_CLASS, evas);

   globalscene.root_node = eo_add(EVAS_3D_NODE_CLASS, evas,
                             evas_3d_node_constructor(EVAS_3D_NODE_TYPE_NODE));

   globalscene.camera = eo_add(EVAS_3D_CAMERA_CLASS, evas);
   eo_do(globalscene.camera,
         evas_3d_camera_projection_perspective_set(60.0, 1.0, 1.0, 1000.0));

   globalscene.camera_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_CAMERA));
   eo_do(globalscene.camera_node,
         evas_3d_node_camera_set(globalscene.camera));
   eo_do(globalscene.root_node,
         evas_3d_node_member_add(globalscene.camera_node));
   eo_do(globalscene.camera_node,
         evas_3d_node_position_set(70.0, 0.0, 300.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, -1000.0,
                                  EVAS_3D_SPACE_PARENT, 0.0, 1.0, 0.0));
   globalscene.light = eo_add(EVAS_3D_LIGHT_CLASS, evas);
   eo_do(globalscene.light,
         evas_3d_light_ambient_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_diffuse_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_specular_set(1.0, 1.0, 1.0, 1.0),
         evas_3d_light_directional_set(EINA_TRUE));

   globalscene.light_node =
      eo_add(EVAS_3D_NODE_CLASS, evas,
                    evas_3d_node_constructor(EVAS_3D_NODE_TYPE_LIGHT));
   eo_do(globalscene.light_node,
         evas_3d_node_light_set(globalscene.light),
         evas_3d_node_position_set(100.0, 50.0, 300.0),
         evas_3d_node_look_at_set(EVAS_3D_SPACE_PARENT, 0.0, 0.0, 0.0,
                                  EVAS_3D_SPACE_PARENT, 1.0, 1.0, 1.0));
   eo_do(globalscene.root_node,
         evas_3d_node_member_add(globalscene.light_node));

   tmp = 0.01;
   for (i = 0; i < globalscene.col * globalscene.row; i++, j++)
     {
        MineM15 *m15tmp;
        m = malloc(sizeof(MineM15));
        m->m15_init = _init_m15;
        if (!i)
          m->m15_init(m, model, texture1, texture2);
        else
          {
             m15tmp = (MineM15 *)eina_list_nth(globalscene.mines, 0);
             m->mesh1 = m15tmp->mesh1;
             m->mesh2 = m15tmp->mesh2;
             m->material1 = m15tmp->material1;
             m->material2 = m15tmp->material2;
             m->texture1 = m15tmp->texture1;
             m->texture2 = m15tmp->texture2;
          }
        m->node = eo_add(EVAS_3D_NODE_CLASS, evas,
                         evas_3d_node_constructor(EVAS_3D_NODE_TYPE_MESH));
        m->speed = tmp;
        if (j >= ANIMATION_COUNT) j = 0;
        m->m15_animate = animate_func[j];
        m->animate = ecore_timer_add(m->speed, m->m15_animate, m);
        eo_do(globalscene.root_node, evas_3d_node_member_add(m->node));
        eo_do(m->node, evas_3d_node_mesh_add(m->mesh1));
        globalscene.mines = eina_list_append(globalscene.mines, m);
        tmp += 0.01;
     }

   eo_do(((MineM15 *)eina_list_nth(globalscene.mines, 0))->node, evas_3d_object_update());
   eo_do(((MineM15 *)eina_list_nth(globalscene.mines, 0))->node,
         evas_3d_node_bounding_box_get(&x0, &y0, &z0, &x1, &y1, &z1));

   for (i = 0; i < globalscene.row; ++i)
     {
        shiftx = i * 2 * x1;
        for(j = 0; j < globalscene.col; ++j)
          {
              shifty = j * 2 * y1;
              m = (MineM15 *)eina_list_nth(globalscene.mines, count);
              eo_do(m->node, evas_3d_node_position_set(shifty, 0.0, shiftx));
              if (!(i % 2))
              {
                 eo_do(m->node, evas_3d_node_mesh_del(m->mesh1));
                 eo_do(m->node, evas_3d_node_mesh_add(m->mesh2));
              }
              count++;
          }
     }

   eo_do(globalscene.scene,
         evas_3d_scene_root_node_set(globalscene.root_node),
         evas_3d_scene_camera_node_set(globalscene.camera_node),
         evas_3d_scene_size_set(WIDTH, HEIGHT));
   return EINA_TRUE;
}

int main(int argc, char **argv)
{
   int row = 0, col = 0;
   char *model = NULL, *texture1 = NULL, *texture2 = NULL;
   Eina_Bool r;

   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_INT(row),
      ECORE_GETOPT_VALUE_INT(col),
      ECORE_GETOPT_VALUE_STR(model),
      ECORE_GETOPT_VALUE_STR(texture1),
      ECORE_GETOPT_VALUE_STR(texture2),
      ECORE_GETOPT_VALUE_NONE
   };

   if (!ecore_evas_init()) return 0;
   if (!ecore_init()) return 0;

   ecore_getopt_parse(&optdesc, values, argc, argv);

   if (!row) row = 2;
   if (!col) col = 5;
   if (!model) model = (char *)model_path;
   if (!texture1) texture1 = (char *)image1_path;
   if (!texture2) texture2 = (char *)image2_path;

   fprintf(stdout, "row - %d, col - %d, model - %s, texture1 - %s, texture2 - %s\n",
           row, col, model, texture1, texture2);

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
   globalscene.scene_init(model, texture1, texture2);

   image = evas_object_image_filled_add(evas);

   eo_do(image,
         evas_obj_size_set(WIDTH, HEIGHT),
         evas_obj_visibility_set(EINA_TRUE));
   evas_object_focus_set(image, EINA_TRUE);
   eo_do(image, evas_obj_image_scene_set(globalscene.scene));

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
