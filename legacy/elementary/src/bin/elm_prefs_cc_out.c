#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_prefs_cc.h"
#include "elm_prefs_edd.x"

Elm_Prefs_File *elm_prefs_file = NULL;

void
data_init()
{
   eet_init();
   _elm_prefs_descriptors_init();
}

void
data_shutdown()
{
   _elm_prefs_descriptors_shutdown();
   eet_shutdown();
}

void
data_write()
{
   Eina_List *l;
   Eet_File *ef;
   Elm_Prefs_Page_Node *page;

   if (!elm_prefs_file || !elm_prefs_file->pages)
     {
        ERR("No data to put in \"%s\"", file_out);
        exit(-1);
     }

   ef = eet_open(file_out, EET_FILE_MODE_WRITE);
   if (!ef)
     {
        ERR("Unable to open \"%s\" for writing output", file_out);
        exit(-1);
     }

   EINA_LIST_FOREACH (elm_prefs_file->pages, l, page)
     {
        if (!(eet_data_write
              (ef, _page_edd, page->name, page, EET_COMPRESSION_DEFAULT)))
          ERR("Failed to write page %s to file %s", page->name, file_out);
     }

   eet_close(ef);
}
