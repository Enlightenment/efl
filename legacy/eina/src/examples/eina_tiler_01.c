//Compile with:
//gcc `pkg-config --cflags --libs ecore-evas ecore evas` eina_tiler_01.c -o eina_tiler_01 

#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eina.h>

#define WINDOW_PAD (20)

static Eina_Tiler *tiler;
static Eina_Rectangle *input_rects;
static unsigned int input_count;
static unsigned int input_idx = 0, input_color_idx = 0, output_color_idx = 0;
static Eina_List *output_objs = NULL;
static Evas_Coord maxw, maxh, winw, winh;
static Evas *evas;

static const struct color {
   unsigned char r, g, b;
} colors[] = {
  {255, 0, 0},
  {0, 255, 0},
  {0, 0, 255},

  {255, 128, 0},
  {0, 255, 128},
  {128, 0, 255},

  {255, 255, 0},
  {0, 255, 255},
  {255, 0, 255},

  {255, 0, 128},
  {128, 255, 0},
  {0, 128, 255},

  {128, 128, 0},
  {0, 128, 128},
  {128, 0, 128},

  {128, 0, 0},
  {0, 128, 0},
  {0, 0, 128},

  {255, 128, 0},
  {0, 255, 128},
  {128, 0, 255},

  {64, 64, 0},
  {0, 64, 64},
  {64, 0, 64},

  {128, 128, 0},
  {0, 128, 128},
  {128, 0, 128},

  {255, 0, 128},
  {128, 255, 0},
  {0, 128, 255},

  {128, 64, 0},
  {0, 128, 64},
  {64, 0, 128},

  {128, 0, 64},
  {64, 128, 0},
  {0, 64, 128}
};

#define MAX_COLORS (sizeof(colors) / sizeof(colors[0]))

static void
add_text(const char *text, int x, int y, int w)
{
   Evas_Object *o = evas_object_text_add(evas);
   evas_object_color_set(o, 0, 0, 0, 255);
   evas_object_move(o, x, y);
   evas_object_resize(o, w, WINDOW_PAD);
   evas_object_text_font_set(o, "Sans", 10);
   evas_object_text_text_set(o, text);
   evas_object_show(o);
}

static void
output_rects_reset(void)
{
   Evas_Object *o;
   EINA_LIST_FREE(output_objs, o)
     evas_object_del(o);
   output_color_idx = 0;
}

static void
add_input_rect(const Eina_Rectangle *r)
{
   Evas_Object *o;
   Evas_Coord bx, by;

   bx = WINDOW_PAD;
   by = WINDOW_PAD;

   o = evas_object_rectangle_add(evas);
#define C(comp) (((int)colors[input_color_idx].comp * 128) / 255)
   evas_object_color_set(o, C(r), C(g), C(b), 128);
#undef C
   evas_object_move(o, r->x + bx, r->y + by);
   evas_object_resize(o, r->w, r->h);
   evas_object_show(o);

   input_color_idx = (input_color_idx + 1) % MAX_COLORS;

   bx += maxw + WINDOW_PAD;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 32, 32, 32, 128);
   evas_object_move(o, r->x + bx, r->y + by);
   evas_object_resize(o, r->w, 1);
   evas_object_layer_set(o, EVAS_LAYER_MAX);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 32, 32, 32, 128);
   evas_object_move(o, r->x + bx, r->y + by);
   evas_object_resize(o, 1, r->h);
   evas_object_layer_set(o, EVAS_LAYER_MAX);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 32, 32, 32, 128);
   evas_object_move(o, r->x + bx, r->y + by + r->h);
   evas_object_resize(o, r->w, 1);
   evas_object_layer_set(o, EVAS_LAYER_MAX);
   evas_object_show(o);

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 32, 32, 32, 128);
   evas_object_move(o, r->x + bx + r->w, r->y + by);
   evas_object_resize(o, 1, r->h);
   evas_object_layer_set(o, EVAS_LAYER_MAX);
   evas_object_show(o);
}

static void
add_output_rect(const Eina_Rectangle *r)
{
   Evas_Object *o = evas_object_rectangle_add(evas);
#define C(comp) (((int)colors[output_color_idx].comp * 128) / 255)
   evas_object_color_set(o, C(r), C(g), C(b), 128);
#undef C
   evas_object_move(o, r->x + maxw + 2 * WINDOW_PAD, r->y + WINDOW_PAD);
   evas_object_resize(o, r->w, r->h);
   evas_object_show(o);

   output_color_idx = (output_color_idx + 1) % MAX_COLORS;

   output_objs = eina_list_append(output_objs, o);
}

