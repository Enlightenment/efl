/**
 * Simple Edje example illustrating table functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc table.edc && gcc -o edje-table edje-table.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (400)
#define HEIGHT (400)

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/table.edj";

static Ecore_Evas *ee;
static Evas *evas;
static Evas_Object *bg, *edje_obj, *rects[4];

static void
_on_delete(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

/* Try to get the number of columns and rows of the table
 * and print them. */
static void
_columns_rows_print(void)
{
   int cols, rows;

   if (edje_object_part_table_col_row_size_get(edje_obj, "table_part", &cols,
                                               &rows))
     fprintf(stdout, "Number of columns: %d\nNumber of rows: %d\n", cols, rows);
   else
     fprintf(stderr, "Cannot get the number of columns and rows\n");
}

/* here just to keep our example's window size and table items
 * size in synchrony. */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int i, w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   evas_object_resize(bg, w, h);
   evas_object_resize(edje_obj, w, h);

   for (i = 0; i < 4; i++)
     evas_object_size_hint_min_set(rects[i], w/2, h/2);
}

/* Mouse button 1 = remove the clicked item
 * any other button = remove all items. */
static void
_on_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button != 1)
     edje_object_part_table_clear(edje_obj, "table_part", EINA_TRUE);
   else if (!edje_object_part_table_unpack(edje_obj, "table_part", obj))
     fprintf(stderr, "Cannot remove the selected rectangle\n");

   evas_object_del(obj);
   _columns_rows_print();
}

static void
_rects_create(void)
{
   int i;

   for (i = 0; i < 4; i++)
     {
        rects[i] = evas_object_rectangle_add(evas);
        evas_object_size_hint_min_set(rects[i], 200, 200);
        evas_object_size_hint_weight_set(rects[i], 1.0, 1.0);
        evas_object_show(rects[i]);
        evas_object_event_callback_add(rects[i], EVAS_CALLBACK_MOUSE_DOWN,
                                       _on_mouse_down, NULL);
     }
}

int
main(void)
{
   int i;

   ecore_evas_init();
   edje_init();

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_callback_resize_set(ee, _canvas_resize_cb);
   ecore_evas_title_set(ee, "Edje Table Example");
   ecore_evas_show(ee);

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   edje_obj = edje_object_add(evas);

   edje_object_file_set(edje_obj, edje_file_path, "example_table");
   evas_object_move(edje_obj, 0, 0); /* at canvas' origin */
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);

   _rects_create();

   /* Colouring the rectangles */
   evas_object_color_set(rects[0], 255, 0, 0, 255);
   evas_object_color_set(rects[1], 0, 255, 0, 255);
   evas_object_color_set(rects[2], 0, 0, 255, 255);
   evas_object_color_set(rects[3], 128, 128, 128, 255);

   /* Packing the rectangles into the table */
   if (!edje_object_part_table_pack(edje_obj, "table_part", rects[0],
                                    0, 0, 1, 2))
     fprintf(stderr, "Cannot add the rectangle 1 to table\n");

   if (!edje_object_part_table_pack(edje_obj, "table_part", rects[1],
                                    0, 1, 1, 1))
     fprintf(stderr, "Cannot add the rectangle 2 to table\n");

   if (!edje_object_part_table_pack(edje_obj, "table_part", rects[2],
                                    1, 0, 1, 1))
     fprintf(stderr, "Cannot add the rectangle 3 to table\n");

   if (!edje_object_part_table_pack(edje_obj, "table_part", rects[3],
                                    1, 1, 1, 1))
     fprintf(stderr, "Cannot add the rectangle 4 to table\n");

   _columns_rows_print();

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;
}
