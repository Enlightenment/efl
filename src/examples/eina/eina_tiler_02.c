//Compile with:
//gcc eina_tiler_02.c -o eina_tiler_02 `pkg-config --cflags --libs ecore-evas ecore evas eina`

#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eina.h>

#define WIN_W    (810)
#define WIN_H    (280)

#define TILER1_X (30)
#define TILER1_Y (30)
#define TILER2_X (530)
#define TILER2_Y (30)
#define TILER_W  (250)
#define TILER_H  (250)
#define RESULT_X (280)
#define RESULT_Y (30)

static Eina_Tiler *tiler1, *tiler2;
static Eina_List *t1_objs, *t2_objs, *tiler_objs;
static Evas *evas;
static Evas_Object *bg;

static unsigned int cur_method;
typedef enum {
     METHOD_UNION = 0,
     METHOD_SUBTRACT,
     METHOD_INTERSECTION,
}Current_Method;

static unsigned int rect_idx =0, rect_count = 4;
static const Eina_Rectangle
   rects[4][2][2] = {
   {{{40, 0, 80, 250}},
    {{0, 150, 250, 70}, {0, 20, 250, 70}}},

    {{{20, 20, 65, 150},  {135, 80, 65, 150}},
     {{20, 80, 65, 150},  {135, 20, 65, 150}}},

    {{{20, 20, 210, 210}},
     {{80, 80, 90, 90}}},

    {{{0, 0, 150, 90}, {180, 180, 70, 70}},
     {{70, 70, 120, 120}}}
};

static const char*
 Methods[3] = {
  "UNION 1+2",
  "SUBTRACT 1-2",
  "INTERSECTION 1&2"
};

static unsigned int tile_idx =0, tile_count = 5;
static const unsigned int
   tilesize[5] = {
        32,
        16,
        8,
        4,
        1
};

static const char commands[] = \
  "commands are:\n"
  "\tn - next arrangement \n"
  "\tu - get union\n"
  "\ts - get subtract\n"
  "\ti - get intersection\n"
  "\tt - chage tile size ([32], 16, 8, 4) \n"
  "\tEsc - Exit\n";


static Evas_Object *
add_text(const char *text, int x, int y, int w)
{
   Evas_Object *o = evas_object_text_add(evas);
   evas_object_color_set(o, 0, 0, 0, 255);
   evas_object_move(o, x, y);
   evas_object_text_font_set(o, "Sans", 20);
   evas_object_text_text_set(o, text);
   evas_object_show(o);

   return o;
}

void tiler_calc()
{
   Eina_Tiler *tiler;
   Eina_Iterator *itr;
   Evas_Object *o;
   Eina_Rectangle *rect;
   int i = 0;

   /* clear evas rectangles */
   if (tiler_objs)
     {
        EINA_LIST_FREE(tiler_objs, o)
          {
             evas_object_hide(o);
             evas_object_del(o);
          }
     }

   tiler = eina_tiler_new(TILER_W, TILER_H);
   eina_tiler_tile_size_set(tiler,  tilesize[tile_idx], tilesize[tile_idx]);

   /* calculate with tiler1 and tiler2 */
   switch(cur_method)
     {
      case METHOD_UNION:
         eina_tiler_union(tiler, tiler1);
         eina_tiler_union(tiler, tiler2);
         break;
      case METHOD_SUBTRACT:
         eina_tiler_union(tiler, tiler1);
         eina_tiler_subtract(tiler, tiler2);
         break;
      case METHOD_INTERSECTION:
         tiler = eina_tiler_intersection(tiler1, tiler2);
         break;
     }

   if (tiler)
     {
        /* draw rects of result tiler */
        itr = eina_tiler_iterator_new(tiler);
        EINA_ITERATOR_FOREACH(itr, rect)
          {
             fprintf(stdout, "Result Rect [%d] (%d, %d) (%d x %d)\n", i++, rect->x, rect->y, rect->w, rect->h);

             o = evas_object_rectangle_add(evas);
             evas_object_color_set(o, 0, 255, 0, 255);
             evas_object_resize(o, rect->w, rect->h);
             evas_object_move(o, RESULT_X + rect->x, RESULT_Y + rect->y);
             evas_object_show(o);
             tiler_objs = eina_list_append(tiler_objs, o);
          }
        eina_iterator_free(itr);

        o = add_text(Methods[cur_method], RESULT_X, 0, TILER_W);
        tiler_objs = eina_list_append(tiler_objs, o);
        eina_tiler_free(tiler);
     }
}

