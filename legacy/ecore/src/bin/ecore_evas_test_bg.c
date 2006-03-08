#include "ecore_evas_test.h"

#ifdef BUILD_ECORE_EVAS

Evas_Object *o_bg_rect  = NULL;
Evas_Object *o_logo     = NULL;
Ecore_Timer *anim_timer = NULL;

#define NUM 32
#define SIZE 32
#define SLIDE 80

Evas_Object *o_ball[NUM];
Evas_Object *o_shad[NUM];

Evas_Object *o_clip = NULL;
Evas_Object *o_bar[2];
Evas_Object *o_bar_shad_1[3];
Evas_Object *o_bar_shad_2[2];
Evas_Object *o_bg = NULL;

static int bg_animate_obj_timer(void *data);
static void bg_cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info);

void
bg_resize(double w, double h)
{
   evas_object_resize(o_bg_rect, w, h);
   evas_object_move(o_logo, (w - 240) / 2, (h - 280) / 2);
   bg_animate_obj_timer(NULL);
}

void
bg_start(void)
{
   Evas_Object *o;
	 
   /* set up a big white rectangle on the bg */
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 240, 320);
   evas_object_layer_set(o, -1000000);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, bg_cb_key_down, ee);
   evas_object_focus_set(o, 1);
   evas_object_show(o);
   o_bg_rect = o;
   
   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"e_logo.png", NULL);
   evas_object_resize(o, 240, 280);
   evas_object_image_fill_set(o, 0, 0, 240, 280);
   evas_object_move(o, (240 - 240) / 2, (320 - 280) / 2);
   evas_object_layer_set(o, -999);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
   o_logo = o;
   
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_resize(o, 240, 0);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
   o_clip = o;

   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bg.png", NULL);
   evas_object_resize(o, 240, 320);
   evas_object_image_fill_set(o, 0, 0, 240, 320);
   evas_object_layer_set(o, -9999);
   evas_object_move(o, 0, 0);
   evas_object_clip_set(o, o_clip);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
   o_bg = o;
   
   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bar.png", NULL);
   evas_object_image_border_set(o, 5, 5, 5, 5);
   evas_object_resize(o, 240, 32);
   evas_object_image_fill_set(o, 0, 0, 240, 32);
   evas_object_layer_set(o, 100);
   evas_object_pass_events_set(o, 1);
   o_bar[0] = o;

   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bar.png", NULL);
   evas_object_image_border_set(o, 5, 5, 5, 5);
   evas_object_resize(o, 240, 32);
   evas_object_image_fill_set(o, 0, 0, 240, 32);
   evas_object_layer_set(o, 100);
   evas_object_pass_events_set(o, 1);
   o_bar[1] = o;
   
   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bar_shad_above.png", NULL);
   evas_object_resize(o, 240, 8);
   evas_object_image_fill_set(o, 0, 0, 240, 8);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_layer_set(o, 100);
   evas_object_pass_events_set(o, 1);
   o_bar_shad_1[0] = o;

   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bar_shad_below.png", NULL);
   evas_object_resize(o, 240, 32);
   evas_object_image_fill_set(o, 0, 0, 240, 32);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_layer_set(o, -100);
   evas_object_pass_events_set(o, 1);
   evas_object_clip_set(o, o_clip);
   o_bar_shad_1[1] = o;

   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bar_shad_below.png", NULL);
   evas_object_resize(o, 240, 16);
   evas_object_image_fill_set(o, 0, 0, 240, 16);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_layer_set(o, 100);
   evas_object_pass_events_set(o, 1);
   evas_object_clip_set(o, o_clip);
   o_bar_shad_1[2] = o;
   
   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bar_shad_above.png", NULL);
   evas_object_resize(o, 240, 8);
   evas_object_image_fill_set(o, 0, 0, 240, 8);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_layer_set(o, 100);
   evas_object_pass_events_set(o, 1);
   evas_object_clip_set(o, o_clip);
   o_bar_shad_2[0] = o;
   
   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, IM"bar_shad_below.png", NULL);
   evas_object_resize(o, 240, 8);
   evas_object_image_fill_set(o, 0, 0, 240, 8);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_layer_set(o, 100);
   evas_object_pass_events_set(o, 1);
   o_bar_shad_2[1] = o;
   
     {
	int i;
	
	for (i = 0; i < NUM; i++)
	  {
	     o = evas_object_image_add(evas);
	     evas_object_image_file_set(o, IM"ball.png", NULL);
	     evas_object_resize(o, SIZE, SIZE);
	     evas_object_image_fill_set(o, 0, 0, SIZE, SIZE);
	     evas_object_move(o, 0, 0);
	     evas_object_pass_events_set(o, 1);
	     evas_object_clip_set(o, o_clip);
	     o_ball[i] = o;
	     o = evas_object_image_add(evas);
	     evas_object_image_file_set(o, IM"shadow.png", NULL);
	     evas_object_resize(o, SIZE, SIZE);
	     evas_object_image_fill_set(o, 0, 0, SIZE, SIZE);
	     evas_object_move(o, 0, 0);
	     evas_object_pass_events_set(o, 1);
	     evas_object_clip_set(o, o_clip);
	     evas_object_lower(o);
	     o_shad[i] = o;
	  }
     }
}

