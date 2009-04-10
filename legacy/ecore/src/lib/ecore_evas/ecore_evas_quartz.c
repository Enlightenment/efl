/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef BUILD_ECORE_EVAS_QUARTZ
#import <Cocoa/Cocoa.h>
#endif

#include "Ecore.h"
#include "Ecore_Input.h"
#include "Ecore_Evas.h"

#include "ecore_private.h"
#include "ecore_evas_private.h"

#ifdef BUILD_ECORE_EVAS_QUARTZ
#include "Ecore_Quartz.h"
#include "Evas_Engine_Quartz.h"

static int                      _ecore_evas_init_count = 0;
static Ecore_Evas               *ecore_evases = NULL;
static Ecore_Event_Handler      *ecore_evas_event_handlers[4] = {
   NULL, NULL, NULL, NULL
};
static Ecore_Idle_Enterer       *ecore_evas_idle_enterer = NULL;
static Ecore_Poller             *ecore_evas_event = NULL;

static const char               *ecore_evas_quartz_default = "EFL Quartz";

@interface EvasView : NSView
{
	CGContextRef ctx;
}
@end

static EvasView * evas_view;
static NSWindow * main_window;

@implementation EvasView

- (id) init
{
   self = [super init];
   if (self != nil)
   {
      ctx = NULL;
   }
   return self;
}

- (void)drawRect:(NSRect)rect
{
   if(ctx != NULL)
   {
      Ecore_List2  *l;

      for (l = (Ecore_List2 *)ecore_evases; l; l = l->next)
      {
         Ecore_Evas *ee;

         ee = (Ecore_Evas *)l;
         if (ee->visible)
            evas_damage_rectangle_add(ee->evas, 0, 0, 400, 400);
      }
      return;
   }

   ctx = [[NSGraphicsContext currentContext] graphicsPort];
   CGContextRetain(ctx);
}

- (CGContextRef)context
{
   return ctx;
}

@end

static Ecore_Evas *
_ecore_evas_quartz_match(void)
{
   return ecore_evases;
}

static int
_ecore_evas_quartz_event_got_focus(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                   *ee;

   ee = _ecore_evas_quartz_match();

   if (!ee) return 1;
   ee->prop.focused = 1;

   return 0;
}

static int
_ecore_evas_quartz_event_lost_focus(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                   *ee;

   ee = _ecore_evas_quartz_match();

   if (!ee) return 1;
   ee->prop.focused = 0;

   return 0;
}

static int
_ecore_evas_quartz_event_video_resize(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   /*Ecore_Quartz_Event_Video_Resize *e;
   Ecore_Evas                   *ee;

   e = event;
   ee = _ecore_evas_quartz_match();

   if (!ee) return 1; // pass on event
   evas_output_size_set(ee->evas, e->w, e->h);

   return 0;*/
}

static int
_ecore_evas_quartz_event_video_expose(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   Ecore_Evas                   *ee;
   int                          w;
   int                          h;

   ee = _ecore_evas_quartz_match();

   if (!ee) return 1;
   evas_output_size_get(ee->evas, &w, &h);
   evas_damage_rectangle_add(ee->evas, 0, 0, w, h);

   return 0;
}

static int
_ecore_evas_idle_enter(void *data __UNUSED__)
{
   Ecore_List2  *l;
   double       t1 = 0.;
   double       t2 = 0.;

   for (l = (Ecore_List2 *)ecore_evases; l; l = l->next)
   {
      Ecore_Evas *ee = (Ecore_Evas *)l;

      if (ee->visible)
         evas_render(ee->evas);
      else
         evas_norender(ee->evas);
   }

   return 1;
}

static int
_ecore_evas_quartz_event(void *data)
{
   ecore_quartz_feed_events();

   return 1;
}

static int
_ecore_evas_quartz_init(int w, int h)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;

   ecore_evas_idle_enterer = ecore_idle_enterer_add(_ecore_evas_idle_enter, NULL);
   ecore_evas_event = ecore_poller_add(ECORE_POLLER_CORE, 1, ecore_evas_event, NULL);
   ecore_poller_poll_interval_set(ECORE_POLLER_CORE, 0.006);

   ecore_evas_event_handlers[0] = ecore_event_handler_add(ECORE_QUARTZ_EVENT_GOT_FOCUS, _ecore_evas_quartz_event_got_focus, NULL);
   ecore_evas_event_handlers[1] = ecore_event_handler_add(ECORE_QUARTZ_EVENT_LOST_FOCUS, _ecore_evas_quartz_event_lost_focus, NULL);
   ecore_evas_event_handlers[2] = ecore_event_handler_add(ECORE_QUARTZ_EVENT_RESIZE, _ecore_evas_quartz_event_video_resize, NULL);
   ecore_evas_event_handlers[3] = ecore_event_handler_add(ECORE_QUARTZ_EVENT_EXPOSE, _ecore_evas_quartz_event_video_expose, NULL);

   return _ecore_evas_init_count;
}

