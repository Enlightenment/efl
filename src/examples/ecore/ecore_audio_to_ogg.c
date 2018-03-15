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

static void _play_finished(void *data EINA_UNUSED, const Efl_Event *event)
{
  const char *name;
  Eo *out;

  ecore_audio_obj_name_get(event->object, &name);
  printf("Done: %s\n", name);

  ecore_audio_obj_in_output_get(event->object, &out);
  efl_unref(event->object);
  efl_unref(out);

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


   in = efl_add_ref(ECORE_AUDIO_OBJ_IN_SNDFILE_CLASS, NULL);
   efl_name_set(in, basename(argv[1]));
   ret = ecore_audio_obj_source_set(in, argv[1]);
   if (!ret) {
     printf("Could not set %s as input\n", argv[1]);
     efl_unref(in);
     return 1;
   }

   efl_event_callback_add(in, ECORE_AUDIO_EV_IN_STOPPED, _play_finished, NULL);

   out = efl_add_ref(ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS, NULL);
   ret = ecore_audio_obj_source_set(out, argv[2]);
   if (!ret) {
     printf("Could not set %s as output\n", argv[2]);
     efl_unref(in);
     efl_unref(out);
     return 1;
   }

   ret = ecore_audio_obj_out_input_attach(out, in);
   if (!ret) {
     printf("Could not attach input\n");
     efl_unref(out);
     efl_unref(in);
     return 1;
   }

   ecore_main_loop_begin();

   ecore_audio_shutdown();
   ecore_shutdown();

   return 0;
}

