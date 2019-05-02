/**
 * gcc -g filter_example.c -o filter_example `pkg-config --cflags --libs elementary`
 */
#define EFL_BETA_API_SUPPORT 1 
#include <Elementary.h>

typedef struct _Filter
{
   const char *name;
   const char *code;
} Filter;

static Filter filters[] = {
   { "no", NULL },
   { "blend",
     "blend { color = '#fff8' }" },
   { "blur",
     "blur { 15 }" },
   { "grow",
     "a = buffer { 'rgba' }\n"
     "blend { dst = a }\n"
     "grow { 6, src = a }" },
   { "curve",
     "a = buffer ('alpha')\n"
     "blur ({ 4, dst = a })\n"
     "p = {}\n"
     "p[0] = 0\n"
     "p[20] = 0\n"
     "p[60] = 255\n"
     "p[160] = 255\n"
     "p[200] = 0\n"
     "p[255] = 0\n"
     "curve ({ points = p, src = a, dst = a })\n"
     "blend ({ src = a, color = 'white' })\n" },
   { "fill",
     "fill { color = 'darkblue' }" },
   { "mask",
     "a = buffer ('alpha')\n"
     "blur ({ 6, dst = a })\n"
     "p = {}\n"
     "p[0] = 255\n"
     "p[128] = 255\n"
     "p[255] = 0\n"
     "curve ({ points = p, src = a, dst = a })\n"
     "blend ({ color = 'black' })\n"
     "mask ({ mask = a, color = 'cyan' })" },
   { "bump",
     "a = buffer { 'alpha' }\n"
     "grow { 5, dst = a }\n"
     "blur { 6, src = a , dst = a }\n"
     "bump { map = a, color = '#f60', specular = 1, compensate = true }" },
   { "trans",
     "t = buffer ('alpha')\n"
     "transform ({ oy = 20, dst = t })\n"
     "blend ({ src = t, color = '#fff8' })\n"
     "blend ({ color = 'white' })" },
};

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Filter *f;
   unsigned int i;
   char buf[PATH_MAX];
   Eo *win, *scroller, *hbox, *box, *text, *img;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_app_info_set(elm_main, "elementary", "images");

   win = elm_win_util_standard_add("Gfx Filter Test", "Gfx Filter Test");
   elm_win_autodel_set(win, EINA_TRUE);

   scroller = elm_scroller_add(win);
   evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, scroller);
   evas_object_show(scroller);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(scroller, hbox);
   evas_object_show(hbox);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbox, box);
   evas_object_show(box);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(filters); i++)
     {
        f = &filters[i];
        text = evas_object_text_add(evas_object_evas_get(win));
        evas_object_size_hint_align_set(text, 0, EVAS_HINT_FILL);
        evas_object_size_hint_min_set(text, 100, 100);
        evas_object_text_font_set(text, "Sans:style=Bold", 50);
        evas_object_text_text_set(text, f->name);
        elm_box_pack_end(box, text);
        evas_object_show(text);

        efl_gfx_filter_program_set(text, f->code, f->name);
     }

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(hbox, box);
   evas_object_show(box);

   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   for (i = 0; i < EINA_C_ARRAY_LENGTH(filters); i++)
     {
        f = &filters[i];
        img = evas_object_image_filled_add(evas_object_evas_get(win));
        evas_object_size_hint_align_set(img, 0, EVAS_HINT_FILL);
        evas_object_image_file_set(img, buf, 0);
        evas_object_size_hint_min_set(img, 100, 100);
        elm_box_pack_end(box, img);
        evas_object_show(img);

        efl_gfx_filter_program_set(img, f->code, f->name);
     }

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
