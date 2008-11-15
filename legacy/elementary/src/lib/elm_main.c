#include <Elementary.h>
#include "elm_priv.h"

static int _elm_signal_exit(void *data, int ev_type, void *ev);
static int _elm_window_property_change(void *data, int ev_type, void *ev);
static void _elm_rescale(void);

char *_elm_appname = NULL;
Elm_Config *_elm_config = NULL;
Ecore_Event_Handler *_elm_event_property_change = NULL;
Ecore_X_Atom _elm_atom_enlightenment_scale = 0;
  
static int
_elm_signal_exit(void *data, int ev_type, void *ev)
{
   elm_exit();
   return 1;
}

static int
_elm_window_property_change(void *data, int ev_type, void *ev)
{
   Ecore_X_Event_Window_Property *event = ev;
   
   if (event->win == ecore_x_window_root_first_get())
     {
        if (event->atom == _elm_atom_enlightenment_scale)
          {
             int val = 1000;
             
             if (ecore_x_window_prop_card32_get(event->win,
                                                event->atom,
                                                &val, 1) > 0)
               {
                  double pscale;
                  
                  pscale = _elm_config->scale;
                  if (val > 0) _elm_config->scale = (double)val / 1000.0;
                  if (pscale != _elm_config->scale) _elm_rescale();
               }
          }
     }
   return 1;
}

static void
_elm_rescale(void)
{
   printf("Scale %3.3f\n", _elm_config->scale); 
   edje_scale_set(_elm_config->scale);
   _elm_win_rescale();
}

EAPI void
elm_init(int argc, char **argv)
{
   int i;
   
   eet_init();
   ecore_init();
   ecore_app_args_set(argc, (const char **)argv);
   ecore_file_init();
   evas_init();
   edje_init();
   ecore_evas_init(); // FIXME: check errors
   
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _elm_signal_exit, NULL);
   
   _elm_appname = strdup(ecore_file_file_get(argv[0]));

   // FIXME: actually load config
   _elm_config = ELM_NEW(Elm_Config);
   _elm_config->engine = ELM_SOFTWARE_X11;
   _elm_config->thumbscroll_enable = 1;
   _elm_config->thumbscroll_threshhold = 24;
   _elm_config->thumbscroll_momentum_threshhold = 100.0;
   _elm_config->thumbscroll_friction = 1.0;
   _elm_config->scale = 1.0;
   _elm_config->bgpixmap = 0;
   _elm_config->compositing = 1;

   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11))
     {
	int val = 1000;
	
	ecore_x_init(NULL);
        _elm_atom_enlightenment_scale = ecore_x_atom_get("ENLIGHTENMENT_SCALE");
        ecore_x_event_mask_set(ecore_x_window_root_first_get(),
                               ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
        _elm_event_property_change = ecore_event_handler_add
          (ECORE_X_EVENT_WINDOW_PROPERTY, _elm_window_property_change, NULL);
	if (ecore_x_window_prop_card32_get(ecore_x_window_root_first_get(),
					   _elm_atom_enlightenment_scale,
					   &val, 1) > 0)
	  {
	     if (val > 0) _elm_config->scale = (double)val / 1000.0;
	  }
	if (!ecore_x_screen_is_composited(0))
	  _elm_config->compositing = 0;
      }
   
}

EAPI void
elm_shutdown(void)
{
   _elm_win_shutdown();
   
   free(_elm_config);
   free(_elm_appname);
   ecore_evas_shutdown();
   edje_shutdown();
   evas_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   eet_shutdown();
}

EAPI void
elm_run(void)
{
   ecore_main_loop_begin();
}

EAPI void
elm_exit(void)
{
   ecore_main_loop_quit();
}
