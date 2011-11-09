#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

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
#define MOVE_COLOR 255, 255, 0, 255
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
};
typedef struct _infra_data infra_data;

void
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

infra_data *
_infra_data_alloc(void)
{
   infra_data *infra = malloc(sizeof(infra_data));
   infra->icons = calloc(N_GESTURE_TYPE, sizeof(icon_properties ));
   infra->colortimer = NULL;

   return infra;
}

static void
my_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{  /* called when my_win_main is requested to be deleted */
   _infra_data_free(data);
}

icon_properties *
_icon_properties_find(icon_properties *icons, char *name)
{
   int n;

   for(n = 0; n < N_GESTURE_TYPE; n++)
     if (!strcmp(icons[n].name, name))
       return &icons[n];

   return NULL;
}

void
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

   for(n = 0; n < N_GESTURE_TYPE; n++)
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


void
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
        elm_icon_file_set(i->icon, infra->buf, NULL);
        _icon_color_set(i, r, g, b, a);
     }
}

/* START - Callbacks for gestures */
static Evas_Event_Flags
n_finger_tap_start(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   _color_and_icon_set(data, TAP_NAME, p->n, MAX_TAP, START_COLOR);
   printf("N tap started <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
n_finger_tap_end(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

   printf("N tap ended <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   _color_and_icon_set(data, TAP_NAME, p->n, MAX_TAP, END_COLOR);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
n_finger_tap_abort(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   printf("N tap abort\n");
   _color_and_icon_set(data, TAP_NAME, p->n, MAX_TAP, ABORT_COLOR);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
n_long_tap_start(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

   printf("N long tap started <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   _color_and_icon_set(data, LONG_TAP_NAME, p->n, MAX_LONG_TAP, START_COLOR);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
n_long_tap_move(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   _color_and_icon_set(data, LONG_TAP_NAME, p->n, MAX_LONG_TAP, MOVE_COLOR);

   printf("N long tap moved <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
n_long_tap_end(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

   printf("N long tap ended <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   _color_and_icon_set(data, LONG_TAP_NAME, p->n, MAX_LONG_TAP, END_COLOR);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
n_long_tap_abort(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   _color_and_icon_set(data, LONG_TAP_NAME, p->n, MAX_LONG_TAP, ABORT_COLOR);
   printf("N long tap abort\n");
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
dbl_click_start(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

   _color_and_icon_set(data,DOUBLE_TAP_NAME, p->n, MAX_DOUBLE_TAP, START_COLOR);
   printf("Double click started <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
dbl_click_end(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   _color_and_icon_set(data, DOUBLE_TAP_NAME, p->n, MAX_DOUBLE_TAP, END_COLOR);

   printf("Double click ended <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
dbl_click_abort(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   _color_and_icon_set(data,DOUBLE_TAP_NAME, p->n, MAX_DOUBLE_TAP, ABORT_COLOR);

   printf("Double click abort\n");
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
triple_click_start(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   _color_and_icon_set(data,TRIPLE_TAP_NAME, p->n, MAX_TRIPLE_TAP, START_COLOR);

   printf("Triple click started <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
triple_click_end(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

   _color_and_icon_set(data, TRIPLE_TAP_NAME, p->n, MAX_TRIPLE_TAP, END_COLOR);
   printf("Triple click ended <%p> x,y=<%d,%d> count=<%d>\n",
         event_info, p->x, p->y, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
triple_click_abort(void *data , void *event_info)
{
   Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
   _color_and_icon_set(data,TRIPLE_TAP_NAME, p->n, MAX_TRIPLE_TAP, ABORT_COLOR);

   printf("Triple click abort\n");
   return EVAS_EVENT_FLAG_ON_HOLD;
}


static Evas_Event_Flags
momentum_start(void *data , void *event_info)
{
   Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
   _color_and_icon_set(data, MOMENTUM_NAME, p->n, MAX_MOMENTUM, START_COLOR);

   printf("momentum started x1,y1=<%d,%d> tx,ty=<%u,%u> n=<%u>\n",
         p->x1, p->y1, p->tx, p->ty, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
momentum_end(void *data , void *event_info)
{
   Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
   _color_and_icon_set(data, MOMENTUM_NAME, p->n, MAX_MOMENTUM, END_COLOR);
   printf("momentum ended x1,y1=<%d,%d> x2,y2=<%d,%d> tx,ty=<%u,%u> mx=<%d> my=<%d> n=<%u>\n",p->x1, p->y1, p->x2, p->y2, p->tx, p->ty, p->mx, p->my, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
momentum_abort(void *data , void *event_info)
{
   Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
   printf("momentum abort\n");
   _color_and_icon_set(data, MOMENTUM_NAME, p->n, MAX_MOMENTUM, ABORT_COLOR);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
momentum_move(void *data , void *event_info)
{
   Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;
   _color_and_icon_set(data, MOMENTUM_NAME, p->n, MAX_MOMENTUM, MOVE_COLOR);
   printf("momentum move x1,y1=<%d,%d> x2,y2=<%d,%d> tx,ty=<%u,%u> mx=<%d> my=<%d> n=<%u>\n", p->x1, p->y1, p->x2, p->y2, p->tx, p->ty, p->mx, p->my, p->n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
line_start(void *data , void *event_info)
{
   Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
   _color_and_icon_set(data, LINE_NAME, p->momentum.n, MAX_LINE, START_COLOR);

   printf("line started x1,y1=<%d,%d> x2,y2=<%d,%d> tx,ty=<%u,%u> n=<%u>\n", p->momentum.x1, p->momentum.y1, p->momentum.x2, p->momentum.y2, p->momentum.tx, p->momentum.ty, p->momentum.n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
line_move(void *data , void *event_info)
{
   Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
   _color_and_icon_set(data, LINE_NAME, p->momentum.n, MAX_LINE, MOVE_COLOR);
   printf("line move x1,y1=<%d,%d> x2,y2=<%d,%d> tx,ty=<%u,%u> n=<%u>\n", p->momentum.x1, p->momentum.y1, p->momentum.x2, p->momentum.y2, p->momentum.tx, p->momentum.ty, p->momentum.n);

   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
line_end(void *data , void *event_info)
{
   Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
   _color_and_icon_set(data, LINE_NAME, p->momentum.n, MAX_LINE, END_COLOR);
   printf("line end x1,y1=<%d,%d> x2,y2=<%d,%d> tx,ty=<%u,%u> n=<%u>\n", p->momentum.x1, p->momentum.y1, p->momentum.x2, p->momentum.y2, p->momentum.tx, p->momentum.ty, p->momentum.n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
line_abort(void *data , void *event_info)
{
   Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
   _color_and_icon_set(data, LINE_NAME, p->momentum.n, MAX_LINE, ABORT_COLOR);
   printf("line abort\n");
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
flick_start(void *data , void *event_info)
{
   Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
   _color_and_icon_set(data, FLICK_NAME, p->momentum.n, MAX_FLICK, START_COLOR);

   printf("flick started x1,y1=<%d,%d> tx,ty=<%u,%u> n=<%u>\n",
         p->momentum.x1, p->momentum.y1, p->momentum.tx,
         p->momentum.ty, p->momentum.n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
flick_end(void *data , void *event_info)
{
   Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
   _color_and_icon_set(data, FLICK_NAME, p->momentum.n, MAX_FLICK, END_COLOR);

   printf("flick ended x1,y1=<%d,%d> x2,y2=<%d,%d> tx,ty=<%u,%u> mx=<%d> my=<%d> n=<%u>\n",p->momentum.x1, p->momentum.y1, p->momentum.x2, p->momentum.y2, p->momentum.tx, p->momentum.ty, p->momentum.mx, p->momentum.my, p->momentum.n);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
flick_abort(void *data , void *event_info)
{
   Elm_Gesture_Line_Info *p = (Elm_Gesture_Line_Info *) event_info;
   _color_and_icon_set(data, FLICK_NAME, p->momentum.n, MAX_FLICK, ABORT_COLOR);
   printf("flick abort\n");
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
zoom_start(void *data , void *event_info)
{
   Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;
   _color_and_icon_set(data, ZOOM_NAME, MAX_ZOOM, MAX_ZOOM, START_COLOR);
   printf("zoom started <%d,%d> zoom=<%f> radius=<%d> momentum=<%f>\n",
         p->x, p->y, p->zoom, p->radius, p->momentum);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
zoom_move(void *data , void *event_info)
{
   Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;
   _color_and_icon_set(data, ZOOM_NAME, MAX_ZOOM, MAX_ZOOM, MOVE_COLOR);
   printf("zoom move <%d,%d> zoom=<%f> radius=<%d> momentum=<%f>\n",
         p->x, p->y, p->zoom, p->radius, p->momentum);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
zoom_end(void *data , void *event_info)
{
   Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;
   _color_and_icon_set(data, ZOOM_NAME, MAX_ZOOM, MAX_ZOOM, END_COLOR);
   printf("zoom end <%d,%d> zoom=<%f> radius=<%d> momentum=<%f>\n",
         p->x, p->y, p->zoom, p->radius, p->momentum);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
zoom_abort(void *data , void *event_info __UNUSED__)
{
   printf("zoom abort\n");
   _color_and_icon_set(data, ZOOM_NAME, MAX_ZOOM, MAX_ZOOM, ABORT_COLOR);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
rotate_start(void *data , void *event_info)
{
   Elm_Gesture_Rotate_Info *p = (Elm_Gesture_Rotate_Info *) event_info;
   _color_and_icon_set(data, ROTATE_NAME, MAX_ROTATE, MAX_ROTATE, START_COLOR);
   printf("rotate started <%d,%d> base=<%f> angle=<%f> radius=<%d> momentum=<%f>\n", p->x, p->y, p->base_angle, p->angle, p->radius, p->momentum);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
rotate_move(void *data , void *event_info)
{
   Elm_Gesture_Rotate_Info *p = (Elm_Gesture_Rotate_Info *) event_info;
   _color_and_icon_set(data, ROTATE_NAME, MAX_ROTATE, MAX_ROTATE, MOVE_COLOR);
   printf("rotate move <%d,%d> base=<%f> angle=<%f> radius=<%d> momentum=<%f>\n", p->x, p->y, p->base_angle, p->angle, p->radius, p->momentum);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
rotate_end(void *data , void *event_info)
{
   Elm_Gesture_Rotate_Info *p = (Elm_Gesture_Rotate_Info *) event_info;
   _color_and_icon_set(data, ROTATE_NAME, MAX_ROTATE, MAX_ROTATE, END_COLOR);
   printf("rotate end <%d,%d> base=<%f> angle=<%f> radius=<%d> momentum=<%f>\n", p->x, p->y, p->base_angle, p->angle, p->radius, p->momentum);
   return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
rotate_abort(void *data , void *event_info __UNUSED__)
{
   _color_and_icon_set(data, ROTATE_NAME, MAX_ROTATE, MAX_ROTATE, ABORT_COLOR);
   printf("rotate abort\n");
   return EVAS_EVENT_FLAG_ON_HOLD;
}
/* END   - Callbacks for gestures */

Evas_Object *create_gesture_box(Evas_Object *win, icon_properties *icons,
      int idx, const char *name, const char *lb_txt)
{  /* Creates a box with icon and label, later placed in a table */
   Evas_Object *lb, *bx = elm_box_add(win);
   char buf[1024];

   elm_box_padding_set(bx, BX_PADDING_X, BX_PADDING_Y);
   icons[idx].icon = elm_icon_add(win);
   icons[idx].name = name;
   snprintf(buf, sizeof(buf), "%s/images/g_layer/%s_1.png",
         elm_app_data_dir_get(), icons[idx].name);
   elm_icon_file_set(icons[idx].icon, buf, NULL);
   elm_icon_scale_set(icons[idx].icon, 0, 0);
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
test_gesture_layer2(void *data __UNUSED__, Evas_Object *obj __UNUSED__,
      void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *tb, *lb, *bx;
   Evas_Object *r; /* Gesture layer transparent object */
   Evas_Object *g; /* The Gesture Layer object */

   infra_data *infra = _infra_data_alloc();

   win = elm_win_add(NULL, "table", ELM_WIN_BASIC);
   elm_win_title_set(win, "Table");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", my_win_del, infra);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   /* START - Building icons table */
   bx = elm_box_add(win);
   tb = elm_table_add(win);
   elm_box_pack_end(bx, tb);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, 0.5, 0.5);
   evas_object_show(tb);
   evas_object_show(bx);

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
   evas_object_color_set(r, MOVE_COLOR);
   evas_object_size_hint_weight_set(r, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(r, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, r, 0, 9, 1, 1);
   evas_object_show(r);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Gesture MOVE");
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

   /* Gesture layer transparent object */
   r = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(r, 0, 0, 0, 0);
   elm_win_resize_object_add(win, r);
   g = elm_gesture_layer_add(r);
   elm_gesture_layer_attach(g, r);
   evas_object_show(r);

   /* START - Setting gestures callbacks */
#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_TRIPLE_TAPS,
         ELM_GESTURE_STATE_START, triple_click_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_TRIPLE_TAPS,
         ELM_GESTURE_STATE_END, triple_click_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_TRIPLE_TAPS,
         ELM_GESTURE_STATE_ABORT, triple_click_abort, infra);
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_DOUBLE_TAPS,
         ELM_GESTURE_STATE_START, dbl_click_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_DOUBLE_TAPS,
         ELM_GESTURE_STATE_END, dbl_click_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_DOUBLE_TAPS,
         ELM_GESTURE_STATE_ABORT, dbl_click_abort, infra);
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_TAPS,
         ELM_GESTURE_STATE_START, n_finger_tap_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_TAPS,
         ELM_GESTURE_STATE_END, n_finger_tap_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_TAPS,
         ELM_GESTURE_STATE_ABORT, n_finger_tap_abort, infra);
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LONG_TAPS,
         ELM_GESTURE_STATE_START, n_long_tap_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LONG_TAPS,
         ELM_GESTURE_STATE_MOVE, n_long_tap_move, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LONG_TAPS,
         ELM_GESTURE_STATE_END, n_long_tap_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LONG_TAPS,
         ELM_GESTURE_STATE_ABORT, n_long_tap_abort, infra);
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_MOMENTUM,
         ELM_GESTURE_STATE_START, momentum_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_MOMENTUM,
         ELM_GESTURE_STATE_END, momentum_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_MOMENTUM,
         ELM_GESTURE_STATE_ABORT, momentum_abort, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_MOMENTUM,
         ELM_GESTURE_STATE_MOVE, momentum_move, infra);
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LINES,
         ELM_GESTURE_STATE_START, line_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LINES,
         ELM_GESTURE_STATE_MOVE, line_move, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LINES,
         ELM_GESTURE_STATE_END, line_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_LINES,
         ELM_GESTURE_STATE_ABORT, line_abort, infra);
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_FLICKS,
         ELM_GESTURE_STATE_START, flick_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_FLICKS,
         ELM_GESTURE_STATE_END, flick_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_N_FLICKS,
         ELM_GESTURE_STATE_ABORT, flick_abort, infra);
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ZOOM,
         ELM_GESTURE_STATE_START, zoom_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ZOOM,
         ELM_GESTURE_STATE_END, zoom_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ZOOM,
         ELM_GESTURE_STATE_ABORT, zoom_abort, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ZOOM,
         ELM_GESTURE_STATE_MOVE, zoom_move, infra);
   /* elm_gesture_layer_zoom_step_set(g, 0.2); */
#endif

#if 1
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ROTATE,
         ELM_GESTURE_STATE_START, rotate_start, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ROTATE,
         ELM_GESTURE_STATE_END, rotate_end, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ROTATE,
         ELM_GESTURE_STATE_ABORT, rotate_abort, infra);
   elm_gesture_layer_cb_set(g, ELM_GESTURE_ROTATE,
         ELM_GESTURE_STATE_MOVE, rotate_move, infra);
   /* elm_gesture_layer_rotate_step_set(g, 5.2); */
#endif
   /* END   - Setting gestures callbacks */

   /* Update color state 20 times a second */
   infra->colortimer = ecore_timer_add(0.05, _icon_color_set_cb, infra->icons);

   evas_object_show(win);
}
#endif
