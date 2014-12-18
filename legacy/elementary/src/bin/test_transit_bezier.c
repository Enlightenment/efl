#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define SEGMENT_MAX 200
#define WIN_W 400
#define BTN_SIZE 50
#define WIN_H (400 + BTN_SIZE)
#define CTRL_W 15
#define CTRL_H 15

typedef struct
{
   Evas *e;
   Evas_Object *win;
   Evas_Object *ctrl_pt1;
   Evas_Object *ctrl_pt2;
   Evas_Object *ctrl_pt1_line;
   Evas_Object *ctrl_pt2_line;
   Evas_Object *label;
   Evas_Object *line[SEGMENT_MAX];
   Eina_Bool ctrl_pt1_down;
   Eina_Bool ctrl_pt2_down;
} transit_data;

static void
v_get(transit_data *td, double *v1, double *v2, double *v3, double *v4)
{
   Evas_Coord x, w, y, h;

   //update control points values
   evas_object_geometry_get(td->ctrl_pt1, &x, &y, &w, &h);
   *v1 = (double) (x - (w/2)) / WIN_W;
   *v2 = 1 - (double) (y - (h/2)) / (WIN_H - BTN_SIZE);

   evas_object_geometry_get(td->ctrl_pt2, &x, &y, &w, &h);
   *v3 = (double) (x - (w/2)) / WIN_W;
   *v4 = 1 - (double) (y - (h/2)) / (WIN_H - BTN_SIZE);
}

static void
update_curve(transit_data *td)
{
   double v[4];
   int i = 0;
   double progress;
   Evas_Coord prev_x = 0, prev_y = (WIN_H - BTN_SIZE - 1);
   Evas_Coord cur_x, cur_y;
   double tx, ty;
   char buf[256];

   v_get(td, &v[0], &v[1], &v[2], &v[3]);

   for (i = 0; i < SEGMENT_MAX; i++)
     {
        progress = (((double) i) / (SEGMENT_MAX - 1));

        tx = (pow((1 - progress), 3) * 0) +
             (3 * progress * pow((1 - progress), 2) * v[0]) +
             (3 * pow(progress, 2) * (1 - progress) * v[2]) +
             (pow(progress, 3) * 1);
        ty = (pow((1 - progress), 3) * 0) +
             (3 * progress * pow((1 - progress), 2) * v[1]) +
             (3 * pow(progress, 2) * (1 - progress) * v[3]) +
             (pow(progress, 3) * 1);

        cur_x = (int)(tx * WIN_W);
        cur_y = ((WIN_H - BTN_SIZE - 1) - ((int)(ty * (WIN_H - BTN_SIZE))));
        evas_object_line_xy_set(td->line[i], prev_x, prev_y, cur_x, cur_y);
        evas_object_anti_alias_set(td->line[i], EINA_TRUE);
        evas_object_color_set(td->line[i], 255, 50, 50, 255);
        prev_x = cur_x;
        prev_y = cur_y;
     }
   snprintf(buf, sizeof(buf),
            "<align=left><b>Control Points:</b></br>"
            "x1: %0.2f   y1: %0.2f</br>"
            "x2: %0.2f   y2: %0.2f</align>",
            v[0], v[1], v[2], v[3]);
   elm_object_text_set(td->label, buf);
}

static void
ctrl_pt1_mouse_move_cb(void *data, Evas *e EINA_UNUSED,
                       Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   transit_data *td = data;
   Evas_Coord x, y, w, h;

   if (!td->ctrl_pt1_down) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   x = (ev->cur.canvas.x - (w/2));
   y = (ev->cur.canvas.y - (h/2));
   if (x < -(w/2)) x = -(w/2);
   if (y < -(h/2)) y = -(h/2);
   if (x > WIN_W) x = WIN_W - (w/2);
   if (y > WIN_H) y = WIN_H - (h/2);
   evas_object_move(obj, x, y);
   evas_object_line_xy_set(td->ctrl_pt1_line,
                           (x + (w/2)), (y + (h/2)), 0, WIN_W);
   update_curve(td);
}

