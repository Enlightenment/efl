#include <Elementary.h>

static Evas_Object *win = NULL;
static Evas_Object *rect = NULL;

struct info
{
   Evas_Object *obj;
   int *pix;
};

// BEGIN - code running in my custom pthread instance
//
static void
mandel(int *pix, int w, int h)
{
   double x, xx, y, cx, cy, cox, coy;
   int iteration, hx, hy, val, r, g, b, rr, gg, bb;
   int itermax = 10000;
   double magnify = 0.02;

   // this mandel calc is run in the worker threads so it's here. it is
   // just here to calculate something and consume cpu to demonstrate the
   // ecore thread worker queue. don't pay much attention to the below code
   magnify += ((double)(rand() % 100) / 100.0) / 4.0;
   cox = (double)(rand() % 100) / 100.0;
   coy = (double)(rand() % 100) / 100.0;
   cox /= (magnify * 3.0);
   r = rand() % 255; g = rand() % 255; b = rand() % 255;
   for (hy = 0; hy < h; hy++) 
     {
        for (hx = 0; hx < w; hx++) 
          {
             cx = (((float)hx) / ((float)w) - 0.5) / (magnify * 3.0);
             cy = (((float)hy) / ((float)h) - 0.5) / (magnify * 3.0);
             cx += cox;
             cy += coy;
             x = 0.0;
             y = 0.0;
             for (iteration = 1; iteration < itermax; iteration++)
               {
                  xx = (x * x) - (y * y) + cx;
                  y = (2.0 * x * y) + cy;
                  x = xx;
                  if (((x * x) + (y * y)) > 100.0) iteration = 999999;
               }
             val = (((x * x) + (y * y)) * 2.55) / 100.0;
             if (val > 255) val = 255;
             if (iteration >= 99999) 
               {
                  rr = (r * val) / 255;
                  gg = (g * val) / 255;
                  bb = (b * val) / 255;
                  pix[(hy * w) + hx] = 
                     (val  << 24) | (rr << 16) | (gg << 8) | (bb);
               }
             else
                pix[(hy * w) + hx] = 0xffffffff;
          }
     }
}

static void
th_do(void *data, Ecore_Thread *th)
{
   struct info *inf = data;
   // CANNOT TOUCH inf->obj here! just inf->pix which is 256x256 @ 32bpp
   // quick and dirty to consume some cpu - do a mandelbrot calc
   mandel(inf->pix, 256, 256);
}
//
// END - code running in my custom pthread instance

static void // thread job finished - collect results and put in img obj
th_end(void *data, Ecore_Thread *th)
{ 
   struct info *inf = data;

   // copy data to object, free calculated data and info struc
   evas_object_image_data_copy_set(inf->obj, inf->pix);
   evas_object_show(inf->obj);
   free(inf->pix);
   free(inf);
}

static void // if the thread is cancelled - free pix, keep obj tho
th_cancel(void *data, Ecore_Thread *th)
{ 
   struct info *inf = data;
   
   // just free pixel data and info struct
   free(inf->pix);
   free(inf);
}

static Eina_Bool // animate the objects so you see all the madels move
anim(void *data)
{
   Evas_Object *o = data;
   double t, z;
   int w, h, v;
   Evas_Coord x, y;

   // just calculate some position using the pointer value of the object as
   // a seed value to make different objects go into different places over time
   v = ((int)o) & 0xff;
   t = ecore_loop_time_get();
   w = 100 + ((v * 100) >> 8);
   h = 100 + ((v * 100) >> 8);
   z = (double)(v) / 100.0;
   x = (w * sin(t));
   y = (h * cos(t + z));
   // do the actual move
   evas_object_move(o, 200 + x - 128, 200 + y - 128);
   // keep looping - return true
   return EINA_TRUE;
}

int
elm_main(int argc, char **argv)
{
   Evas_Object *o, *bg;
   int i;
   
   win = elm_win_add(NULL, "efl-thread-1", ELM_WIN_BASIC);
   elm_win_title_set(win, "EFL Thread 1");
   evas_object_resize(win, 400, 400);
   evas_object_show(win);
   
   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);
   
   // queue up 64 mandel generation thread jobs
   for (i = 0; i < 64; i++)
     {
        struct info *inf;
        
        // create ecore thread to do some threaded job inside the worker pool
        inf = malloc(sizeof(struct info));
        if (inf)
          {
             Evas_Object *o;
             
             o = evas_object_image_filled_add(evas_object_evas_get(win));
             evas_object_image_size_set(o, 256, 256);
             evas_object_image_alpha_set(o, EINA_TRUE);
             evas_object_resize(o, 256, 256);
             inf->obj = o;
             inf->pix = malloc(256 * 256 * sizeof(int));
             ecore_thread_run(th_do, th_end, th_cancel, inf);
             // bonus - slide the objects around all the time with an
             // animator that ticks off every frame.
             ecore_animator_add(anim, o);
          }
     }
   
   elm_run();
   return 0;
}

ELM_MAIN()