void rect_update()
{
   Evas_Object *o;
   Eina_Rectangle *rect;
   Eina_Iterator *itr;
   int i = 0;

   if (t1_objs)
     {
        EINA_LIST_FREE(t1_objs, o)
          {
             evas_object_hide(o);
             evas_object_del(o);
          }
     }
   if (t2_objs)
     {
        EINA_LIST_FREE(t2_objs, o)
          {
             evas_object_hide(o);
             evas_object_del(o);
          }
     }

   eina_tiler_clear(tiler1);
   eina_tiler_clear(tiler2);

   eina_tiler_tile_size_set(tiler1, tilesize[tile_idx], tilesize[tile_idx]);
   eina_tiler_tile_size_set(tiler2, tilesize[tile_idx], tilesize[tile_idx]);

   for (i = 0; i < 2; i ++)
     {
        if (rects[rect_idx][0][i].w > 0 && rects[rect_idx][0][i].w > 0)
          eina_tiler_rect_add(tiler1, &rects[rect_idx][0][i]);
        if (rects[rect_idx][1][i].w > 0 && rects[rect_idx][1][i].w > 0)
          eina_tiler_rect_add(tiler2, &rects[rect_idx][1][i]);
     }

   /* draw rects of tiler1 */
   itr = eina_tiler_iterator_new(tiler1);
   EINA_ITERATOR_FOREACH(itr, rect)
     {
        o = evas_object_rectangle_add(evas);
        evas_object_color_set(o, 255, 0, 0, 255);
        evas_object_resize(o, rect->w, rect->h);
        evas_object_move(o, TILER1_X + rect->x, TILER1_Y + rect->y);
        evas_object_show(o);
        t1_objs = eina_list_append(t1_objs, o);
     }
   eina_iterator_free(itr);

   /* draw rects of tiler2 */
   itr = eina_tiler_iterator_new(tiler2);
   EINA_ITERATOR_FOREACH(itr, rect)
     {
        o = evas_object_rectangle_add(evas);
        evas_object_color_set(o, 0, 0, 255, 255);
        evas_object_resize(o, rect->w, rect->h);
        evas_object_move(o, TILER2_X + rect->x, TILER2_Y + rect->y);
        evas_object_show(o);
        t2_objs = eina_list_append(t2_objs, o);
     }
   eina_iterator_free(itr);

   tiler_calc();
}

static void
_on_key_down(void *data,
             Evas *evas EINA_UNUSED,
             Evas_Object *o EINA_UNUSED,
             void *einfo)
{
  Evas_Event_Key_Down *ev;
  int x,y;

  ev = (Evas_Event_Key_Down *)einfo;

  if(strcmp(ev->key,"Escape") == 0)
    {
       ecore_main_loop_quit();
    }
  else if(strcmp(ev->key, "u") == 0)
    {
       if (cur_method == METHOD_UNION)
         return;

       cur_method = METHOD_UNION;
    }
  else if(strcmp(ev->key, "s") == 0)
    {
       if (cur_method == METHOD_SUBTRACT)
         return;

       cur_method = METHOD_SUBTRACT;
    }
  else if(strcmp(ev->key, "i") == 0)
    {
       if (cur_method == METHOD_INTERSECTION)
         return;

       cur_method = METHOD_INTERSECTION;
    }
  else if(strcmp(ev->key, "t") == 0)
    {
       /* change tiler tile size */
       tile_idx ++;
       tile_idx %= tile_count;
       fprintf(stdout, "Tiler Size : %d\n", tilesize[tile_idx]);

       rect_update();
       return;
    }
  else if(strcmp(ev->key, "n") == 0)
    {
       /* change arrangement of tilers's rects */
       rect_idx ++;
       rect_idx %= rect_count;
       rect_update();
       return;
    }
  else
    {
       fprintf(stdout, "Key %s not supported.\nCommands:%s", ev->key, commands);
       return;
    }

  tiler_calc();
}

int
main(int argc, char *argv[])
{
   Ecore_Evas *ee;
   Evas_Object *o;
   int i;
   Eina_Rectangle *rect;
   Eina_Iterator *itr;

   fprintf(stdout, "Usage.\nCommands:%s", commands);

   ecore_evas_init();
   ecore_init();
   evas_init();
   eina_init();

   ee = ecore_evas_new(NULL, 0, 0, WIN_W, WIN_H, NULL);
   if (!ee)
     {
        fputs("ERROR: Could not create window. Check ecore-evas install.\n",
              stderr);
        goto end;
     }

   evas = ecore_evas_get(ee);

   /* window bg */
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_resize(o, WIN_W, WIN_H);
   evas_object_show(o);
   evas_object_focus_set(o, EINA_TRUE);
   bg = o;

   /* tiler1 bg */
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 0, 0, 0, 255);
   evas_object_resize(o, TILER_W, TILER_H);
   evas_object_move(o, TILER1_X, TILER1_Y);
   evas_object_show(o);

   /* tiler2 bg */
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 0, 0, 0, 255);
   evas_object_resize(o, TILER_W, TILER_H);
   evas_object_move(o, TILER2_X, TILER2_Y);
   evas_object_show(o);

   add_text("Tiler1", TILER1_X, 0, TILER_W);
   add_text("Tiler2", TILER2_X, 0, TILER_W);

   tiler1 = eina_tiler_new(TILER_W, TILER_H);
   tiler2 = eina_tiler_new(TILER_W, TILER_H);

   rect_update();

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_key_down, NULL);

   ecore_evas_show(ee);
   ecore_main_loop_begin();

   eina_list_free(tiler_objs);
   eina_list_free(t1_objs);
   eina_list_free(t2_objs);
   eina_tiler_free(tiler1);
   eina_tiler_free(tiler2);
   ecore_evas_free(ee);

 end:
   eina_shutdown();
   evas_shutdown();
   ecore_shutdown();
   ecore_evas_shutdown();

   return 0;
}