static void
ctrl_pt2_mouse_move_cb(void *data, Evas *e EINA_UNUSED,
                       Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   transit_data *td = data;
   Evas_Coord x, y, w, h;

   if (!td->ctrl_pt2_down) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   x = (ev->cur.canvas.x - (w/2));
   y = (ev->cur.canvas.y - (h/2));
   if (x < -(w/2)) x = -(w/2);
   if (y < -(h/2)) y = -(h/2);
   if (x > WIN_W) x = WIN_W - (w/2);
   if (y > WIN_H) y = WIN_H - (h/2);
   evas_object_move(obj, x, y);
   evas_object_line_xy_set(td->ctrl_pt2_line,
                           (x + (w/2)), (y + (h/2)), WIN_W, 0);
   update_curve(td);
}

static void
ctrl_pt1_mouse_down_cb(void *data, Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   transit_data *td = data;
   td->ctrl_pt1_down = EINA_TRUE;
}

static void
ctrl_pt1_mouse_up_cb(void *data, Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   transit_data *td = data;
   td->ctrl_pt1_down = EINA_FALSE;
}

static void
ctrl_pt2_mouse_down_cb(void *data, Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   transit_data *td = data;
   td->ctrl_pt2_down = EINA_TRUE;
}

static void
ctrl_pt2_mouse_up_cb(void *data, Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   transit_data *td = data;
   td->ctrl_pt2_down = EINA_FALSE;
}

static void
transit_del_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
   transit_data *td = data;
   evas_object_show(td->ctrl_pt1);
   evas_object_show(td->ctrl_pt2);
   evas_object_show(td->ctrl_pt1_line);
   evas_object_show(td->ctrl_pt2_line);
}

static void
btn_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double v[4];
   transit_data *td = data;
   v_get(td, &v[0], &v[1], &v[2], &v[3]);

   Elm_Transit *transit = elm_transit_add();
   elm_transit_object_add(transit, obj);
   elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_BEZIER_CURVE);
   elm_transit_tween_mode_factor_n_set(transit, 4, v);
   elm_transit_effect_translation_add(transit, 0, 0, (WIN_W - BTN_SIZE), 0);
   elm_transit_auto_reverse_set(transit, EINA_TRUE);
   elm_transit_del_cb_set(transit, transit_del_cb, td);
   elm_transit_duration_set(transit, 1);
   elm_transit_go(transit);

   evas_object_hide(td->ctrl_pt1);
   evas_object_hide(td->ctrl_pt2);
   evas_object_hide(td->ctrl_pt1_line);
   evas_object_hide(td->ctrl_pt2_line);
}

