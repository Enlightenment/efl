#include "evas_test_main.h"

#include "Evas.h"
#include "Evas_Engine_FB.h"

#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
   int                 rot = 0;

   if ((argc >= 3) && (!strcmp(argv[1], "-rot")))
      rot = atoi(argv[2]);
   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("fb"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
   {
      Evas_Engine_Info_FB *einfo;

      einfo = (Evas_Engine_Info_FB *) evas_engine_info_get(evas);

      /* the following is specific to the engine */
      einfo->info.virtual_terminal = 0;
      einfo->info.device_number = 0;
      einfo->info.refresh = 0;
      einfo->info.rotation = rot;

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
