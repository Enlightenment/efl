// Compile with:
// gcc -o ecore_audio_to_ogg ecore_audio_to_ogg.c `pkg-config --libs --cflags ecore eina ecore-audio`

#include <stdio.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <Ecore.h>
#include <Ecore_Audio.h>
#include <Eina.h>

Ecore_Audio_Object *out = NULL;
double volume = 1;
Eina_List *inputs = NULL;

static Eina_Bool _play_finished(void *data, int type, void *event)
{
  Ecore_Audio_Object *in = (Ecore_Audio_Object *)event;

  printf("Done: %s\n", ecore_audio_input_name_get(in));

  ecore_audio_output_input_del(out, in);
  ecore_audio_input_del(in);
  ecore_audio_output_del(out);

  ecore_main_loop_quit();

  return EINA_TRUE;
}

int
main(int argc, const char *argv[])
{

   Ecore_Audio_Object *in;

   if (argc < 2)
     {
       printf("Please provide a filename\n");
       exit(EXIT_FAILURE);
     }

   ecore_init();
   ecore_audio_init();

   ecore_event_handler_add(ECORE_AUDIO_INPUT_ENDED, _play_finished, NULL);

   in = ecore_audio_input_add(ECORE_AUDIO_TYPE_SNDFILE);
   ecore_audio_input_name_set(in, basename(argv[1]));
   ecore_audio_input_sndfile_filename_set(in, argv[1]);
   if (!in)
     {
        printf("error when creating ecore audio source.\n");
        goto end;
     }

   out = ecore_audio_output_add(ECORE_AUDIO_TYPE_SNDFILE);
   ecore_audio_output_sndfile_filename_set(out, "foo.ogg");

   ecore_audio_output_input_add(out, in);

   ecore_main_loop_begin();

end:
   ecore_audio_shutdown();
   ecore_shutdown();

   return 0;
}

