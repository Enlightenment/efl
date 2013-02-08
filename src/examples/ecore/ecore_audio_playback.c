// Compile with:
// gcc -o ecore_audio_playback ecore_audio_playback.c `pkg-config --libs --cflags ecore eina ecore-audio`

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <Ecore.h>
#include <Ecore_Audio.h>
#include <Eina.h>

Ecore_Audio_Object *out = NULL;
double volume = 1;
Eina_List *inputs = NULL;

void
handle_cmd(char *cmd, size_t bread)
{
   Eina_List *out_inputs, *input;
   Ecore_Audio_Object *in;
   Eina_Bool paused;
   double pos;
   int min;

   if (!out)
     return;

   out_inputs = ecore_audio_output_inputs_get(out);
   EINA_LIST_FOREACH(out_inputs, input, in)
     {
        pos = ecore_audio_input_seek(in, 0, SEEK_CUR);
        if (!strncmp(cmd, "<", bread))
          pos = ecore_audio_input_seek(in, -10, SEEK_CUR);
        else if (!strncmp(cmd, ">", bread))
          pos = ecore_audio_input_seek(in, 10, SEEK_CUR);

        min = pos / 60;
        printf("Position: %2im %5.02fs (%0.2f%%) - %s\n", min, pos - min * 60, pos/ecore_audio_input_length_get(in)*100, ecore_audio_input_name_get(in));

     }

   if (!strncmp(cmd, "p", bread))
     {
     }
   else if (!strncmp(cmd, "n", bread))
     {
        in = eina_list_data_get(out_inputs);
        ecore_audio_output_input_del(out, in);
        inputs = eina_list_remove(inputs, in);

        if (eina_list_count(inputs) > 0)
          {
             in = (Ecore_Audio_Object *)eina_list_data_get(inputs);

             printf("Start: %s (%0.2fs)\n", ecore_audio_input_name_get(in), ecore_audio_input_length_get(in));
             ecore_audio_output_input_add(out, in);
          }
       else
         {
           printf("Done\n");
           ecore_main_loop_quit();
         }
     }
   else if (!strncmp(cmd, "m", bread))
     {
        inputs = eina_list_remove(inputs, eina_list_data_get(inputs));
        if (eina_list_count(inputs) > 0)
          {
             in = (Ecore_Audio_Object *)eina_list_data_get(inputs);

             printf("Start: %s (%0.2fs)\n", ecore_audio_input_name_get(in), ecore_audio_input_length_get(in));
             ecore_audio_output_input_add(out, in);
          }
     }
   else if (!strncmp(cmd, "l", bread))
     {
        EINA_LIST_FOREACH(out_inputs, input, in)
          {
             Eina_Bool loop = !ecore_audio_input_looped_get(in);
             printf("%s song %s\n", loop?"Looping":"Not looping", ecore_audio_input_name_get(in));
             ecore_audio_input_looped_set(in, loop);
          }
     }
   else if (!strncmp(cmd, "+", bread))
     {
        if (volume <= 1.5)
          volume += 0.01;
        ecore_audio_output_volume_set(out, volume);
        printf("Volume: %3.0f%%\n", volume * 100);
     }
   else if (!strncmp(cmd, "-", bread))
     {
        if (volume >= 0)
          volume -= 0.01;
        ecore_audio_output_volume_set(out, volume);
        printf("Volume: %3.0f%%\n", volume * 100);
     }
   else if (!strncmp(cmd, " ", bread))
     {
        EINA_LIST_FOREACH(out_inputs, input, in)
          {
             paused = ecore_audio_input_paused_get(in);
             ecore_audio_input_paused_set(in, !paused);
             printf("%s %s\n%0.2f remaining\n", !paused ? "Paused" : "Unpaused", ecore_audio_input_name_get(in), ecore_audio_input_remaining_get(in));
          }
     }
   else if (!strncmp(cmd, "q", bread))
     {
        ecore_main_loop_quit();
     }
   else
     printf("Unhandled cmd '%s'\n", cmd);

}

Eina_Bool
handle_input(void *data, Ecore_Fd_Handler *handler)
{
   size_t bread;
   char buf[20];
   int fd, i;

   if (!ecore_main_fd_handler_active_get(handler, ECORE_FD_READ))
     return EINA_TRUE;

   fd = ecore_main_fd_handler_fd_get(handler);

   bread = read(fd, buf, sizeof(buf));
   if (bread == 0)
     {
        printf("Ctrl-D?\n");
        ecore_main_loop_quit();

        return EINA_FALSE;
     }

   handle_cmd(buf, bread);

   return EINA_TRUE;
}

