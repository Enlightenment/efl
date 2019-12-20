#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define TAP_NAME "tap"
#define DOUBLE_TAP_NAME "double_tap"
#define TRIPLE_TAP_NAME "triple_tap"
#define LONG_TAP_NAME "long_tap"
#define FLICK_NAME "flick"
#define LINE_NAME "line"
#define MOMENTUM_NAME "momentum"
#define ROTATE_NAME "rotate"
#define ZOOM_NAME "zoom"

#define N_GESTURE_TYPE 9
#define MAX_DOUBLE_TAP 5
#define MAX_FLICK 5
#define MAX_LINE 5
#define MAX_LONG_TAP 5
#define MAX_MOMENTUM 5
#define MAX_ROTATE 1
#define MAX_TAP 5
#define MAX_TRIPLE_TAP 5
#define MAX_ZOOM 1

#define TB_PADDING_X 4
#define TB_PADDING_Y 12

#define BX_PADDING_X 0
#define BX_PADDING_Y 2

/* Define initial RGBA values for icons */
#define INI_R 60
#define INI_G 66
#define INI_B 64
#define INI_A 128
#define COLOR_STEP 4

#define START_COLOR 220, 220, 200, 255
#define UPDATE_COLOR 255, 255, 0, 255
#define ABORT_COLOR 255, 0, 0, 255
#define END_COLOR 0, 255, 0, 255

struct _icon_properties
{
   Evas_Object *icon;
   int r; /* current r */
   int g;
   int b;
   int a;

   const char *name;
};
typedef struct _icon_properties icon_properties;

struct _infra_data
{  /* Some data that is passed aroung between callbacks (replacing globals) */
   icon_properties *icons;
   Ecore_Timer *colortimer;
   char buf[1024];
   int long_tap_count;
};
typedef struct _infra_data infra_data;

static void
_infra_data_free(infra_data *infra)
{
   if (infra)
     {
        if (infra->colortimer)
          ecore_timer_del(infra->colortimer);

        if (infra->icons)
          free(infra->icons);

        free (infra);
     }
}

static infra_data *
_infra_data_alloc(void)
{
   infra_data *infra = malloc(sizeof(infra_data));
   if (!infra) return NULL;

   infra->icons = calloc(N_GESTURE_TYPE, sizeof(icon_properties ));
   infra->colortimer = NULL;

   return infra;
}

static void
my_win_del(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{  /* called when my_win_main is requested to be deleted */
   _infra_data_free(data);
}

static icon_properties *
_icon_properties_find(icon_properties *icons, char *name)
{
   int n;

   for (n = 0; n < N_GESTURE_TYPE; n++)
     if (!strcmp(icons[n].name, name))
       return &icons[n];

   return NULL;
}

static void
_icon_color_set(icon_properties *i, int r, int g, int b, int a)
{
   i->r =  r;
   i->g =  g;
   i->b =  b;
   i->a =  a;
   evas_object_color_set(i->icon, i->r,  i->g,  i->b,  i->a);
}

static Eina_Bool
_icon_color_set_cb(void *data)
{
#define INC_COLOR(C, NC) \
   do { \
        if (abs(i->C - NC) < COLOR_STEP) \
        i->C = NC; \
        else \
        i->C += ((i->C < NC) ? COLOR_STEP : (-COLOR_STEP)); \
   } while (0)

   int n;
   icon_properties *icons = data;
   icon_properties *i;

   for (n = 0; n < N_GESTURE_TYPE; n++)
     {
        i = &icons[n];

        INC_COLOR(r,INI_R);
        INC_COLOR(g,INI_G);
        INC_COLOR(b,INI_B);
        INC_COLOR(a,INI_A);

        /* Change Icon color */
        evas_object_color_set(i->icon, i->r,  i->g,  i->b,  i->a);
    }

   return ECORE_CALLBACK_RENEW;
}

static void
_color_and_icon_set(infra_data *infra, char *name, int n, int max,
      int r, int g, int b, int a)
{
   icon_properties *i;
   int nn = n;
   i = _icon_properties_find(infra->icons, name);
   if (i)
     {
        if (n < 1)
          nn = 1;

        if (n > max)
          nn = max;

        snprintf(infra->buf, sizeof(infra->buf),
              "%s/images/g_layer/%s_%d.png", elm_app_data_dir_get(), i->name, nn);
        elm_image_file_set(i->icon, infra->buf, NULL);
        _icon_color_set(i, r, g, b, a);
     }
}

/* START - Callbacks for gestures */
static void
finger_tap_start(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, TAP_NAME, 1, MAX_TAP, START_COLOR);
   printf("Tap Gesture started x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_tap_end(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, TAP_NAME, 1, MAX_TAP, END_COLOR);
   printf("Tap Gesture ended x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_tap_abort(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   _color_and_icon_set(data, TAP_NAME, 1, MAX_TAP, ABORT_COLOR);
   printf("Tap Aborted\n");
}

static void
finger_flick_start(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, FLICK_NAME, 1, MAX_TAP, START_COLOR);
   printf("Flick Gesture started x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_flick_end(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);
   double angle = efl_gesture_flick_angle_get(tap);

   _color_and_icon_set(data, FLICK_NAME, 1, MAX_TAP, END_COLOR);
   printf("Flick Gesture ended x,y=<%d,%d> angle=<%f>\n", pos.x, pos.y, angle);
}

static void
finger_flick_abort(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   _color_and_icon_set(data, FLICK_NAME, 1, MAX_TAP, ABORT_COLOR);
   printf("Flick Aborted\n");
}

static void
finger_momentum_start(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);
   unsigned int t = efl_gesture_timestamp_get(tap);

   _color_and_icon_set(data, MOMENTUM_NAME, 1, MAX_TAP, START_COLOR);
   printf("Momentum Gesture started x,y=<%d,%d> time=<%d>\n", pos.x, pos.y, t);
}

static void
finger_momentum_update(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);
   Eina_Vector2 m = efl_gesture_momentum_get(tap);
   unsigned int t = efl_gesture_timestamp_get(tap);

   _color_and_icon_set(data, MOMENTUM_NAME, 1, MAX_TAP, UPDATE_COLOR);
   printf("Momentum Gesture updated x,y=<%d,%d> momentum=<%f %f> time=<%d>\n",
          pos.x, pos.y, m.x, m.y, t);
}

