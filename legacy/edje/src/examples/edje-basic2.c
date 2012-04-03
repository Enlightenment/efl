/**
* @verbatim
* gcc -o edje-basic2 edje-basic2.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
* @endverbatim
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define __UNUSED__
#endif

#include <stdio.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (700)
#define HEIGHT (700)

#define WALK 10

static const char commands[] = "commands are:\nEsc - Exit\nUp - move image up\nDown - move image down\n"\
    "Right - move image to right\nLeft - move image to left\n";

static void
_on_key_down(void *data,
	      Evas *evas __UNUSED__,
	      Evas_Object *o __UNUSED__,
	      void *einfo)
{
  Evas_Event_Key_Down *ev;
  Evas_Object *edje_obj;
  int x,y;

  ev = (Evas_Event_Key_Down *)einfo;
  edje_obj = (Evas_Object *)data;

  evas_object_geometry_get(edje_obj, &x, &y, NULL, NULL);

  if(strcmp(ev->keyname,"Escape") == 0)
  {
	ecore_main_loop_quit();
  }
  else if(strcmp(ev->keyname, "Down") == 0)
  {
	y+=WALK;
  }
  else if(strcmp(ev->keyname, "Up") == 0)
  {
	y-=WALK;
  }
  else if(strcmp(ev->keyname, "Right") == 0)
  {
	x+=WALK;
  }
  else if(strcmp(ev->keyname, "Left") == 0)
  {
	x-=WALK;
  }
  else
  {
     fprintf(stdout, "Key %s not supported.\nCommands:%s", ev->keyname, commands);
     return;
  }

  evas_object_move(edje_obj, x, y);
}

static void
_on_delete(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

int
main(int argc __UNUSED__, char **argv)
{
  const char *edje_file = "basic2.edj";
  char edje_file_path[PATH_MAX];
  Eina_Prefix *pfx;
  Ecore_Evas *ee;
  Evas *evas;
  Evas_Object *bg;
  Evas_Object *edje_obj;

  if(!ecore_evas_init())
	return EXIT_FAILURE;

  if(!edje_init())
	goto shutdown_ecore_evas;

  pfx = eina_prefix_new(argv[0], main,
                         "EDJE_EXAMPLES",
                         "edje/examples",
                         edje_file,
                         PACKAGE_BIN_DIR,
                         PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR,
                         PACKAGE_DATA_DIR);

  if(!pfx)
    goto shutdown_edje;

  ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

  if(!ee)
    goto eina_prefix_free;

  ecore_evas_callback_delete_request_set(ee, _on_delete);
  ecore_evas_title_set(ee, "Edje show image");

  evas = ecore_evas_get(ee);

  bg = evas_object_rectangle_add(evas);
  evas_object_color_set(bg, 255, 255, 255, 255); //White
  evas_object_move(bg, 0, 0); //orign
  evas_object_resize(bg, WIDTH, HEIGHT); //cover the window
  evas_object_show(bg);

  ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
  evas_object_focus_set(bg, EINA_TRUE);


  edje_obj = edje_object_add(evas);

  snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);

  if(!edje_object_file_set(edje_obj, edje_file_path, "image_group"))
  {
    int err = edje_object_load_error_get(edje_obj);
    const char *errmsg = edje_load_error_str(err);
    fprintf(stderr, "Could not load the edje file - reason:%s\n", errmsg);
    goto eina_prefix_free;
  }



  evas_object_move(edje_obj, 50, 50);
  evas_object_resize(edje_obj, 64, 64);
  evas_object_show(edje_obj);

  evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_key_down, edje_obj);

  ecore_evas_show(ee);

  fprintf(stdout, "%s", commands);

  ecore_main_loop_begin();

  eina_prefix_free(pfx);
  ecore_evas_free(ee);
  edje_shutdown();
  ecore_evas_shutdown();

  return EXIT_SUCCESS;

eina_prefix_free:
  eina_prefix_free(pfx);

shutdown_edje:
  edje_shutdown();

shutdown_ecore_evas:
  ecore_evas_shutdown();

  return EXIT_FAILURE;
}
