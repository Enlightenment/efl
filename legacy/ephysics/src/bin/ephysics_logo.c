#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include <EPhysics.h>
#include <Evas.h>

/* TODO: move to EPhysics.h */
#define EPHYSICS_BODY_MASS_IMMUTABLE (0.0)

#define WIDTH (512)
#define HEIGHT (384)
#define FLOOR_Y (HEIGHT - 80)
#define SH_THRESHOLD (250)
#define SH_OFFSET_X (- 16)
#define OFFSET_X (90)
#define PADDING_X_1 (16)
#define PADDING_X_2 (12)
#define PADDING_X_3 (22)
#define E_THRESHOLD (WIDTH + 560)
#define LAYER_SHADOW (10)
#define LAYER_LETTER (20)

#define CENTER(total, item)  (((total) - (item)) / 2)
#define LIMIT(val, op, ref) (((val) op (ref)) ? (val) : (ref));
#define PROP_GET(pos, min, max) (((min) + ((max) - (min)) * (pos)) / (max))

static void
_update_box_cb(void *data __UNUSED__, EPhysics_Body *body, void *event_info)
{
   Evas_Object *image = event_info;
   Evas_Object *shadow = evas_object_data_get(image, "shadow");
   Evas_Object *light = evas_object_data_get(image, "light");
   int x, y, w, h, floor_distance, alpha = 0;

   /* modify the evas object according to the body */
   ephysics_body_evas_object_update(body);
   evas_object_geometry_get(image, &x, &y, &w, &h);

   floor_distance = FLOOR_Y - h;

   /* Bodies penetrates the ground slightly before bouncing. */
   /* This is to be expected in realtime physics engines. */
   // TODO BUG: should we move the object up by the difference????
   //y = LIMIT(y, <=, floor_distance);

   /* We should show the shadow when we're close enough to ground */
   if (y > SH_THRESHOLD)
     {
        int sh_w, sh_h;
        double pos_x;

        evas_object_image_size_get(shadow, &sh_w, &sh_h);
        /* shadow is darker with bigger y */
        alpha = 255 * (y - SH_THRESHOLD) / (floor_distance - SH_THRESHOLD);
        /* and with bigger x -- it's proportional to x / WIDTH, but varies
         * from 100 to 255.
         */
        pos_x = (double) x / (WIDTH - w);
        alpha = alpha * PROP_GET(pos_x, 100, 255);
        /* box shadow is not resized, just moved */
        evas_object_move(shadow, x + CENTER(w, sh_w) + SH_OFFSET_X,
                         FLOOR_Y - sh_h + 2);
     }
   evas_object_color_set(shadow, alpha, alpha, alpha, alpha);

   evas_object_move(light, x, y);
   /* it's lighter with bigger y */
   alpha = (y <= 0) ? 0 : y * 255 / floor_distance;
   /* and with bigger x */
   alpha = alpha * (x - OFFSET_X + 80) / (WIDTH - OFFSET_X);
   evas_object_color_set(light, alpha, alpha, alpha, alpha);
}

static void
_update_circle_cb(void *data __UNUSED__, EPhysics_Body *body, void *event_info)
{
   Evas_Object *image = event_info;
   Evas_Object *shadow = evas_object_data_get(image, "shadow");
   Evas_Object *light = evas_object_data_get(image, "light");
   int x, y, w, h, sh_w, sh_h, alpha = 0;
   const Evas_Map *map;

   /* modify the evas object according to the body */
   ephysics_body_evas_object_update(body);
   evas_object_geometry_get(image, &x, &y, &w, &h);

   evas_object_move(light, x, y);
   alpha = x * 255 / (WIDTH - w);
   evas_object_color_set(light, alpha, alpha, alpha, alpha);

   /* use the same map from image to the light (rotate it) */
   map = evas_object_map_get(image);
   evas_object_map_set(light, map);
   evas_object_map_enable_set(light, EINA_TRUE);

   evas_object_image_size_get(shadow, &sh_w, &sh_h);
   evas_object_move(shadow, x + CENTER(w, sh_w) + SH_OFFSET_X,
                    FLOOR_Y - sh_h + 2);
   alpha = 127 + alpha / 2;
   evas_object_color_set(shadow, alpha, alpha, alpha, alpha);

   if (x > E_THRESHOLD)
     ephysics_body_move(body, -w - 1, y);
}

static void
_letter_add(Evas *evas, const char *letter, Evas_Object **image, Evas_Object **light, Evas_Object **shadow)
{
   int w, h, sh_w, sh_h;
   char buf[1024];

   snprintf(buf, sizeof(buf), PACKAGE_DATA_DIR "/logo_shadow-%s.png", letter);
   *shadow = evas_object_image_filled_add(evas);
   evas_object_image_file_set(*shadow, buf, NULL);
   evas_object_image_size_get(*shadow, &sh_w, &sh_h);
   evas_object_resize(*shadow, sh_w, sh_h);
   evas_object_color_set(*shadow, 0, 0, 0, 0);
   evas_object_layer_set(*shadow, LAYER_SHADOW);
   evas_object_show(*shadow);

   snprintf(buf, sizeof(buf), PACKAGE_DATA_DIR "/logo_letter-%s.png", letter);
   *image = evas_object_image_filled_add(evas);
   evas_object_image_file_set(*image, buf, NULL);
   evas_object_image_size_get(*image, &w, &h);
   evas_object_resize(*image, w, h);
   evas_object_layer_set(*image, LAYER_LETTER);
   evas_object_show(*image);

   snprintf(buf, sizeof(buf), PACKAGE_DATA_DIR "/logo_light-%s.png", letter);
   *light = evas_object_image_filled_add(evas);
   evas_object_image_file_set(*light, buf, NULL);
   evas_object_resize(*light, w, h);
   evas_object_layer_set(*light, LAYER_LETTER);
   evas_object_show(*light);

   /* allow easy access to shadow and light from the letter image */
   evas_object_data_set(*image, "shadow", *shadow);
   evas_object_data_set(*image, "light", *light);
}

