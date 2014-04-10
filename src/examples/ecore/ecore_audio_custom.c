// Compile with:
// gcc -o ecore_audio_custom ecore_audio_custom.c `pkg-config --libs --cflags ecore ecore-audio`

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <Ecore.h>
#include <Ecore_Audio.h>
#include <math.h>

Eo *in = NULL;
Eo *out = NULL;
unsigned char volume = 255;


int read_cb(void *data EINA_UNUSED, Eo *eo_obj, void *buf, int len)
{
  static long int phase1 = 0, phase2 = 0;
  static int modulation = 0;

  double volume;
  float *val = buf;
  int i;

  eo_do(eo_obj, ecore_audio_obj_volume_get(&volume));

  for(i=0; i<len/4; i++, phase1++)
    {
       if (phase1 >= 44100/(440+modulation))
         {
            phase1 = 0;
            modulation = sin(2 * M_PI * phase2 / 1000) * 100;
            phase2++;
         }

       val[i] = sin(2 * M_PI * (440+modulation) * (phase1) / 44100.0) * volume;
    }

  return len;
}

Ecore_Audio_Vio vio = {
    .read = read_cb,
};

int
main(int argc, const char *argv[])
{
   Eina_Bool ret;

   ecore_init();
   ecore_audio_init();

   ecore_app_args_set(argc, argv);

   out = eo_add(ECORE_AUDIO_OBJ_OUT_PULSE_CLASS, NULL);

   in = eo_add(ECORE_AUDIO_OBJ_IN_CLASS, NULL);
   if (!in)
     {
        printf("error when creating ecore audio source.\n");
        goto end;
     }
   eo_do(in, ecore_audio_obj_name_set("wail"));
   eo_do(in, ecore_audio_obj_in_samplerate_set(44100));
   eo_do(in, ecore_audio_obj_in_channels_set(1));
   eo_do(in, ecore_audio_obj_volume_set(0.7));
   eo_do(in, ecore_audio_obj_vio_set(&vio, NULL, NULL));

   eo_do(out, ret = ecore_audio_obj_out_input_attach(in));
   if (!ret) {
     printf("Could not attach input");
     goto end;
   }

   ecore_main_loop_begin();

end:
   ecore_audio_shutdown();
   ecore_shutdown();

   return 0;
}

