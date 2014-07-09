/**
 * Simple Elementary's <b>index widget</b> example, illustrating its
 * usage and API -- now with sorted insertions.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g index_example_02.c -o index_example_02 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static const char *items[] =
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
_index_changed(void        *data,
               Evas_Object *obj,
               void        *event_info)
{
   Elm_Object_Item *item = elm_object_item_data_get(event_info);
   elm_gengrid_item_bring_in(item, ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static char *
_grid_label_get(void        *data,
                Evas_Object *obj,
                const char  *part)
{
   int idx = (int)(uintptr_t)data;
   return strdup(items[idx]);
}

Evas_Object *
_grid_content_get(void        *data,
                  Evas_Object *obj,
                  const char  *part)
{
   if (!strcmp(part, "elm.swallow.icon"))
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(),
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

   const Elm_Object_Item *index_it1 = data1;
   const Elm_Object_Item *index_it2 = data2;

   label1 = elm_index_item_letter_get(index_it1);
   label2 = elm_index_item_letter_get(index_it2);

   return strcasecmp(label1, label2);
}

EAPI_MAIN int
elm_main(int    argc,
         char **argv)
{
   Evas_Object *win, *grid, *idx;
   Elm_Object_Item *gg_it;
   unsigned int i;

   Elm_Gengrid_Item_Class gic;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_info_set(elm_main, "elementary", "images");

   win = elm_win_util_standard_add("index", "Index Example");
   elm_win_autodel_set(win, EINA_TRUE);

   grid = elm_gengrid_add(win);
   elm_gengrid_item_size_set(grid, 150, 150);

   gic.item_style = "default";
   gic.func.text_get = _grid_label_get;
   gic.func.content_get = _grid_content_get;
   gic.func.state_get = NULL;
   gic.func.del = NULL;

   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, grid);
   evas_object_show(grid);

   idx = elm_index_add(win);
   evas_object_size_hint_weight_set(idx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, idx);
   evas_object_show(idx);

   for (i = 0; i < (sizeof(items) / sizeof(items[0])); i++)
     {
        char buf[32];

        gg_it = elm_gengrid_item_append(grid, &gic, (void *)(uintptr_t)i, NULL, NULL);

        /* indexing by first letters */
        snprintf(buf, sizeof(buf), "%c", items[i][0]);
        elm_index_item_sorted_insert(idx, buf, NULL, gg_it, _index_icmp, NULL);
     }

   evas_object_smart_callback_add(idx, "delay,changed", _index_changed, NULL);

   elm_index_autohide_disabled_set(idx, EINA_FALSE);
   elm_index_level_go(idx, 0);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
