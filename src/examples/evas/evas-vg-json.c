/**
 * Example of lottie animation using Evas_VG.
 *
 * This example show how to play lottie files(json).
 *
 * @verbatim
 * gcc -o evas_vg_json evas-vg-json.c `pkg-config --libs --cflags evas ecore ecore-evas eina ector eo efl`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef PACKAGE_EXAMPLES_DIR
#define PACKAGE_EXAMPLES_DIR "."
#endif

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif


#include <Ecore.h>
#include <Ecore_Evas.h>
#include "evas-common.h"

#define WIDTH  400
#define HEIGHT 600

static Eo *gvg[5];

static void
running_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Canvas_Animation_Player_Event_Running *event_running = event->info;
   double progress = event_running->progress;

   int i;
   for (i = 0; i < 5; i++)
     {
        double frameCnt = (double) (efl_gfx_frame_controller_frame_count_get(gvg[i]) - 1);
        int frame = (int) (frameCnt * progress);
        efl_gfx_frame_controller_frame_set(gvg[i], frame);
     }
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto panic;

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_show(ee);

   Eo *evas = ecore_evas_get(ee);

   Eo *bg = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas);
   efl_gfx_entity_size_set(bg, EINA_SIZE2D(WIDTH, HEIGHT));
   efl_gfx_color_set(bg, 255, 255, 255, 255);
   efl_gfx_entity_visible_set(bg, EINA_TRUE);

   char buf[PATH_MAX];

   //1
   Eo *vg = gvg[0] = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, evas);

   snprintf(buf, sizeof(buf), "%s/browser.json", PACKAGE_EXAMPLES_DIR EVAS_VG_FOLDER);
   efl_file_simple_load(vg, buf, NULL);
   efl_gfx_entity_size_set(vg, EINA_SIZE2D(200, 200));
   efl_gfx_entity_visible_set(vg, EINA_TRUE);

   //2
   Eo *vg2 = gvg[1] = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, evas);
   snprintf(buf, sizeof(buf), "%s/jolly_walker.json", PACKAGE_EXAMPLES_DIR EVAS_VG_FOLDER);
   efl_file_simple_load(vg2, buf, NULL);
   efl_gfx_entity_position_set(vg2, EINA_POSITION2D(200, 200));
   efl_gfx_entity_size_set(vg2, EINA_SIZE2D(200, 200));
   efl_gfx_entity_visible_set(vg2, EINA_TRUE);

   //3
   Eo *vg3 = gvg[2] = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, evas);
   snprintf(buf, sizeof(buf), "%s/windmill.json", PACKAGE_EXAMPLES_DIR EVAS_VG_FOLDER);
   efl_file_simple_load(vg3, buf, NULL);
   efl_gfx_entity_position_set(vg3, EINA_POSITION2D(0, 200));
   efl_gfx_entity_size_set(vg3, EINA_SIZE2D(200, 200));
   efl_gfx_entity_visible_set(vg3, EINA_TRUE);

   //4
   Eo* vg4 = gvg[3] = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, evas);
   snprintf(buf, sizeof(buf), "%s/emoji_wink.json", PACKAGE_EXAMPLES_DIR EVAS_VG_FOLDER);
   efl_file_simple_load(vg4, buf, NULL);
   efl_gfx_entity_position_set(vg4, EINA_POSITION2D(200, 0));
   efl_gfx_entity_size_set(vg4, EINA_SIZE2D(200, 200));
   efl_gfx_entity_visible_set(vg4, EINA_TRUE);

   //5
   Eo* vg5 = gvg[4] = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, evas);
   snprintf(buf, sizeof(buf), "%s/image_embedded.json", PACKAGE_EXAMPLES_DIR EVAS_VG_FOLDER);
   efl_file_simple_load(vg5, buf, NULL);
   efl_gfx_entity_position_set(vg5, EINA_POSITION2D(0, 400));
   efl_gfx_entity_size_set(vg5, EINA_SIZE2D(200, 200));
   efl_gfx_entity_visible_set(vg5, EINA_TRUE);

   //Play custom animation
   Eo *anim = efl_add(EFL_CANVAS_ANIMATION_CLASS, evas);
   efl_animation_duration_set(anim, efl_gfx_frame_controller_frame_duration_get(vg, 0, 0));

   Eo *player = efl_add(EFL_CANVAS_ANIMATION_PLAYER_CLASS, evas);
   efl_animation_player_animation_set(player, anim);
   efl_event_callback_add(player, EFL_ANIMATION_PLAYER_EVENT_RUNNING, running_cb, NULL);
   efl_player_playing_set(player, EINA_TRUE);

   ecore_main_loop_begin();
   ecore_evas_shutdown();

   return 0;

panic:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
   return -2;
}
