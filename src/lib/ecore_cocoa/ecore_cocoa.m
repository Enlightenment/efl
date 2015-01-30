#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#import <Cocoa/Cocoa.h>
#import "ecore_cocoa_window.h"
#import "ecore_cocoa_app.h"

#include <Eina.h>

#include <Ecore.h>
#include <ecore_cocoa_private.h>
#include <Ecore_Input.h>

#include "Ecore_Cocoa.h"
#include "Ecore_Cocoa_Keys.h"


EAPI int ECORE_COCOA_EVENT_GOT_FOCUS = 0;
EAPI int ECORE_COCOA_EVENT_LOST_FOCUS = 0;
EAPI int ECORE_COCOA_EVENT_RESIZE = 0;
EAPI int ECORE_COCOA_EVENT_EXPOSE = 0;
EAPI int ECORE_COCOA_EVENT_WINDOW_DESTROY = 0;

static int _ecore_cocoa_init_count = 0;

static int old_flags;

int _ecore_cocoa_log_domain = -1;

EAPI int
ecore_cocoa_init(void)
{
   if (++_ecore_cocoa_init_count != 1)
     return _ecore_cocoa_init_count;

   if (!ecore_init())
     return --_ecore_cocoa_init_count;

   if (!ecore_event_init())
     return --_ecore_cocoa_init_count;

   _ecore_cocoa_log_domain = eina_log_domain_register("ecore_cocoa", EINA_COLOR_BLUE);
   if(_ecore_cocoa_log_domain < 0)
     {
        EINA_LOG_ERR("Unable to create a log domain for ecore_cocoa.");
        return 0;
     }

   ECORE_COCOA_EVENT_GOT_FOCUS  = ecore_event_type_new();
   ECORE_COCOA_EVENT_LOST_FOCUS = ecore_event_type_new();
   ECORE_COCOA_EVENT_RESIZE     = ecore_event_type_new();
   ECORE_COCOA_EVENT_EXPOSE     = ecore_event_type_new();
   ECORE_COCOA_EVENT_WINDOW_DESTROY = ecore_event_type_new();

   /* Init the Application handler */
   [Ecore_Cocoa_Application sharedApplication];
   [NSApp setDelegate:[Ecore_Cocoa_AppDelegate appDelegate]];

   /* Start events monitoring */
   [NSApp run];

   return _ecore_cocoa_init_count;
}

/**
 * Shuts down the Ecore_Cocoa library.
 * @return  @c The number of times the system has been initialised without
 *             being shut down.
 * @ingroup Ecore_Cocoa_Library_Group
 */
EAPI int
ecore_cocoa_shutdown(void)
{
   if (--_ecore_cocoa_init_count != 0)
     return _ecore_cocoa_init_count;

   eina_log_domain_unregister(_ecore_cocoa_log_domain);
   ecore_event_shutdown();

   return _ecore_cocoa_init_count;
}

static unsigned int
_ecore_cocoa_event_modifiers(unsigned int mod)
{
   unsigned int modifiers = 0;

   if(mod & NSShiftKeyMask) modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if(mod & NSControlKeyMask) modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if(mod & NSAlternateKeyMask) modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
   if(mod & NSCommandKeyMask) modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if(mod & NSNumericPadKeyMask) modifiers |= ECORE_EVENT_LOCK_NUM;

   DBG("key modifiers: %d, %d\n", mod, modifiers);
   return modifiers;
}


static inline Ecore_Event_Key*
_ecore_cocoa_event_key(NSEvent *event, int keyType)
{
   static Eina_Bool compose = EINA_FALSE;
   static NSText *edit;
   unsigned int i;

   Ecore_Event_Key *ev;

   EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
   NSString *keychar = [event charactersIgnoringModifiers];
   NSString *keycharRaw = [event characters];

   ev = calloc(1, sizeof (Ecore_Event_Key));
   if (!ev) return NULL;

   if (compose && keyType == NSKeyDown)
     {
        [edit interpretKeyEvents:[NSArray arrayWithObject:event]];
        compose=EINA_FALSE;
     }

   ev->modifiers = _ecore_cocoa_event_modifiers([event modifierFlags]);

   ev->keycode = event.keyCode;
   ev->string = [keycharRaw cStringUsingEncoding:NSUTF8StringEncoding];
   ev->compose = ev->string;

   ev->window = (Ecore_Window)window.ecore_window_data;
   ev->event_window = ev->window;

   if ([keychar length] > 0)
     {
        for (i = 0; i < sizeof (keystable) / sizeof (struct _ecore_cocoa_keys_s); ++i)
          {
             if (keystable[i].code == [keychar characterAtIndex:0])
               {
                  ev->keyname = keystable[i].name;
                  ev->key = ev->keyname;
                  break;
               }
          }
     }

   if ([keycharRaw length] == 0  && keyType == NSKeyDown)
     {
        compose=EINA_TRUE;
        edit = [[event window]  fieldEditor:YES forObject:nil];
        [edit interpretKeyEvents:[NSArray arrayWithObject:event]];
        free(ev);
        return NULL;
     }

   return ev;
}

static inline Eina_Bool
_nsevent_window_is_type_of(NSEvent *event, Class class)
{
   /* An NSPeriodic event has no window (undefined behaviour) */
   if ([event type] == NSPeriodic) return EINA_FALSE;
   return [[event window] isKindOfClass:class];
}

static inline Eina_Bool
_has_ecore_cocoa_window(NSEvent *event)
{
   return _nsevent_window_is_type_of(event, [EcoreCocoaWindow class]);
}

