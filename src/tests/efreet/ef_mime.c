#include "config.h" 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Efreet.h"
#include "Efreet_Mime.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <Ecore.h>
#include "ef_test.h"

#define THEME "Tango"
#define SIZE 128

int
ef_mime_cb_get(void)
{
   const char *mime = NULL, *icon;
   int misses = 0, i = 0;
   struct
     {
        char *file;
        char *mime;
     } files[] = {
        {"test_type.desktop", "application/x-desktop"},
        {"entry.png", "image/png"},
        {"entry", "image/png"},
        {"sub", "inode/directory"},
        { }
     };
   double start;

   if (!efreet_mime_init())
     {
        printf("Could not init efreet\n");
        return 1;
     }

   for (i = 0; files[i].file; ++i)
     {
        mime = efreet_mime_type_get(ef_test_path_get(files[i].file));
        if (!mime)
          {
             printf("Got %s as null instead of %s\n", files[i].file, files[i].mime);
             misses ++;
          }
        else if (strcmp(mime, files[i].mime))
          {
             printf("Got %s as %s instead of %s\n", files[i].file, mime, files[i].mime);
             misses ++;
          }
        start = ecore_time_get();
        icon = efreet_mime_type_icon_get(files[i].mime, THEME, SIZE);
        printf("mime icon: %s %s %f\n", files[i].mime, icon, ecore_time_get() - start);
     }

   efreet_mime_shutdown();

   return !misses;
}