void
bg_go(void)
{
   /* add a timer to animate them */
   anim_timer = ecore_timer_add(0.01, bg_animate_obj_timer, NULL);
}

static int
bg_animate_obj_timer(void *data)
{
   Evas_Object *o;
   Evas_Coord x, y, sx, sy;
   Evas_Coord w, h;
   double v;
   int ow, oh;
   double t;
   int l, m, n;
   int i;
   static double t_first = 0;
   static int done = 0;
   
   t = ecore_time_get() - start_time;
   if (t_first == 0) t_first = t;
   if (t - t_first > 2.0) done = 1;
   evas_output_viewport_get(evas_object_evas_get(o_clip), NULL, NULL, &w, &h);
   if (!done)
     {
	v = ((cos(((t - t_first) / 2) * 3.14159) - 1.0) / -2);
	l = (SLIDE) * v;
     }
   else
     {
	l = SLIDE;
     }
   evas_object_move(o_bar[0], 0, (h / 2) - 32 - l);
   evas_object_move(o_bar[1], 0, (h / 2) + l);
   evas_object_move(o_bar_shad_1[0], 0, (h / 2) - 32 - l - 8);
   evas_object_move(o_bar_shad_1[1], 0, (h / 2) - l);
   evas_object_move(o_bar_shad_1[2], 0, (h / 2) - l);
   evas_object_move(o_bar_shad_2[0], 0, (h / 2) + l - 8);
   evas_object_move(o_bar_shad_2[1], 0, (h / 2) + 32 + l);

   evas_object_resize(o_bar[0], w, 32);
   evas_object_image_fill_set(o_bar[0], 0, 0, w, 32);

   evas_object_resize(o_bar[1], w, 32);
   evas_object_image_fill_set(o_bar[1], 0, 0, w, 32);
   
   evas_object_resize(o_bar_shad_1[0], w, 8);
   evas_object_image_fill_set(o_bar_shad_1[0], 0, 0, w, 8);

   evas_object_resize(o_bar_shad_1[1], w, 32);
   evas_object_image_fill_set(o_bar_shad_1[1], 0, 0, w, 32);

   evas_object_resize(o_bar_shad_1[2], w, 16);
   evas_object_image_fill_set(o_bar_shad_1[2], 0, 0, w, 16);

   evas_object_resize(o_bar_shad_2[0], w, 8);
   evas_object_image_fill_set(o_bar_shad_2[0], 0, 0, w, 8);
   
   evas_object_resize(o_bar_shad_2[1], w, 8);
   evas_object_image_fill_set(o_bar_shad_2[1], 0, 0, w, 8);
   
   evas_object_move(o_clip, 0, (h / 2) - l);
   evas_object_resize(o_clip, w, l * 2);
   
   evas_object_resize(o_bg, w, h);
   evas_object_image_fill_set(o_bg, 0, 0, w, h);
   
   evas_object_show(o_bar[0]);
   evas_object_show(o_bar[1]);
   evas_object_show(o_bar_shad_1[0]);
   evas_object_show(o_bar_shad_1[1]);
   evas_object_show(o_bar_shad_1[2]);
   evas_object_show(o_bar_shad_2[0]);
   evas_object_show(o_bar_shad_2[1]);
   for (i = 0; i < NUM; i++)
     {
	o = o_ball[i];
	evas_output_viewport_get(evas_object_evas_get(o), NULL, NULL, &w, &h);
	l = ((int)o / 179) & 0x0f;
	v = (sin(t * (double)l / 12) + 1.0) / 2;
	v = 0.25 + (0.75 * v);
	m = 1 + (((int)o / 36) & 0x0f);
	n = 1 + (((int)o / 763) & 0x0f);
	ow = SIZE * v;
	oh = SIZE * v;
	evas_object_resize(o, ow, oh);
	evas_object_image_fill_set(o, 0, 0, ow, oh);
	x = sin(t * (double)m / 12) * ((w - ow) / 2);
	y = sin(t * (double)n / 12) * ((h - oh) / 2);
	evas_object_move(o, (w / 2) - (ow / 2) + x, (h / 2) - (oh / 2) + y);
	evas_object_show(o);
	o = o_shad[i];
	evas_object_resize(o, ow, oh);
	evas_object_image_fill_set(o, 0, 0, ow, oh);
	sx = v * (((w - ow) / 2)  + x) / 8;
	sy = v * (((h - oh) / 2)  + y) / 8;
	evas_object_move(o,
			 (w / 2) - (ow / 2) + x + sx, 
			 (h / 2) - (oh / 2) + y + sy);
	evas_object_show(o);	
     }
   return 1;
}