static void
_letter_body_setup_common(EPhysics_Body *body, Evas_Object *view)
{
   ephysics_body_evas_object_set(body, view, EINA_TRUE);
   ephysics_body_mass_set(body, 1.2);
   ephysics_body_restitution_set(body, 0.6);
   ephysics_body_rotation_on_z_axis_enable_set(body, EINA_FALSE);
}

static EPhysics_Body *
_letter_body_box_add(EPhysics_World *world, Evas_Object *image)
{
   EPhysics_Body *body = ephysics_body_box_add(world);

   _letter_body_setup_common(body, image);

   ephysics_body_event_callback_add
     (body, EPHYSICS_CALLBACK_BODY_UPDATE, _update_box_cb, NULL);

   return body;
}

static EPhysics_Body *
_letter_body_circle_add(EPhysics_World *world, Evas_Object *image)
{
   EPhysics_Body *body = ephysics_body_circle_add(world);

   _letter_body_setup_common(body, image);

   ephysics_body_event_callback_add
     (body, EPHYSICS_CALLBACK_BODY_UPDATE, _update_circle_cb, NULL);

   return body;
}

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *image, *light, *shadow;
   EPhysics_Body *ground_body, *letter_body;
   EPhysics_World *world;
   unsigned int i = 0;
   int x, w, h, sh_w;
   Evas *evas;

   struct letter_desc {
      const char *letter;
      int padding;
   } falling_letters[] = {
     {"P", PADDING_X_1},
     {"H", PADDING_X_1},
     {"Y", PADDING_X_3},
     {"S1", PADDING_X_2},
     {"I", PADDING_X_2},
     {"C", PADDING_X_3},
     {"S2", 0}
   };

   if (!ephysics_init())
     return - 1;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "main", ELM_WIN_SPLASH);
   elm_win_title_set(win, "EPhysics Logo");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_show(win);

   bg = elm_bg_add(win);
   elm_bg_file_set(bg, PACKAGE_DATA_DIR "/logo_background.png", NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_min_set(bg, WIDTH, HEIGHT);
   evas_object_size_hint_max_set(bg, WIDTH, HEIGHT);
   evas_object_show(bg);

   world = ephysics_world_new();
   ephysics_world_render_geometry_set(world, 0, 0, WIDTH, HEIGHT);

   ground_body = ephysics_body_box_add(world);
   ephysics_body_mass_set(ground_body, EPHYSICS_BODY_MASS_IMMUTABLE);
   ephysics_body_geometry_set(ground_body, -100, FLOOR_Y, WIDTH + 800, 10);
   ephysics_body_restitution_set(ground_body, 0.65);
   ephysics_body_friction_set(ground_body, 0.8);

   evas = evas_object_evas_get(win);
   x = OFFSET_X;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(falling_letters); i++)
     {
        _letter_add(evas, falling_letters[i].letter, &image, &light, &shadow);

        evas_object_image_size_get(shadow, &sh_w, NULL);
        evas_object_image_size_get(image, &w, &h);

        /* place image and light on top, above what the viewport can show */
        evas_object_move(image, x, -h * (i + 1) - 50);
        evas_object_move(light, x, -h * (i + 1) - 50);
        /* place shadow below the hit-line: FLOOR_Y, centered at image */
        evas_object_move(shadow, x + CENTER(w, sh_w), FLOOR_Y);

        x += falling_letters[i].padding + w;

        letter_body = _letter_body_box_add(world, image);
        ephysics_body_friction_set(letter_body, 0.4);
     }

   /* E is a circle that comes rolling on the floor */
   _letter_add(evas, "E", &image, &light, &shadow);
   evas_object_color_set(shadow, 255, 255, 255, 255);

   evas_object_image_size_get(shadow, &sh_w, NULL);
   evas_object_image_size_get(image, &w, &h);
   /* place image and light so they are above the floor,
    * and to the left of viewport
    */
   evas_object_move(image, -w - 1, FLOOR_Y - h + 1);
   evas_object_move(light, -w - 1, FLOOR_Y - h + 1);
   /* place the shadow below the hit-line: FLOOR_Y, centered at image */
   evas_object_move(shadow, -w - 1 + CENTER(w, sh_w), FLOOR_Y);

   letter_body = _letter_body_circle_add(world, image);
   ephysics_body_friction_set(letter_body, 1);
   ephysics_body_mass_set(letter_body, 1);
   ephysics_body_rotation_on_z_axis_enable_set(letter_body, EINA_TRUE);

   /* make the "E" logo get into the viewport by applying an horizontal force */
   ephysics_body_central_impulse_apply(letter_body, 13, 0);

   elm_run();

   ephysics_world_del(world);
   ephysics_shutdown();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
