#define EFL_BETA_API_SUPPORT 1

#include <Efl_Ui.h>

 /**
 * Example of canvas textblock obstacles.
 *
 * You start with two registered obstacle objects. They are not visible
 * at first, so the canvas textblock simply shows the text that has been set to it.
 * Once the obstacle is visible (show/hide keys in the example), the text will
 * wrap around it.
 * This example allows you to test two obstacles registered to the same
 * canvas textblock object. Also, you can play with size and position for each.
 * Use the 'h' key to show the provided options for this test.
 *
 * @verbatim
 * gcc -g efl_canvas_textblock_obstacles_example.c -o efl_canvas_textblock_obstacles_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#define WIDTH  (360)
#define HEIGHT (240)

#define POINTER_CYCLE(_ptr, _array)                             \
  do                                                            \
    {                                                           \
       if ((unsigned int)(((unsigned char *)(_ptr)) - ((unsigned char *)(_array))) >= \
           sizeof(_array))                                      \
         _ptr = _array;                                         \
    }                                                           \
  while(0)

static const char *commands = \
  "commands are:\n"
  "\tt - change currently controlled obstacle\n"
  "\tv - show/hide current obstacle\n"
  "\ts - cycle current obstacle's size\n"
  "\tp - change current obstacle's position (random)\n"
  "\tw - cycle text wrapping modes (none/word/char/mixed)\n"
  "\th - print help\n";

struct text_preset_data
{
   const char        **font_ptr;
   const char         *font[3];

   const char        **wrap_ptr;
   const char         *wrap[4];

   int                *obs_size_ptr;
   int                 obs_size[3];

   Eo **obs_ptr; /* pointer to the currently controlled obstacle object */
   Eo *obs[2];
};

struct test_data
{
   Eo                     *win, *box, *bg, *text;
   struct text_preset_data t_data;
   Eina_Size2D             size;
};

static struct test_data d = {0};

static unsigned int
_getrand(unsigned int low, unsigned int high)
{
   return (rand() % (high - low)) + low;
}

static void
_style_set(const char *wrap)
{
   char buf[2000];
   snprintf(buf,
         2000,
         "font=Sans font_size=16 color=#000 wrap=%s",
         wrap);

   efl_canvas_textblock_style_apply(d.text, buf);
}

static void
_key_down(void *data EINA_UNUSED, const Efl_Event *ev)
{
   const char *key = efl_input_key_string_get(ev->info);
   if (!key)
     return;

   if (strcmp(key, "h") == 0) /* print help */
     {
        printf("%s\n", commands);
        return;
     }

   if (strcmp(key, "t") == 0) /* change obstacle type */
     {
        (d.t_data.obs_ptr)++;
        POINTER_CYCLE(d.t_data.obs_ptr, d.t_data.obs);

        printf("Now controlling obstacle: %p\n", *d.t_data.obs_ptr);

        return;
     }

   if (strcmp(key, "v") == 0) /* change obstacle visibility */
     {
        Eo *obj = *d.t_data.obs_ptr;
        if (efl_gfx_entity_visible_get(obj))
           efl_gfx_entity_visible_set(obj, EINA_FALSE);
        else
           efl_gfx_entity_visible_set(obj, EINA_TRUE);

        printf("Show/hide toggle for obstacle %p\n",
               *d.t_data.obs_ptr);

        efl_canvas_textblock_obstacles_update(d.text);

        return;
     }

   if (strcmp(key, "s") == 0) /* change obstacle size */
     {
        (d.t_data.obs_size_ptr)++;
        POINTER_CYCLE(d.t_data.obs_size_ptr, d.t_data.obs_size);

        efl_gfx_entity_size_set(*d.t_data.obs_ptr, EINA_SIZE2D(*d.t_data.obs_size_ptr, *d.t_data.obs_size_ptr));

        efl_canvas_textblock_obstacles_update(d.text);

        printf("Changing obstacle size to: %d,%d\n",
               *d.t_data.obs_size_ptr, *d.t_data.obs_size_ptr);

        return;
     }

   if (strcmp(key, "p") == 0) /* change obstacle position */
     {
        int  x, y;
        x = _getrand(0, d.size.w);
        y = _getrand(0, d.size.h);

        efl_gfx_entity_position_set(*d.t_data.obs_ptr, EINA_POSITION2D(x, y));
        efl_canvas_textblock_obstacles_update(d.text);

        printf("Changing obstacles position\n");
        efl_gfx_entity_position_set(*d.t_data.obs_ptr, EINA_POSITION2D(x, y));

        Eina_Position2D r_rec = efl_gfx_entity_position_get(d.t_data.obs[0]);
        Eina_Position2D g_rec = efl_gfx_entity_position_get(d.t_data.obs[1]);

        printf("Obstacle #1 (red)  : [%d,%d]\n", r_rec.x, r_rec.y);
        printf("Obstacle #2 (green): [%d,%d]\n", g_rec.x, g_rec.y);

        return;
     }

   if (strcmp(key, "w") == 0) /* change obstacle position */
     {
        (d.t_data.wrap_ptr)++;
        POINTER_CYCLE(d.t_data.wrap_ptr, d.t_data.wrap);
        printf("Changing wrap mode to: %s\n",
               *d.t_data.wrap_ptr);
        _style_set(*d.t_data.wrap_ptr);
        efl_canvas_textblock_obstacles_update(d.text);

        return;
     }
}

