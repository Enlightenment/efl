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

Eo *out = NULL;
double volume = 1;
Eina_List *inputs = NULL;

void
handle_cmd(char *cmd, size_t bread)
{
   const char *name;
   Eina_List *out_inputs, *input;
   Eo *in;
   Eina_Bool paused, ret;
   double pos, length;
   int min;

   if (!out)
     return;

   ecore_audio_obj_out_inputs_get(out, &out_inputs);
   EINA_LIST_FOREACH(out_inputs, input, in)
     {
        ecore_audio_obj_in_seek(in, 0, SEEK_CUR, &pos);
        if (!strncmp(cmd, "<", bread))
          ecore_audio_obj_in_seek(in, -10, SEEK_CUR, &pos);
        else if (!strncmp(cmd, ">", bread))
          ecore_audio_obj_in_seek(in, 10, SEEK_CUR, &pos);

        min = pos / 60;

        ecore_audio_obj_name_get(in, &name);
        ecore_audio_obj_in_length_get(in, &length);
        printf("Position: %2im %5.02fs (%0.2f%%) - %s\n", min, pos - min * 60, pos/length*100, name);

     }

   if (!strncmp(cmd, "p", bread))
     {
     }
   else if (!strncmp(cmd, "n", bread))
     {
        in = eina_list_data_get(out_inputs);
        ret = ecore_audio_obj_out_input_detach(out, in);
        if (!ret)
          printf("Could not detach input\n");

        inputs = eina_list_remove(inputs, in);

        if (eina_list_count(inputs) > 0)
          {
             in = (Eo *)eina_list_data_get(inputs);

             ecore_audio_obj_name_get(in, &name);
             ecore_audio_obj_in_length_get(in, &length);

             printf("Start: %s (%0.2fs)\n", name, length);
             ret = ecore_audio_obj_out_input_attach(out, in);
             if (!ret)
               printf("Could not attach input %s\n", name);
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
             in = (Eo *)eina_list_data_get(inputs);

             ecore_audio_obj_name_get(in, &name);
             ecore_audio_obj_in_length_get(in, &length);

             printf("Start: %s (%0.2fs)\n", name, length);
             ret = ecore_audio_obj_out_input_attach(out, in);
             if (!ret)
               printf("Could not attach input %s\n", name);
          }
     }
   else if (!strncmp(cmd, "l", bread))
     {
        EINA_LIST_FOREACH(out_inputs, input, in)
          {
             Eina_Bool loop;
             ecore_audio_obj_in_looped_get(in, &loop);
             ecore_audio_obj_name_get(in, &name);

             printf("%s song %s\n", !loop?"Looping":"Not looping", name);
             ecore_audio_obj_in_looped_set(in, !loop);
          }
     }
   else if (!strncmp(cmd, "+", bread))
     {
        if (volume < 1.5)
          volume += 0.01;
        ecore_audio_obj_volume_set(out, volume);
        printf("Volume: %3.0f%%\n", volume * 100);
     }
   else if (!strncmp(cmd, "-", bread))
     {
        if (volume > 0)
          volume -= 0.01;
        ecore_audio_obj_volume_set(out, volume);
        printf("Volume: %3.0f%%\n", volume * 100);
     }
   else if (!strncmp(cmd, "*", bread))
     {
        double speed;
        EINA_LIST_FOREACH(out_inputs, input, in)
          {
             ecore_audio_obj_in_speed_get(in, &speed);
             if (speed < 2.0)
               speed += 0.01;
             ecore_audio_obj_in_speed_set(in, speed);
             ecore_audio_obj_name_get(in, &name);
             printf("Speed: %3.0f%% (%s)\n", speed * 100, name);
          }
     }
   else if (!strncmp(cmd, "/", bread))
     {
        double speed;
        EINA_LIST_FOREACH(out_inputs, input, in)
          {
             ecore_audio_obj_in_speed_get(in, &speed);
             if (speed > 0.5)
               speed -= 0.01;
             ecore_audio_obj_in_speed_set(in, speed);
             ecore_audio_obj_name_get(in, &name);
             printf("Speed: %3.0f%% (%s)\n", speed * 100, name);
          }
     }
   else if (!strncmp(cmd, " ", bread))
     {
        EINA_LIST_FOREACH(out_inputs, input, in)
          {
             ecore_audio_obj_paused_get(in, &paused);
             ecore_audio_obj_name_get(in, &name);
             ecore_audio_obj_in_remaining_get(in, &length);
             printf("%s %s\n%0.2f remaining\n", !paused ? "Paused" : "Unpaused", name, length);
             ecore_audio_obj_paused_set(in, !paused);
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
handle_input(void *data EINA_UNUSED, Ecore_Fd_Handler *handler)
{
   size_t bread;
   char buf[20];
   int fd;

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

static void _play_finished(void *data EINA_UNUSED, const Efl_Event *event)
{
  const char *name;
  Eina_Bool ret;

  ecore_audio_obj_name_get(event->object, &name);
  printf("Done: %s\n", name);

  inputs = eina_list_remove(inputs, event->object);
  ret = ecore_audio_obj_out_input_detach(out, event->object);
  efl_unref(event->object);

  if (!ret)
    printf("Could not detach input %s\n", name);


  if (eina_list_count(inputs) > 0)
    {
      const char *name;
      Eo *in = (Eo *)eina_list_data_get(inputs);

      ecore_audio_obj_name_get(in, &name);
      printf("Start: %s\n", name);
      ret = ecore_audio_obj_out_input_attach(out, in);
      if (!ret)
        printf("Could not attach input %s\n", name);
    }
  else
    {
      printf("Stream done\n");
      ecore_main_loop_quit();
    }

  return EINA_TRUE;
}

int
main(int argc, const char *argv[])
{
   int i, freq;

   Eina_Bool ret;
   struct termios tcorig, tcnew;
   Eo *in;
   char *tmp, *tmp2, *val;

   if (argc < 2)
     {
       printf("Please provide a filename\n");
       exit(EXIT_FAILURE);
     }

   ecore_init();
   ecore_audio_init();

   ecore_app_args_set(argc, argv);

   for (i=1;i<argc;i++)
     {
       if (!strncmp(argv[i], "tone:", 5))
         {
            in = efl_add_ref(ECORE_AUDIO_OBJ_IN_TONE_CLASS, NULL);
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
                 if (!strncmp(val, "freq=", 5)) {
                   freq = atoi(&val[5]);
                   efl_key_data_set(in, ECORE_AUDIO_ATTR_TONE_FREQ, &freq);
                 } else if (!strncmp(val, "duration=", 9)) {
                   ecore_audio_obj_in_length_set(in, atof(&val[9]));
                 }
              }
            free(tmp);
            efl_name_set(in, argv[i]);
         }
       else
         {
            in = efl_add_ref(ECORE_AUDIO_OBJ_IN_SNDFILE_CLASS, NULL);
            if (!in)
              {
                 printf("error when creating ecore audio source.\n");
                 goto end;
              }
            tmp = strdup(argv[i]);
            efl_name_set(in, basename(tmp));
            free(tmp);
            ret = ecore_audio_obj_source_set(in, argv[i]);
            if (!ret) {
              printf("Could not set %s as input\n", argv[i]);
              continue;
            }
         }
       efl_event_callback_add(in, ECORE_AUDIO_EV_IN_STOPPED, _play_finished, NULL);
       inputs = eina_list_append(inputs, in);
     }

   const char *name;
   double length;
   in = (Eo *)eina_list_data_get(inputs);

   if (!in)
     return 1;

   ecore_audio_obj_name_get(in, &name);
   ecore_audio_obj_in_length_get(in, &length);

   printf("Start: %s (%0.2fs)\n", name, length);

   out = efl_add_ref(ECORE_AUDIO_OBJ_OUT_PULSE_CLASS, NULL);
   ret = ecore_audio_obj_out_input_attach(out, in);
   if (!ret)
     printf("Could not attach input %s\n", name);


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

