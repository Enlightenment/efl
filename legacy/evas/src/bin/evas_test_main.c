#include "config.h"
#include "evas_test_main.h"

#define EVAS_PI	(3.141592654)

#ifndef _WIN32_WCE

# define IM PACKAGE_DATA_DIR"/data/"
# define FN PACKAGE_DATA_DIR"/data/"

#else

# define IM PACKAGE_DATA_DIR"\\data\\"
# define FN PACKAGE_DATA_DIR"\\data\\"
wchar_t buf[1024];

#endif
 
 
Evas               *evas = NULL;

int                 win_w = 240;
int                 win_h = 320;

int                 mode = 0;
int                 loop_count = 0;
int                 orig_loop_count = 0;

double              start_time = 0.0;
double              orig_start_time = 0.0;

Evas_Object        *backdrop, *panel, *panel_top, *panel_shadow, *panel_clip;
Evas_Object        *evas_logo, *e_logo;

Evas_Object        *title = NULL, *comment;

Evas_Object        *t1 = NULL, *t2 = NULL;
Evas_Object        *test_pattern = NULL;
Evas_Object        *c1 = NULL, *c2 = NULL;
Evas_Object        *cv1 = NULL, *cv2 = NULL;

Evas_Object        *scroll[16];

#ifndef _WIN32_WCE
double
get_time(void)
{

   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);

}

#else
double
get_time(void)
{

	return (double)GetTickCount()/1000.0;
}
#endif

