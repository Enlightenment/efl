// Compile with:
// gcc -o ecore_audio_to_ogg ecore_audio_to_ogg.c `pkg-config --libs --cflags ecore eina ecore-audio`

#include <stdio.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <Eo.h>
#include <Ecore.h>
#include <Ecore_Audio.h>
#include <Eina.h>

double volume = 1;
Eina_List *inputs = NULL;

static Eina_Bool _play_finished(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
  const char *name;
  Eo *out;

  eo_do(obj, ecore_audio_obj_name_get(&name));
  printf("Done: %s\n", name);

  eo_do(obj, ecore_audio_obj_in_output_get(&out));
  eo_del(obj);
  eo_del(out);

  ecore_main_loop_quit();

  return EINA_TRUE;
}

int
main(int argc, char *argv[])
{
   Eo *out;
   Eo *in;
   Eina_Bool ret;

   if (argc < 3)
     {
       printf("Usage: ./example <infile> <outfile>\n");
       exit(EXIT_FAILURE);
     }

   ecore_init();
   ecore_audio_init();


   in = eo_add(ECORE_AUDIO_OBJ_IN_SNDFILE_CLASS, NULL);
   eo_do(in, ecore_audio_obj_name_set(basename(argv[1])));
   eo_do(in, ret = ecore_audio_obj_source_set(argv[1]));
   if (!ret) {
     printf("Could not set %s as input\n", argv[1]);
     eo_del(in);
     return 1;
   }

   eo_do(in, eo_event_callback_add(ECORE_AUDIO_EV_IN_STOPPED, _play_finished, NULL));

   out = eo_add(ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS, NULL);
   eo_do(out, ret = ecore_audio_obj_source_set(argv[2]));
   if (!ret) {
     printf("Could not set %s as output\n", argv[2]);
     eo_del(in);
     eo_del(out);
     return 1;
   }

   eo_do(out, ret = ecore_audio_obj_out_input_attach(in));
   if (!ret) {
     printf("Could not attach input\n");
     eo_del(out);
     eo_del(in);
     return 1;
   }

   ecore_main_loop_begin();

   ecore_audio_shutdown();
   ecore_shutdown();

   return 0;
}

