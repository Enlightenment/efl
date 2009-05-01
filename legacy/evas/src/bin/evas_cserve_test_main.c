#include "evas_cs.h"

int
main(int argc, char **argv)
{
   evas_init();
   
   printf("evas_cserve_init = %i\n", evas_cserve_init());
   
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
   
   evas_cserve_shutdown();
   evas_shutdown();
   return 0;
}
