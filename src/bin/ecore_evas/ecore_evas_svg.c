#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_Evas.h>

#undef EINA_LOG_DOMAIN_DEFAULT
#define EINA_LOG_DOMAIN_DEFAULT _log_dom
static int _log_dom = -1;

static void
_cb_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static unsigned char
_parse_color(const Ecore_Getopt *parser EINA_UNUSED,
             const Ecore_Getopt_Desc *desc EINA_UNUSED,
             const char *str,
             void *data EINA_UNUSED, Ecore_Getopt_Value *storage)
{
   unsigned char *color = (unsigned char *)storage->ptrp;

   if (sscanf(str, "%hhu,%hhu,%hhu", color, color + 1, color + 2) != 3)
     {
        fprintf(stderr, "ERROR: incorrect color value '%s'\n", str);
        return 0;
     }

   return 1;
}

const Ecore_Getopt optdesc = {
  "ecore_evas_svg",
  "%prog [options] <filename-source> [<filename-destination>]",
  PACKAGE_VERSION,
  "(C) 2014 Enlightenment",
  "BSD with advertisement clause",
  "Simple application to display or convert SVG in their vector form.",
  0,
  {
    ECORE_GETOPT_STORE_INT('q', "quality", "define encoding quality in percent."),
    ECORE_GETOPT_STORE_TRUE('c', "compress", "define if data should be compressed."),
    ECORE_GETOPT_STORE_STR('c', "codec", "define the codec (for TGV files: etc1, etc2)"),
    ECORE_GETOPT_CALLBACK_NOARGS('E', "list-engines", "list Ecore-Evas engines",
                                 ecore_getopt_callback_ecore_evas_list_engines, NULL),
    ECORE_GETOPT_STORE_STR('e', "engine", "The Ecore-Evas engine to use (see --list-engines)"),
    ECORE_GETOPT_CALLBACK_ARGS('c', "bg-color",
                               "Color of the background (if not shaped or alpha)",
                               "RRGGBB", _parse_color, NULL),
    ECORE_GETOPT_CALLBACK_ARGS('Z', "size", "size to use in wxh form.", "WxH",
                               ecore_getopt_callback_size_parse, NULL),
    ECORE_GETOPT_STORE_TRUE('a', "alpha", "Display window with alpha channel "
                            " (needs composite manager!)"),
    ECORE_GETOPT_STORE_STR('t', "title", "Define the window title string"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char *argv[])
{
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *im = NULL;
   Evas_Object *vg;
   Evas_Object *r;
   char *encoding = NULL;
   char *engine = NULL;
   char *title = NULL;
   Eina_Rectangle size = { 0, 0, 800, 600 };
   unsigned char color[3] = { 0, 0, 0 };

   int arg_index;
   int quality = -1;

   Eina_Bool compress = 1;
   Eina_Bool quit_option = EINA_FALSE;
   Eina_Bool display = EINA_FALSE;
   Eina_Bool alpha = EINA_FALSE;

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_INT(quality),
     ECORE_GETOPT_VALUE_BOOL(compress),
     ECORE_GETOPT_VALUE_STR(encoding),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_STR(engine),
     ECORE_GETOPT_VALUE_PTR_CAST(color),
     ECORE_GETOPT_VALUE_PTR_CAST(size),
     ECORE_GETOPT_VALUE_BOOL(alpha),
     ECORE_GETOPT_VALUE_STR(title),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

   eina_init();
   _log_dom = eina_log_domain_register(argv[0], EINA_COLOR_CYAN);

   ecore_init();
   ecore_evas_init();

   arg_index = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (quit_option) goto end;

   if (arg_index < 0)
     {
        EINA_LOG_ERR("Could not parse argument.");
        goto end;
     }
   if (arg_index + 1 == argc)
     {
        display = EINA_TRUE;
     }
   else if (arg_index + 2 != argc)
     {
        EINA_LOG_ERR("File not correctly specified.");
        goto end;
     }

   if (!display)
     {
        Ecore_Evas *sub_ee;

        ee = ecore_evas_buffer_new(1, 1);
        im = ecore_evas_object_image_new(ee);
        sub_ee = ecore_evas_object_ecore_evas_get(im);
        ecore_evas_resize(sub_ee, size.w, size.h);

        e = ecore_evas_object_evas_get(im);
     }
   else
     {
        ee = ecore_evas_new(engine, 0, 0, size.w, size.h, NULL);

        e = ecore_evas_get(ee);
     }

   ecore_evas_alpha_set(ee, alpha);
   ecore_evas_callback_delete_request_set(ee, _cb_delete);
   ecore_evas_title_set(ee, title ? title : "Ecore Evas SVG");


   r = eo_add(EVAS_RECTANGLE_CLASS, e,
              efl_gfx_color_set(color[0], color[1], color[2], alpha ? 0 : 255),
              efl_gfx_visible_set(EINA_TRUE));
   ecore_evas_object_associate(ee, r, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   vg = eo_add(EVAS_VG_CLASS, e,
               efl_file_set(argv[arg_index], NULL),
               efl_gfx_visible_set(EINA_TRUE));
   ecore_evas_object_associate(ee, vg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   if (display)
     {
        ecore_evas_show(ee);

        ecore_main_loop_begin();
     }
   else
     {
        Eina_Strbuf *flags = NULL;

        flags = eina_strbuf_new();
        eina_strbuf_append_printf(flags, "compress=%d", compress);
        if (quality >= 0)
          eina_strbuf_append_printf(flags, " quality=%d", quality);
        if (encoding)
          eina_strbuf_append_printf(flags, " encoding=%s", encoding);

        evas_object_image_save(im, argv[arg_index + 1],
                               NULL, eina_strbuf_string_get(flags));
        eina_strbuf_free(flags);
     }

   ecore_evas_free(ee);
 end:
   ecore_evas_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return 0;
}
