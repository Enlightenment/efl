/*
#define EFL_EO_API_SUPPORT 1
#define EFL_BETA_API_SUPPORT 1
*/

#include "Eo.h"
#include "Evas.h"
#include "Ecore_Evas.h"

#include <stdio.h>

/* Beginning of the file used for taking a shot of an object.
 * Used for font effect screenshots.
 */

typedef struct
{
   Ecore_Evas *ee;
   const char *file;
   const char *key;
   const char *flags;
} Widget_Preview_Data;

/* This comes from Elementary's elm_win.c */
static void
_shot_do(Widget_Preview_Data *wpd)
{
   Ecore_Evas *ee;
   Evas_Object *o;
   unsigned int *pixels;
   int w, h;

   if (!wpd->file) return;

   ecore_evas_manual_render(wpd->ee);
   pixels = (void *)ecore_evas_buffer_pixels_get(wpd->ee);
   if (!pixels) return;

   ecore_evas_geometry_get(wpd->ee, NULL, NULL, &w, &h);
   if ((w < 1) || (h < 1)) return;

   ee = ecore_evas_buffer_new(1, 1);
   o = evas_object_image_add(ecore_evas_get(ee));
   evas_object_image_alpha_set(o, ecore_evas_alpha_get(wpd->ee));
   evas_object_image_size_set(o, w, h);
   evas_object_image_data_set(o, pixels);
   if (!evas_object_image_save(o, wpd->file, wpd->key, wpd->flags))
     fprintf(stderr, "Cannot save window to '%s' (key '%s', flags '%s')\n",
             wpd->file, wpd->key, wpd->flags);

   ecore_evas_free(ee);
}

static void
_usage(const char *argv0, FILE *f)
{
   fprintf(f, "Usage: %s <text> <filter code> <output file> [font] [fontsize]\n", argv0);
}

int
main(int argc, char **argv)
{
   Widget_Preview_Data wpd = { NULL, NULL, NULL, NULL };
   const char *font = "Sans";
   const char *filter, *text;
   int fontsize = 32;
   Evas_Object *o, *rect;
   Evas *e;
   int w, h;

   if (argc < 4)
     {
        _usage(argv[0], stderr);
        return 1;
     }

   w = 1;
   h = 1;
   text = argv[1];
   filter = argv[2];
   wpd.file = argv[3];
   if (argc >= 5) font = argv[4];
   if (argc >= 6) fontsize = atoi(argv[5]);

   evas_init();
   ecore_evas_init();

   wpd.ee = ecore_evas_buffer_new(w, h);
   if (!wpd.ee) return 2;

   ecore_evas_show(wpd.ee);
   ecore_evas_manual_render_set(wpd.ee, EINA_TRUE);
   ecore_evas_transparent_set(wpd.ee, EINA_TRUE);
   ecore_evas_alpha_set(wpd.ee, EINA_TRUE);
   e = ecore_evas_get(wpd.ee);

   rect = evas_object_rectangle_add(e);
   evas_object_move(rect, 0, 0);
   evas_object_resize(rect, w, h);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   o = evas_object_text_add(e);
   evas_object_stack_above(o, rect);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, w, h);
   evas_object_text_font_set(o, font, fontsize);
   evas_object_text_text_set(o, text);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_show(o);

   eo_do(o, evas_obj_text_filter_program_set(filter));

   ecore_evas_manual_render(wpd.ee);
   evas_object_geometry_get(o, NULL, NULL, &w, &h);
   ecore_evas_resize(wpd.ee, w, h);

   _shot_do(&wpd);

   ecore_evas_shutdown();
   evas_shutdown();

   return 0;
}