static Eina_Bool _play_started(void *data, int type, void *event)
{
  Ecore_Audio_Object *in = (Ecore_Audio_Object *)event;
  printf("Start: %s\n", ecore_audio_input_name_get(in));
  return EINA_TRUE;
}

static Eina_Bool _play_finished(void *data, int type, void *event)
{
  Ecore_Audio_Object *in = (Ecore_Audio_Object *)event;

  printf("Done: %s\n", ecore_audio_input_name_get(in));

  inputs = eina_list_remove(inputs, in);
  ecore_audio_output_input_del(out, in);
  ecore_audio_input_del(in);


  if (eina_list_count(inputs) > 0)
    {
      in = (Ecore_Audio_Object *)eina_list_data_get(inputs);

      printf("Start: %s\n", ecore_audio_input_name_get(in));
      ecore_audio_output_input_add(out, in);
    }
  else
    {
      printf("Stream done\n");
      ecore_main_loop_quit();
    }

  return EINA_TRUE;
}

Eina_Bool
output_add(void *data)
{
   Ecore_Audio_Object *in = (Ecore_Audio_Object *)eina_list_data_get(inputs);

   printf("Start: %s (%0.2fs)\n", ecore_audio_input_name_get(in), ecore_audio_input_length_get(in));
   out = ecore_audio_output_add(ECORE_AUDIO_TYPE_PULSE);
   ecore_audio_output_input_add(out, in);
   return EINA_FALSE;
}

int
main(int argc, const char *argv[])
{
   int i;

   struct termios tcorig, tcnew;
   Ecore_Audio_Object *in;
   char *tmp, *tmp2, *val;

   if (argc < 2)
     {
       printf("Please provide a filename\n");
       exit(EXIT_FAILURE);
     }

   ecore_init();
   ecore_audio_init();

   ecore_event_handler_add(ECORE_AUDIO_INPUT_STARTED, _play_started, NULL);
   ecore_event_handler_add(ECORE_AUDIO_INPUT_LOOPED, _play_started, NULL);
   ecore_event_handler_add(ECORE_AUDIO_INPUT_ENDED, _play_finished, NULL);

   for (i=1;i<argc;i++)
     {
       if (!strncmp(argv[i], "tone:", 5))
         {
            in = ecore_audio_input_add(ECORE_AUDIO_TYPE_TONE);
            if (!in)
              {
                 printf("error when creating ecore audio source.\n");
                 goto end;
              }
            /* The name has the format "tone:freq=x,duration=y */
            tmp = strdup(argv[i]);
            val = strtok_r(tmp, ":", &tmp2);

            while ((val = strtok_r(NULL, ",", &tmp2)) != NULL)
              {
                 if (!strncmp(val, "freq=", 5))
                   ecore_audio_input_tone_frequency_set(in, atoi(&val[5]));
                 else if (!strncmp(val, "duration=", 9))
                   ecore_audio_input_tone_duration_set(in, atof(&val[9]));
              }
            free(tmp);
            ecore_audio_input_name_set(in, argv[i]);
         }
       else
         {
            in = ecore_audio_input_add(ECORE_AUDIO_TYPE_SNDFILE);
            if (!in)
              {
                 printf("error when creating ecore audio source.\n");
                 goto end;
              }
            ecore_audio_input_name_set(in, basename(argv[i]));
            ecore_audio_input_sndfile_filename_set(in, argv[i]);
         }
       inputs = eina_list_append(inputs, in);
     }


   ecore_timer_add(1, output_add, NULL);

   /* Disable canonical mode for stdin */
   if (tcgetattr(0, &tcorig) == -1)
     printf("Problem getting termcaps: %s\n", strerror(errno));

   tcnew = tcorig;
   tcnew.c_lflag &= ~(ICANON | ECHO);
   if (tcsetattr(0, TCSANOW, &tcnew) == -1)
     printf("Problem setting termcaps: %s\n", strerror(errno));

   ecore_main_fd_handler_add(0, ECORE_FD_READ, handle_input, NULL, NULL, NULL);

   ecore_main_loop_begin();

end:
   ecore_audio_shutdown();
   ecore_shutdown();

   /* Reset stdin */
   if (tcsetattr(0, TCSANOW, &tcorig) == -1)
     printf("Problem restoring termcaps: %s\n", strerror(errno));

   return 0;
}

