#include "config.h" 

#include "Efreet.h"
#include <stdio.h>

int
ef_cb_utils(void)
{
   Efreet_Desktop *desktop;
   const char *tmp2;

   printf("\n");

   tmp2 = efreet_util_path_to_file_id("/usr/share/applications/this/tmp/test.desktop");
   if (tmp2) printf("%s\n", tmp2);

   desktop = efreet_util_desktop_file_id_find("kde-kresources.desktop");
   printf("kde-kresources.desktop: %p\n", desktop);
   efreet_desktop_free(desktop);

   desktop = efreet_util_desktop_file_id_find("mplayer.desktop");
   printf("mplayer.desktop: %p\n", desktop);
   efreet_desktop_free(desktop);

   desktop = efreet_util_desktop_file_id_find("nautilus-computer.desktop");
   printf("nautilus-computer.desktop: %p\n", desktop);
   efreet_desktop_free(desktop);

   return 1;
}
