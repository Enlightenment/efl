#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include "option_pool.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// defines

// function prototypes
void resize_cb(Ecore_Evas *ee);
void print_help (char *app_name);
void list_engines ();
void list_groups (const char *edje_file);
void parse_options (int argc, char **argv);
void show_version ();
void option_pool_from_data_block ();
void string_to_bool (char *str, bool *value);

// variables
Ecore_Evas *ee;
Evas *evas;

Evas_Object *o_bg;
Evas_Object *o_edje;

struct option_pool_t option_pool;

void resize_cb(Ecore_Evas *ee)
{
  Evas_Coord x, y, w, h;

  ecore_evas_geometry_get (ee, &x, &y, &w, &h);

  if (w < h)
  {
    evas_object_resize(o_bg, w, w);
    evas_object_resize(o_edje, w, w);
  }
  else
  {
    evas_object_resize(o_bg, h, h);
    evas_object_resize(o_edje, h, h);
  }

}

void print_help (char *app_name)
{
  printf("Usage: %s --help\n", app_name);
  exit(-1);
}

int main(int argc, char **argv)
{
  int edje_w = 240;
  int edje_h = 240;
  bool edje_load = false;
  
  option_pool_constructor (&option_pool);
  
  switch (argc)
  {
    case 1:
      print_help (argv[0]);
      break;
    default:
      option_pool_parse (&option_pool, argc, argv);
  }

  ecore_init();
  ecore_app_args_set(argc, (const char **)argv);
  ecore_evas_init();
  edje_init();
  
  if (option_pool.show_version)
  {
    show_version ();
    exit (0);
  }
  
  if (option_pool.list_engines)
  {
    list_engines ();
    exit (0);
  }  
  
  ee = ecore_evas_new (option_pool.engine, 0, 0, edje_w, edje_h, NULL);

  if (ee)
  {
    printf ("Using engine %s to create a canvas...\n", option_pool.engine);
  }
  else
  {
    fprintf (stderr, "Engine %s failed to create a canvas\n", option_pool.engine);
    fprintf (stderr, "Exiting...\n");
    exit (1);
  }
  
  if (option_pool.list_groups_flag)
  {
    list_groups (option_pool.file);
    exit (0);
  }
  
  evas = ecore_evas_get(ee);
  evas_image_cache_set(evas, 8192 * 1024);
  evas_font_cache_set(evas, 512 * 1024);
  
  option_pool_from_data_block ();

  o_bg = evas_object_rectangle_add(evas);
  evas_object_move(o_bg, 0, 0);
  evas_object_resize(o_bg, edje_w, edje_h);
  
  if (option_pool.title)
  {
    ecore_evas_title_set (ee, option_pool.title);
  }

  if (option_pool.borderless)
  {
    ecore_evas_borderless_set (ee, 1);
  }

  if (option_pool.sticky)
  {
    ecore_evas_sticky_set (ee, 1);
  }
  
  if (option_pool.alpha || option_pool.shaped) 
  {
    if (option_pool.alpha)
    {
       ecore_evas_alpha_set(ee, 1);
    }
    else if (option_pool.shaped)
    {
       ecore_evas_shaped_set(ee, 1);
    }
    
    // set alpha transparent background if window has alpha support or is shaped...
    evas_object_color_set(o_bg, 0, 0, 0, 0);
  } 
  else
  {
    // ...and set black background in other case
    evas_object_color_set(o_bg, 0, 0, 0, 255);
  }
  
  o_edje = edje_object_add (evas);
  
  evas_object_focus_set(o_edje, 1);
  
  edje_load = edje_object_file_set(o_edje, option_pool.file, option_pool.group);
  if (!edje_load)
  {
    fprintf (stderr, "The Edje file '%s' and group '%s' couldn't be set!\n", option_pool.file, option_pool.group);
    exit (1);
  }
  
  evas_object_move(o_edje, 0, 0);
  evas_object_resize(o_edje, edje_w, edje_h);

  evas_object_show(o_edje);
  evas_object_show(o_bg);

  ecore_evas_callback_resize_set(ee, resize_cb);
  ecore_evas_show(ee);

  ecore_main_loop_begin();

  edje_shutdown();
  ecore_evas_shutdown();
  ecore_shutdown();
  
  option_pool_destructor (&option_pool);

  return 0;
}

void option_pool_from_data_block ()
{
  char *tmp_str = NULL;
  
  if (!option_pool.alpha)
  {
    tmp_str = edje_file_data_get (option_pool.file, "alpha");
    string_to_bool (tmp_str, &option_pool.alpha);
    free (tmp_str);
  }
  
  if (!option_pool.borderless)
  {
    tmp_str = edje_file_data_get (option_pool.file, "borderless");
    string_to_bool (tmp_str, &option_pool.borderless);
    free (tmp_str);
  }
  
  if (!option_pool.sticky)
  {
    tmp_str = edje_file_data_get (option_pool.file, "sticky");
    string_to_bool (tmp_str, &option_pool.sticky);
    free (tmp_str);
  }
  
  if (!option_pool.shaped)
  {
    tmp_str = edje_file_data_get (option_pool.file, "shaped");
    string_to_bool (tmp_str, &option_pool.shaped);
    free (tmp_str);
  }
  
  if (!option_pool.title)
  {
    option_pool.title = edje_file_data_get (option_pool.file, "title");
  }
}

void list_engines ()
{
  Eina_List *engines_list;
  Eina_List *l; // list element

  printf ("supported engines:\n"); 
  engines_list = ecore_evas_engines_get ();
  for (l = engines_list; l; l = eina_list_next(l))
  {
    const char *engine = (const char*) l->data;
    
    printf ("\t%s\n", engine);
  }
}

void list_groups (const char *edje_file)
{
  Eina_List *group_list;
  Eina_List *l; // list element

  group_list = edje_file_collection_list (edje_file);

  printf("Available groups in 'edje_file':\n");
  for (l = group_list; l; l = eina_list_next(l))
  {
    printf("%s\n", (const char*) l->data);
  }

  edje_file_collection_list_free (group_list);
}

void show_version ()
{
  printf ("Package name: ");
  printf ("%s %s\n", PACKAGE, VERSION);
  printf ("Build information: ");
  printf ("%s %s\n", __DATE__, __TIME__);
}

void string_to_bool (char *str, bool *value)
{
  if (str)
  {
    if ((!strncmp (str, "FALSE", 5)) || (!strncmp (str, "false", 5)) || (!strncmp (str, "0", 2)))
    {
      *value = false;
    }
    else if ((!strncmp (str, "TRUE", 5)) || (!strncmp (str, "true", 5)) || (!strncmp (str, "1", 2)))
    {
      *value = true;
    }
  }
  
  value = NULL;
}
