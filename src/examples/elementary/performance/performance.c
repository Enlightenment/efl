/**
 * Example for testing performance of Evas 3D library
 *
 * Program should be runned with 4 parameters:
 * 1 - count parameter (NP)
 * 2 - speed parameter (SP)
 * 3 - precision parameter (PP)
 * 4 - camera parameter (CP)
 *
 * NP sets number of spheres. Count of spheres is (NP+1)^2.
 * SP sets speed of rotation. One revolution of root node takes 1200/SP seconds. It is count of calculation of position per second.
 * PP sets number of sphere's vertices. Count of vertices is (NP+1)^2.
 * CP sets camera's perspective angle in degrees.
 *
 * Right click of mouse show/hide navigation panel
 *
 * @verbatim
 * Just use script ./run with key -a: enable automation testing, -l: change which only in local repository, -p: disable navigation panel
 * @endverbatim
 */

/*enable automation test*/
#ifdef ENABLE_ATPORT
   #include "at_port.h"
#endif

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

#include "Tools.h"

#include "graphical_struct.h"

#define CONVERT        20
#define WIDTH          1000
#define HEIGHT         600

Graphical globalGraphical;

/*Variable for fps*/
int fps_frames = 0;

void _clear_buf(char *buf)
{
   int i;
   for (i = 0; i < CONVERT; i++)
     buf[i] = '\0';
}

Eina_Slstr * _value_int_to_char(int value, const char *description)
{
   if (description)
     return eina_slstr_printf("%s %d", description, value);
   return eina_slstr_printf("%d", value);
}

static Eina_Bool
_timer_update(void *data EINA_UNUSED)
{
   printf(" frame rate = %f fps\n", fps_frames / 10.0);
   fps_frames = 0;
   return EINA_TRUE;
}
static Eina_Bool
_animate_scene(void *data)
{
   Eina_List *l;
   Evas_Canvas3D_Node *node;

   static float angle = 0.0f;
   angle += 0.03;

   evas_canvas3d_node_look_at_set((Evas_Canvas3D_Node *)data, EVAS_CANVAS3D_SPACE_PARENT, 8 * sin(angle), 0.0, 8 * cos(angle), EVAS_CANVAS3D_SPACE_PARENT, 0.0, 1.0, 0.0);

   EINA_LIST_FOREACH (globalGraphical.list_nodes, l, node)
     {
        evas_canvas3d_node_orientation_angle_axis_set(node, 10 * angle, 1.0, 1.0, 0.0);
     }

   /* Rotate */
   if (angle > 360.0) angle -= 360.0f;

   fps_frames++;

   return EINA_TRUE;
}


static void
_on_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   Evas_Coord obj_x, obj_y;
   int scene_w, scene_h;
   Evas_Real scene_x, scene_y;
   Evas_Real s, t;
   Evas_Canvas3D_Node *n;
   Evas_Canvas3D_Mesh *m;
   Eina_Bool pick;
   clock_t time;
   float diff_sec;
   if (ev->button == 3)
     {
        if (evas_object_visible_get((Evas_Object*)data))
          evas_object_hide((Evas_Object*)data);
        else
          evas_object_show((Evas_Object*)data);
     }
   else
     {
        evas_object_geometry_get(obj, &x, &y, &w, &h);

        obj_x = ev->canvas.x - x;
        obj_y = ev->canvas.y - y;

        evas_canvas3d_scene_size_get(globalGraphical.scene, &scene_w, &scene_h);

        scene_x = obj_x * scene_w / (Evas_Real)w;
        scene_y = obj_y * scene_h / (Evas_Real)h;

        time = clock();
        if (globalGraphical.flags.colorpick_enable)
          {
             pick = evas_canvas3d_scene_color_pick_enable_set(globalGraphical.scene, EINA_TRUE);
             pick = evas_canvas3d_scene_pick(globalGraphical.scene, ev->canvas.x, ev->canvas.y, &n, &m, NULL, NULL);
             time = clock() - time;
             diff_sec = ((float)time) / CLOCKS_PER_SEC / 10;
             if (pick)
               {
                  printf("Boom! Color pick time expended for pick: %2.7f \n", diff_sec);
                  if (n != globalGraphical.mesh_nodebox)
                    evas_canvas3d_node_scale_set(n, 0.5, 0.5, 0.5);
               }
          }
        else
          {
             pick = evas_canvas3d_scene_color_pick_enable_set(globalGraphical.scene, EINA_FALSE);
             pick = evas_canvas3d_scene_pick(globalGraphical.scene, scene_x, scene_y, &n, &m, &s, &t);
             time = clock() - time;
             diff_sec = ((float)time) / CLOCKS_PER_SEC / 10;
             if (pick)
               {
                  printf("Boom! Geometry pick time expended for pick: %2.7f, TexCoord (%f, %f)\n", diff_sec, s, t);
                  if (n != globalGraphical.mesh_nodebox)
                     evas_canvas3d_node_scale_set(n, 0.5, 0.5, 0.5);
               }
          }
     }
}
static void
_btnstart_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   globalGraphical.change_scene_setup();
}