void
loop(void)
{
   double              t;

   loop_count++;
   orig_loop_count++;
   t = get_time() - start_time;
   
   if (mode == 1)
     {
	int iw, ih;
	int i;
	
	evas_object_image_fill_set(scroll[0], 0, loop_count, 240, 320);
	for (i = 1; i < 16; i++)
	  {
	     evas_object_image_size_get(scroll[i], &iw, &ih);
	     evas_object_image_fill_set(scroll[i], 0, loop_count * (i + 1), iw, ih);
	  }
	if ((loop_count % 500) == 499)
	  {
	     printf("FPS: %3.3f\n", (double)loop_count/t);
	  }
	return;
     }
   
   if (t <= 2.0)
     {
	double              y;

	y = sin((t / 2) * (EVAS_PI / 2)) * 240;
	evas_object_move(panel, 0, win_h - y);
	evas_object_move(panel_clip, 0, win_h - y);
	evas_object_move(panel_top, 0, win_h - y - 10);
	evas_object_move(panel_shadow, 0, win_h - y);
	evas_object_resize(backdrop, win_w, win_h - y);
     }
   else if (t <= (6.0))
     {
	evas_object_move(panel, 0, win_h - 240);
	evas_object_move(panel_clip, 0, win_h - 240);
	evas_object_move(panel_top, 0, win_h - 240 - 10);
	evas_object_move(panel_shadow, 0, win_h - 240);
	evas_object_color_set(evas_logo, 255, 255, 255,
			      (int)(255 * (((6.0) - t) / 4.0)));
     }
   else if (t <= 16.0)
     {
	double              tw, th;

	if (evas_logo)
	  {
	     Evas_Object        *ob;

	     evas_object_del(evas_logo);
	     evas_logo = NULL;

	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t1.png", NULL);
	     evas_object_move(ob, (win_w - 48) / 2,
			      (win_h - 240) + ((240 - 48) / 2));
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 1);
	     evas_object_show(ob);
	     t1 = ob;
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t2.png", NULL);
	     evas_object_move(ob, (win_w - 48) / 2,
			      (win_h - 240) + ((240 - 48) / 2));
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 0);
	     evas_object_show(ob);
	     t2 = ob;
	     ob = evas_object_text_add(evas);
	     evas_object_text_font_set(ob, "arial", 8);
	     evas_object_geometry_get(ob, NULL, NULL, &tw, &th);
	     evas_object_move(ob, (win_w - tw) / 2, win_h - th);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_layer_set(ob, 9);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     title = ob;
	     ob = evas_object_text_add(evas);
	     evas_object_text_font_set(ob, "arial", 8);
	     evas_object_geometry_get(ob, NULL, NULL, &tw, &th);
	     evas_object_move(ob, 0, win_h - 240);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_layer_set(ob, 9);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     comment = ob;
	  }

	/* Japanese: "日本語]" */
	/* Korean: "한국" */
	/* Chinese: "汉语" */
	evas_object_text_text_set(comment, "Image objects can be moved");
	evas_object_text_text_set(title, "Test 1: Move 2 Images ");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 48) / 2);
	   y = sin(v * 1.9) * ((240 - 48) / 2);
	   evas_object_move(t1, 120 + x - 24, win_h - 120 + y - 24);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 48) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 48) / 2);
	   evas_object_move(t2, 120 + x - 24, win_h - 120 + y - 24);
	}
     }
   else if (t <= 26.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "Image objects can be resized");
	evas_object_text_text_set(title, "Test 2: Resize 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 16.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	}
     }

   else if (t <= 36.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "Image objects are filled and scaled differently");
	evas_object_text_text_set(title, "Test 3: Resize & Fill 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 26.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   evas_object_image_fill_set(t1, 0, 0, x, y);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	   evas_object_image_fill_set(t2, 0, 0, x, y);
	}
     }
   else if (t <= 46.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "Image objects are offset filled");
	evas_object_text_text_set(title, "Test 4: Fill Offset 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);

	evas_object_resize(t1, 100, 100);
	evas_object_image_fill_set(t1, -25, -25, 100, 100);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 36.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   evas_object_image_fill_set(t1, -x / 4, -y / 4, x / 2, y / 2);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	   evas_object_image_fill_set(t2, -x / 4, -y / 4, x / 2, y / 2);
	}
     }

   else if (t <= 56.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "Edges of images can be scaled differently");
	evas_object_text_text_set(title, "Test 5: Border Fill 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_image_border_set(t1, 8, 8, 8, 8);
	evas_object_image_border_set(t2, 8, 8, 8, 8);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 46.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   evas_object_image_fill_set(t1, 0, 0, x, y);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	   evas_object_image_fill_set(t2, 0, 0, x, y);
	}
     }
   else if (t <= 76.0)
     {
	double              tw, th;
	int                 iw, ih;

	evas_object_text_text_set(title, "Test 7: Image Quality");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!test_pattern)
	  {
	     Evas_Object        *ob;

	     evas_object_hide(t1);
	     evas_object_hide(t2);
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "test_pattern.png", NULL);
	     evas_object_image_size_get(ob, &iw, &ih);
	     evas_object_move(ob, (win_w - iw) / 2,
			      (win_h - 240) + ((240 - ih) / 2));
	     evas_object_resize(ob, iw, ih);
	     evas_object_image_fill_set(ob, 0, 0, iw, ih);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     test_pattern = ob;
	  }
	{
	   double              v;
	   double              x, y;

	   evas_object_image_size_get(test_pattern, &iw, &ih);
	   v = 2.0 - ((t - 56.0) / 10.0);
	   if ((v >= 0.4) && (v <= 1.4))
	     {
		evas_object_text_text_set(comment,
					  "Look and see if it looks right");
		v = 1.4;
	     }
	   else if (v <= 0.4)
	     {
		evas_object_text_text_set(comment, "Scaling test pattern");
		v += 1.0;
	     }
	   else
	     {
		evas_object_text_text_set(comment, "Scaling test pattern");
	     }
	   v -= 1.0;
	   x = iw * v;
	   y = ih * v;
	   evas_object_move(test_pattern, (win_w - x) / 2,
			    (win_h - 240) + ((240 - y) / 2));
	   evas_object_resize(test_pattern, x, y);
	   evas_object_image_fill_set(test_pattern, 0, 0, x, y);
	}
     }
   else if (t <= 86.0)
     {
	double              tw, th;
	int                 iw, ih;

	evas_object_text_text_set(title, "Test 8: Raise & Lower");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     evas_object_image_border_set(t1, 0, 0, 0, 0);
	     evas_object_image_border_set(t2, 0, 0, 0, 0);
	     evas_object_resize(t1, 48, 48);
	     evas_object_resize(t2, 48, 48);
	     evas_object_move(t1, 40, win_h - 240 + 40);
	     evas_object_move(t2, 60, win_h - 240 + 60);
	     evas_object_image_fill_set(t1, 0, 0, 48, 48);
	     evas_object_image_fill_set(t2, 0, 0, 48, 48);
	     evas_object_show(t1);
	     evas_object_show(t2);
	  }
	if (((int)(t)) & 0x1)
	  {
	     evas_object_text_text_set(comment, "2 On Top");
	     evas_object_raise(t2);
	  }
	else
	  {
	     evas_object_text_text_set(comment, "1 On Top");
	     evas_object_raise(t1);
	  }
     }
   else if (t <= 96.0)
     {
	double              tw, th;
	int                 iw, ih;

	evas_object_text_text_set(title, "Test 9: Layer Setting");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     evas_object_image_border_set(t1, 0, 0, 0, 0);
	     evas_object_image_border_set(t2, 0, 0, 0, 0);
	     evas_object_resize(t1, 48, 48);
	     evas_object_resize(t2, 48, 48);
	     evas_object_move(t1, 40, win_h - 240 + 40);
	     evas_object_move(t2, 60, win_h - 240 + 60);
	     evas_object_image_fill_set(t1, 0, 0, 48, 48);
	     evas_object_image_fill_set(t2, 0, 0, 48, 48);
	     evas_object_show(t1);
	     evas_object_show(t2);
	  }
	if (((int)(t)) & 0x1)
	  {
	     evas_object_text_text_set(comment, "2 On Top");
	     evas_object_layer_set(t1, 5);
	     evas_object_layer_set(t2, 6);
	  }
	else
	  {
	     evas_object_text_text_set(comment, "1 On Top");
	     evas_object_layer_set(t1, 6);
	     evas_object_layer_set(t2, 5);
	  }
     }
   else if (t <= 106.0)
     {
	double              tw, th;
	int                 iw, ih;

	evas_object_text_text_set(title, "Test 10: Show & Hide");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     evas_object_image_border_set(t1, 0, 0, 0, 0);
	     evas_object_image_border_set(t2, 0, 0, 0, 0);
	     evas_object_resize(t1, 48, 48);
	     evas_object_resize(t2, 48, 48);
	     evas_object_move(t1, 40, win_h - 240 + 40);
	     evas_object_move(t2, 60, win_h - 240 + 60);
	     evas_object_image_fill_set(t1, 0, 0, 48, 48);
	     evas_object_image_fill_set(t2, 0, 0, 48, 48);
	     evas_object_show(t1);
	     evas_object_show(t2);
	  }
	if (((int)(t)) & 0x1)
	  {
	     evas_object_text_text_set(comment, "1 Show");
	     evas_object_show(t1);
	  }
	else
	  {
	     evas_object_text_text_set(comment, "1 Hide");
	     evas_object_hide(t1);
	  }
     }
   else if (t <= 136.0)
     {
	double              tw, th;
	int                 iw, ih;

	evas_object_text_text_set(title, "Test 11: Image Data Access");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!test_pattern)

	  {
	     Evas_Object        *ob;

	     evas_object_hide(t1);
	     evas_object_hide(t2);
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "test_pattern.png", NULL);
	     evas_object_image_size_get(ob, &iw, &ih);
	     ih = ((win_w - 8) * ih) / iw;
	     iw = (win_w - 8);
	     evas_object_resize(ob, iw, ih);
	     evas_object_image_fill_set(ob, 0, 0, iw, ih);
	     evas_object_move(ob, 4, (win_h - 240) + ((240 - ih) / 2));
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     test_pattern = ob;
	  }
	if (t <= 116.0)
	  {
	     int                 iw, ih;
	     int                 x, y;
	     int                *data;

	     evas_object_text_text_set(comment, "Play with all the image data");
	     evas_object_image_size_get(test_pattern, &iw, &ih);
	     evas_object_image_alpha_set(test_pattern, 0);
	     data = evas_object_image_data_get(test_pattern, 1);
	     if (data)
	       {
		  for (y = 0; y < ih; y++)
		    {
		       for (x = 0; x < iw; x++)
			 {
			    data[(y * iw) + x] =
			       (((x * y / 10) + (int)(t * 1000))) | 0xff000000;
			 }
		    }
		  evas_object_image_data_update_add(test_pattern, 0, 0, iw, ih);
		  evas_object_image_data_set(test_pattern, data);
	       }
	  }
	else if (t <= 126.0)
	  {
	     int                 iw, ih;
	     int                 x, y;
	     int                *data;

	     evas_object_text_text_set(comment,
				       "Play with part of the image data");
	     evas_object_image_size_get(test_pattern, &iw, &ih);
	     evas_object_image_alpha_set(test_pattern, 0);
	     data = evas_object_image_data_get(test_pattern, 1);
	     if (data)
	       {
		  for (y = ih / 4; y < ((3 * ih) / 4); y++)
		    {
		       for (x = iw / 4; x < ((3 * iw) / 4); x++)
			 {
			    data[(y * iw) + x] =
			       (((x * y / 10) + (int)(t * 1000))) | 0xff000000;
			 }
		    }
		  evas_object_image_data_update_add(test_pattern, iw / 4,
						    ih / 4, iw / 2, ih / 2);
		  evas_object_image_data_set(test_pattern, data);
	       }
	  }
	else
	  {
	     int                 iw, ih;

	     evas_object_text_text_set(comment, "Play with image size");
	     evas_object_image_size_get(test_pattern, &iw, &ih);
	     if ((iw > 1) && (ih > 1))
	       {
		  evas_object_image_size_set(test_pattern, iw - 1, ih - 1);
	       }
	  }
     }
   else if (t <= 146.0)
     {
	double              tw, th;

	evas_object_text_text_set(title, "Test 12: Image Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;

	  }
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy image 2, create 1");
		  ob = evas_object_image_add(evas);
		  evas_object_image_file_set(ob, IM "t1.png", NULL);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_image_fill_set(ob, 0, 0, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_image_smooth_scale_set(ob, 1);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy image 1, create 2");
		  ob = evas_object_image_add(evas);
		  evas_object_image_file_set(ob, IM "t2.png", NULL);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_image_fill_set(ob, 0, 0, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_image_smooth_scale_set(ob, 0);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 156.0)
     {
	double              tw, th;

	evas_object_text_text_set(title, "Test 13: Line Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy line blue, create red");
		  ob = evas_object_line_add(evas);
		  evas_object_line_xy_set(ob, 10, win_h - 240 + 10, win_w - 10,
					  win_h - 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 255, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy line red, create blue");
		  ob = evas_object_line_add(evas);
		  evas_object_line_xy_set(ob, win_w - 10, win_h - 240 + 10, 10,
					  win_h - 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 255, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 166.0)
     {
	double              tw, th;

	evas_object_text_text_set(title, "Test 14: Rectangle Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy rectangle blue, create red");
		  ob = evas_object_rectangle_add(evas);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 255, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy rectangle red, create blue");
		  ob = evas_object_rectangle_add(evas);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 255, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 176.0)
     {
	double              tw, th;

	evas_object_text_text_set(title, "Test 15: Gradient Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy gradient blue, create red");
		  ob = evas_object_gradient_add(evas);
		  evas_object_gradient_color_add(ob, 255, 0, 0, 0, 1);
		  evas_object_gradient_color_add(ob, 255, 0, 0, 255, 1);
		  evas_object_gradient_color_add(ob, 255, 255, 255, 255, 1);
		  evas_object_gradient_angle_set(ob, 45.0);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy gradient red, create blue");
		  ob = evas_object_gradient_add(evas);
		  evas_object_gradient_color_add(ob, 0, 0, 255, 0, 1);
		  evas_object_gradient_color_add(ob, 0, 0, 255, 255, 1);
		  evas_object_gradient_color_add(ob, 255, 255, 255, 255, 1);
		  evas_object_gradient_angle_set(ob, 45.0);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 186.0)
     {
	double              tw, th;

	evas_object_text_text_set(title, "Test 16: Polygon Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy polygon blue, create red");
		  ob = evas_object_polygon_add(evas);
		  evas_object_polygon_point_add(ob, 10, win_h - 240 + 10);
		  evas_object_polygon_point_add(ob, win_w - 10,
						win_h - 240 + 10);
		  evas_object_polygon_point_add(ob, 10, win_h - 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 255, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy polygon red, create blue");
		  ob = evas_object_polygon_add(evas);
		  evas_object_polygon_point_add(ob, 10, win_h - 10);
		  evas_object_polygon_point_add(ob, win_w - 10, win_h - 10);
		  evas_object_polygon_point_add(ob, win_w - 10,
						win_h - 240 + 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 255, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 196.0)
     {
	double              tw, th;

	evas_object_text_text_set(title, "Test 17: Text Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy text blue, create red");
		  ob = evas_object_text_add(evas);
		  evas_object_text_font_set(ob, "arial", 12);
		  evas_object_text_text_set(ob, "Red test string");
		  evas_object_move(ob, (win_w / 2) - 50, win_h - 120);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 255, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy text red, create blue");
		  ob = evas_object_text_add(evas);
		  evas_object_text_font_set(ob, "arial", 12);
		  evas_object_text_text_set(ob, "Blue test string");
		  evas_object_move(ob, (win_w / 2) - 50, win_h - 120);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 255, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 197.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 207.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "Image objects can be clipped by rectangles");
	evas_object_text_text_set(title, "Test 18: Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t1.png", NULL);
	     evas_object_move(ob, 5, win_h - 240 + 5);
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 1);
	     evas_object_show(ob);
	     t1 = ob;
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t2.png", NULL);
	     evas_object_move(ob, 40, win_h - 240 + 40);
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 0);
	     evas_object_show(ob);
	     t2 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, 10, win_h - 240 + 10);
	     evas_object_resize(ob, 180, 180);
	     evas_object_color_set(ob, 255, 255, 128, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     c1 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, 10, win_h - 240 + 10);
	     evas_object_resize(ob, 180, 180);
	     evas_object_layer_set(ob, 5);
	     evas_object_color_set(ob, 255, 255, 128, 100);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     cv1 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(ob, 180, 180);
	     evas_object_color_set(ob, 128, 255, 255, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     c2 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(ob, 180, 180);
	     evas_object_layer_set(ob, 5);
	     evas_object_color_set(ob, 128, 255, 255, 100);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     cv2 = ob;
	     evas_object_clip_set(t1, c1);
	     evas_object_clip_set(t2, c2);
	  }
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 48) / 2);
	   y = sin(v * 1.9) * ((240 - 48) / 2);
	   evas_object_move(t1, 120 + x - 24, win_h - 120 + y - 24);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 48) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 48) / 2);
	   evas_object_move(t2, 120 + x - 24, win_h - 120 + y - 24);
	}
     }
   else if (t <= 217.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "Image objects can be clipped by recursively");
	evas_object_text_text_set(title, "Test 19: Recusrively Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_clip_set(c2, c1);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 48) / 2);
	   y = sin(v * 1.9) * ((240 - 48) / 2);
	   evas_object_move(t1, 120 + x - 24, win_h - 120 + y - 24);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 48) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 48) / 2);
	   evas_object_move(t2, 120 + x - 24, win_h - 120 + y - 24);
	}
     }
   else if (t <= 227.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "Clip objects can move around too...");
	evas_object_text_text_set(title, "Test 19: Recusrively Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 180) / 2);
	   y = sin(v * 1.9) * ((240 - 180) / 2);
	   evas_object_move(c1, 120 + x - 90, win_h - 120 + y - 80);
	   evas_object_move(cv1, 120 + x - 90, win_h - 120 + y - 80);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 180) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 180) / 2);
	   evas_object_move(c2, 120 + x - 90, win_h - 120 + y - 90);
	   evas_object_move(cv2, 120 + x - 90, win_h - 120 + y - 90);
	}
     }
   else if (t <= 237.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "Clip objects can resize around too...");
	evas_object_text_text_set(title, "Test 20: Recusrively Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   evas_object_move(t1, 40, win_h - 240 + 60);
	   evas_object_move(t2, 60, win_h - 240 + 100);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((180) / 2);
	   y = sin(v * 1.9) * ((180) / 2);
	   evas_object_resize(c1, 90 + x, 90 + y);
	   evas_object_resize(cv1, 90 + x, 90 + y);
	   x = cos(1.0 + (v * 2.3)) * ((180) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((180) / 2);
	   evas_object_resize(c2, 90 + x, 90 + y);
	   evas_object_resize(cv2, 90 + x, 90 + y);
	}
     }
   else if (t <= 247.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "Clip objects can hide and show...");
	evas_object_text_text_set(title, "Test 21: Show and Hide Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_move(c1, 10, win_h - 240 + 10);
	evas_object_resize(c1, 180, 180);
	evas_object_move(cv1, 10, win_h - 240 + 10);
	evas_object_resize(cv1, 180, 180);
	evas_object_move(c2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(c2, 180, 180);
	evas_object_move(cv2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(cv2, 180, 180);
	if ((((int)(t)) & 0x3) == 0)
	  {
	     evas_object_text_text_set(comment, "Show yellow");
	     evas_object_show(c1);
	     evas_object_show(cv1);
	     evas_object_hide(c2);
	     evas_object_hide(cv2);
	  }
	else if ((((int)(t)) & 0x3) == 1)
	  {
	     evas_object_text_text_set(comment, "Show blue");
	     evas_object_hide(c1);
	     evas_object_hide(cv1);
	     evas_object_show(c2);
	     evas_object_show(cv2);
	  }
	else if ((((int)(t)) & 0x3) == 2)
	  {
	     evas_object_text_text_set(comment, "Show yellow, blue");
	     evas_object_show(c1);
	     evas_object_show(cv1);
	     evas_object_show(c2);
	     evas_object_show(cv2);
	  }
	else if ((((int)(t)) & 0x3) == 3)
	  {
	     evas_object_text_text_set(comment, "Hide both");
	     evas_object_hide(c1);
	     evas_object_hide(cv1);
	     evas_object_hide(c2);
	     evas_object_hide(cv2);
	  }
     }
   else if (t <= 257.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "Clip objects can clip and unclip...");
	evas_object_text_text_set(title, "Test 22: Clip and Unclip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_move(t1, 20, win_h - 240 + 20);
	evas_object_move(t2, 160, win_h - 240 + 40);
	evas_object_move(c1, 10, win_h - 240 + 10);
	evas_object_resize(c1, 180, 180);
	evas_object_move(cv1, 10, win_h - 240 + 10);
	evas_object_resize(cv1, 180, 180);
	evas_object_move(c2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(c2, 180, 180);
	evas_object_move(cv2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(cv2, 180, 180);
	if ((((int)(t)) & 0x1) == 0)
	  {
	     evas_object_text_text_set(comment, "Unlip 2 from blue");
	     evas_object_clip_unset(t2);
	  }
	else if ((((int)(t)) & 0x1) == 1)
	  {
	     evas_object_text_text_set(comment, "Clip 2 to blue");
	     evas_object_clip_set(t2, c2);
	  }
     }
   else if (t <= 267.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "Clip objects can be destroyed...");
	evas_object_text_text_set(title, "Test 23: Destroy Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_move(t1, 20, win_h - 240 + 20);
	evas_object_move(t2, 160, win_h - 240 + 40);
	if (c1)
	  {
	     evas_object_move(c1, 10, win_h - 240 + 10);
	     evas_object_resize(c1, 180, 180);
	  }
	if (cv1)
	  {
	     evas_object_move(cv1, 10, win_h - 240 + 10);
	     evas_object_resize(cv1, 180, 180);
	  }
	if (c2)
	  {
	     evas_object_move(c2, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(c2, 180, 180);
	  }
	if (cv2)
	  {
	     evas_object_move(cv2, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(cv2, 180, 180);
	  }
	if (c2)
	  {
	     evas_object_clip_set(t2, c2);
	  }
	if (t < 262.0)
	  {
	     evas_object_text_text_set(comment, "Destroy yellow");
	     if (c1)
		evas_object_del(c1);
	     if (cv1)
		evas_object_del(cv1);
	     c1 = NULL;
	     cv1 = NULL;
	  }
	else
	  {
	     evas_object_text_text_set(comment, "Destroy blue");
	     if (c2)
		evas_object_del(c2);
	     if (cv2)
		evas_object_del(cv2);
	     c2 = NULL;
	     cv2 = NULL;
	  }
     }
   else if (t <= 268.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 278.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can change the text of a text object");
	evas_object_text_text_set(title, "Test 24: Set Text Object Text");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_text_add(evas);
	     evas_object_move(ob, 10, win_h - 240 + 60);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_text_font_set(ob, "arial", 8);
	     evas_object_text_text_set(ob, "Hello World [\xe2\x88\x9e]");
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	if ((((int)(t * 4)) & 0x3) == 0)
	  {
	     evas_object_text_text_set(t1, "Hello World [ \xe2\x88\x9e ]");
	  }
	else if ((((int)(t * 4)) & 0x3) == 1)
	  {
	     evas_object_text_text_set(t1, "Heizölrückstoßabdämpfung");
	  }
	else if ((((int)(t * 4)) & 0x3) == 2)
	  {
	     evas_object_text_text_set(t1,
				       "В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!");
	  }
	else if ((((int)(t * 4)) & 0x3) == 3)
	  {
	     evas_object_text_text_set(t1,
				       "All Evas text objects use UTF-8 Unicode");
	  }
     }
   else if (t <= 288.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "We can change the font size");
	evas_object_text_text_set(title, "Test 25: Set Text Object Size");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_text_font_set(t1, "arial", (t - 278.0 + 0.5) * 8.0);
     }
   else if (t <= 298.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "We can move text");
	evas_object_text_text_set(title, "Test 26: Move Text Object");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_text_font_set(t1, "arial", 24);
	evas_object_text_text_set(t1, "Evas");
	{
	   double              x, y;
	   double              v;

	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	}
     }
   else if (t <= 308.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "We can change the font");
	evas_object_text_text_set(title, "Test 27: Set Text Object Font");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if ((((int)(t * 4)) & 0x3) == 0)
	  {
	     evas_object_text_font_set(t1, "arial", 48);
	  }
	else if ((((int)(t * 4)) & 0x3) == 1)
	  {
	     evas_object_text_font_set(t1, "notepad", 48);
	  }
	else if ((((int)(t * 4)) & 0x3) == 2)
	  {
	     evas_object_text_font_set(t1, "grunge", 48);
	  }
	else if ((((int)(t * 4)) & 0x3) == 3)
	  {
	     evas_object_text_font_set(t1, "morpheus", 48);
	  }
	evas_object_text_text_set(t1, "Evas");
	evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	evas_object_move(t1, 120 - (tw / 2), win_h - 120 - (th / 2));
     }
   else if (t <= 318.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "We can change the color");
	evas_object_text_text_set(title, "Test 28: Set Text Object Color");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_text_font_set(t1, "grunge", 48);
	evas_object_text_text_set(t1, "Evas");
	evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	evas_object_move(t1, 120 - (tw / 2), win_h - 120 - (th / 2));
	evas_object_color_set(t1,
			      ((int)(t * 255)) & 0xff,
			      ((int)(t * 200)) & 0xff,
			      ((int)(t * 133)) & 0xff, ((int)(t * 128)) & 0xff);
     }
   else if (t <= 319.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 329.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "We can move gradients around");
	evas_object_text_text_set(title, "Test 29: Move Gradients");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_gradient_add(evas);
	     evas_object_gradient_color_add(ob, 255, 255, 255, 255, 1);
	     evas_object_gradient_color_add(ob, 250, 240, 50, 180, 1);
	     evas_object_gradient_color_add(ob, 220, 60, 0, 120, 1);
	     evas_object_gradient_color_add(ob, 200, 0, 0, 80, 1);
	     evas_object_gradient_color_add(ob, 0, 0, 0, 0, 0);
	     evas_object_gradient_angle_set(ob, 45.0);
	     evas_object_move(ob, 60, win_h - 240 + 60);
	     evas_object_resize(ob, 120, 120);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	{
	   double              x, y;
	   double              v;

	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	}
     }
   else if (t <= 339.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "We can resize gradients");
	evas_object_text_text_set(title, "Test 30: Resize Gradients");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((220) / 2);
	   y = sin(v * 1.9) * ((220) / 2);
	   evas_object_move(t1, 10, win_h - 240 + 10);
	   evas_object_resize(t1, x + 110, y + 110);
	}
     }
   else if (t <= 349.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can change gradient angle fills & resize");
	evas_object_text_text_set(title, "Test 30: Gradient angles");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((220) / 2);
	   y = sin(v * 1.9) * ((220) / 2);
	   evas_object_resize(t1, x + 110, y + 110);
	   evas_object_gradient_angle_set(t1, t * 60);
	}
     }
   else if (t <= 359.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can change color gradient, angle fill & resize");
	evas_object_text_text_set(title, "Test 31: Gradient Range");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((220) / 2);
	   y = sin(v * 1.9) * ((220) / 2);
	   evas_object_resize(t1, x + 110, y + 110);
	   evas_object_gradient_angle_set(t1, t * 60);
	   evas_object_gradient_colors_clear(t1);
	   evas_object_gradient_color_add(t1, 255, 255, 255, 255, 1);
	   evas_object_gradient_color_add(t1,
					  ((int)(t * 250)) & 0xff,
					  ((int)(t * 200)) & 0xff,
					  ((int)(t * 150)) & 0xff,
					  ((int)(t * 350)) & 0xff, 1);
	   evas_object_gradient_color_add(t1,
					  ((int)(t * 150)) & 0xff,
					  ((int)(t * 250)) & 0xff,
					  ((int)(t * 350)) & 0xff,
					  ((int)(t * 100)) & 0xff, 1);
	   evas_object_gradient_color_add(t1,
					  ((int)(t * 120)) & 0xff,
					  ((int)(t * 260)) & 0xff,
					  ((int)(t * 490)) & 0xff,
					  ((int)(t * 230)) & 0xff, 1);
	   evas_object_gradient_color_add(t1, 0, 0, 0, 0, 0);
	}
     }
   else if (t <= 369.0)		/* FAIL! software_x11, fb */
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can change color of a gradient object");
	evas_object_text_text_set(title, "Test 32: Gradient Object Color");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((220) / 2);
	   y = sin(v * 1.9) * ((220) / 2);
	   evas_object_resize(t1, x + 110, y + 110);
	   evas_object_gradient_angle_set(t1, t * 60);
	   evas_object_gradient_colors_clear(t1);
	   evas_object_gradient_color_add(t1, 255, 255, 255, 255, 1);
	   evas_object_gradient_color_add(t1, 0, 0, 0, 255, 0);
	   evas_object_color_set(t1,
				 ((int)(t * 120)) & 0xff,
				 ((int)(t * 260)) & 0xff,
				 ((int)(t * 490)) & 0xff,
				 ((int)(t * 230)) & 0xff);
	}
     }
   else if (t <= 370.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 380.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can move, resize and recolor rectangles");
	evas_object_text_text_set(title, "Test 33: Rectangles");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, 60, win_h - 240 + 60);
	     evas_object_resize(ob, 120, 120);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((220) / 2);
	   y = sin(v * 1.1) * ((220) / 2);
	   evas_object_resize(t1, 110 + x, 110 + y);
	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	   evas_object_color_set(t1,
				 ((int)(t * 120)) & 0xff,
				 ((int)(t * 260)) & 0xff,
				 ((int)(t * 490)) & 0xff,
				 ((int)(t * 230)) & 0xff);
	}
     }
   else if (t <= 381.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 391.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can modify line begin & end coords");
	evas_object_text_text_set(title, "Test 34: Lines");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_line_add(evas);
	     evas_object_line_xy_set(ob,
				     10, win_h - 240 + 10,
				     win_w - 10, win_h - 10);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	{
	   double              x, y, xx, yy;
	   double              v;

	   v = (((t - 2.0) / 12.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((240 - 20) / 2);
	   y = sin(v * 1.8) * ((240 - 20) / 2);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   xx = cos(v * 1.7) * ((240 - 20) / 2);
	   yy = sin(v * 1.9) * ((240 - 20) / 2);
	   evas_object_line_xy_set(t1,
				   (win_w / 2) + x, (win_h - 120) + y,
				   (win_w / 2) + xx, (win_h - 120) + yy);
	   evas_object_color_set(t1,
				 ((int)(t * 120)) & 0xff,
				 ((int)(t * 260)) & 0xff,
				 ((int)(t * 490)) & 0xff,
				 ((int)(t * 230)) & 0xff);
	}
     }
   else if (t <= 401.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can move, resize and recolor lines");
	evas_object_text_text_set(title, "Test 35: Line Move & Resize");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((220) / 2);
	   y = sin(v * 1.1) * ((220) / 2);
	   evas_object_resize(t1, 112 + x, 112 + y);
	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	   evas_object_color_set(t1,
				 ((int)(t * 120)) & 0xff,
				 ((int)(t * 260)) & 0xff,
				 ((int)(t * 490)) & 0xff,
				 ((int)(t * 230)) & 0xff);
	}
     }
   else if (t <= 402.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 412.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment, "We can modify polygon points");
	evas_object_text_text_set(title, "Test 36: Polygons");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_polygon_add(evas);
	     evas_object_polygon_point_add(ob, 10, 10);
	     evas_object_polygon_point_add(ob, 80, 20);
	     evas_object_polygon_point_add(ob, 30, 100);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	{
	   double              x, y;
	   double              v;

	   evas_object_polygon_points_clear(t1);
	   v = (((t - 2.0) / 12.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((240 - 20) / 2);
	   y = sin(v * 1.8) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 20) / 2);
	   y = sin(v * 1.9) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   v = (((t - 3.0) / 11.0) * EVAS_PI * 2) * 3;
	   x = cos(v * 1.2) * ((240 - 20) / 2);
	   y = sin(v * 1.1) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   v = (((t - 8.0) / 13.0) * EVAS_PI * 2) * 1;
	   x = cos(v * 1.7) * ((240 - 20) / 2);
	   y = sin(v * 1.4) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   evas_object_color_set(t1, ((int)(t * 120)) & 0xff,
				 ((int)(t * 260)) & 0xff,
				 ((int)(t * 490)) & 0xff,
				 ((int)(t * 230)) & 0xff);
	}
     }
   else if (t <= 422.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We can move, resize and recolor polygons");
	evas_object_text_text_set(title, "Test 37: Polygon Move & Resize");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   double              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((220) / 2);
	   y = sin(v * 1.1) * ((220) / 2);
	   evas_object_resize(t1, 110 + x, 110 + y);
	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	   evas_object_color_set(t1,
				 ((int)(t * 120)) & 0xff,
				 ((int)(t * 260)) & 0xff,
				 ((int)(t * 490)) & 0xff,
				 ((int)(t * 230)) & 0xff);
	}
     }
   else if (t <= 423.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 433.0)
     {
	double              tw, th;

	evas_object_text_text_set(comment,
				  "We are now done with testing. Goodbye!");
	evas_object_text_text_set(title, "The End.");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
     }
   else if (t <= 443.0)
     {
	printf("####################################################\n");
	printf("# FRAME COUNT: %i frames\n", loop_count);
	printf("# TIME:        %3.3f seconds\n", t);
	printf("# AVERAGE FPS: %3.3f fps\n", (double)loop_count / t);
	printf("####################################################\n");
	printf("################ evas free\n");
	evas_free(evas);
	printf("evas freed. DONE\n");
	exit(0);
	sleep(10000000);
     }
   /* need to test:
    * 
    * callbacks & events...
    *
    */
}

