/**
 * Simple Edje Edit example illustrating BOX part's and it's items editing.
 *
 * This example is intended for people who want to write an Edje editor. There
 * is no other application that should use it, EVER ! If you don't plan to write
 * an Edje editor please move away from this file.
 *
 * See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc box_example.edc && gcc -o edje-edit-part-box edje-edit-part-box.c `pkg-config --libs --cflags evas ecore ecore-evas edje eina`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje_Edit.h>
#include <Edje.h>

#define WIDTH  (500)
#define HEIGHT (300)

static const char commands[] = \
  "commands are:\n"
  "\tl - change box layout\n"
  "\tL - change box alternative layout\n"
  "\ti - append one/remove item\n"
  "\tg - change source of middle item\n"
  "\tM - change max of middle item\n"
  "\tm - change min of middle item\n"
  "\ts - change spread (w and h) of middle item\n"
  "\tb - change paddings of middle item\n"
  "\tw - change weight of middle item\n"
  "\tn - change align of middle item\n"
  "\tp - print resulted source code of edj file (after changing with edje edit)\n"
  "\th - print help\n"
  "\tEsc - exit\n";

static int group_number = 0;
static const char *groups[] =
{
   "blue_group", "red_group", "green_group", "complex_group"
};

static int layout_number = 0;
static int alt_layout_number = 0;
static char *layouts[] =
{
   "horizontal", "vertical", "horizontal_homogeneous", "vertical_homogeneous",
   "horizontal_max", "vertical_max", "horizontal_flow", "vertical_flow", "stack",
   NULL
};

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_bg_key_down(void *data EINA_UNUSED,
                Evas *e EINA_UNUSED,
                Evas_Object *o,
                void *event_info)
{
   const char  *edje_file = PACKAGE_DATA_DIR"/box_example.edj";
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_edit_obj = o;

   ev = (Evas_Event_Key_Down *)event_info;

   if (!strcmp(ev->key, "h"))
     {
        printf(commands);
        return;
     }
   if (!strcmp(ev->key, "p"))
     {
        printf("SOURCE CODE:\n%s \n", edje_edit_source_generate(edje_edit_obj));
        return;
     }
   if (!strcmp(ev->key, "n"))
     {
        double align_x = edje_edit_part_item_index_align_x_get(edje_edit_obj, "green", 1);
        double align_y = edje_edit_part_item_index_align_y_get(edje_edit_obj, "green", 1);

        align_x += 0.1; align_y += 0.2;
        if (align_x >= 1.0) align_x = 0.0;
        if (align_y >= 1.0) align_y = 0.0;

        printf("Changing align to x[%f] y[%f] \n", align_x, align_y);

        if (!edje_edit_part_item_index_align_x_set(edje_edit_obj, "green", 1, align_x))
          fprintf(stderr, "Couldn't set align x for item x2, something is absolutely wrong!!!\n");
        if (!edje_edit_part_item_index_align_y_set(edje_edit_obj, "green", 1, align_y))
          fprintf(stderr, "Couldn't set align y for item x2, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        return;
     }
   if (!strcmp(ev->key, "w"))
     {
        double weight_x = edje_edit_part_item_index_weight_x_get(edje_edit_obj, "green", 1);
        double weight_y = edje_edit_part_item_index_weight_y_get(edje_edit_obj, "green", 1);

        weight_x += 5.0; weight_y += 10.0;
        if (weight_x >= 30.0) weight_x = 0.0;
        if (weight_y >= 30.0) weight_y = 0.0;

        printf("Changing weight to x[%f] y[%f] \n", weight_x, weight_y);

        if (!edje_edit_part_item_index_weight_x_set(edje_edit_obj, "green", 1, weight_x))
          fprintf(stderr, "Couldn't set weight x for item x2, something is absolutely wrong!!!\n");
        if (!edje_edit_part_item_index_weight_y_set(edje_edit_obj, "green", 1, weight_y))
          fprintf(stderr, "Couldn't set weight y for item x2, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        return;
     }
   if (!strcmp(ev->key, "b"))
     {
        Evas_Coord l, r, t, b;
        edje_edit_part_item_index_padding_get(edje_edit_obj, "green", 1, &l, &r, &t, &b);

        l += 1; r += 2; t += 4; b += 8;
        if (l >= 32) l = 0;
        if (r >= 32) r = 0;
        if (t >= 32) t = 0;
        if (b >= 32) b = 0;

        printf("Changing paddings. left[%d], right[%d], top[%d], bottom[%d]\n", l, r, t, b);

        if (!edje_edit_part_item_index_padding_set(edje_edit_obj, "green", 1, l, r, t, b))
          fprintf(stderr, "Couldn't set paddings for item x2, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        return;
     }
   if (!strcmp(ev->key, "s"))
     {
        /* numbder of spread item is (spread_w * spread_h) */
        int spread_w = edje_edit_part_item_index_spread_w_get(edje_edit_obj, "green", 1);
        int spread_h = edje_edit_part_item_index_spread_h_get(edje_edit_obj, "green", 1);

        spread_w++;
        if (spread_w >= 5)
          {
             spread_w = 1;
             spread_h++;
             if (spread_h >= 5)
               spread_h = 1;
          }

        printf("Changing spread to w[%d] h[%d] \n", spread_w, spread_h);

        if (!edje_edit_part_item_index_spread_w_set(edje_edit_obj, "green", 1, spread_w))
          fprintf(stderr, "Couldn't set spread w for item x2, something is absolutely wrong!!!\n");
        if (!edje_edit_part_item_index_spread_h_set(edje_edit_obj, "green", 1, spread_h))
          fprintf(stderr, "Couldn't set spread h for item x2, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        return;
     }
   if (!strcmp(ev->key, "m"))
     {
        int min_w = edje_edit_part_item_index_min_w_get(edje_edit_obj, "green", 1);
        int min_h = edje_edit_part_item_index_min_h_get(edje_edit_obj, "green", 1);

        min_w += 100; min_h += 100;
        if (min_w >= 500) min_w = 200;
        if (min_h >= 500) min_h = 200;

        printf("Changing min to w[%d] h[%d] \n", min_w, min_h);

        if (!edje_edit_part_item_index_min_w_set(edje_edit_obj, "green", 1, min_w))
          fprintf(stderr, "Couldn't set min w for item x2, something is absolutely wrong!!!\n");
        if (!edje_edit_part_item_index_min_h_set(edje_edit_obj, "green", 1, min_h))
          fprintf(stderr, "Couldn't set min h for item x2, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        return;
     }
   if (!strcmp(ev->key, "M"))
     {
        int max_w = edje_edit_part_item_index_max_w_get(edje_edit_obj, "green", 1);
        int max_h = edje_edit_part_item_index_max_h_get(edje_edit_obj, "green", 1);

        max_w -= 100; max_h -= 100;
        if (max_w <= 0) max_w = 800;
        if (max_h <= 0) max_h = 800;

        printf("Changing max to w[%d] h[%d] \n", max_w, max_h);

        if (!edje_edit_part_item_index_max_w_set(edje_edit_obj, "green", 1, max_w))
          fprintf(stderr, "Couldn't set max w for item x2, something is absolutely wrong!!!\n");
        if (!edje_edit_part_item_index_max_h_set(edje_edit_obj, "green", 1, max_h))
          fprintf(stderr, "Couldn't set max h for item x2, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        return;
     }
   if (!strcmp(ev->key, "g"))
     {
        Eina_Stringshare *source = edje_edit_part_item_index_source_get(edje_edit_obj, "green", 1);
        const char *new_source = NULL;

        group_number++;
        if (group_number > 3)
          group_number = 0;
         new_source = groups[group_number];

        printf("Changing item's source from [%s] to [%s] \n", source, new_source);

        if (!edje_edit_part_item_index_source_set(edje_edit_obj, "green", 1, new_source))
          fprintf(stderr, "Couldn't change item's source, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        eina_stringshare_del(source);
        return;
     }
   if (!strcmp(ev->key, "i"))
     {
        if (!edje_edit_part_item_append(edje_edit_obj, "green", "x_new", "complex_group"))
          {
             edje_edit_part_item_index_del(edje_edit_obj, "green", 3);
             printf("Successfully removed new item!\n");
          }
        else
          {
             if (!edje_edit_part_item_index_min_w_set(edje_edit_obj, "green", 3, 200))
               fprintf(stderr, "Couldn't set min w for item x_new, something is absolutely wrong!!!\n");
             if (!edje_edit_part_item_index_min_h_set(edje_edit_obj, "green", 3, 200))
               fprintf(stderr, "Couldn't set min h for item x_new, something is absolutely wrong!!!\n");
             printf("Successfully added new item!\n");
          }

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        return;
     }
   if (!strcmp(ev->key, "l"))
     {
        Eina_Stringshare *layout = edje_edit_state_box_layout_get(edje_edit_obj, "green", "default", 0.0);
        char *new_layout = NULL;

        layout_number++;
        if (layout_number > 9)
          layout_number = 0;
        new_layout = layouts[layout_number];

        printf("Changing box's layout from [%s] to [%s] \n", layout, new_layout);

        if (!edje_edit_state_box_layout_set(edje_edit_obj, "green", "default", 0.0, new_layout))
          fprintf(stderr, "Couldn't change layout, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        eina_stringshare_del(layout);
        return;
     }
   if (!strcmp(ev->key, "L"))
     {
        Eina_Stringshare *layout = edje_edit_state_box_alt_layout_get(edje_edit_obj, "green", "default", 0.0);
        char *new_layout = NULL;

        alt_layout_number++;
        if (alt_layout_number > 9)
          alt_layout_number = 0;
        new_layout = layouts[alt_layout_number];

        printf("Changing box's alternative layout from [%s] to [%s] \n", layout, new_layout);

        if (!edje_edit_state_box_alt_layout_set(edje_edit_obj, "green", "default", 0.0, new_layout))
          fprintf(stderr, "Couldn't change alternative layout, something is absolutely wrong!!!\n");

        edje_edit_save_all(edje_edit_obj);
        if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
          printf("Couldn't load edje edit object! \n");

        eina_stringshare_del(layout);
        return;
     }
   else if (!strcmp(ev->key, "Escape"))
     ecore_main_loop_quit();
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char  *edje_file = PACKAGE_DATA_DIR"/box_example.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *edje_edit_obj;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_delete_request_set(ee, _on_delete);

   ecore_evas_title_set(ee, "Edje Edit Example: BOX part");

   evas = ecore_evas_get(ee);

   /* loading edje edit object here */
   edje_edit_obj = edje_edit_object_add(evas);
   if (!edje_object_file_set(edje_edit_obj, edje_file, "box_group"))
     printf("Couldn't load edje edit object! \n");

   evas_object_resize(edje_edit_obj, WIDTH, HEIGHT);
   evas_object_show(edje_edit_obj);

   ecore_evas_data_set(ee, "background", edje_edit_obj);
   ecore_evas_object_associate(ee, edje_edit_obj, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
   evas_object_focus_set(edje_edit_obj, EINA_TRUE);
   evas_object_event_callback_add(edje_edit_obj, EVAS_CALLBACK_KEY_DOWN, _on_bg_key_down, ee);

   printf(commands);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}
