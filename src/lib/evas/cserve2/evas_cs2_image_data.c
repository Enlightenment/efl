#include "config.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "evas_cs2.h"
#include "evas_cs2_private.h"

#ifdef EVAS_CSERVE2

void *
evas_cserve2_image_data_get(Image_Entry *ie)
{
   Data_Entry *dentry = ie->data2;

   if (!dentry)
     return NULL;

   return dentry->shm.data;
}

unsigned int
evas_cserve2_image_hit(Image_Entry *ie)
{
   Data_Entry *dentry = ie->data2;

   if (!dentry) return 0;
   return ++dentry->hit_count;
}

#endif