void
cb_mouse_down(void *data, Evas * e, Evas_Object * obj,
	      Evas_Event_Mouse_Down * ev)
{
   if (mode == 0)
     {
	mode = 1;
	setdown();
	scroll_setup();
	orig_start_time = start_time = get_time();
	loop_count = orig_loop_count = 0;	
     }
   else
     {
	mode = 0;
	scroll_setdown();
	setup();
	orig_start_time = start_time = get_time();
	loop_count = orig_loop_count = 0;	
     }
   printf("cb_mouse_down() [%i], %4i,%4i | %4.1f,%4.1f\n", ev->button,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_up(void *data, Evas * e, Evas_Object * obj, Evas_Event_Mouse_Up * ev)
{
   printf("cb_mouse_up()   [%i], %4i,%4i | %4.1f,%4.1f\n", ev->button,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_in(void *data, Evas * e, Evas_Object * obj, Evas_Event_Mouse_In * ev)
{
   printf("cb_mouse_in()   [%08x], %4i,%4i | %4.1f,%4.1f\n", ev->buttons,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_out(void *data, Evas * e, Evas_Object * obj, Evas_Event_Mouse_Out * ev)
{
   printf("cb_mouse_out()  [%08x], %4i,%4i | %4.1f,%4.1f\n", ev->buttons,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_move(void *data, Evas * e, Evas_Object * obj,
	      Evas_Event_Mouse_Move * ev)
{
   printf
      ("cb_mouse_move()  [%08x], %4i,%4i | %4.1f,%4.1f (%4i,%4i | %4.1f,%4.1f)\n",
       ev->buttons, ev->cur.output.x, ev->cur.output.y, ev->cur.canvas.x,
       ev->cur.canvas.y, ev->prev.output.x, ev->prev.output.y,
       ev->prev.canvas.x, ev->prev.canvas.y);
}

void
scroll_setdown(void)
{
   int i;
   
   evas_object_del(scroll[0]);   
   for (i = 1; i < 16; i++)
     evas_object_del(scroll[i]);   
}

void
scroll_setup(void)
{
   Evas_Object        *ob;
   int                 iw, ih;
   int                 i;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "backdrop.png", NULL);
   evas_object_move(ob, 0, 0);
   evas_object_resize(ob, 240, 320);
   evas_object_image_fill_set(ob, 0, 0, 240, 320);
   evas_object_layer_set(ob, 0);
   evas_object_show(ob);
   scroll[0] = ob;

   for (i = 1; i < 16; i++)
     {
	ob = evas_object_image_add(evas);
	evas_object_image_file_set(ob, IM "e_logo.png", NULL);
	evas_object_move(ob, 0, 0);
	evas_object_resize(ob, 240, 320);
	evas_object_image_size_get(ob, &iw, &ih);
	evas_object_image_fill_set(ob, 0, 0, iw, ih);
	evas_object_layer_set(ob, 1);
	evas_object_show(ob);
	evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, cb_mouse_down, NULL);
	scroll[i] = ob;
     }
}

void
setdown(void)
{
   evas_object_del(backdrop);
   evas_object_del(e_logo);
   evas_object_del(panel);
   evas_object_del(panel_top);
   evas_object_del(panel_shadow);
   evas_object_del(panel_clip);
   evas_object_del(evas_logo);
}

void
setup(void)
{
   Evas_Object        *ob;
   int                 iw, ih;

   evas_object_font_path_prepend(evas, FN);
   //  evas_object_image_cache_set(evas, 1024 * 1024);
   //  evas_object_font_cache_set(evas, 256 * 1024);
   evas_object_image_cache_set(evas, 0);
   evas_object_font_cache_set(evas, 0);

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "backdrop.png", NULL);
   evas_object_move(ob, 0, 0);
   evas_object_resize(ob, 240, 320);
   evas_object_image_fill_set(ob, 0, 0, 240, 320);
   evas_object_layer_set(ob, 0);
   evas_object_show(ob);
   backdrop = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "e_logo.png", NULL);
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, (win_w - iw) / 2, 0);
   evas_object_resize(ob, iw, ih);
   evas_object_image_fill_set(ob, 0, 0, iw, ih);
   evas_object_layer_set(ob, 1);
   evas_object_show(ob);
   evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, cb_mouse_down, NULL);
   e_logo = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "panel.png", NULL);
   evas_object_move(ob, 0, win_h - 240);
   evas_object_resize(ob, win_w, 240);
   evas_object_image_fill_set(ob, 0, 0, win_w, 240);
   evas_object_layer_set(ob, 3);
   evas_object_show(ob);
   panel = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "panel_top.png", NULL);
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, 0, win_h - 240 - ih);
   evas_object_resize(ob, win_w, ih);
   evas_object_image_fill_set(ob, 0, 0, win_w, ih);
   evas_object_layer_set(ob, 3);
   evas_object_show(ob);
   panel_top = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "panel_shadow.png", NULL);
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, 0, win_h - 240);
   evas_object_resize(ob, win_w, ih);
   evas_object_image_fill_set(ob, 0, 0, win_w, ih);
   evas_object_layer_set(ob, 10);
   evas_object_show(ob);
   panel_shadow = ob;

   ob = evas_object_rectangle_add(evas);
   evas_object_move(ob, 0, win_h - 240);
   evas_object_resize(ob, win_w, 240);
   evas_object_color_set(ob, 255, 255, 255, 255);
   evas_object_show(ob);
   panel_clip = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "evas_logo.png", NULL);
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, (win_w - iw) / 2, (win_h - 240) + ((240 - ih) / 2));
   evas_object_resize(ob, iw, ih);
   evas_object_image_fill_set(ob, 0, 0, iw, ih);
   evas_object_layer_set(ob, 5);
   evas_object_clip_set(ob, panel_clip);
   evas_object_show(ob);
   evas_logo = ob;
}

