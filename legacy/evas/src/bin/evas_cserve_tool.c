#include "evas_cs.h"

int
main(int argc, char **argv)
{
   int i;
   
   evas_init();
   if (!evas_cserve_init())
     {
        printf("ERROR: Cannot connect to cserve. abort\n");
        exit(-1);
     }

   for (i = 1; i < argc; i++)
     {
        if ((!strcmp(argv[i], "-h")) ||
            (!strcmp(argv[i], "-help")) ||
            (!strcmp(argv[i], "--help")))
          {
             printf("Options:\n"
                    "\t-h                                  This help\n"
                    "\tgetconfig                           Get configuration values\n"
                    "\tsetconfig csize ctimeout ctimecheck Set the config values\n"
                    "\tgetstats                            Get current cache statistics\n"
                    );
             exit(0);
          }
        else if ((!strcmp(argv[i], "getconfig")))
          {
             Op_Getconfig_Reply config;
             
             if (!evas_cserve_config_get(&config))
               {
                  printf("ERROR: cannot fetch config.\n");
                  exit(-1);
               }
             printf("csize: %i\n", config.cache_max_usage / 1024);
             printf("ctime: %i\n", config.cache_item_timeout);
             printf("ctimecheck: %i\n", config.cache_item_timeout_check);
             printf("-OK-\n");
          }
        else if ((!strcmp(argv[i], "setconfig")) && (i < (argc - 3)))
          {
             Op_Setconfig config;
             
             i++;
             config.cache_max_usage = atoi(argv[i]) * 1024;
             i++;
             config.cache_item_timeout = atoi(argv[i]);
             i++;
             config.cache_item_timeout_check = atoi(argv[i]);
             if (!evas_cserve_config_set(&config))
               {
                  printf("ERROR: cannot set config.\n");
                  exit(-1);
               }
          }
        else if ((!strcmp(argv[i], "getstats")))
          {
             Op_Getstats_Reply stats;
             
             if (!evas_cserve_stats_get(&stats))
               {
                  printf("ERROR: cannot fetch stats.\n");
                  exit(-1);
               }
             printf("saved_memory: %i Kb\n", stats.saved_memory / 1024);
             printf("wasted_memory: %i Kb\n", stats.wasted_memory / 1024);
             printf("saved_memory_peak: %i Kb\n", stats.saved_memory_peak / 1024);
             printf("wasted_memory_peak: %i Kb\n", stats.wasted_memory_peak / 1024);
             printf("saved_time_image_header_load: %1.3f sec\n", stats.saved_time_image_header_load);
             printf("saved_time_image_data_load: %1.3f sec\n", stats.saved_time_image_data_load);
             printf("-OK-\n");
          }
     }
/*   
     {
        Image_Entry *ie;
        RGBA_Image_Loadopts lopt = { 0, 0.0, 0, 0};
        
        ie = malloc(sizeof(Image_Entry));
        if (evas_cserve_image_load(ie, argv[1], NULL, &lopt))
          {
             printf("load ok\n");
             if (evas_cserve_image_data_load(ie))
               {
                  Mem *m;
                  
                  m = ie->data2;
                  printf("first pixel: %08x\n", *((int *)m->data));
                  printf("load data ok\n");
//                  evas_cserve_image_free(ie);
               }
          }
     }
 */
   evas_cserve_shutdown();
   evas_shutdown();
   return 0;
}
