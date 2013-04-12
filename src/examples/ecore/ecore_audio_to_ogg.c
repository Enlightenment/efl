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

static Eina_Bool _play_finished(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
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
main(int argc, const char *argv[])
{
   int ret;
   Eo *out;
   Eo *in;

   if (argc < 2)
     {
       printf("Please provide a filename\n");
       exit(EXIT_FAILURE);
     }

   ecore_init();
   ecore_audio_init();


   in = eo_add(ECORE_AUDIO_OBJ_IN_SNDFILE_CLASS, NULL);
   eo_do(in, ecore_audio_obj_name_set(basename(argv[1])));
   eo_do(in, ecore_audio_obj_source_set(argv[1]));

   eo_do(in, eo_event_callback_add(ECORE_AUDIO_EV_IN_STOPPED, _play_finished, NULL));

   out = eo_add(ECORE_AUDIO_OBJ_OUT_SNDFILE_CLASS, NULL);
   eo_do(out, ecore_audio_obj_source_set("foo.ogg"));

   eo_do(out, ecore_audio_obj_out_input_attach(in));

   ecore_main_loop_begin();

end:
   ecore_audio_shutdown();
   ecore_shutdown();

   return 0;
}