EAPI Eina_Bool
ecore_cocoa_feed_events(void *anEvent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(anEvent, EINA_FALSE);

   NSEvent *event = anEvent;
   unsigned int time = (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff);
   Eina_Bool pass = EINA_FALSE;

   switch ([event type])
   {
      case NSMouseMoved:
      case NSLeftMouseDragged:
      case NSRightMouseDragged:
      case NSOtherMouseDragged:
      case NSLeftMouseDown:
      case NSRightMouseDown:
      case NSOtherMouseDown:
      case NSLeftMouseUp:
      case NSRightMouseUp:
      case NSOtherMouseUp:
      {
        //mouse events are managed in EcoreCocoaWindow
         return EINA_TRUE;
      }
      case NSKeyDown:
      {
         Ecore_Event_Key *ev;

         ev = _ecore_cocoa_event_key(event, NSKeyDown);
         if (ev == NULL) return EINA_TRUE;

         ev->timestamp = time;
         ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);

         break;
      }
      case NSKeyUp:
      {
         Ecore_Event_Key *ev;

         ev = _ecore_cocoa_event_key(event, NSKeyUp);
         if (ev == NULL) return EINA_TRUE;

         ev->timestamp = time;
         ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);

         break;
      }
      case NSFlagsChanged:
      {
         int flags = [event modifierFlags];

         Ecore_Event_Key *evDown = NULL;
         Ecore_Event_Key *evUp = NULL;

         evDown = calloc(1, sizeof (Ecore_Event_Key));
         if (!evDown) return pass;

         // Turn special key flags on
         if (flags & NSShiftKeyMask)
            evDown->key = "Shift_L";
         else if (flags & NSControlKeyMask)
            evDown->key = "Control_L";
         else if (flags & NSAlternateKeyMask)
            evDown->key = "Alt_L";
         else if (flags & NSCommandKeyMask)
            evDown->key = "Super_L";
         else if (flags & NSAlphaShiftKeyMask)
            evDown->key = "Caps_Lock";

         if (evDown->key)
         {
            evDown->keyname = evDown->key;
            evDown->timestamp = time;
            evDown->string = NULL;
            ecore_event_add(ECORE_EVENT_KEY_DOWN, evDown, NULL, NULL);
            old_flags = flags;
            break;
         }

         free(evDown);

         evUp = calloc(1, sizeof (Ecore_Event_Key));
         if (!evUp)
           {
              return pass;
           }

         int changed_flags = flags ^ old_flags;

         // Turn special key flags off
         if (changed_flags & NSShiftKeyMask)
            evUp->key = "Shift_L";
         else if (changed_flags & NSControlKeyMask)
            evUp->key = "Control_L";
         else if (changed_flags & NSAlternateKeyMask)
            evUp->key = "Alt_L";
         else if (changed_flags & NSCommandKeyMask)
            evUp->key = "Super_L";
         else if (changed_flags & NSAlphaShiftKeyMask)
            evUp->key = "Caps_Lock";

         if (evUp->key)
         {
            evUp->keyname = evDown->key;
            evUp->timestamp = time;
            evUp->string = NULL;
            ecore_event_add(ECORE_EVENT_KEY_UP, evUp, NULL, NULL);
            old_flags = flags;
            break;
         }

         break;
      }
      case NSAppKitDefined:
      {
         if ([event subtype] == NSApplicationActivatedEventType)
     {
        Ecore_Cocoa_Event_Window *ev;

            ev = malloc(sizeof(Ecore_Cocoa_Event_Window));
            if (!ev)
            {
              pass = EINA_FALSE;
              break;
            }
            ev->wid = [event window];
            ecore_event_add(ECORE_COCOA_EVENT_GOT_FOCUS, ev, NULL, NULL);
         }
         else if ([event subtype] == NSApplicationDeactivatedEventType)
     {
            Ecore_Cocoa_Event_Window *ev;

            ev = malloc(sizeof(Ecore_Cocoa_Event_Window));
            if (!ev)
            {
              pass = EINA_FALSE;
              break;
            }
            ev->wid = [event window];
            ecore_event_add(ECORE_COCOA_EVENT_LOST_FOCUS, ev, NULL, NULL);
         }
         pass = EINA_TRUE; // pass along AppKit events, for window manager
         break;
      }
      case NSScrollWheel:
      {
         DBG("Scroll Wheel\n");

         EcoreCocoaWindow *window = (EcoreCocoaWindow *)[event window];
         Ecore_Event_Mouse_Wheel *ev;

         ev = malloc(sizeof(Ecore_Event_Mouse_Wheel));
         if (!ev) return pass;

         ev->window = (Ecore_Window)window.ecore_window_data;
         ev->event_window = ev->window;
         ev->modifiers = 0; /* FIXME: keep modifier around. */
         ev->timestamp = time;
         ev->z = [event deltaX] != 0 ? [event deltaX] : -([event deltaY]);
         ev->direction = [event deltaX] != 0 ? 0 : 1;

         ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);

         break;
      }
      default:
      {
         pass = EINA_TRUE;
         break;
      }
   }

   return pass;
}

EAPI void
ecore_cocoa_screen_size_get(Ecore_Cocoa_Screen *screen, int *w, int *h)
{
   NSSize pt =  [[[NSScreen screens] objectAtIndex:0] frame].size;

   if (w) *w = (int)pt.width;
   if (h) *h = (int)pt.height;
}

EAPI int
ecore_cocoa_titlebar_height_get(void)
{
   static int height = -1;

   if (height == -1)
     {
        NSRect frame = NSMakeRect(0, 0, 100, 100);
        NSRect contentRect;
        contentRect = [NSWindow contentRectForFrameRect:frame
                                              styleMask:NSTitledWindowMask];
        height = (frame.size.height - contentRect.size.height);
     }
   return height;
}
