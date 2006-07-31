#include "ecore_evas_test.h"

#ifdef BUILD_ECORE_EVAS

static Evas_Object *o_events = NULL;
static Evas_Object *o_crosshair = NULL;
static Evas_Object *o_text = NULL;
static int cal_pos = 0;
static int down = 0;

static int cal_coords[] = {15, 15, -15, 15, 15, -15, -15, -15};
static char *cal_lines[] = 
{
   "Please click on the crosshair",
     "Again please",
     "And again",
     "Last one,  then calibration is complete"
};
static int cal_input[8];

static int tmp_input_count = 0;
static int tmp_input[16];

static void calibrate_cb_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void calibrate_cb_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void calibrate_cb_move(void *data, Evas *e, Evas_Object *obj, void *event_info);

void
calibrate_pos_set(int pos)
{
   Evas_Coord w, h, ow, oh;
   int x, y;
   
   cal_pos = pos;
   evas_object_geometry_get(o_crosshair, NULL, NULL, &w, &h);
   x = cal_coords[(cal_pos * 2) + 0];
   y = cal_coords[(cal_pos * 2) + 1];
   evas_output_viewport_get(evas, NULL, NULL, &ow, &oh);
   if (x < 0) x = ow + x - 1;
   if (y < 0) y = oh + y - 1;
   cal_coords[(cal_pos * 2) + 0] = x;
   cal_coords[(cal_pos * 2) + 1] = y;
   evas_object_move(o_crosshair, x - (((int)w - 1) / 2), y - (((int)h - 1) / 2));
   evas_object_text_text_set(o_text, cal_lines[cal_pos]);
   evas_object_geometry_get(o_text, NULL, NULL, &w, &h);   
   evas_object_move(o_text, (ow - w) / 2, (oh - h) / 2);
}

int
calibrate_pos_get(void)
{
   return cal_pos;
}

void
calibrate_finish(void)
{
   int m0, m1;
   int y0, y1;
   int x[4], y[4], xi[4], yi[4];
   int i, rot;
   
   int xscale, xtrans, yscale, ytrans, xyswap;
   
   rot = ecore_evas_rotation_get(ee);
   for (i = 0; i < 4; i++)
     {
	x[i] = cal_coords[(i * 2) + 0];
	y[i] = cal_coords[(i * 2) + 1];
	xi[i] = cal_input[(i * 2) + 0];
	yi[i] = cal_input[(i * 2) + 1];
     }
   xyswap = 0;
   
   m0 = ((x[1] - x[0]) * 256) / (xi[1] - xi[0]);
   y0 = ((x[1] - ((xi[1] * m0) / 256)) + (x[0] - ((xi[0] * m0) >> 8)) ) / 2;

   m1 = ((x[3] - x[2]) * 256) / (xi[3] - xi[2]);
   y1 = ((x[3] - ((xi[3] * m1) / 256)) + (x[2] - ((xi[2] * m1) >> 8)) ) / 2;

   xscale = (m0 + m1) / 2;
   xtrans = (y0 + y1) / 2;

   m0 = ((y[2] - y[0]) * 256) / (yi[2] - yi[0]);
   y0 = ((y[2] - ((yi[2] * m0) / 256)) + (y[0] - ((yi[0] * m0) >> 8)) ) / 2;

   m1 = ((y[3] - y[1]) * 256) / (yi[3] - yi[1]);
   y1 = ((y[3] - ((yi[3] * m1) / 256)) + (y[1] - ((yi[1] * m1) >> 8)) ) / 2;
   
   yscale = (m0 + m1) / 2;
   ytrans = (y0 + y1) / 2;
   
   if (rot == 0)
     {
#ifdef BUILD_ECORE_FB
#if 0
	ecore_fb_touch_screen_calibrate_set(xscale, xtrans, yscale, ytrans, xyswap);
#endif
#endif	
     }
   else if (rot == 270)
     {
	int ow, oh;
	
	evas_output_size_get(evas, &ow, &oh);
	ytrans = oh - (ytrans + ((oh * yscale) / 256));
#ifdef BUILD_ECORE_FB	
#if 0
	ecore_fb_touch_screen_calibrate_set(yscale, ytrans, xscale, xtrans, xyswap);
#endif	
#endif	
     }
     
   evas_object_del(o_crosshair);
   evas_object_del(o_events);
   evas_object_del(o_text);
   o_crosshair = NULL;
   o_events = NULL;
   o_text = NULL;
   cal_pos = 0;
   bg_go();
}