static void
finger_momentum_end(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);
   Eina_Vector2 m = efl_gesture_momentum_get(tap);
   unsigned int t = efl_gesture_timestamp_get(tap);

   _color_and_icon_set(data, MOMENTUM_NAME, 1, MAX_TAP, END_COLOR);
   printf("Momentum Gesture ended x,y=<%d,%d> momentum=<%f %f> time=<%d>\n",
     pos.x, pos.y, m.x, m.y, t);
}

static void
finger_momentum_abort(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   _color_and_icon_set(data, MOMENTUM_NAME, 1, MAX_TAP, ABORT_COLOR);
   printf("Momentum Aborted\n");
}

static void
finger_triple_tap_start(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, TRIPLE_TAP_NAME, 1, MAX_TAP, START_COLOR);
   printf("Triple Tap Gesture started x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_triple_tap_update(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, TRIPLE_TAP_NAME, 1, MAX_TAP, UPDATE_COLOR);
   printf("Triple Tap Gesture updated x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_triple_tap_end(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, TRIPLE_TAP_NAME, 1, MAX_TAP, END_COLOR);
   printf("Triple Tap Gesture ended x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_triple_tap_abort(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   _color_and_icon_set(data, TRIPLE_TAP_NAME, 1, MAX_TAP, ABORT_COLOR);
   printf("Triple Tap Aborted\n");
}

static void
finger_double_tap_start(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, DOUBLE_TAP_NAME, 1, MAX_TAP, START_COLOR);
   printf("Double Tap Gesture started x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_double_tap_update(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, DOUBLE_TAP_NAME, 1, MAX_TAP, UPDATE_COLOR);
   printf("Double Tap Gesture updated x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_double_tap_end(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, DOUBLE_TAP_NAME, 1, MAX_TAP, END_COLOR);
   printf("Double Tap Gesture ended x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_double_tap_abort(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   _color_and_icon_set(data, DOUBLE_TAP_NAME, 1, MAX_TAP, ABORT_COLOR);
   printf("Double Tap Aborted\n");
}

static void
finger_long_tap_start(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, LONG_TAP_NAME, 1, MAX_TAP, START_COLOR);
   printf("Long Tap Gesture started x,y=<%d,%d> \n", pos.x, pos.y);
}

static void
finger_long_tap_update(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   _color_and_icon_set(data, LONG_TAP_NAME, 1, MAX_TAP, UPDATE_COLOR);
   printf("Long Tap Gesture updated\n");
}

static void
finger_long_tap_end(void *data , Efl_Canvas_Gesture *tap)
{
   Eina_Position2D pos = efl_gesture_hotspot_get(tap);

   _color_and_icon_set(data, LONG_TAP_NAME, 1, MAX_TAP, END_COLOR);
   printf("Long Tap Gesture ended x,y=<%d,%d> \n",pos.x, pos.y);
}

static void
finger_long_tap_abort(void *data , Efl_Canvas_Gesture *tap EINA_UNUSED)
{
   _color_and_icon_set(data, LONG_TAP_NAME, 1, MAX_TAP, ABORT_COLOR);
   printf("Long Tap Aborted\n");
}

static void
tap_gesture_cb(void *data , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;
   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STARTED:
         finger_tap_start(data, g);
         break;
      case EFL_GESTURE_CANCELED:
         finger_tap_abort(data, g);
         break;
      case EFL_GESTURE_FINISHED:
         finger_tap_end(data, g);
         break;
      default:
         break;
   }
}

static void
flick_gesture_cb(void *data , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;
   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STARTED:
         finger_flick_start(data, g);
         break;
      case EFL_GESTURE_CANCELED:
         finger_flick_abort(data, g);
         break;
      case EFL_GESTURE_FINISHED:
         finger_flick_end(data, g);
         break;
      default:
         break;
   }
}

static void
momentum_gesture_cb(void *data , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;
   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STARTED:
         finger_momentum_start(data, g);
         break;
      case EFL_GESTURE_UPDATED:
         finger_momentum_update(data, g);
         break;
      case EFL_GESTURE_CANCELED:
         finger_momentum_abort(data, g);
         break;
      case EFL_GESTURE_FINISHED:
         finger_momentum_end(data, g);
         break;
      default:
         break;
   }
}

