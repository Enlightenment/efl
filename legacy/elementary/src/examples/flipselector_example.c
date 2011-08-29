/**
 * Simple Elementary's <b>flip selector widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` flipselector_example.c -o flipselector_example
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static const char *commands = \
  "commands are:\n"
  "\tn - flip to next item\n"
  "\tp - flip to previous item\n"
  "\tf - print first item's label\n"
  "\tl - print last item's label\n"
  "\ts - print selected item's label\n"
  "\th - print help\n";

static void
_on_done(void        *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void        *event_info __UNUSED__)
{
   elm_exit();
}

void /* unselect the item shown in the flip selector */
_unsel_cb(void        *data,
          Evas_Object *obj __UNUSED__,
          void        *event_info __UNUSED__)
{
   Elm_Flipselector_Item *it;
   Evas_Object *fp = data;

   it = elm_flipselector_selected_item_get(fp);
   elm_flipselector_item_selected_set(it, EINA_FALSE);
}

void /* delete the item shown in the flip selector */
_del_cb(void        *data,
          Evas_Object *obj __UNUSED__,
          void        *event_info __UNUSED__)
{
   Elm_Flipselector_Item *it;
   Evas_Object *fp = data;

   it = elm_flipselector_selected_item_get(fp);
   if (it) elm_flipselector_item_del(it);
}

void /* underflow callback */
_underflow_cb(void        *data __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void        *event_info __UNUSED__)
{
   fprintf(stdout, "Underflow!\n");
}

void /* overflow callback */
_overflow_cb(void        *data __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void        *event_info __UNUSED__)
{
   fprintf(stdout, "Overflow!\n");
}

static void
_on_keydown(void              *data,
            Evas_Object       *object __UNUSED__,
            Evas_Object       *src __UNUSED__,
            Evas_Callback_Type type,
            void              *event_info)
{
   Evas_Object *fs = data;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return;

   if (strcmp(ev->keyname, "h") == 0) /* print help */
     {
        fprintf(stdout, commands);
        return;
     }

   if (strcmp(ev->keyname, "n") == 0) /* flip to next item */
     {
         elm_flipselector_flip_next(fs);

        fprintf(stdout, "Flipping to next item\n");

        return;
     }

   if (strcmp(ev->keyname, "p") == 0) /* flip to previous item */
     {
         elm_flipselector_flip_prev(fs);

        fprintf(stdout, "Flipping to previous item\n");

        return;
     }

   if (strcmp(ev->keyname, "f") == 0) /* print first item's label */
     {
         Elm_Flipselector_Item *it;

         it = elm_flipselector_first_item_get(fs);

         fprintf(stdout, "Flip selector's first item is: %s\n", it ?
                 elm_flipselector_item_label_get(it) : "none");

        return;
     }

   if (strcmp(ev->keyname, "l") == 0) /* print last item's label */
     {
         Elm_Flipselector_Item *it;

         it = elm_flipselector_last_item_get(fs);

         fprintf(stdout, "Flip selector's last item is: %s\n", it ?
                 elm_flipselector_item_label_get(it) : "none");

        return;
     }

   if (strcmp(ev->keyname, "s") == 0) /* print selected item's label */
     {
         Elm_Flipselector_Item *it;

         it = elm_flipselector_selected_item_get(fs);

         fprintf(stdout, "Flip selector's selected item is: %s\n", it ?
                 elm_flipselector_item_label_get(it) : "none");

        return;
     }
}

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   unsigned int i;
   Evas_Object *win, *bg, *bx, *fp, *bt;
   static const char *lbl[] =
   {
      "Elementary",
      "Evas",
      "Eina",
      "Edje",
      "Eet",
      "Ecore",
      "Efreet",
      "Edbus"
   };

   win = elm_win_add(NULL, "flipselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Flip Selector Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fp = elm_flipselector_add(win);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_flipselector_item_append(fp, lbl[i], NULL, NULL);
   elm_box_pack_end(bx, fp);
   evas_object_show(fp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Unselect item");
   evas_object_smart_callback_add(bt, "clicked", _unsel_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete item");
   evas_object_smart_callback_add(bt, "clicked", _del_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   elm_object_event_callback_add(win, (Elm_Event_Cb)_on_keydown, fp);

   evas_object_show(win);

   fprintf(stdout, commands);
   elm_run();
   return 0;
}
ELM_MAIN()
