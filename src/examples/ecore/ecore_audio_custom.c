#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <Ecore.h>
#include <Ecore_Audio.h>
#include <math.h>

Ecore_Audio_Object *in = NULL;
Ecore_Audio_Object *out = NULL;
unsigned char volume = 255;


int read_cb(void *user_data, void *data, int len)
{
  static long int phase1 = 0, phase2 = 0;
  static int modulation = 0;

  float *val = data;
  int i;

  for(i=0; i<len/4; i++, phase1++)
    {
       if (phase1 >= 44100/(440+modulation))
         {
            phase1 = 0;
            modulation = sin(2 * M_PI * phase2 / 1000) * 100;
            phase2++;
         }

       val[i++] = sin(2 * M_PI * (440+modulation) * (phase1) / 44100.0);
       val[i] = sin(2 * M_PI * (440+modulation) * (phase1) / 44100.0);
    }

  return len;
}

Eina_Bool
output_add(void *data)
{
   out = ecore_audio_output_add(ECORE_AUDIO_TYPE_PULSE);
   ecore_audio_output_input_add(out, in);
//   ecore_event_handler_add(ECORE_AUDIO_EVENT_SOURCE_DONE, _play_finished, NULL);

   return EINA_FALSE;
}

int
main(int argc, const char *argv[])
{
   ecore_init();
   ecore_audio_init();

   in = ecore_audio_input_add(ECORE_AUDIO_TYPE_CUSTOM);
   if (!in)
     {
        printf("error when creating ecore audio source.\n");
        goto end;
     }
   ecore_audio_input_name_set(in, "custom");
   ecore_audio_input_callback_setup(in, read_cb, NULL);

   ecore_timer_add(1, output_add, NULL);

   ecore_main_loop_begin();

end:
   ecore_audio_shutdown();
   ecore_shutdown();

   return 0;
}

