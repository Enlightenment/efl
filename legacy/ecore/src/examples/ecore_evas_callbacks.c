/**
 * Ecore example illustrating ecore evas callbacks.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o ecore_evas_callbacks ecore_evas_callbacks.c `pkg-config --libs --cflags ecore-evas`
 * @endverbatim
 */

#include <Ecore.h>
#include <Ecore_Evas.h>

static void
_destroy(Ecore_Evas *ee)
{
   printf("destroy\n");
   ecore_main_loop_quit();
}

static void
_delete(Ecore_Evas *ee)
{
   printf("delete\n");
   ecore_main_loop_quit();
}

static void
_focus_in(Ecore_Evas *ee)
{
   printf("focus_in\n");
}

static void
_focus_out(Ecore_Evas *ee)
{
   printf("focus_out\n");
}

static void
_hide(Ecore_Evas *ee)
{
   printf("hide\n");
}

static void
_mouse_in(Ecore_Evas *ee)
{
   printf("mouse_in\n");
}

static void
_show(Ecore_Evas *ee)
{
   printf("show\n");
}

static void
_mouse_out(Ecore_Evas *ee)
{
   printf("mouse_out\n");
}

static void
_move(Ecore_Evas *ee)
{
   printf("move\n");
}

static void
_post_render(Ecore_Evas *ee)
{
   printf("post_render\n");
}

static void
_pre_free(Ecore_Evas *ee)
{
   printf("pre_free\n");
}

static void
_pre_render(Ecore_Evas *ee)
{
   printf("pre_render\n");
}

static void
_resize(Ecore_Evas *ee)
{
   printf("resize\n");
}

int
main(void)
{
   Ecore_Evas *ee;

   ecore_evas_init();

   ee = ecore_evas_new(NULL, 0, 0, 200, 100, NULL);
   ecore_evas_title_set(ee, "Ecore Evas Callbacks Example");
   ecore_evas_show(ee);

   //callbacks
   ecore_evas_callback_delete_request_set(ee, _delete);
   ecore_evas_callback_destroy_set(ee, _destroy);
   ecore_evas_callback_focus_in_set(ee, _focus_in);
   ecore_evas_callback_focus_out_set(ee, _focus_out);
   ecore_evas_callback_hide_set(ee, _hide);
   ecore_evas_callback_mouse_in_set(ee, _mouse_in);
   ecore_evas_callback_mouse_out_set(ee, _mouse_out);
   ecore_evas_callback_move_set(ee, _move);
   ecore_evas_callback_post_render_set(ee, _post_render);
   ecore_evas_callback_pre_free_set(ee, _pre_free);
   ecore_evas_callback_pre_render_set(ee, _pre_render);
   ecore_evas_callback_resize_set(ee, _resize);
   ecore_evas_callback_show_set (ee, _show);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;
}
