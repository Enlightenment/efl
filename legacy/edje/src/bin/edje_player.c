#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Edje.h>
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Evas.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct opts {
   char *file;
   char *group;
   Eina_Bool list_groups;
   char *engine;
   Eina_Rectangle size;
   unsigned char color[3];
   Eina_Bool borderless;
   Eina_Bool sticky;
   Eina_Bool shaped;
   Eina_Bool alpha;
   char *title;
};

static void
_reset_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *stack, void *event_info __UNUSED__)
{
   Evas_Coord minw, minh;
   Evas_Object *edje = data;

   edje_object_size_min_get(edje, &minw, &minh);
   if ((minw <= 0) && (minh <= 0))
     edje_object_size_min_calc(edje, &minw, &minh);

   evas_object_size_hint_min_set(stack, minw, minh);
}

static Evas_Object *
_create_stack(Evas *evas, const struct opts *opts)
{
   Evas_Object *stack = evas_object_box_add(evas);
   if (!stack)
     {
	fputs("ERROR: could not create object stack (box).\n", stderr);
	return NULL;
     }
   evas_object_box_layout_set(stack, evas_object_box_layout_stack, NULL, NULL);
   evas_object_resize(stack, opts->size.w, opts->size.h);
   evas_object_size_hint_weight_set(stack, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(stack, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(stack);
   return stack;
}

static Evas_Object *
_create_bg(Evas *evas, const struct opts *opts)
{
   const unsigned char *color = opts->color;
   Evas_Object *bg = evas_object_rectangle_add(evas);
   if (!bg)
     {
	fputs("ERROR: could not create background.\n", stderr);
	return NULL;
     }
   evas_object_resize(bg, opts->size.w, opts->size.h);
   evas_object_color_set(bg, color[0], color[1], color[2], 255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bg);
   return bg;
}

static Eina_Bool
_edje_load_or_show_error(Evas_Object *edje, const char *file, const char *group)
{
   const char *errmsg;
   int err;

   if (edje_object_file_set(edje, file, group))
     return EINA_TRUE;

   err = edje_object_load_error_get(edje);
   errmsg = edje_load_error_str(err);
   fprintf(stderr, "ERROR: could not load edje file '%s', group '%s': %s\n",
	   file, group, errmsg);
   return EINA_FALSE;
}

static Evas_Object *
_create_edje(Evas *evas, const struct opts *opts)
{
   Evas_Coord minw, minh, maxw, maxh;
   Evas_Object *edje = edje_object_add(evas);
   if (!edje)
     {
	fputs("ERROR: could not create edje.\n", stderr);
	return NULL;
     }

   if (opts->group)
     {
	if (!_edje_load_or_show_error(edje, opts->file, opts->group))
	  {
	     evas_object_del(edje);
	     return NULL;
	  }
     }
   else
     {
	if (edje_file_group_exists(opts->file, "main"))
	  {
	     if (!_edje_load_or_show_error(edje, opts->file, "main"))
	       {
		  evas_object_del(edje);
		  return NULL;
	       }
	  }
	else
	  {
	     Eina_List *groups = edje_file_collection_list(opts->file);
	     const char *group;
	     if (!groups)
	       {
		  fprintf(stderr, "ERROR: file '%s' has no groups!\n",
			  opts->file);
		  evas_object_del(edje);
		  return NULL;
	       }
	     group = groups->data;
	     if (!_edje_load_or_show_error(edje, opts->file, group))
	       {
		  edje_file_collection_list_free(groups);
		  evas_object_del(edje);
		  return NULL;
	       }
	     edje_file_collection_list_free(groups);
	  }
     }

   edje_object_size_max_get(edje, &maxw, &maxh);
   edje_object_size_min_get(edje, &minw, &minh);
   if ((minw <= 0) && (minh <= 0))
     edje_object_size_min_calc(edje, &minw, &minh);

   evas_object_size_hint_max_set(edje, maxw, maxh);
   evas_object_size_hint_min_set(edje, minw, minh);

   evas_object_size_hint_weight_set(edje, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(edje, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(edje);

   return edje;
}

static  unsigned char _parse_color(const Ecore_Getopt *parser, const Ecore_Getopt_Desc *desc, const char *str, void *data, Ecore_Getopt_Value *storage)
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
  "edje_player",
  "%prog [options] <filename.edj>",
  PACKAGE_VERSION,
  "(C) 2010 Enlightenment",
  "BSD with advertisement clause",
  "Simple application to view edje files.",
  0,
  {
    ECORE_GETOPT_STORE_STR
    ('g', "group", "The edje group to view (defaults to 'main')."),
    ECORE_GETOPT_STORE_TRUE
    ('G', "list-groups", "The groups in the given file."),
    ECORE_GETOPT_STORE_STR
    ('e', "engine", "The Ecore-Evas engine to use (see --list-engines)"),
    ECORE_GETOPT_CALLBACK_NOARGS
    ('E', "list-engines", "list Ecore-Evas engines",
     ecore_getopt_callback_ecore_evas_list_engines, NULL),
    ECORE_GETOPT_CALLBACK_ARGS
    ('Z', "size", "size to use in wxh form.", "WxH",
     ecore_getopt_callback_size_parse, NULL),
    ECORE_GETOPT_CALLBACK_ARGS
    ('c', "bg-color", "Color of the background (if not shaped or alpha)",
     "RRGGBB", _parse_color, NULL),
    ECORE_GETOPT_STORE_TRUE
    ('b', "borderless", "Display window without border."),
    ECORE_GETOPT_STORE_TRUE
    ('y', "sticky", "Display window sticky."),
    ECORE_GETOPT_STORE_TRUE
    ('s', "shaped", "Display window shaped."),
    ECORE_GETOPT_STORE_TRUE
    ('a', "alpha", "Display window with alpha channel "
     "(needs composite manager!)"),
    ECORE_GETOPT_STORE_STR
    ('t', "title", "Define the window title string"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

int main(int argc, char **argv)
{
   Ecore_Evas *win;
   Evas *evas;
   Evas_Object *stack, *edje;
   struct opts opts;
   Eina_Bool quit_option = EINA_FALSE;
   int args, ret;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(opts.group),
     ECORE_GETOPT_VALUE_BOOL(opts.list_groups),
     ECORE_GETOPT_VALUE_STR(opts.engine),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_PTR_CAST(opts.size),
     ECORE_GETOPT_VALUE_PTR_CAST(opts.color),
     ECORE_GETOPT_VALUE_BOOL(opts.borderless),
     ECORE_GETOPT_VALUE_BOOL(opts.sticky),
     ECORE_GETOPT_VALUE_BOOL(opts.shaped),
     ECORE_GETOPT_VALUE_BOOL(opts.alpha),
     ECORE_GETOPT_VALUE_STR(opts.title),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

   memset(&opts, 0, sizeof(opts));

   evas_init();
   ecore_init();
   ecore_evas_init();
   edje_init();

   args = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (args < 0)
     {
	fputs("Could not parse arguments.\n", stderr);
	ret = -1;
	goto end;
     }
   else if (quit_option)
     {
	ret = 0;
	goto end;
     }
   else if (args >= argc)
     {
	fputs("Missing edje file to load.\n", stderr);
	ret = -2;
	goto end;
     }

   ret = 0;
   opts.file = argv[args];
   if (opts.list_groups)
     {
	Eina_List *groups, *n;
	const char *group;
	groups = edje_file_collection_list(opts.file);
	printf("%d groups in file '%s':\n", eina_list_count(groups), opts.file);
	EINA_LIST_FOREACH(groups, n, group)
	  printf("\t'%s'\n", group);
	edje_file_collection_list_free(groups);
	goto end;
     }

   win = ecore_evas_new(opts.engine, 0, 0, opts.size.w, opts.size.h, NULL);
   if (!win)
     {
	fprintf(stderr,
		"ERROR: could not create window of "
		"size %dx%d using engine %s.\n",
		opts.size.w, opts.size.h, opts.engine ? opts.engine : "(auto)");
	ret = -3;
	goto end;
     }

   evas = ecore_evas_get(win);
   stack = _create_stack(evas, &opts);
   if (!stack)
     {
	ret = -4;
	goto end_win;
     }

   ecore_evas_object_associate(win, stack, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);

   if (opts.alpha)
     ecore_evas_alpha_set(win, EINA_TRUE);
   else if (opts.shaped)
     ecore_evas_shaped_set(win, EINA_TRUE);
   else
     {
	Evas_Object *bg = _create_bg(evas, &opts);
	if (bg) evas_object_box_append(stack, bg);
     }

   edje = _create_edje(evas, &opts);
   if (edje)
     evas_object_box_append(stack, edje);
   else
     {
	ret = -5;
	goto end_win;
     }

   evas_object_event_callback_add(stack, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _reset_size_hints, edje);

   ecore_evas_borderless_set(win, opts.borderless);
   ecore_evas_sticky_set(win, opts.sticky);
   if (opts.title)
     ecore_evas_title_set(win, opts.title);
   else
     {
	char buf[1024];
	snprintf(buf, sizeof(buf), "Edje_Player - %s of %s",
		 opts.group, opts.file);
	ecore_evas_title_set(win, buf);
     }

   if (opts.size.w <= 0) opts.size.w = 320;
   if (opts.size.h <= 0) opts.size.h = 240;
   ecore_evas_resize(win, opts.size.w, opts.size.h);
   ecore_evas_show(win);
   ecore_main_loop_begin();

 end_win:
   ecore_evas_free(win);
 end:
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   evas_shutdown();

   return ret;
}