/* NB: on ipaq
 * 
 * "F1"   "F2"    "F3"   "F4"
 *            "Up"
 *   "Left" "Return" "Right"
 *           "Down"
 * 
 */
static void
bg_cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   
   ev = event_info;
   if (!strcmp(ev->keyname, "Escape")) ecore_main_loop_quit();
   if (!strcmp(ev->keyname, "q")) ecore_main_loop_quit();
   if (!strcmp(ev->keyname, "Q")) ecore_main_loop_quit();
   if ((!strcmp(ev->keyname, "f")) ||
       (!strcmp(ev->keyname, "F1")))
     {
	if (!ecore_evas_fullscreen_get(ee))
	  ecore_evas_fullscreen_set(ee, 1);
	else
	  ecore_evas_fullscreen_set(ee, 0);
     }
   if (!strcmp(ev->keyname, "o"))
     {
	if (!ecore_evas_override_get(ee))
	  ecore_evas_override_set(ee, 1);
	else
	  ecore_evas_override_set(ee, 0);
     }
   if ((!strcmp(ev->keyname, "r")) ||
       (!strcmp(ev->keyname, "F4")))
     {
	int r;
	
	r = ecore_evas_rotation_get(ee);
	if      (r == 0)   r = 90;
	else if (r == 90)  r = 270;
	else if (r == 270) r = 0;
	ecore_evas_rotation_set(ee, r);
     }
   if (!strcmp(ev->keyname, "b"))
     {
	if (!ecore_evas_borderless_get(ee))
	  ecore_evas_borderless_set(ee, 1);
	else
	  ecore_evas_borderless_set(ee, 0);
     }
   if (!strcmp(ev->keyname, "d"))
     {
	if (!ecore_evas_avoid_damage_get(ee))
	  ecore_evas_avoid_damage_set(ee, 1);
	else
	  ecore_evas_avoid_damage_set(ee, 0);
     }
   if (!strcmp(ev->keyname, "s"))
     {
	if (!ecore_evas_shaped_get(ee))
	  {
	     evas_object_hide(o_bg_rect);
	     evas_object_hide(o_bg);
	     ecore_evas_shaped_set(ee, 1);
	  }
	else
	  {
	     evas_object_show(o_bg_rect);
	     evas_object_show(o_bg);
	     ecore_evas_shaped_set(ee, 0);
	  }
     }
   if (!strcmp(ev->keyname, "a"))
     {
	if (!ecore_evas_alpha_get(ee))
	  {
	     evas_object_hide(o_bg_rect);
	     evas_object_hide(o_bg);
	     ecore_evas_alpha_set(ee, 1);
	  }
	else
	  {
	     evas_object_show(o_bg_rect);
	     evas_object_show(o_bg);
	     ecore_evas_alpha_set(ee, 0);
	  }
     }
   if (!strcmp(ev->keyname, "Up"))
     {
#ifdef BUILD_ECORE_FB
	double br;
	
	br = ecore_fb_backlight_brightness_get();
	ecore_fb_backlight_brightness_set(br + 0.1);
#endif	
     }
   if (!strcmp(ev->keyname, "Down"))
     {
#ifdef BUILD_ECORE_FB
	double br;
	
	br = ecore_fb_backlight_brightness_get();
	ecore_fb_backlight_brightness_set(br - 0.1);
#endif	
     }
   if (!strcmp(ev->keyname, "F2"))
     {
#ifdef BUILD_ECORE_FB
	if (ecore_fb_backlight_get())
	  ecore_fb_backlight_set(0);
	else
	  ecore_fb_backlight_set(1);
#endif	
     }
   if (!strcmp(ev->keyname, "F3"))
     {
#ifdef BUILD_ECORE_FB
	static int v = 0;
	
	if (v) ecore_fb_led_set(0);
	else ecore_fb_led_set(1);
	if (!v) v = 1;
	else v = 0;
#endif	
     }
   if (!strcmp(ev->keyname, "Left"))
     {
#ifdef BUILD_ECORE_FB
	ecore_fb_led_blink_set(0.1);
#endif	
     }
   if (!strcmp(ev->keyname, "Right"))
     {
#ifdef BUILD_ECORE_FB
	ecore_fb_led_blink_set(0.5);
#endif	
     }
   if ((!strcmp(ev->keyname, "p")) ||
       (!strcmp(ev->keyname, "Return")))
     {
	char *fl;
	
	ecore_evas_cursor_get(ee, &fl, NULL, NULL, NULL);
	if (!fl)
	  ecore_evas_cursor_set(ee, PT"mouse_pointer.png", 1000000, 2, 2);
	else
	  ecore_evas_cursor_set(ee, NULL, 0, 0, 0);
#ifdef BUILD_ECORE_FB
	printf("%3.3f\n", ecore_fb_light_sensor_get());
#endif	
     }
}
#endif