#if 0
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, cb_mouse_down,
			       NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_UP, cb_mouse_up, NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_IN, cb_mouse_in, NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_OUT, cb_mouse_out, NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move,
			       NULL);

ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 40, 10);
evas_object_resize(ob, 60, 40);
evas_object_color_set(ob, 0, 0, 0, 155);
evas_object_show(ob);
r1 = ob;
ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 60, 20);
evas_object_resize(ob, 80, 50);
evas_object_color_set(ob, 100, 200, 30, 200);
evas_object_show(ob);
r2 = ob;
ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 90, 45);
evas_object_resize(ob, 100, 170);
evas_object_color_set(ob, 200, 120, 80, 120);
evas_object_show(ob);
r3 = ob;
ob = evas_object_gradient_add(evas);
evas_object_move(ob, 20, 120);
evas_object_resize(ob, 60, 80);
evas_object_color_set(ob, 255, 255, 255, 255);
evas_object_gradient_color_add(ob, 255, 255, 255, 255, 10);
evas_object_gradient_color_add(ob, 250, 240, 50, 180, 10);
evas_object_gradient_color_add(ob, 220, 60, 0, 120, 10);
evas_object_gradient_color_add(ob, 200, 0, 0, 80, 10);
evas_object_gradient_color_add(ob, 0, 0, 0, 0, 10);
evas_object_gradient_angle_set(ob, 45.0);
evas_object_show(ob);
g1 = ob;
ob = evas_object_gradient_add(evas);
evas_object_move(ob, 40, 110);
evas_object_resize(ob, 64, 48);
evas_object_color_set(ob, 255, 255, 255, 255);
evas_object_gradient_color_add(ob, 255, 255, 255, 255, 10);
evas_object_gradient_color_add(ob, 50, 240, 250, 255, 10);
evas_object_gradient_color_add(ob, 0, 60, 220, 255, 10);
evas_object_gradient_color_add(ob, 0, 0, 200, 255, 10);
evas_object_gradient_color_add(ob, 0, 0, 0, 255, 10);
evas_object_gradient_angle_set(ob, 45.0);
evas_object_show(ob);
g2 = ob;
ob = evas_object_line_add(evas);
evas_object_color_set(ob, 0, 0, 0, 255);
evas_object_line_xy_set(ob, 20, 40, 80, 130);
evas_object_show(ob);
l1 = ob;
ob = evas_object_polygon_add(evas);
evas_object_color_set(ob, 10, 80, 200, 100);
evas_object_polygon_point_add(ob, 10, 10);
evas_object_polygon_point_add(ob, 80, 20);
evas_object_polygon_point_add(ob, 30, 100);
evas_object_show(ob);
p1 = ob;
ob = evas_object_image_add(evas);
evas_object_image_file_set(ob, PACKAGE_SOURCE_DIR "/data/image.png", NULL);
evas_object_move(ob, 40, 30);
evas_object_resize(ob, 120, 120);
evas_object_image_fill_set(ob, 0, 0, 120, 120);
evas_object_show(ob);
i2 = ob;
ob = evas_object_image_add(evas);
evas_object_image_file_set(ob, PACKAGE_SOURCE_DIR "/data/test_32.png", NULL);
evas_object_move(ob, 2, 2);
evas_object_image_border_set(ob, 5, 5, 5, 5);
evas_object_image_smooth_scale_set(ob, 1);
evas_object_resize(ob, 128, 128);
evas_object_image_fill_set(ob, 0, 0, 64, 64);
evas_object_show(ob);
i3 = ob;
   /*
    * Danish:
    * "Quizdeltagerne spiste jordbær med fløde, mens cirkusklovnen"
    * german:
    * "Heizölrückstoßabdämpfung"
    * Spanish:
    * "El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y"
    * French:
    * "Le cœur déçu mais l'âme plutôt naïve, Louÿs rêva de crapaüter en"
    * Irish Gaelic:
    * "D'fhuascail Íosa, Úrmhac na hÓighe Beannaithe, pór Éava agus Ádhaimh"
    * Hungarian:
    * "Árvíztűrő tükörfúrógép"
    * Icelandic:
    * "Kæmi ný öxi hér ykist þjófum nú bæði víl og ádrepa"
    * Japanese (hiragana):
    * "いろはにほへとちりぬるを"
    * Japanese (katakana):
    * "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"
    * Hebrew:
    * "? דג סקרן שט בים מאוכזב ולפתע מצא לו חברה איך הקליטה"
    * Polish:
    * "Pchnąć w tę łódź jeża lub ośm skrzyń fig"
    * Russian:
    * "В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!"
    * IPA:
    * "ˈjunɪˌkoːd"
    * American Dictionary:
    * "Ūnĭcōde̽"
    * Anglo-saxon:
    * "ᛡᚢᚾᛁᚳᚩᛞ"
    * Arabic:
    * "يونِكود"
    * Armenian:
    * "Յունիկօդ"
    * Bengali:
    * "য়ূনিকোড"
    * Bopomofo:
    * "ㄊㄨㄥ˅ ㄧˋ ㄇㄚ˅"
    * Canadian Syllabics:
    * "ᔫᗂᑰᑦ"
    * Cherokee:
    * "ᏳᏂᎪᏛ"
    * Chinese:
    * "萬國碼"
    * Ethiopic:
    * "ዩኒኮድ"
    * Georgian:
    * "უნიკოდი"
    * Greek:
    * "Γιούνικοντ"
    * Gujarati:
    * "યૂનિકોડ"
    * Gurmukhi:
    * "ਯੂਨਿਕੋਡ"
    * Hindi:
    * "यूनिकोड"
    * Kannada:
    * "ಯೂನಿಕೋಡ್"
    * Khmer:
    * "យូនីគោដ"
    * Korean:
    * "유니코드"
    * Malayalam:
    * "യൂനികോഡ്"
    * Ogham:
    * "ᚔᚒᚅᚔᚉᚑᚇ"
    * Oriya:
    * "ୟୂନିକୋଡ"
    * Persian:
    * "یونی‌کُد"
    * Sinhala:
    * "යණනිකෞද්"
    * Syriac:
    * "ܝܘܢܝܩܘܕ"
    * Tamil:
    * "யூனிகோட்"
    * Telugu:
    * "యూనికోడ్"
    * Thai:
    * "ยูนืโคด"
    * Tibetan:
    * "ཨུ་ནི་ཀོཌྲ།"
    * Yiddish:
    * "יוניקאָד"
    * 
    */
ob = evas_object_text_add(evas);
evas_object_move(ob, 10, 10);
evas_object_color_set(ob, 0, 0, 0, 255);
evas_object_text_font_set(ob, "arial", 14);
evas_object_text_text_set(ob, "Hello World [\xe2\x88\x9e]");
/* the 2 following commented out sections dont display japanese */
/* and i'm not sure why they don't. something deep down in the font rendering */
/* code.. but i don't know what */
/*   evas_object_text_font_set(ob, "cyberbit", 14);*/
/*   evas_object_text_text_set(ob, "Hello World - いろはにほへとちりぬるを");*/
evas_object_show(ob);
t1 = ob;

ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 10, 10);
evas_object_resize(ob, 220, 300);
evas_object_color_set(ob, 255, 255, 255, 255);
evas_object_show(ob);
c1 = ob;

evas_object_clip_set(r1, c1);
evas_object_clip_set(r2, c1);
evas_object_clip_set(r3, c1);
evas_object_clip_set(g1, c1);
#endif
