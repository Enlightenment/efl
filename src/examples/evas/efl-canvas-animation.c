/**
 * Example of animation in efl canvas
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH  (640)
#define HEIGHT (480)

struct example_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg, *scale;
   double current_speed;
};

static struct example_data d;

static Evas_Object * /* new rectangle to be put in the box */
_new_rectangle_add(Evas *e)
{
   Efl_Canvas_Rectangle *rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, e);

   efl_gfx_entity_size_set(rect, EINA_SIZE2D(10, 10));
   efl_gfx_color_set(rect, 0, 255, 0, 255);
   efl_gfx_entity_visible_set(rect, EINA_TRUE);

   return rect;
}

/* use the following commands to interact with this example - 'h' is
 * the key for help */
static void
_on_keydown(void *data EINA_UNUSED, const Efl_Event *ev)
{
   if (strcmp(efl_input_key_sym_get(ev->info) , "r") == 0)
     {
        Efl_Canvas_Animation *animation = efl_canvas_object_animation_get(d.scale);
        double current_pos = efl_canvas_object_animation_progress_get(d.scale);
        d.current_speed *= -1;
        efl_canvas_object_animation_start(d.scale, animation, d.current_speed, 1.0 - current_pos);
     }
   if (strcmp(efl_input_key_sym_get(ev->info), "p") == 0)
     {
        efl_canvas_object_animation_pause_set(d.scale, !efl_canvas_object_animation_pause_get(d.scale));
     }
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void /* adjust canvas' contents on resizes */
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   efl_gfx_entity_geometry_set(d.bg, EINA_RECT(0, 0, w, h));
}

static void
print_help(void)
{
   printf("evas-animation example\n Press r to reverse the animation of the red rect.\n Press p to pause the animation of the red rect.\n");
}

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto panic;

   print_help();

   ecore_evas_name_class_set(d.ee, "Evas Animation example", "Evas Animation Example");
   ecore_evas_callback_delete_request_set(d.ee, _on_delete);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   d.evas = ecore_evas_get(d.ee);

   d.bg = _new_rectangle_add(d.evas);
   efl_gfx_color_set(d.bg, 255, 255, 255, 255);
   efl_gfx_entity_visible_set(d.bg, EINA_TRUE);
   efl_canvas_object_key_focus_set(d.bg, EINA_TRUE);
   efl_event_callback_add(d.bg, EFL_EVENT_KEY_DOWN, _on_keydown, NULL);

   _canvas_resize_cb(d.ee);

   Evas_Object *scale_rect = _new_rectangle_add(d.evas);
   efl_gfx_entity_geometry_set(scale_rect, EINA_RECT(50, 50, 50, 50));
   efl_canvas_object_animation_start(scale_rect,
    efl_new(EFL_CANVAS_SCALE_ANIMATION_CLASS,
      efl_animation_scale_set(efl_added, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(3.0, 3.0), scale_rect, EINA_VECTOR2(0.5, 0.5)),
      efl_animation_start_delay_set(efl_added, 5.0),
      efl_animation_duration_set(efl_added, 2.0),
      efl_animation_play_count_set(efl_added, 0)
    ),
   1.0, 0.0);

   Evas_Object *scale_rect2 = _new_rectangle_add(d.evas);
   efl_gfx_entity_geometry_set(scale_rect2, EINA_RECT(50, 200, 50, 50));
   efl_canvas_object_animation_start(scale_rect2,
    efl_new(EFL_CANVAS_SCALE_ANIMATION_CLASS,
      efl_animation_scale_set(efl_added, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(3.0, 3.0), scale_rect2, EINA_VECTOR2(0.5, 0.5)),
      efl_animation_duration_set(efl_added, 2.0),
      efl_animation_play_count_set(efl_added, 0),
      efl_animation_repeat_mode_set(efl_added, EFL_CANVAS_ANIMATION_REPEAT_MODE_REVERSE)
    ),
   1.0, 0.0);

   Evas_Object *scale_rect3 = _new_rectangle_add(d.evas);
   efl_gfx_entity_geometry_set(scale_rect3, EINA_RECT(50, 350, 50, 50));
   efl_canvas_object_animation_start(scale_rect3,
    efl_new(EFL_CANVAS_SCALE_ANIMATION_CLASS,
      efl_animation_scale_set(efl_added, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(3.0, 3.0), scale_rect3, EINA_VECTOR2(0.5, 0.5)),
      efl_animation_duration_set(efl_added, 2.0),
      efl_animation_play_count_set(efl_added, 4),
      efl_animation_repeat_mode_set(efl_added, EFL_CANVAS_ANIMATION_REPEAT_MODE_REVERSE)
    ),
   1.0, 0.0);

   Evas_Object *scale_rect4 = _new_rectangle_add(d.evas);
   efl_gfx_entity_geometry_set(scale_rect4, EINA_RECT(200, 50, 50, 50));
   efl_canvas_object_animation_start(scale_rect4,
    efl_new(EFL_CANVAS_SCALE_ANIMATION_CLASS,
      efl_animation_scale_set(efl_added, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(3.0, 3.0), scale_rect4, EINA_VECTOR2(0.5, 0.5)),
      efl_animation_duration_set(efl_added, 2.0),
      efl_animation_final_state_keep_set(efl_added, EINA_TRUE)
    ),
   1.0, 0.0);


   Evas_Object *scale_rect5 = d.scale = _new_rectangle_add(d.evas);
   efl_gfx_color_set(scale_rect5, 255, 0, 0, 255);
   efl_gfx_entity_geometry_set(scale_rect5, EINA_RECT(200, 200, 50, 50));
   efl_canvas_object_animation_start(scale_rect5,
    efl_new(EFL_CANVAS_SCALE_ANIMATION_CLASS,
      efl_animation_scale_set(efl_added, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(5.0, 5.0), scale_rect5, EINA_VECTOR2(0.5, 0.5)),
      efl_animation_duration_set(efl_added, 5.0),
      efl_animation_play_count_set(efl_added, 0)
    ),
   1.0, 0.0);
   d.current_speed = 1.0;

   ecore_main_loop_begin();
   ecore_evas_shutdown();
   return 0;

panic:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
   return -2;
}

