#include "Ethumb.h"
#include "ethumb_private.h"
#include "Ethumb_Plugin.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <Evas.h>
#include <Epdf.h>

static void
_shutdown(Ethumb_Plugin *plugin)
{
   epdf_shutdown();
}

static int
_generate_thumb(Ethumb_File *ef)
{
   Ethumb *e = ef->ethumb;
   Epdf_Document *document;
   Epdf_Page *page;
   Evas_Object *o;
   int w, h, ww, hh;
   int fx, fy, fw, fh;
   int npages;

   document = epdf_document_new(ef->src_path);
   if (!document)
     {
	fprintf(stderr, "ERROR: could not read document: %s\n", ef->src_path);
	return 0;
     }

   page = epdf_page_new(document);
   if (!page)
     {
	fprintf(stderr, "ERROR: could not read document: %s\n", ef->src_path);
	epdf_document_delete(document);
	return 0;
     }

   npages = epdf_document_page_count_get(document);
   if (e->document.page < npages)
     epdf_page_page_set(page, e->document.page);
   epdf_page_size_get(page, &w, &h);
   ethumb_calculate_aspect(e, w, h, &ww, &hh);
   ethumb_plugin_image_resize(ef, ww, hh);

   o = evas_object_image_add(e->sub_e);
   epdf_page_render(page, o);
   evas_object_resize(o, ww, hh);
   evas_object_move(o, 0, 0);

   ethumb_calculate_fill(e, w, h, &fx, &fy, &fw, &fh);
   evas_object_image_fill_set(o, fx, fy, fw, fh);

   evas_object_show(o);
   ethumb_image_save(ef);

   evas_object_del(o);
   epdf_page_delete(page);
   epdf_document_delete(document);

   ethumb_finished_callback_call(ef);

   return 1;
}

Ethumb_Plugin *
ethumb_plugin_init(void)
{
   static const char *extensions[] = { "pdf", NULL };
   static Ethumb_Plugin plugin =
     {
	extensions,
	_generate_thumb,
	_shutdown
     };

   epdf_init();

   return &plugin;
}
