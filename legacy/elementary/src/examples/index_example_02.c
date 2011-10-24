/**
 * Simple Elementary's <b>index widget</b> example, illustrating its
 * usage and API -- now with sorted insertions.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` index_example.c -o index_example
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
# define PACKAGE_DATA_DIR "../../data"
#endif

static const char *items[] = \
{
   "Judith",
   "Paulina",
   "Cathy",
   "Vendella",
   "Naomi",
   "Ashley",
   "Stacey",
   "Gail"
};

static void
_index_changed(void        *data __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void        *event_info)
{
   elm_gengrid_item_bring_in(event_info);
}

static void
_on_done(void        *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void        *event_info __UNUSED__)
{
   elm_exit();
}

static char *
_grid_label_get(void        *data,
                Evas_Object *obj __UNUSED__,
                const char  *part __UNUSED__)
{
   int idx = (int)data;
   return strdup(items[idx]);
}

Evas_Object *
_grid_content_get(void        *data __UNUSED__,
               Evas_Object *obj,
               const char  *part)
{
   if (!strcmp(part, "elm.swallow.icon"))
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%s/images/%s", PACKAGE_DATA_DIR,
                 "sky_01.jpg");

        Evas_Object *icon = elm_bg_add(obj);
        elm_bg_file_set(icon, buf, NULL);
        evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1,
                                         1);
        evas_object_show(icon);
        return icon;
     }

   return NULL;
}

/* ordering alphabetically */
static int
_index_icmp(const void *data1,
            const void *data2)
{
   const char *label1, *label2;

   const Elm_Index_Item *it1 = data1;
   const Elm_Index_Item *it2 = data2;

   label1 = elm_index_item_letter_get(it1);
   label2 = elm_index_item_letter_get(it2);

   return strcasecmp(label1, label2);
}

int
elm_main(int    argc __UNUSED__,
         char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *grid, *index;
   Elm_Gengrid_Item *it;
   unsigned int i;

   Elm_Gengrid_Item_Class gic;

   win = elm_win_add(NULL, "index", ELM_WIN_BASIC);
   elm_win_title_set(win, "Index Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 150, 150);

   gic.item_style = "default";
   gic.func.label_get = _grid_label_get;
   gic.func.content_get = _grid_icon_get;
   gic.func.state_get = NULL;
   gic.func.del = NULL;

   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   evas_object_show(grid);

   index = elm_index_add(win);
   evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, index);

   evas_object_show(index);

   for (i = 0; i < (sizeof(items) / sizeof(items[0])); i++)
     {
        char buf[32];

        it = elm_gengrid_item_append(grid, &gic, (void *)i, NULL, NULL);

        /* indexing by first letters */
        snprintf(buf, sizeof(buf), "%c", items[i][0]);
        elm_index_item_sorted_insert(index, buf, it, _index_icmp, NULL);
     }

   evas_object_smart_callback_add(index, "delay,changed", _index_changed, NULL);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);

   elm_index_active_set(index, EINA_TRUE);

   elm_run();
   return 0;
}

ELM_MAIN()