static void
_win_resize(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Size2D sz;

   sz = efl_gfx_entity_size_get(ev->object);
   efl_gfx_entity_size_set(d.bg, sz);
   efl_gfx_entity_size_set(d.text, sz);

   d.size = sz;
}

static void
_text_init()
{
   _style_set("word");

   efl_text_markup_set(d.text,
         "This example text demonstrates the textblock object"
         " with obstacle objects support."
         " Any evas object <item size=72x16></item>can register itself as an obstacle to the textblock"
         " object. Upon regi<color=#0ff>stering, it aff</color>ects the layout of the text in"
         " certain situations. Usually, when the obstacle shows above the text"
         " area, it will cause the layout of the text to split and move"
         " parts of it, so that all text area is apparent."
         );
}

static void
_gui_quit_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   efl_exit(0);
}

static void
_gui_setup()
{
   /* init values one is going to cycle through while running this
    * example */
   struct text_preset_data init_data =
   {
      .font = {"DejaVu", "Courier", "Utopia"},
      .wrap = {"word", "char", "mixed", "none"},
      .obs_size = {50, 70, 100},
      .obs = {NULL, NULL},
   };

   d.t_data = init_data;
   d.t_data.font_ptr = d.t_data.font;
   d.t_data.obs_size_ptr = d.t_data.obs_size;
   d.t_data.obs_ptr = d.t_data.obs;

   d.win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Obstacles Example"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_entity_size_set(d.win, EINA_SIZE2D(WIDTH, HEIGHT));
   printf("Window size set to [%d,%d]\n", WIDTH, HEIGHT);

   efl_event_callback_add(d.win, EFL_UI_WIN_EVENT_DELETE_REQUEST, _gui_quit_cb, NULL);
   efl_event_callback_add(d.win, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _win_resize, NULL);
   efl_event_callback_add(d.win, EFL_EVENT_KEY_DOWN, _key_down, NULL);

   d.bg = efl_add(EFL_CANVAS_RECTANGLE_CLASS, d.win,
           efl_gfx_color_set(efl_added, 255, 255, 255, 255));

   efl_gfx_entity_size_set(d.bg, EINA_SIZE2D(WIDTH, HEIGHT));
   efl_gfx_entity_position_set(d.bg, EINA_POSITION2D(0, 0));

   d.text = efl_add(EFL_CANVAS_TEXTBLOCK_CLASS, d.win,
           efl_text_multiline_set(efl_added, EINA_TRUE));

   _text_init();
   efl_gfx_entity_size_set(d.text, EINA_SIZE2D(WIDTH, HEIGHT));
   efl_gfx_entity_position_set(d.text, EINA_POSITION2D(0, 0));

   d.size.w = WIDTH;
   d.size.h = HEIGHT;

   /* init obstacles */
   d.t_data.obs[0] = efl_add(EFL_CANVAS_RECTANGLE_CLASS, d.win,
           efl_gfx_color_set(efl_added, 255, 0, 0, 255));

   efl_gfx_entity_size_set(d.t_data.obs[0], EINA_SIZE2D(50,50));

   d.t_data.obs[1] = efl_add(EFL_CANVAS_RECTANGLE_CLASS, d.win,
           efl_gfx_color_set(efl_added, 0, 255, 0, 255));

   efl_gfx_entity_size_set(d.t_data.obs[1], EINA_SIZE2D(50,50));

   efl_canvas_textblock_obstacle_add(d.text, d.t_data.obs[0]);
   efl_canvas_textblock_obstacle_add(d.text, d.t_data.obs[1]);

   printf("%s\n", commands);
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   _gui_setup();
}
EFL_MAIN()