static void
triple_tap_gesture_cb(void *data , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;
   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STARTED:
         finger_triple_tap_start(data, g);
         break;
      case EFL_GESTURE_UPDATED:
         finger_triple_tap_update(data, g);
         break;
      case EFL_GESTURE_CANCELED:
         finger_triple_tap_abort(data, g);
         break;
      case EFL_GESTURE_FINISHED:
         finger_triple_tap_end(data, g);
         break;
      default:
         break;
   }
}

static void
double_tap_gesture_cb(void *data , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;
   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STARTED:
         finger_double_tap_start(data, g);
         break;
      case EFL_GESTURE_UPDATED:
         finger_double_tap_update(data, g);
         break;
      case EFL_GESTURE_CANCELED:
         finger_double_tap_abort(data, g);
         break;
      case EFL_GESTURE_FINISHED:
         finger_double_tap_end(data, g);
         break;
      default:
         break;
   }
}

static void
long_tap_gesture_cb(void *data , const Efl_Event *ev)
{
   Efl_Canvas_Gesture *g = ev->info;
   switch(efl_gesture_state_get(g))
   {
      case EFL_GESTURE_STARTED:
         finger_long_tap_start(data, g);
         break;
      case EFL_GESTURE_UPDATED:
         finger_long_tap_update(data, g);
         break;
      case EFL_GESTURE_CANCELED:
         finger_long_tap_abort(data, g);
         break;
      case EFL_GESTURE_FINISHED:
         finger_long_tap_end(data, g);
         break;
      default:
         break;
   }
}

/* END   - Callbacks for gestures */

static Evas_Object *
create_gesture_box(Evas_Object *win, icon_properties *icons,
                   int idx, const char *name, const char *lb_txt)
{  /* Creates a box with icon and label, later placed in a table */
   Evas_Object *lb, *bx = elm_box_add(win);
   char buf[1024];

   elm_box_padding_set(bx, BX_PADDING_X, BX_PADDING_Y);
   icons[idx].icon = elm_icon_add(win);
   icons[idx].name = name;
   snprintf(buf, sizeof(buf), "%s/images/g_layer/%s_1.png",
         elm_app_data_dir_get(), icons[idx].name);
   elm_image_file_set(icons[idx].icon, buf, NULL);
   elm_image_resizable_set(icons[idx].icon, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(icons[idx].icon, 0.5, 0.5);
   _icon_color_set(&icons[idx], INI_R, INI_G, INI_B, INI_A);
   elm_box_pack_end(bx, icons[idx].icon);
   evas_object_show(icons[idx].icon);

   lb = elm_label_add(win);
   elm_object_text_set(lb, lb_txt);
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, 0.5, 0.5);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);
   evas_object_show(bx);

   return bx;
}

void
_tb_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w,h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(data, w, h);
   evas_object_color_set(data, 0, 0, 0, 0);
   evas_object_show(data);
}

