// gcc -o efl_canvas_layout_text efl_canvas_layout_text.c `pkg-config --cflags --libs elementary`
// edje_cc efl_canvas_layout.edc
// ./efl_canvas_layout_text

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define EFL_BETA_API_SUPPORT 1
# define EFL_EO_API_SUPPORT 1
#endif

#include <Efl.h>
#include <Elementary.h>
#include <string.h>

static void
_on_win_delete(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_exit(0);
}

static const char* groups[] = { "test", "test2" };
static size_t group_itr = 0;

static double ellipsis[] = { -1.0, 1.0 };
static size_t ellipsis_itr = 0;

static struct
{
   Efl_Text_Format_Wrap wrap;
   const char *desc;
} wraps[] =
{
     { EFL_TEXT_FORMAT_WRAP_NONE, "none" },
     { EFL_TEXT_FORMAT_WRAP_WORD, "word" },
     { EFL_TEXT_FORMAT_WRAP_CHAR, "char" },
     { EFL_TEXT_FORMAT_WRAP_MIXED, "mixed" }
};

static struct
{
   Efl_Text_Format_Wrap wrap;
   const char *desc;
} group[] =
{
     { EFL_TEXT_FORMAT_WRAP_NONE, "none" },
     { EFL_TEXT_FORMAT_WRAP_WORD, "word" },
     { EFL_TEXT_FORMAT_WRAP_CHAR, "char" },
     { EFL_TEXT_FORMAT_WRAP_MIXED, "mixed" }
};
static struct
{
   Efl_Canvas_Layout_Part_Text_Expand expand;
   const char *desc;
} expands[] =
{
     { EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_NONE, "none" },
     { EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_X, "min_x" },
     { EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_Y, "min_y" },
     { EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MAX_X, "max_x" },
     { EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MAX_Y, "max_y" },
     { EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_X | EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_Y , "min_x AND min_y"}
};

static size_t wrap_itr = 0;
static size_t expand_itr = 0;

static void
_help(void)
{
   printf("Press 'w' to cycle wrap modes.\n"
          "Press 's' to cycle expand hints and print min_size result.\n"
          "Press 'g' to cycle group examples.\n"
          "Press 'e' to cycle ellipsis modes.\n"
          "Press 'h' to display this help.\n");
}

static const char *edjefile = "efl_canvas_layout_text.edj";

static void
_on_key_down(void *data, const Efl_Event *event)
{
   Efl_Input_Key *ev = event->info;
   Eo *layout = data;
   const char *key = efl_input_key_name_get(ev);

   if (!strcmp(key, "g"))
     {
        // edje group
        group_itr = (group_itr + 1) % 2;
        efl_file_set(layout, edjefile, groups[group_itr]);
        printf("Changed layout group to: %s\n", groups[group_itr]);
     }
   else if (!strcmp(key, "w"))
     {
        // wrap mode
        wrap_itr = (wrap_itr + 1) % 4;
        efl_text_wrap_set(efl_part(layout, "text"), wraps[wrap_itr].wrap);
        printf("Changed wrap to %s\n", wraps[wrap_itr].desc);
     }
   else if (!strcmp(key, "e"))
     {
        // ellipsis value
        ellipsis_itr = (ellipsis_itr + 1) % 2;
        efl_text_ellipsis_set(efl_part(layout, "text"), ellipsis[ellipsis_itr]);
        printf("Changed ellipsis to %f\n", ellipsis[ellipsis_itr]);
     }
   else if (!strcmp(key, "s"))
     {
        Eina_Size2D sz;
        // expand mode
        expand_itr = (expand_itr + 1) % 6;
        efl_canvas_layout_part_text_expand_set(efl_part(layout, "text"),
              expands[expand_itr].expand);
        printf("Changed expand mode to: %s\n", expands[expand_itr].desc);

        sz = efl_layout_calc_size_min(layout, EINA_SIZE2D(10, 10));
        printf("new expand_min: %dx%d\n", sz.w, sz.h);
     }
   else if (!strcmp(key, "h"))
     {
        _help();
     }
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{

   Eo *layout;
   Eo *win;

   win = efl_add(EFL_UI_WIN_CLASS, NULL,
         efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
         efl_text_set(efl_added, "Efl Canvas_Layout"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE),
         efl_event_callback_add(efl_added, EFL_UI_WIN_EVENT_DELETE_REQUEST, _on_win_delete, NULL));


   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, win);
   efl_file_set(layout, edjefile, groups[group_itr]);

   efl_content_set(win, layout);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(110, 100));

   efl_text_markup_set(efl_part(layout, "text"), "This is an example text. This is a layout text part.");
   efl_text_wrap_set(efl_part(layout, "text"), wraps[wrap_itr].wrap);
   efl_text_normal_color_set(efl_part(layout, "text"), 255, 255, 0, 255);
   efl_text_font_set(efl_part(layout, "text"), "Serif", 12);

   efl_canvas_layout_part_text_expand_set(efl_part(layout, "text"),
         expands[expand_itr].expand);

   efl_event_callback_add(win, EFL_EVENT_KEY_DOWN, _on_key_down, layout);

}
EFL_MAIN()
