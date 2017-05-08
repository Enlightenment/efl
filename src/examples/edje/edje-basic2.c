/**
* @verbatim
* edje_cc basic2.edc && gcc -o edje-basic2 edje-basic2.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
* @endverbatim
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
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
	      Evas *evas EINA_UNUSED,
	      Evas_Object *o EINA_UNUSED,
	      void *einfo)
{
  Evas_Event_Key_Down *ev;
  Evas_Object *edje_obj;
  int x,y;

  ev = (Evas_Event_Key_Down *)einfo;
  edje_obj = (Evas_Object *)data;

  evas_object_geometry_get(edje_obj, &x, &y, NULL, NULL);

  if(strcmp(ev->key,"Escape") == 0)
  {
	ecore_main_loop_quit();
  }
  else if(strcmp(ev->key, "Down") == 0)
  {
	y+=WALK;
  }
  else if(strcmp(ev->key, "Up") == 0)
  {
	y-=WALK;
  }
  else if(strcmp(ev->key, "Right") == 0)
  {
	x+=WALK;
  }
  else if(strcmp(ev->key, "Left") == 0)
  {
	x-=WALK;
  }
  else
  {
     printf("Key %s not supported.\nCommands:%s", ev->key, commands);
     return;
  }

  evas_object_move(edje_obj, x, y);
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
  const char *edje_file = PACKAGE_DATA_DIR"/basic2.edj";
  Ecore_Evas *ee;
  Evas *evas;
  Evas_Object *bg;
  Evas_Object *edje_obj;

  if(!ecore_evas_init())
	return EXIT_FAILURE;

  if(!edje_init())
	goto shutdown_ecore_evas;

  ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

  if(!ee) goto shutdown_edje;

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

  if(!edje_object_file_set(edje_obj, edje_file, "image_group"))
  {
    int err = edje_object_load_error_get(edje_obj);
    const char *errmsg = edje_load_error_str(err);
    fprintf(stderr, "Could not load the edje file - reason:%s\n", errmsg);
    goto shutdown_edje;
  }

  evas_object_move(edje_obj, 50, 50);
  evas_object_resize(edje_obj, 64, 64);
  evas_object_show(edje_obj);

  evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_key_down, edje_obj);

  ecore_evas_show(ee);

  printf("%s", commands);

  ecore_main_loop_begin();

  ecore_evas_free(ee);
  edje_shutdown();
  ecore_evas_shutdown();

  return EXIT_SUCCESS;

shutdown_edje:
  edje_shutdown();

shutdown_ecore_evas:
  ecore_evas_shutdown();

  return EXIT_FAILURE;
}