void
test_gesture_framework(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
      void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *lb, *bx;
   Evas_Object *r, *target; /* Gesture layer transparent object */

   infra_data *infra = _infra_data_alloc();

   win = elm_win_util_standard_add("gesture-layer2", "Gesture (EO)");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", my_win_del, infra);

   /* START - Building icons table */
   bx = elm_box_add(win);
   tb = elm_table_add(win);
   elm_box_pack_end(bx, tb);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, 0.5, 0.5);
   elm_win_resize_object_add(win, bx);
   evas_object_show(tb);
   evas_object_show(bx);

   target = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _tb_resize, target);

   /* Box of Tap icon and label */
   bx = create_gesture_box(win, infra->icons, 0, TAP_NAME, "Tap");
   elm_table_pack(tb, bx, 0, 0, 1, 1);

   /* Box of Double Tap icon and label */
   bx = create_gesture_box(win, infra->icons, 1, DOUBLE_TAP_NAME, "Double Tap");
   elm_table_pack(tb, bx, 1, 0, 1, 1);

   /* Box of Triple Tap icon and label */
   bx = create_gesture_box(win, infra->icons, 2, TRIPLE_TAP_NAME, "Triple Tap");
   elm_table_pack(tb, bx, 2, 0, 1, 1);

   /* Box of Long Tap icon and label */
   bx = create_gesture_box(win, infra->icons, 3, LONG_TAP_NAME, "Long Tap");
   elm_table_pack(tb, bx, 3, 0, 1, 1);

   /* Box of Momentum icon and label */
   bx = create_gesture_box(win, infra->icons, 4, MOMENTUM_NAME, "Momentum");
   elm_table_pack(tb, bx, 0, 2, 1, 1);

   /* Box of Line icon and label */
   bx = create_gesture_box(win, infra->icons, 5, LINE_NAME, "Line");
   elm_table_pack(tb, bx, 1, 2, 1, 1);

   /* Box of Flick icon and label */
   bx = create_gesture_box(win, infra->icons, 6, FLICK_NAME, "Flick");
   elm_table_pack(tb, bx, 2, 2, 1, 1);

   /* Box of Zoom icon and label */
   bx = create_gesture_box(win, infra->icons, 7, ZOOM_NAME, "Zoom");
   elm_table_pack(tb, bx, 0, 3, 1, 1);

   /* Box of Rotate icon and label */
   bx = create_gesture_box(win, infra->icons, 8, ROTATE_NAME, "Rotate");
   elm_table_pack(tb, bx, 1, 3, 1, 1);

   /* Legend of gestures - states */
   lb = elm_label_add(win);
   elm_object_text_set(lb, "<b>Gesture States</b>");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, lb, 0, 6, 4, 2);
   evas_object_show(lb);

   r = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_resize(r, 64, 64);
   evas_object_color_set(r, INI_R, INI_G, INI_B, INI_A);
   evas_object_size_hint_weight_set(r, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(r, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, r, 0, 7, 1, 1);
   evas_object_show(r);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Not Started");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, lb, 1, 7, 3, 1);
   evas_object_show(lb);

   r = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_resize(r, 64, 64);
   evas_object_color_set(r, START_COLOR);
   evas_object_size_hint_weight_set(r, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(r, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, r, 0, 8, 1, 1);
   evas_object_show(r);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Gesture START");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, lb, 1, 8, 3, 1);
   evas_object_show(lb);

   r = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_resize(r, 64, 64);
   evas_object_color_set(r, UPDATE_COLOR);
   evas_object_size_hint_weight_set(r, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(r, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, r, 0, 9, 1, 1);
   evas_object_show(r);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Gesture UPDATE");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, lb, 1, 9, 3, 1);
   evas_object_show(lb);

   r = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_resize(r, 64, 64);
   evas_object_color_set(r, END_COLOR);
   evas_object_size_hint_weight_set(r, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(r, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, r, 0, 10, 1, 1);
   evas_object_show(r);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Gesture END");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, lb, 1, 10, 3, 1);
   evas_object_show(lb);

   r = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_resize(r, 64, 64);
   evas_object_color_set(r, ABORT_COLOR);
   evas_object_size_hint_weight_set(r, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(r, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, r, 0, 11, 1, 1);
   evas_object_show(r);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Gesture ABORT");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, lb, 1, 11, 3, 1);

   elm_table_padding_set(tb, TB_PADDING_X, TB_PADDING_Y);
   evas_object_show(lb);
   /* END   - Building icons table */

   // LISTEN FOR GESTURES
   efl_event_callback_add(target, EFL_EVENT_GESTURE_TAP, tap_gesture_cb, infra);
   efl_event_callback_add(target, EFL_EVENT_GESTURE_LONG_TAP, long_tap_gesture_cb, infra);
   efl_event_callback_add(target, EFL_EVENT_GESTURE_DOUBLE_TAP, double_tap_gesture_cb, infra);
   efl_event_callback_add(target, EFL_EVENT_GESTURE_TRIPLE_TAP, triple_tap_gesture_cb, infra);
   efl_event_callback_add(target, EFL_EVENT_GESTURE_MOMENTUM, momentum_gesture_cb, infra);
   efl_event_callback_add(target, EFL_EVENT_GESTURE_FLICK, flick_gesture_cb, infra);

   /* Update color state 20 times a second */
   infra->colortimer = ecore_timer_add(0.05, _icon_color_set_cb, infra->icons);

   evas_object_show(win);
}