static int
_ecore_evas_quartz_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
   {
      int i;

      while (ecore_evases) _ecore_evas_free(ecore_evases);

      for (i = 0; i < sizeof (ecore_evas_event_handlers) / sizeof (Ecore_Event_Handler*); i++)
         ecore_event_handler_del(ecore_evas_event_handlers[i]);
      ecore_event_evas_shutdown();
      ecore_idle_enterer_del(ecore_evas_idle_enterer);
      ecore_evas_idle_enterer = NULL;
      ecore_poller_del(ecore_evas_event);
      ecore_evas_event = NULL;
   }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_quartz_free(Ecore_Evas *ee)
{
   ecore_evases = _ecore_list2_remove(ecore_evases, ee);
   ecore_event_window_unregister(0);
   _ecore_evas_quartz_shutdown();
   ecore_quartz_shutdown();
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;

   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x __UNUSED__, int y __UNUSED__, int w, int h)
{
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;

   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_object_cursor_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee)
     ee->prop.cursor.object = NULL;
}

static void
_ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   int x, y;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (obj == NULL)
   {
      ee->prop.cursor.object = NULL;
      ee->prop.cursor.layer = 0;
      ee->prop.cursor.hot.x = 0;
      ee->prop.cursor.hot.y = 0;
      return;
   }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_move(ee->prop.cursor.object,
                    x - ee->prop.cursor.hot.x,
                    y - ee->prop.cursor.hot.y);

   evas_object_pass_events_set(ee->prop.cursor.object, 1);

   if (evas_pointer_inside_get(ee->evas))
      evas_object_show(ee->prop.cursor.object);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, ee);
}

static const Ecore_Evas_Engine_Func _ecore_quartz_engine_func =
{
   _ecore_evas_quartz_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_resize,
   _ecore_evas_move_resize,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_object_cursor_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL
};
#endif

EAPI Ecore_Evas *
ecore_evas_quartz_new(const char* name, int w, int h)
{
#ifdef BUILD_ECORE_EVAS_QUARTZ
   Evas_Engine_Info_Quartz *einfo;
   Ecore_Evas           *ee;
   int                  rmethod;

   if (!name)
     name = ecore_evas_quartz_default;

   rmethod = evas_render_method_lookup("quartz");
   if (!rmethod) return NULL;

   if (!ecore_quartz_init(name)) return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)
     goto shutdown_ecore_quartz;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_quartz_init(w, h);

   ecore_event_window_register(0, ee, ee->evas, _ecore_evas_mouse_move_process);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_quartz_engine_func;

   ee->driver = "quartz";
   if (name) ee->name = strdup(name);
   if (!ee->name)
     goto free_ee;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->visible = 1;
   ee->w = w;
   ee->h = h;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = 1;
   ee->prop.borderless = 1;
   ee->prop.override = 1;
   ee->prop.maximized = 1;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;

   // init evas here
   ee->evas = evas_new();
   if (!ee->evas)
     goto free_name;
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);

   // Set up the Cocoa runtime
   [[NSAutoreleasePool alloc] init];
   [NSApplication sharedApplication];

   // Register ourselves as a full-fledged Cocoa app, instead of a NSUIElement.
   // This gives benefits like window focus and a dock icon!
   ProcessSerialNumber psn = { 0, kCurrentProcess };
   TransformProcessType (&psn, kProcessTransformToForegroundApplication);

   [NSApp finishLaunching];

   // Create our main window, and embed an EvasView in it
   main_window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,w,h) styleMask:(NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask) backing:NSBackingStoreBuffered defer:NO screen:nil];
   /* FIXME: manage the case where main_window is NULL witht a goto free_evas; */
   [main_window makeKeyAndOrderFront:NSApp];
   [main_window setTitle:[NSString stringWithUTF8String:name]];
   [main_window makeMainWindow];
   [main_window setAcceptsMouseMovedEvents:YES];
   [NSApp activateIgnoringOtherApps:YES];

   evas_view = [[EvasView alloc] initWithFrame:NSMakeRect(0,0,w,h)];
   [[main_window contentView] addSubview:evas_view];

   // drawRect: must be run at least once, to make sure we've set ctx
   [evas_view display];

   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   einfo = (Evas_Engine_Info_Quartz*) evas_engine_info_get(ee->evas);
   if (!einfo)
     goto free_window;

   einfo->info.context = [[evas_view context] retain];
   evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);

   evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);

   ecore_evases = _ecore_list2_prepend(ecore_evases, ee);
   return ee;

 free_window:
   /* FIXME: free window here */
 free_evas:
   free(ee->evas);
 free_name:
   free(ee->name);
 free_ee:
   _ecore_evas_quartz_shutdown();
   free(ee);
 shutdown_ecore_quartz:
   ecore_quartz_shutdown();

   return NULL;
#else
   fprintf(stderr, "OUTCH !\n");
   return NULL;
#endif
}
