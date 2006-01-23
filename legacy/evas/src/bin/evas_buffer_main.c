#include "evas_test_main.h"

#include "Evas.h"
#include "Evas_Engine_Buffer.h"

#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
   int                 rot = 0;
   unsigned char      *img_buf = NULL;

   if ((argc >= 3) && (!strcmp(argv[1], "-rot")))
      rot = atoi(argv[2]);
   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("buffer"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_Buffer *einfo;

      einfo = (Evas_Engine_Info_Buffer *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
      img_buf = malloc(win_w * win_h * 3);
      einfo->info.dest_buffer = img_buf;
      einfo->info.dest_buffer_row_bytes = win_w * 3;
      einfo->info.use_color_key = 1;
      einfo->info.alpha_threshold = 128;
      einfo->info.color_key_r = 0xff;
      einfo->info.color_key_g = 0x00;
      einfo->info.color_key_b = 0xff;
      evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
   }
   setup();
   orig_start_time = start_time = get_time();
   for (;;)
     {
	loop();
	evas_render(evas);
     }
   evas_shutdown();
   return 0;
}