static Eina_Bool
process_input(void *data)
{
   Eina_Iterator *itr;
   Eina_Rectangle r, *r1;
   unsigned int out = 0;

   if (input_idx == input_count)
     {
        add_text("Done. Close the window to exit",
                 WINDOW_PAD, winh - WINDOW_PAD, winw - 2 * WINDOW_PAD);
        return EINA_FALSE;
     }

   output_rects_reset();

   r = input_rects[input_idx];
   printf("Iteration #%u: %dx%d%+d%+d\n", input_idx, r.w, r.h, r.x, r.y);
   input_idx++;
   add_input_rect(&r);

   eina_tiler_rect_add(tiler, &r);
   itr = eina_tiler_iterator_new(tiler);
   EINA_ITERATOR_FOREACH(itr, r1)
     {
        printf("\tOutput #%u: %dx%d%+d%+d\n", out, r1->w, r1->h, r1->x, r1->y);
        add_output_rect(r1);
        out++;
     }
   eina_iterator_free(itr);

   return EINA_TRUE;
}

static void
usage(const char *progname)
{
   fprintf(stderr,
           "Usage:\n\n"
           "\t%s <rect1> ... <rectN>\n\n"
           "with rectangles being in the format:\n"
           "\tWIDTHxHEIGHT<+->X<+->Y\n"
           "examples:\n"
           "\t100x100+10+10 - width=100, height=100 at x=10, y=10\n"
           "\t150x50+5+6    - width=150, height=50 at x=5, y=6\n",
           progname);
}

int
main(int argc, char *argv[])
{
   Ecore_Evas *ee;
   Evas_Object *o;
   int i;

   if (argc < 2)
     {
        usage(argv[0]);
        return -2;
     }

   input_rects = calloc(argc - 1, sizeof(Eina_Rectangle));
   input_count = 0;
   maxw = 0;
   maxh = 0;
   for (i = 1; i < argc; i++)
     {
        Eina_Rectangle *r = input_rects + input_count;
        char sx, sy;

        if (sscanf(argv[i], "%dx%d%c%d%c%d",
                   &(r->w), &(r->h), &sx, &(r->x), &sy, &(r->y)) == 6)
          {
             if (sx == '-') r->x *= -1;
             if (sy == '-') r->y *= -1;

             if (maxw < r->x + r->w) maxw = r->x + r->w;
             if (maxh < r->y + r->h) maxh = r->y + r->h;
             input_count++;
          }
        else
          fprintf(stderr, "ERROR: invalid rectangle ignored: %s\n", argv[i]);
     }

   if (input_count == 0)
     {
        fputs("ERROR: Could not find any valid rectangle. Exit!\n", stderr);
        usage(argv[0]);
        free(input_rects);
        return -3;
     }

   if ((maxw == 0) || (maxh == 0))
     {
        fputs("ERROR: All rectangles with size 0x0. Exit!\n", stderr);
        usage(argv[0]);
        free(input_rects);
        return -3;
     }

   ecore_evas_init();
   ecore_init();
   evas_init();
   eina_init();

   winw = 2 * maxw + 3 * WINDOW_PAD;
   winh = maxh + 2 * WINDOW_PAD;

   ee = ecore_evas_new(NULL, 0, 0, winw, winh, NULL);
   if (!ee)
     {
        fputs("ERROR: Could not create window. Check ecore-evas install.\n",
              stderr);
        goto end;
     }

   evas = ecore_evas_get(ee);

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_resize(o, winw, winh);
   evas_object_show(o);

   add_text("Input", WINDOW_PAD, 0, maxw);
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 200, 200, 200, 255);
   evas_object_move(o, WINDOW_PAD, WINDOW_PAD);
   evas_object_resize(o, maxw, maxh);
   evas_object_show(o);

   add_text("Output", maxw + 2 * WINDOW_PAD, 0, maxw);
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 200, 200, 200, 255);
   evas_object_move(o, maxw + 2 * WINDOW_PAD, WINDOW_PAD);
   evas_object_resize(o, maxw, maxh);
   evas_object_show(o);

   tiler = eina_tiler_new(maxw, maxh);
   ecore_timer_add(2.0, process_input, NULL);

   ecore_evas_show(ee);
   ecore_main_loop_begin();

   eina_list_free(output_objs);
   eina_tiler_free(tiler);
   ecore_evas_free(ee);

 end:
   free(input_rects);

   eina_shutdown();
   evas_shutdown();
   ecore_shutdown();
   ecore_evas_shutdown();

   return 0;
}