static void
_btnexit_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_exit();
}

static void
_btnstop_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   const char *status = NULL;
   status = elm_object_text_get(obj);
   if (status != NULL && !strcmp(status, "Stop"))
     {
        elm_object_text_set(obj, "Start");
        ecore_animator_freeze((Ecore_Animator *)data);
     }
   else
     {
        elm_object_text_set(obj, "Stop");
        ecore_animator_thaw((Ecore_Animator *)data);
     }
}
static void
_node_orientation_change_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it;
   Eina_List *l;
   Evas_Canvas3D_Node *node;
   it = elm_flipselector_selected_item_get((Evas_Object*)data);
   const char *str = elm_object_item_text_get(it);
   if (str && !strcmp(str, "root node"))
     evas_canvas3d_node_orientation_angle_axis_set(globalGraphical.root_node, 15, 1.0, 1.0, 1.0);
   else
     {
        EINA_LIST_FOREACH (globalGraphical.list_nodes, l, node)
          {
             evas_canvas3d_node_orientation_angle_axis_set((Eo*)node, 15, 1.0, 1.0, 1.0);
          }
     }
}
static void
_node_position_change_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it;
   Eina_List *l;
   Evas_Canvas3D_Node *node;
   it = elm_flipselector_selected_item_get((Evas_Object*)data);
   const char *str = elm_object_item_text_get(it);
   if (str && !strcmp(str, "root node"))
     evas_canvas3d_node_position_set(globalGraphical.root_node, 10.0, 1.0, 1.0);
   else
     {
        EINA_LIST_FOREACH (globalGraphical.list_nodes, l, node)
          {
             evas_canvas3d_node_position_set((Eo*)node, 1.0, 1.0, 1.0);
          }
     }
}
static void
_node_scale_change_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it;
   Eina_List *l;
   Evas_Canvas3D_Node *node;
   it = elm_flipselector_selected_item_get((Evas_Object*)data);
   const char *str = elm_object_item_text_get(it);
   if (str && !strcmp(str, "root node"))
     evas_canvas3d_node_scale_set(globalGraphical.root_node, 1.0, 1.0, 1.0);
   else
     {
        EINA_LIST_FOREACH (globalGraphical.list_nodes, l, node)
          {
             evas_canvas3d_node_scale_set((Eo*)node, 1.0, 1.0, 1.0);
          }
     }
}
static void
_countdec10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;
   int tmp;

   globalGraphical.count -= 10;
   if (globalGraphical.count < 0) tmp = globalGraphical.count = 0;
   else
     tmp = ((globalGraphical.count + 1) * (globalGraphical.count + 1));

   buf = _value_int_to_char(((tmp <= 1) ? 0 : tmp), "quantity:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_countdec_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;
   int tmp;

   globalGraphical.count--;
   if (globalGraphical.count < 0) tmp = globalGraphical.count = 0;
   else
     tmp = ((globalGraphical.count + 1) * (globalGraphical.count + 1));
   buf = _value_int_to_char(((tmp <= 1) ? 0 : tmp), "quantity:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_countinc_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.count++;

   buf = _value_int_to_char(((globalGraphical.count + 1) * (globalGraphical.count + 1)), "quantity:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_countinc10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.count += 10;

   buf = _value_int_to_char(((globalGraphical.count + 1) * (globalGraphical.count + 1)), "quantity:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_speeddec10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.speed -= 10;
   if (globalGraphical.speed <= 0) globalGraphical.speed = 10;

   buf = _value_int_to_char(globalGraphical.speed, "speed:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_speeddec_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.speed--;
   if (globalGraphical.speed <= 0) globalGraphical.speed = 10;

   buf = _value_int_to_char(globalGraphical.speed, "speed:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_speedinc_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.speed++;

   buf = _value_int_to_char(globalGraphical.speed, "speed:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_speedinc10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.speed += 10;

   buf = _value_int_to_char(globalGraphical.speed, "speed:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_precisiondec10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.precision -= 10;
   if (globalGraphical.precision <= 0) globalGraphical.precision = 10;

   buf = _value_int_to_char(globalGraphical.precision, "precision:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_precisiondec_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.precision--;
   if (globalGraphical.precision <= 0) globalGraphical.precision = 10;

   buf = _value_int_to_char(globalGraphical.precision, "precision:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_precisioninc_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.precision++;

   buf = _value_int_to_char(globalGraphical.precision, "precision:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_precisioninc10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.precision += 10;

   buf = _value_int_to_char(globalGraphical.precision, "precision:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_angledec10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.angle -= 10;
   if (globalGraphical.angle <= 0) globalGraphical.angle = 10;

   buf = _value_int_to_char(globalGraphical.angle, "angle:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_angledec_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.angle--;
   if (globalGraphical.angle <= 0) globalGraphical.angle = 10;

   buf = _value_int_to_char(globalGraphical.angle, "angle:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_angleinc_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.angle++;
   if (globalGraphical.angle >= 180) globalGraphical.angle = 180;

   buf = _value_int_to_char(globalGraphical.angle, "angle:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_angleinc10_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Slstr *buf;

   globalGraphical.angle += 10;
   if (globalGraphical.angle >= 180) globalGraphical.angle = 180;

   buf = _value_int_to_char(globalGraphical.angle, "angle:");
   elm_object_text_set((Evas_Object*)data, buf);
}

static void
_shadow_enable(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     globalGraphical.flags.shadow_enable = EINA_TRUE;
   else
     globalGraphical.flags.shadow_enable = EINA_FALSE;
}

static void
_colorpick_enable(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     globalGraphical.flags.colorpick_enable = EINA_TRUE;
   else
     globalGraphical.flags.colorpick_enable = EINA_FALSE;
}

static void
_fog_enable(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     globalGraphical.flags.fog_enable = EINA_TRUE;
   else
     globalGraphical.flags.fog_enable = EINA_FALSE;
}

static void
_blend_enable(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     globalGraphical.flags.blend_enable = EINA_TRUE;
   else
     globalGraphical.flags.blend_enable = EINA_FALSE;
}

static void
_cb_fps_enable(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   if (elm_check_state_get(obj))
     {
        globalGraphical.flags.fps_enable = EINA_TRUE;
        ecore_timer_thaw((Ecore_Timer *)data);
        fps_frames = 0;
     }
   else
     {
        globalGraphical.flags.fps_enable = EINA_FALSE;
        ecore_timer_freeze((Ecore_Timer *)data);
        fps_frames = 0;
     }
}

static void
_load_mesh(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   char *format = NULL, *a = NULL;
   char name[FILENAME_MAX];
   int i = 0, j = 0;
   if (event_info)
     {
        char *tmp = NULL;
        tmp = strdup(event_info);
        /*Get filename in backward*/
        for (i = strlen(tmp) - 1; i >= 0; i--, j++)
          {
             if (tmp[i] != '/')
               name[j] = tmp[i];
             else
               {
                  name[j] = '\0';
                  break;
               }
          }
        free(tmp);
        tmp = NULL;
        j = 0;
        /*Reverse filename*/
        tmp = strdup(name);
        for (i = strlen(tmp) - 1; i >= 0 ; i--, j++)
           name[j] = tmp[i];

        free(tmp);
        free(a);
        tmp = NULL;
        a = NULL;
        /*Get extension of file*/
        tmp = strdup(event_info);
        a = strrchr(tmp,'.');
        format = malloc(sizeof (char) * (strlen(a) - 1));
        for (i = 0; i <= (int)strlen(a) - 1; i++)
          format[i] = a[i + 1];
        if (!strcmp(format, "md2"))
          {
             globalGraphical.model_path = event_info;
             elm_object_text_set(obj, name);
           }
        else if (!strcmp(format, "obj"))
          {
             globalGraphical.model_path = event_info;
             elm_object_text_set(obj, name);
          }
        else if (!strcmp(format, "ply"))
          {
             globalGraphical.model_path = event_info;
             elm_object_text_set(obj, name);
          }
        else
          {
             printf("\nUnsupported fromat file\n");
             globalGraphical.model_path = NULL;
             elm_object_text_set(obj, "load");
          }
        free(format);
        free(tmp);
     }
   else
     {
        globalGraphical.model_path = NULL;
        elm_object_text_set(obj, "load");
     }
}

EAPI_MAIN
 int elm_main(int argc, char **argv)
{
   Evas *evas = NULL;
   Evas_Object *win = NULL, *bg = NULL, *image = NULL;
   Ecore_Animator *timer = NULL;
   Evas_Object *layout = NULL;
   Evas_Object *btnstart = NULL, *btnexit = NULL, *btnstop = NULL;
   Evas_Object *startexitbox = NULL,*testbox = NULL, *controlbox = NULL, *newfeaturesbox = NULL;
   Evas_Object *countdec10 = NULL, *countinc = NULL, *countdec = NULL, *countinc10 = NULL;
   Evas_Object *speeddec10 = NULL, *speedinc = NULL, *speeddec = NULL, *speedinc10 = NULL;
   Evas_Object *precisiondec10 = NULL, *precisioninc = NULL, *precisiondec = NULL, *precisioninc10 = NULL;
   Evas_Object *vcount = NULL, *vspeed = NULL, *vprecision = NULL, *vangle = NULL;
   Evas_Object *angledec10 = NULL, *angleinc = NULL, *angledec = NULL, *angleinc10 = NULL;
   Evas_Object *cb_shadow = NULL, *cb_colorpick = NULL, *cb_fog = NULL, *cb_blend = NULL, *cb_fps = NULL;
   Evas_Object *fs_bt = NULL;
   Evas_Object *navigation = NULL;
   Evas_Object *fs_node = NULL, *bt_orientation = NULL, *bt_position = NULL, *bt_scale = NULL;
   Ecore_Timer *fps_timer = NULL;
   Eina_Slstr *buf;

#ifdef ENABLE_ATPORT
    at_port_h mf_at_port = NULL;
#endif

   if (!elm_init(argc, argv)) return 1;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   win = elm_win_add(NULL, "3D Performance", ELM_WIN_BASIC);
   evas_object_resize(win, WIDTH, HEIGHT);
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_show(win);

#ifdef ENABLE_ATPORT
    at_port_create(&(mf_at_port), win);
    if(mf_at_port != NULL)
      {
         at_port_enable(mf_at_port);
      }
#endif

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_bg_file_set(bg, "background.png", NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, layout);
   elm_layout_file_set(layout, "layout.edj", "new/layout/0");
   evas_object_show(layout);

   startexitbox = elm_box_add(win);
   elm_box_horizontal_set(startexitbox, EINA_TRUE);
   elm_object_part_content_set(layout, "swallow.btn.startexit", startexitbox);
   evas_object_size_hint_weight_set(startexitbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(startexitbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_homogeneous_set(startexitbox, EINA_TRUE);
   evas_object_show(startexitbox);

   evas = evas_object_evas_get(win);
   image = efl_add(EFL_CANVAS_SCENE3D_CLASS, evas);

   btnstart = elm_button_add(win);
   elm_object_text_set(btnstart, "Apply");
   evas_object_size_hint_weight_set(btnstart, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btnstart, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(startexitbox, btnstart);
   evas_object_show(btnstart);
   evas_object_smart_callback_add(btnstart, "clicked", _btnstart_cb, image);

   testbox = elm_box_add(win);
   elm_box_horizontal_set(testbox, EINA_TRUE);
   elm_box_homogeneous_set(testbox, EINA_TRUE);
   elm_box_pack_end(startexitbox, testbox);
   evas_object_show(testbox);

   btnstop = elm_button_add(win);
   elm_object_text_set( btnstop, "Stop");
   elm_box_pack_end(testbox, btnstop);
   evas_object_size_hint_weight_set(btnstop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btnstop, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(btnstop);

   fs_node = elm_flipselector_add(win);
   elm_box_pack_end(testbox, fs_node);
   elm_flipselector_item_append(fs_node, "root node", NULL, NULL);
   elm_flipselector_item_append(fs_node, "mesh node", NULL, NULL);
   evas_object_size_hint_weight_set(btnstop, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btnstop, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fs_node);

   bt_orientation = elm_button_add(win);
   elm_object_text_set(bt_orientation, "Orientation");
   elm_box_pack_end(testbox, bt_orientation);
   evas_object_size_hint_weight_set(bt_orientation, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt_orientation, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt_orientation);
   evas_object_smart_callback_add(bt_orientation, "clicked", _node_orientation_change_cb, fs_node);

   bt_position = elm_button_add(win);
   elm_object_text_set(bt_position, "Position");
   elm_box_pack_end(testbox, bt_position);
   evas_object_size_hint_weight_set(bt_position, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt_position, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt_position);
   evas_object_smart_callback_add(bt_position, "clicked", _node_position_change_cb, fs_node);

   bt_scale = elm_button_add(win);
   elm_object_text_set(bt_scale, "Scale");
   elm_box_pack_end(testbox, bt_scale);
   evas_object_size_hint_weight_set(bt_scale, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt_scale, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt_scale);
   evas_object_smart_callback_add(bt_scale, "clicked", _node_scale_change_cb, fs_node);

   btnexit = elm_button_add(win);
   elm_object_text_set(btnexit, "Exit");
   elm_box_pack_end(startexitbox, btnexit);
   evas_object_size_hint_weight_set(btnexit, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btnexit, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(btnexit);
   evas_object_smart_callback_add(btnexit, "clicked", _btnexit_cb, NULL);

   controlbox = elm_box_add(win);
   elm_box_horizontal_set(controlbox, EINA_TRUE);
   elm_object_part_content_set(layout, "swallow.controls.btn", controlbox);
   evas_object_size_hint_weight_set(controlbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(controlbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(controlbox);

   vcount = elm_label_add(win);
   buf = _value_int_to_char(121, "quantity:");
   elm_object_text_set(vcount, buf);

   countdec10 = elm_button_add(win);
   elm_object_text_set(countdec10, "<<");
   elm_box_pack_end(controlbox, countdec10);
   evas_object_size_hint_weight_set(countdec10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(countdec10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(countdec10);
   evas_object_smart_callback_add(countdec10, "clicked", _countdec10_cb, vcount);

   countdec = elm_button_add(win);
   elm_object_text_set(countdec, "<");
   elm_box_pack_end(controlbox, countdec);
   evas_object_size_hint_weight_set(countdec, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(countdec, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(countdec);
   evas_object_smart_callback_add(countdec, "clicked", _countdec_cb, vcount);

   elm_box_pack_end(controlbox, vcount);
   evas_object_show(vcount);

   countinc = elm_button_add(win);
   elm_object_text_set(countinc, ">");
   elm_box_pack_end(controlbox, countinc);
   evas_object_size_hint_weight_set(countinc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(countinc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(countinc);
   evas_object_smart_callback_add(countinc, "clicked", _countinc_cb, vcount);

   countinc10 = elm_button_add(win);
   elm_object_text_set(countinc10, ">>");
   elm_box_pack_end(controlbox, countinc10);
   evas_object_size_hint_weight_set(countinc10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(countinc10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(countinc10);
   evas_object_smart_callback_add(countinc10, "clicked", _countinc10_cb, vcount);

   vspeed = elm_label_add(win);
   buf = _value_int_to_char(30, "speed:");
   elm_object_text_set(vspeed, buf);

   speeddec10 = elm_button_add(win);
   elm_object_text_set(speeddec10, "<<");
   elm_box_pack_end(controlbox, speeddec10);
   evas_object_size_hint_weight_set(speeddec10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(speeddec10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(speeddec10);
   evas_object_smart_callback_add(speeddec10, "clicked", _speeddec10_cb, vspeed);

   speeddec = elm_button_add(win);
   elm_object_text_set(speeddec, "<");
   elm_box_pack_end(controlbox, speeddec);
   evas_object_size_hint_weight_set(speeddec, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(speeddec, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(speeddec);
   evas_object_smart_callback_add(speeddec, "clicked", _speeddec_cb, vspeed);

   elm_box_pack_end(controlbox, vspeed);
   evas_object_show(vspeed);

   speedinc = elm_button_add(win);
   elm_object_text_set(speedinc, ">");
   elm_box_pack_end(controlbox, speedinc);
   evas_object_size_hint_weight_set(speedinc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(speedinc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(speedinc);
   evas_object_smart_callback_add(speedinc, "clicked", _speedinc_cb, vspeed);

   speedinc10 = elm_button_add(win);
   elm_object_text_set(speedinc10, ">>");
   elm_box_pack_end(controlbox, speedinc10);
   evas_object_size_hint_weight_set(speedinc10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(speedinc10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(speedinc10);
   evas_object_smart_callback_add(speedinc10, "clicked", _speedinc10_cb, vspeed);

   vprecision = elm_label_add(win);
   buf = _value_int_to_char(100, "precision:");
   elm_object_text_set(vprecision, buf);

   precisiondec10 = elm_button_add(win);
   elm_object_text_set(precisiondec10, "<<");
   elm_box_pack_end(controlbox, precisiondec10);
   evas_object_size_hint_weight_set(precisiondec10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(precisiondec10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(precisiondec10);
   evas_object_smart_callback_add(precisiondec10, "clicked", _precisiondec10_cb, vprecision);

   precisiondec = elm_button_add(win);
   elm_object_text_set(precisiondec, "<");
   elm_box_pack_end(controlbox, precisiondec);
   evas_object_size_hint_weight_set(precisiondec, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(precisiondec, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(precisiondec);
   evas_object_smart_callback_add(precisiondec, "clicked", _precisiondec_cb, vprecision);

   elm_box_pack_end(controlbox, vprecision);
   evas_object_show(vprecision);

   precisioninc = elm_button_add(win);
   elm_object_text_set(precisioninc, ">");
   elm_box_pack_end(controlbox, precisioninc);
   evas_object_size_hint_weight_set(precisioninc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(precisioninc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(precisioninc);
   evas_object_smart_callback_add(precisioninc, "clicked", _precisioninc_cb, vprecision);

   precisioninc10 = elm_button_add(win);
   elm_object_text_set(precisioninc10, ">>");
   elm_box_pack_end(controlbox, precisioninc10);
   evas_object_size_hint_weight_set(precisioninc10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(precisioninc10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(precisioninc10);
   evas_object_smart_callback_add(precisioninc10, "clicked", _precisioninc10_cb, vprecision);

   vangle = elm_label_add(win);
   buf = _value_int_to_char(120, "angle:");
   elm_object_text_set(vangle, buf);

   angledec10 = elm_button_add(win);
   elm_object_text_set(angledec10, "<<");
   elm_box_pack_end(controlbox, angledec10);
   evas_object_size_hint_weight_set(angledec10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(angledec10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(angledec10);
   evas_object_smart_callback_add(angledec10, "clicked", _angledec10_cb, vangle);

   angledec = elm_button_add(win);
   elm_object_text_set(angledec, "<");
   elm_box_pack_end(controlbox, angledec);
   evas_object_size_hint_weight_set(angledec, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(angledec, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(angledec);
   evas_object_smart_callback_add(angledec, "clicked", _angledec_cb, vangle);

   elm_box_pack_end(controlbox, vangle);
   evas_object_show(vangle);

   angleinc = elm_button_add(win);
   elm_object_text_set(angleinc, ">");
   elm_box_pack_end(controlbox, angleinc);
   evas_object_size_hint_weight_set(angleinc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(angleinc, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(angleinc);
   evas_object_smart_callback_add(angleinc, "clicked", _angleinc_cb, vangle);

   angleinc10 = elm_button_add(win);
   elm_object_text_set(angleinc10, ">>");
   elm_box_pack_end(controlbox, angleinc10);
   evas_object_size_hint_weight_set(angleinc10, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(angleinc10, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(angleinc10);
   evas_object_smart_callback_add(angleinc10, "clicked", _angleinc10_cb, vangle);

   newfeaturesbox = elm_box_add(win);
   elm_box_horizontal_set(newfeaturesbox, EINA_TRUE);
   elm_object_part_content_set(layout, "new_features", newfeaturesbox);
   evas_object_size_hint_align_set(newfeaturesbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(newfeaturesbox);

   cb_shadow = elm_check_add(win);
   elm_object_text_set(cb_shadow, "shadow");
   elm_check_state_set(cb_shadow, EINA_FALSE);
   evas_object_size_hint_align_set(cb_shadow, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(cb_shadow, "changed", _shadow_enable, NULL);
   elm_box_pack_end(newfeaturesbox, cb_shadow);
   evas_object_show(cb_shadow);

   cb_fog = elm_check_add(win);
   elm_object_text_set(cb_fog, "fog");
   elm_check_state_set(cb_fog, EINA_FALSE);
   evas_object_size_hint_align_set(cb_fog, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(cb_fog, "changed", _fog_enable, NULL);
   elm_box_pack_end(newfeaturesbox, cb_fog);
   evas_object_show(cb_fog);

   cb_colorpick = elm_check_add(win);
   elm_object_text_set(cb_colorpick, "colorpick");
   elm_check_state_set(cb_colorpick, EINA_FALSE);
   evas_object_size_hint_align_set(cb_colorpick, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(cb_colorpick, "changed", _colorpick_enable, NULL);
   elm_box_pack_end(newfeaturesbox, cb_colorpick);
   evas_object_show(cb_colorpick);

   cb_blend = elm_check_add(win);
   elm_object_text_set(cb_blend, "blend");
   elm_check_state_set(cb_blend, EINA_FALSE);
   evas_object_size_hint_align_set(cb_blend, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(cb_blend, "changed", _blend_enable, NULL);
   elm_box_pack_end(newfeaturesbox, cb_blend);
#ifndef LOCAL_REPOSITORY
   elm_object_disabled_set(cb_blend, EINA_TRUE);
#endif
      evas_object_show(cb_blend);

   cb_fps = elm_check_add(win);
   elm_object_text_set(cb_fps, "fps");
   elm_check_state_set(cb_fps, EINA_FALSE);
   evas_object_size_hint_align_set(cb_fps, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(newfeaturesbox, cb_fps);
   evas_object_show(cb_fps);

   /*Select model file*/
   fs_bt = elm_fileselector_button_add(win);
   elm_fileselector_path_set(fs_bt, ".");
   elm_object_text_set(fs_bt, "load");
   elm_fileselector_button_inwin_mode_set(fs_bt, EINA_TRUE);
   evas_object_size_hint_align_set(fs_bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(newfeaturesbox, fs_bt);
   evas_object_show(fs_bt);
   evas_object_smart_callback_add(fs_bt, "file,chosen", _load_mesh, NULL);

   elm_object_part_content_set(layout, "swallow.content", image);
   evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(image);
   evas_render(evas); //make edje do recalculation
   image = init_graphical_window(image);

   /*Update data for count, speed, precision, angle*/
   buf = _value_int_to_char((globalGraphical.count + 1) * (globalGraphical.count + 1), "quantity:");
   elm_object_text_set(vcount, buf);
   buf = _value_int_to_char(globalGraphical.speed, "speed:");
   elm_object_text_set(vspeed, buf);
   buf = _value_int_to_char(globalGraphical.precision, "precision:");
   elm_object_text_set(vprecision, buf);
   buf = _value_int_to_char(globalGraphical.angle, "angle:");
   elm_object_text_set(vangle, buf);
   if (globalGraphical.model_path)
     evas_object_smart_callback_call(fs_bt, "file,chosen", (void *)globalGraphical.model_path);

   /*Create navigation panel*/
#ifndef DISABLE_NAVIGATION_PANEL
   navigation = init_panel_camera_light(win, globalGraphical.camera_node, globalGraphical.light_node, 1.0, 0.5);
   panel_camera_coord_step_set(navigation, 5);
   panel_camera_angle_step_set(navigation, 5);
   panel_light_coord_step_set(navigation, 5);
   panel_light_angle_step_set(navigation, 5);
#endif

   evas_object_event_callback_add(image, EVAS_CALLBACK_MOUSE_DOWN,
                                  _on_mouse_down, navigation);

   ecore_animator_frametime_set(1.0 / (globalGraphical.speed));
   timer = ecore_animator_add(_animate_scene, globalGraphical.light_node);
   fps_timer = ecore_timer_add(10, _timer_update, NULL);
   ecore_timer_freeze(fps_timer);
   evas_object_smart_callback_add(cb_fps, "changed", _cb_fps_enable, (void *)fps_timer);

   evas_object_smart_callback_add(btnstop, "clicked", _btnstop_cb, timer);

   elm_run();
   elm_shutdown();

#ifndef DISABLE_NAVIGATION_PANEL
   panel_camera_light_fini(navigation);
#endif
   return 0;
}
ELM_MAIN()