void
calibrate_start(void)
{
   Evas_Object *o;
   
#ifdef BUILD_ECORE_FB	
#if 0
   ecore_fb_touch_screen_calibrate_set(256, 0, 256, 0, 0);
#endif	
#endif
   
   o = evas_object_rectangle_add(evas);
   evas_object_layer_set(o, 1000000); 
   evas_object_color_set(o, 255, 255, 255, 120);
   evas_object_move(o, -12000, -16000);
   evas_object_resize(o, 24000, 32000);
   evas_object_show(o);   
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, calibrate_cb_down, ee);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, calibrate_cb_up, ee);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, calibrate_cb_move, ee);
   o_events = o;
   
   o = evas_object_image_add(evas);
   evas_object_layer_set(o, 1000001);
   evas_object_image_file_set(o, IM"crosshair.png", NULL);   
   evas_object_resize(o, 31, 31);
   evas_object_image_fill_set(o, 0, 0, 31, 31);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
   o_crosshair = o;
   
   o = evas_object_text_add(evas);
   evas_object_layer_set(o, 1000002);
   evas_object_color_set(o, 0, 0, 0, 255);
   evas_object_text_font_set(o, "Vera", 10);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
   o_text = o;

   calibrate_pos_set(0);
}


static void
calibrate_cb_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   
   ev = event_info;
   
   evas_object_move(o_crosshair, ev->output.x - 15, ev->output.y - 15);
   tmp_input_count = 0;   
   tmp_input[((tmp_input_count & 0x7) * 2) + 0] = ev->output.x;
   tmp_input[((tmp_input_count & 0x7) * 2) + 1] = ev->output.y;
   tmp_input_count++;
   down = 1;
}

static void
calibrate_cb_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   int n, i, avx, avy, c, mx, my;
   int dists[8];
   int indexes[8];
   int sorted;
   
   ev = event_info;
   down = 0;
   tmp_input[((tmp_input_count & 0x7) * 2) + 0] = ev->output.x;
   tmp_input[((tmp_input_count & 0x7) * 2) + 1] = ev->output.y;
   tmp_input_count++;
   n = 8;
   if (tmp_input_count < 8) n = tmp_input_count;
   avx = 0; avy = 0; c = 0;   
   for (i = 0; i < n; i++)
     {
	dists[i] = tmp_input[(i * 2) + 0];
	indexes[i] = i;
     }
   sorted = 0;
   while (!sorted)
     {
	sorted = 1;
	for (i = 0; i < n - 1; i++)
	  {
	     if (dists[i + 1] < dists[i])
	       { 
		  int tmp;
		  
		  sorted = 0;
		  tmp = dists[i];
		  dists[i] = dists[i + 1];
		  dists[i + 1] = tmp;
		  tmp = indexes[i];
		  indexes[i] = indexes[i + 1];
		  indexes[i + 1] = tmp;
	       }
	  }
     }
   mx = dists[(n + 1) / 2];
   for (i = 0; i < n; i++)
     {
	dists[i] = tmp_input[(i * 2) + 1];
	indexes[i] = i;
     }
   sorted = 0;
   while (!sorted)
     {
	sorted = 1;
	for (i = 0; i < n - 1; i++)
	  {
	     if (dists[i + 1] < dists[i])
	       { 
		  int tmp;
		  
		  sorted = 0;
		  tmp = dists[i];
		  dists[i] = dists[i + 1];
		  dists[i + 1] = tmp;
		  tmp = indexes[i];
		  indexes[i] = indexes[i + 1];
		  indexes[i + 1] = tmp;
	       }
	  }
     }
   my = dists[(n + 1) / 2];

   for (i = 0; i < n; i++)
     {
	int x, y, dx, dy;
	
	x = tmp_input[(i * 2) + 0];
	y = tmp_input[(i * 2) + 1];
	dx = x - mx;
	dy = y - my;
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	if ((dx <= 8) && (dy <= 8))
	  {
	     avx += tmp_input[(i * 2) + 0];
	     avy += tmp_input[(i * 2) + 1];
	     c++;
	  }
     }
   cal_input[(cal_pos * 2) + 0] = avx / c;
   cal_input[(cal_pos * 2) + 1] = avy / c;
   n = calibrate_pos_get();
   if (n < 3)
     {
	calibrate_pos_set(n + 1);
	return;
     }
   calibrate_finish();
}

static void
calibrate_cb_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   
   ev = event_info;
   if (!down) return;
   tmp_input[((tmp_input_count & 0x7) * 2) + 0] = ev->cur.output.x;
   tmp_input[((tmp_input_count & 0x7) * 2) + 1] = ev->cur.output.y;
   tmp_input_count++;
}
#endif