void
test_transit_bezier(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   static transit_data td;
   char buf[PATH_MAX];
   Evas_Object *bg;
   int i;

   memset(&td, 0x0, sizeof(td));

   //Win
   td.win = elm_win_add(NULL, "test",  ELM_WIN_BASIC);
   elm_win_title_set(td.win, "Transit Bezier");
   elm_win_autodel_set(td.win, EINA_TRUE);

   td.e = evas_object_evas_get(td.win);

   //BG. Fix window size
   bg = elm_bg_add(td.win);
   evas_object_size_hint_min_set(bg, WIN_W, WIN_H);
   elm_win_resize_object_add(td.win, bg);
   evas_object_show(bg);

   snprintf(buf, sizeof(buf), "%s/images/bubble.png", elm_app_data_dir_get());

   //Create Lines
   for (i = 0; i < SEGMENT_MAX; i++)
     {
        td.line[i] = evas_object_line_add(td.e);
        evas_object_resize(td.line[i], WIN_W, (WIN_H - BTN_SIZE));
        evas_object_show(td.line[i]);
     }

   //Control Point 1 Line
   td.ctrl_pt1_line = evas_object_line_add(td.e);
   evas_object_pass_events_set(td.ctrl_pt1_line, EINA_TRUE);
   evas_object_resize(td.ctrl_pt1_line, WIN_W, WIN_H);
   evas_object_anti_alias_set(td.ctrl_pt1_line, EINA_TRUE);
   evas_object_color_set(td.ctrl_pt1_line, 0, 174, 219, 255);
   evas_object_line_xy_set(td.ctrl_pt1_line, 0, WIN_W,
                           (100 + (CTRL_W/2)) , (300 + (CTRL_H/2)));
   evas_object_show(td.ctrl_pt1_line);

   //Control Point 1
   td.ctrl_pt1 = evas_object_image_filled_add(td.e);
   evas_object_image_file_set(td.ctrl_pt1, buf, NULL);
   evas_object_resize(td.ctrl_pt1, CTRL_W, CTRL_H);
   evas_object_move(td.ctrl_pt1, 100, 300);
   evas_object_event_callback_add(td.ctrl_pt1, EVAS_CALLBACK_MOUSE_MOVE,
                                  ctrl_pt1_mouse_move_cb, &td);
   evas_object_event_callback_add(td.ctrl_pt1, EVAS_CALLBACK_MOUSE_DOWN,
                                  ctrl_pt1_mouse_down_cb, &td);
   evas_object_event_callback_add(td.ctrl_pt1, EVAS_CALLBACK_MOUSE_UP,
                                  ctrl_pt1_mouse_up_cb, &td);
   evas_object_show(td.ctrl_pt1);

   //Control Point 2 Line
   td.ctrl_pt2_line = evas_object_line_add(td.e);
   evas_object_pass_events_set(td.ctrl_pt2_line, EINA_TRUE);
   evas_object_resize(td.ctrl_pt2_line, WIN_W, WIN_H);
   evas_object_anti_alias_set(td.ctrl_pt2_line, EINA_TRUE);
   evas_object_color_set(td.ctrl_pt2_line, 219, 174, 0, 255);
   evas_object_line_xy_set(td.ctrl_pt2_line, WIN_W, 0,
                           (300 + (CTRL_W/2)), (100 + (CTRL_H/2)));
   evas_object_show(td.ctrl_pt2_line);

   //Control Point 2
   td.ctrl_pt2 = evas_object_image_filled_add(td.e);
   evas_object_image_file_set(td.ctrl_pt2, buf, NULL);
   evas_object_resize(td.ctrl_pt2, CTRL_W, CTRL_H);
   evas_object_move(td.ctrl_pt2, 300, 100);
   evas_object_event_callback_add(td.ctrl_pt2, EVAS_CALLBACK_MOUSE_MOVE,
                                  ctrl_pt2_mouse_move_cb, &td);
   evas_object_event_callback_add(td.ctrl_pt2, EVAS_CALLBACK_MOUSE_DOWN,
                                  ctrl_pt2_mouse_down_cb, &td);
   evas_object_event_callback_add(td.ctrl_pt2, EVAS_CALLBACK_MOUSE_UP,
                                  ctrl_pt2_mouse_up_cb, &td);
   evas_object_show(td.ctrl_pt2);

   //Label
   td.label = elm_label_add(td.win);
   evas_object_resize(td.label, WIN_W, 50);
   evas_object_pass_events_set(td.label, EINA_TRUE);
   evas_object_show(td.label);

   //Button
   Evas_Object *btn = elm_button_add(td.win);
   elm_object_text_set(btn, "Go");
   evas_object_smart_callback_add(btn, "clicked", btn_clicked_cb, &td);
   evas_object_resize(btn, BTN_SIZE, BTN_SIZE);
   evas_object_move(btn, 0, (WIN_H - BTN_SIZE));
   evas_object_show(btn);

   update_curve(&td);

   evas_object_resize(td.win, WIN_W, WIN_H);
   evas_object_show(td.win);
}
